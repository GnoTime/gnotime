#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0` 
test -z "$srcdir" && srcdir=.

PKG_NAME="GnoTime"

(test -f $srcdir/configure.in \
  && test -f $srcdir/ChangeLog \
  && test -d $srcdir/src) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level gnotime directory"
    exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common"
    exit 1
}
REQUIRED_AUTOMAKE_VERSION=1.5 USE_COMMON_DOC_BUILD=yes USE_GNOME2_MACROS=1 . gnome-autogen.sh
