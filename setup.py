#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name='navigation',
      version='1.0',
      description='Navigation Tools',
      author='Curtis L. Olson',
      author_email='curtolson@flightgear.org',
      url='https://github.com/AuraUAS',
      #py_modules=['props', 'props_json', 'props_xml'],
      package_dir = {'': 'lib'},
      packages=['nav', 'nav.data', 'nav.weather'],
      ext_package='nav',
      ext_modules=[
            Extension('structs',
                      define_macros=[('HAVE_BOOST_PYTHON', '1')],
                      sources=['src/core/structs.cxx'],
                      libraries=['boost_python']),
            Extension('wgs84',
                      define_macros=[('HAVE_BOOST_PYTHON', '1')],
                      sources=['src/core/wgs84.cxx'],
                      libraries=['boost_python']),
            Extension('EKF15',
                      define_macros=[('HAVE_BOOST_PYTHON', '1')],
                      sources=['src/nav_eigen/EKF_15state.cxx',
                               'src/core/nav_functions.cxx'],
                      libraries=['boost_python'],
                      depends=['src/nav_eigen/EKF_15state.hxx',
                               'src/core/nav_functions.hxx']),
            Extension('EKF15_mag',
                      define_macros=[('HAVE_BOOST_PYTHON', '1')],
                      sources=['src/nav_eigen_mag/EKF_15state_mag.cxx',
                               'src/core/nav_functions.cxx',
                               'src/core/coremag.c'],
                      libraries=['boost_python'],
                      depends=['src/nav_eigen_mag/EKF_15state_mag.hxx',
                               'src/core/nav_functions.hxx',
                               'src/core/coremag.h']),
            Extension('openloop',
                      define_macros=[('HAVE_BOOST_PYTHON', '1')],
                      sources=['src/nav_openloop/openloop.cxx',
                               'src/nav_openloop/glocal.cxx',
                               'src/core/nav_functions.cxx'],
                      libraries=['boost_python'],
                      depends=['src/nav_openloop/openloop.hxx',
                               'src/nav_openloop/glocal.hxx',
                               'src/core/nav_functions.hxx'])
      ],
     )
