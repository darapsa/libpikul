AC_INIT([pikul], [0.0], [prabowo@darapsa.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AM_PROG_AR
LT_INIT
PKG_CHECK_MODULES([DEPS], [libcurl json-c])
AC_C_INLINE
AC_CHECK_FUNCS([strstr strcspn strtol memset])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_MKTIME
AC_TYPE_SIZE_T
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile pikul.pc])
AC_OUTPUT
