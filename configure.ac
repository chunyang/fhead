#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([fhead],[0.0.1],[x@cyang.info])
AC_CONFIG_SRCDIR([src/main.c])
#AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([fit], [FitConvert_Init])

# Checks for header files.
AC_CHECK_HEADER([stdint.h], [AC_DEFINE([FIT_USE_STDINT_H])])
AC_CHECK_HEADER([fit/fit_config.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT16_T
AC_TYPE_UINT32_T
AC_TYPE_INT32_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
