AC_INIT([GnoTime],[2.99.0],[https://github.com/GnoTime/gnotime/issues],[gnotime],[gnotime.org])
AX_IS_RELEASE([git-directory])

dnl For AC_CONFIG_SRCDIR I picked gtt.h because it's more unique than our main.c.
AC_CONFIG_SRCDIR([src/gtt.h])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_CONFIG_MACRO_DIR([m4])

AM_MAINTAINER_MODE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])], [AC_SUBST([AM_DEFAULT_VERBOSITY],1)])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl TODO: Enable this when we have fixed the issues...
dnl AX_COMPILER_FLAGS([WARN_CFLAGS],[WARN_LDFLAGS])
GLIB_GSETTINGS

AC_PATH_XTRA

dnl *****************************************
dnl Look for a portable way to reap zombie processes
dnl *****************************************
AC_CHECK_HEADER([signal.h], AC_DEFINE([HAVE_SIGNAL_H], [1], [Define to 1 if we have signal.h on our system.]),
	AC_MSG_ERROR([signal.h is needed to compile this program.]))

AC_CHECK_HEADERS([sys/wait.h])
if test x$ac_cv_header_wait_h != "x"; then
	AC_CHECK_FUNCS(waitpid)
	AC_CHECK_DECLS(WNOHANG,,,[#include<sys/wait.h>])
fi
AC_CHECK_DECLS(SA_NOCLDWAIT,,,[#include<signal.h>])

dnl *****************************************
dnl pkg-config check time
dnl *****************************************

GCONF_REQUIRED=3.2.6
GLIB_REQUIRED=2.40.2
GTK_REQUIRED=3.10.0
LIBXML2_REQUIRED=2.9.1
SCROLLKEEPER_BUILD_REQUIRED=0.8.1
LIBQOF_REQUIRED_MIN=0.8.6
LIBDBUS_REQUIRED_MIN=0.100.2
X11_REQUIRED=1.6.2
XSCRNSAVER_REQUIRED=1.2.2
WEBKITGTK_REQUIRED=2.32.0

dnl *****************************************
dnl Check for guile
dnl *****************************************

# http://www.gnu.org/software/guile/manual/html_node/Autoconf-Macros.html
GUILE_PKG
GUILE_FLAGS


dnl *****************************************
dnl Check for glib
dnl *****************************************
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl *****************************************
dnl Check for GTK+
dnl *****************************************
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)
AC_SUBST(GTK_FLAGS)
AC_SUBST(GTK_LIBS)

dnl *****************************************
dnl Check for X11
dnl *****************************************
PKG_CHECK_MODULES(X11, x11 >= $X11_REQUIRED)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

dnl *********************************************
dnl for anything which calls gconftool-2 correctly
dnl *********************************************
AC_PATH_PROG(GCONFTOOL, gconftool-2, no)

if test x"$GCONFTOOL" = xno; then
  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi

dnl *****************************************
dnl Check for gconf
dnl *****************************************
PKG_CHECK_MODULES(GCONF, gconf-2.0 >= $GCONF_REQUIRED)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

dnl **************************************************************
dnl If QOF is found on the build system, then use it.
dnl **************************************************************
PKG_CHECK_MODULES(LIBQOF, qof >= $LIBQOF_REQUIRED_MIN)
AC_SUBST(LIBQOF_CFLAGS)
AC_SUBST(LIBQOF_LIBS)

dnl **************************************************************
dnl pkgconfig looks for xscrnsaver.pc, however, this is 
dnl packaged in unexpected ways for different OS'es:
dnl called xscrnsaver in RedHat/SuSE
dnl called libxss, libxss-dev in Debian/Ubuntu
dnl **************************************************************
PKG_CHECK_MODULES(XSS_EXTENSION, xscrnsaver >= $XSCRNSAVER_REQUIRED)
AC_SUBST(XSS_EXTENSION_CFLAGS)
AC_SUBST(XSS_EXTENSION_LIBS)


dnl *************************************************************
dnl Check for DBUS
dnl *************************************************************

PKG_CHECK_MODULES(LIBDBUS, dbus-glib-1 >= $LIBDBUS_REQUIRED_MIN,
[
AC_SUBST(LIBDBUS_CFLAGS)
AC_SUBST(LIBDBUS_LIBS)
WITH_DBUS=1
],
[
WITH_DBUS=0
])
AC_SUBST(WITH_DBUS)


dnl *******************************
dnl WebKitGTK
dnl *******************************

PKG_CHECK_MODULES(WEBKITGTK, webkit2gtk-4.0 >= $WEBKITGTK_REQUIRED)
AC_SUBST(WEBKITGTK_CFLAGS)
AC_SUBST(WEBKITGTK_LIBS)


dnl ********************
dnl libxml2
dnl The libxml2 library is required 
dnl ********************

PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= $LIBXML2_REQUIRED)
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

dnl *****************************
dnl scrollkeeper
dnl *****************************
dnl To substitute into the gnotime.spec
AC_SUBST(SCROLLKEEPER_BUILD_REQUIRED)
AC_PATH_PROG(SK_CONFIG, scrollkeeper-config,no)
if test x$SK_CONFIG = xno; then
	AC_MSG_ERROR(cannot find scrollkeeper-config.  In order to properly install the documentation please install the scrollkeeper package: http://scrollkeeper.sourceforge.net)
fi
AC_SUBST(SK_CONFIG)

dnl *****************************
dnl popt is needed 
dnl *****************************

# There is no popt under mac OSX but it still compiles ... 
# so remove this check so that Mac OSX can work.
#
# AC_CHECK_LIB(popt, poptHelpOptions,, AC_MSG_ERROR([popt is required to build gnome-utils. You can download the latest version from ftp://people.redhat.com/sopwith/popt/]))



dnl **************************************************************
dnl Checks for typedefs, structures, and compiler characteristics.
dnl **************************************************************

AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T

dnl *****************************
dnl Checks for library functions.
dnl *****************************

AC_FUNC_STRFTIME
AC_CHECK_FUNCS(getcwd gettimeofday getwd stpcpy strdup strtoul)
AC_REPLACE_FUNCS(strptime localtime_r)

dnl ************************************************
dnl X development libraries check
dnl ************************************************

if $PKG_CONFIG --exists pangoxft ; then
  PANGO_PACKAGES="pangox pangoxft"
else
  PANGO_PACKAGES="pangox"
fi

x_libs="`$PKG_CONFIG --libs $PANGO_PACKAGES`"
case x_libs in
  *-lX11*) pango_omitted_x_deps=no ;;
  *)       pango_omitted_x_deps=yes ;;
esac

if test $pango_omitted_x_deps = yes ; then
  AC_PATH_XTRA

  if test x$no_x = xyes ; then
    AC_MSG_ERROR([X development libraries not found])
  else
    X_LIBS="$X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"
  fi
fi

AC_SUBST(X_LIBS)


dnl *******************************
dnl Internationalization
dnl ******************************* 

GETTEXT_PACKAGE=gnotime
AC_SUBST(GETTEXT_PACKAGE)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

# AM_GLIB_GNU_GETTEXT above substs $DATADIRNAME
# this is the directory where the *.{mo,gmo} files are installed
gnomeutilslocaledir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(gnomeutilslocaledir)

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",
		[Define the gettext package.])

