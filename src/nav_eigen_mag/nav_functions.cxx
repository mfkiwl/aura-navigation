/*! \file nav_functions.c
 *	\brief Auxiliary functions for nav filter
 *
 *	\details
 *     Module:          Navfuncs.c
 *     Modified:        Brian Taylor (convert to eigen3)
 *						Adhika Lie (revamp all functions)
 * 						Gokhan Inalhan (remaining)
 *                      Demoz Gebre (first three functions)
 *                      Jung Soon Jang
 *
 *     Description:     navfunc.c contains the listing for all the
 *                      real-time inertial navigation software.
 *
 *		Note: all the functions here do not create memory without
 *			  clearing it.
 *	\ingroup nav_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: nav_functions.c 922 2012-10-17 19:14:09Z joh07594 $
 */

/*     Include Pertinent Header Files */

#include <math.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>
using namespace Eigen;

#include "nav_functions.hxx"

Matrix<double,3,1> llarate(Matrix<double,3,1> V, Matrix<double,3,1> lla) {
    /* This function calculates the rate of change of latitude,
     * longitude, and altitude using WGS-84.
     */

    double lat = lla(0,0);
    double h = lla(2,0);
	
    double denom = fabs(1.0 - (ECC2 * sin(lat) * sin(lat)));
    double sqrt_denom = sqrt(denom);
    
    double Rew = EARTH_RADIUS / sqrt_denom;
    double Rns = EARTH_RADIUS*(1-ECC2) / (denom*sqrt_denom);
	
    Matrix<double,3,1> lla_dot;
    lla_dot(0,0) = V(0,0)/(Rns + h);
    lla_dot(1,0) = V(1,0)/((Rew + h)*cos(lat));
    lla_dot(2,0) = -V(2,0);
	
    return lla_dot;
}

Matrix<double,3,1> navrate(Matrix<double,3,1> V, Matrix<double,3,1> lla) {
    /* This function calculates the angular velocity of the NED frame,
     * also known as the navigation rate using WGS-84.
     */

    double lat = lla(0,0);
    double h = lla(2,0);
	
    double denom = fabs(1.0 - (ECC2 * sin(lat) * sin(lat)));
    double sqrt_denom = sqrt(denom);
    
    double Rew = EARTH_RADIUS / sqrt_denom;
    double Rns = EARTH_RADIUS*(1-ECC2) / (denom*sqrt_denom);
	
    Matrix<double,3,1> nr;
    nr(0,0) = V(1,0)/(Rew + h);
    nr(1,0) = -V(0,0)/(Rns + h);
    nr(2,0) = -V(1,0)*tan(lat)/(Rew + h);
	
    return nr;
}

Matrix<double,3,1> lla2ecef(Matrix<double,3,1> lla) {  
    /* This function calculates the ECEF Coordinate given the
     * Latitude, Longitude and Altitude.
     */
	

    double sinlat = sin(lla(0,0));
    double coslat = cos(lla(0,0));
    double coslon = cos(lla(1,0));
    double sinlon = sin(lla(1,0));
    double alt = lla(2,0);

    double denom = fabs(1.0 - (ECC2 * sinlat * sinlat));

    double Rew = EARTH_RADIUS / sqrt(denom);
  
    Matrix<double,3,1> ecef;
    ecef(0,0) = (Rew + alt) * coslat * coslon;
    ecef(1,0) = (Rew + alt) * coslat * sinlon;
    ecef(2,0) = (Rew * (1.0 - ECC2) + alt) * sinlat;
	
    return ecef;
}

Matrix<double,3,1> ecef2ned(Matrix<double,3,1> ecef, Matrix<double,3,1> pos_ref) {
    /* This function converts a vector in ecef to ned coordinate
     * centered at ecef_ref.
     */
	
    double lat = pos_ref(0,0);
    double lon = pos_ref(1,0);
    double sin_lat = sin(lat);
    double sin_lon = sin(lon);
    double cos_lat = cos(lat);
    double cos_lon = cos(lon);
    
    Matrix<double,3,1> ned;
    ned(2,0) = -cos_lat*cos_lon*ecef(0,0) - cos_lat*sin_lon*ecef(1,0) - sin_lat*ecef(2,0);
    ned(1,0) = -sin_lon*ecef(0,0) + cos_lon*ecef(1,0);
    ned(0,0) = -sin_lat*cos_lon*ecef(0,0) - sin_lat*sin_lon*ecef(1,0) + cos_lat*ecef(2,0);
	
    return ned;
}

