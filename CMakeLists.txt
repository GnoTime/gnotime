# CMake is not intended as primary build system of GnoTime. Currently Autotools
# fulfills this role and in the long run Meson shall take it over. CMake project
# files are added for easy integration into common IDE environments.
#
# GnoTime can be built by means of CMake if e.g. CMake >=3.13 has been installed
# on Ubuntu Trusty Tahr manually.

cmake_minimum_required(VERSION 3.13.0)

project(gnotime
    VERSION 3.0.0
    DESCRIPTION "A to-do list organizer, diary and billing system"
    HOMEPAGE_URL "https://gttr.sourceforge.net"
    LANGUAGES C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGSEAL_ENABLE -DGDK_DISABLE_DEPRECATED")
set(GTT_VERSION_SUFFIX "_dev")

find_package(PkgConfig)

pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1>=0.100.2)
pkg_check_modules(GCONF REQUIRED gconf-2.0>=3.2.6)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.40.2)
pkg_check_modules(GTK REQUIRED gtk+-2.0>=3.20.0)
pkg_check_modules(WEBKIT_GTK REQUIRED webkit2gtk-4.0>=2.32.0)
pkg_check_modules(GUILE REQUIRED guile-2.0>=2.0.9)
pkg_check_modules(LIBXML REQUIRED libxml-2.0>=2.9.1)
pkg_check_modules(QOF REQUIRED qof>=0.8.7)
pkg_check_modules(X11 REQUIRED x11>=1.6.2)
pkg_check_modules(XSCRNSAVER REQUIRED xscrnsaver>=1.2.2)

add_subdirectory(src)
