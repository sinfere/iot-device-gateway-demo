#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gateway-demo], [0.0.1], [sinfere@gmail.com])
# AC_CONFIG_SRCDIR([])
# AC_CONFIG_HEADERS([])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wno-gnu -Werror])

# Checks for programs.
AC_PROG_CC
# AC_PROG_INSTALL

LT_INIT

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CHECK_LIB(socket, connect)

dnl Checks for library functions.
AC_CHECK_FUNCS([malloc memset posix_memalign socket])


AC_CHECK_HEADERS([ev.h libev/ev.h], [], [])
AC_CHECK_LIB([ev], [ev_loop_destroy], [LIBS="-lev $LIBS"], [AC_MSG_ERROR([couldn't find libev])])

AC_CHECK_HEADERS([ev.h libev/ev.h], [], [])
AC_CHECK_LIB([json-c], [json_tokener_parse_ex], [LIBS="-ljson-c $LIBS"], [AC_MSG_ERROR([couldn't find json-c])])

AC_CONFIG_FILES([Makefile src/Makefile libmqtt/Makefile])
AC_OUTPUT