Matrix<double,3,3> sk(Matrix<double,3,1> w) {
    /* This function gives a skew symmetric matrix from a given vector w */

    Matrix<double,3,3> C;

    C(0,0) = 0.0;	C(0,1) = -w(2,0);	C(0,2) = w(1,0);
    C(1,0) = w(2,0);	C(1,1) = 0.0;		C(1,2) = -w(0,0);
    C(2,0) = -w(1,0);	C(2,1) = w(0,0);	C(2,2) = 0.0;
	
    return C;
}

// depricated
void qmult(double *p, double *q, double *r) {
    /* Quaternion Multiplication: r = p x q */
    int i;
	
    for(i=0;i<3;i++) r[i] = 0.0;
	
    r[0] = p[0]*q[0] - (p[1]*q[1] + p[2]*q[2] + p[3]*q[3]);
    r[1] = p[0]*q[1] + q[0]*p[1] + p[2]*q[3] - p[3]*q[2];
    r[2] = p[0]*q[2] + q[0]*p[2] + p[3]*q[1] - p[1]*q[3];
    r[3] = p[0]*q[3] + q[0]*p[3] + p[1]*q[2] - p[2]*q[1];
}

void quat2eul(Quaterniond q, double *phi, double *the, double *psi) {
    /* Quaternion to euler angle */

    double q0, q1, q2, q3;
    double m11, m12, m13, m23, m33;
	
    q0 = q.w();
    q1 = q.x();
    q2 = q.y();
    q3 = q.z();

    m11 = 2*(q0*q0 + q1*q1) - 1;
    m12 = 2*(q1*q2 + q0*q3);
    m13 = 2*(q1*q3 - q0*q2);
    m23 = 2*(q2*q3 + q0*q1);
    m33 = 2*(q0*q0 + q3*q3) - 1;
	
    *psi = atan2(m12,m11);
    *the = asin(-m13);
    *phi = atan2(m23,m33);
}

void eul2quat(double *q, double phi, double the, double psi) {
    double sin_psi = sin(psi*0.5);
    double cos_psi = cos(psi*0.5);
    double sin_the = sin(the*0.5);
    double cos_the = cos(the*0.5);
    double sin_phi = sin(phi*0.5);
    double cos_phi = cos(phi*0.5);

    q[0] = cos_psi*cos_the*cos_phi + sin_psi*sin_the*sin_phi;  
    q[1] = cos_psi*cos_the*sin_phi - sin_psi*sin_the*cos_phi;
    q[2] = cos_psi*sin_the*cos_phi + sin_psi*cos_the*sin_phi;  
    q[3] = sin_psi*cos_the*cos_phi - cos_psi*sin_the*sin_phi;
}

// fixme: clean up math operations
Matrix<double,3,3> quat2dcm(Quaterniond q) {
    /* Quaternion to C_N2B */

    double q0, q1, q2, q3;
    Matrix<double,3,3> C_N2B;

    q0 = q.w(); q1 = q.x(); q2 = q.y(); q3 = q.z();

    C_N2B(0,0) = 2*(q0*q0 + q1*q1) - 1;
    C_N2B(1,1) = 2*(q0*q0 + q2*q2) - 1;
    C_N2B(2,2) = 2*(q0*q0 + q3*q3) - 1;
	
    C_N2B(0,1) = 2*(q1*q2 + q0*q3);
    C_N2B(0,2) = 2*(q1*q3 - q0*q2);
	
    C_N2B(1,0) = 2*(q1*q2 - q0*q3);
    C_N2B(1,2) = 2*(q2*q3 + q0*q1);
	
    C_N2B(2,0) = 2*(q1*q3 + q0*q2);
    C_N2B(2,1) = 2*(q2*q3 - q0*q1);
	
    return C_N2B;
}