AC_DEFINE_UNQUOTED(GNOME_ICONDIR, "${prefix}/share/pixmaps",
		[Define a gnome pixmaps directory.])

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([
Makefile
data/Makefile
doc/C/Makefile
doc/C/man/Makefile
doc/Makefile
gconf/Makefile
ghtml/C/Makefile
ghtml/Makefile
po/Makefile.in
redhat/Makefile
redhat/gnotime.spec
fedora/Makefile
fedora/gnotime.spec
debian/Makefile
src/Makefile
scripts/Makefile
ui/Makefile
])
AC_OUTPUT

dnl <= Configuration summary =>

echo ""
echo "gnotime configuration summary:"
echo "=================================="
echo ""

dnl <= CFLAGS and LDFLAGS =>
echo "CFLAGS : $CFLAGS"
echo "LDFLAGS : $LDFLAGS"
echo "WARN_CFLAGS : $WARN_CFLAGS"
echo "WARN_LDFLAGS : $WARN_LDFLAGS"
echo "X_LIBS : $X_LIBS"
echo "Guile version : $GUILE_EFFECTIVE_VERSION"
echo "GUILE_CFLAGS : $GUILE_CFLAGS"
echo "GUILE_LDFLAGS : $GUILE_LDFLAGS"
echo "GUILE_LIBS : $GUILE_LIBS"
echo "LIBQOF_LIBS : $LIBQOF_LIBS"
echo "LIBQOF_CFLAGS : $LIBQOF_CFLAGS"
echo "LIBDBUS_LIBS : $LIBDBUS_LIBS"
echo "LIBDBUS_CFLAGS : $LIBDBUS_CFLAGS"
echo "XSS_EXTENSION_CFLAGS : $XSS_EXTENSION_CFLAGS"
echo "XSS_EXTENSION_LIBS : $XSS_EXTENSION_LIBS"
echo "WEBKITGTK_CFLAGS : $WEBKITGTK_CFLAGS"
echo "WEBKITGTK_LIBS: $WEBKITGTK_LIBS"
echo ""

dnl <= Prefixes =>
echo "prefix : $prefix"
# echo "sysconf dir : "`eval echo $sysconfdir`
echo "bin dir : "`eval echo $bindir`
# echo "sbin dir : "`eval echo $sbindir`
echo "data dir : "`eval echo $datadir`
echo ""


echo ""
dnl <= End of configuration summary =>
