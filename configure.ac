AC_INIT([rem], [1.0], [eponier@via.ecp.fr])

LT_CONFIG_LTDL_DIR([libltdl])
AC_CONFIG_AUX_DIR([libltdl/config])
AC_CONFIG_MACRO_DIR([libltdl/m4])

AM_PROG_AR
LT_INIT
LTDL_INIT

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CC_C99
AC_PROG_CXX
PKG_PROG_PKG_CONFIG()

PKG_CHECK_MODULES(VLC_PLUGIN, [vlc-plugin >= 2.1.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
