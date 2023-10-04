# Hacking

This document is intended to give a short outline of the current development
goal and approach. See further information in the following sections.

## Current Goal And Approach

GnoTime fell victim to heavy bit rot and is for this reason not able to be
compiled on modern day Linux distributions. The current primary priority is to
replace the outdated dependencies by their up-to-date alternatives (e.g.
libgnome and GTK+ 2.0 by GTK 3 and GtkHTML by WebKitGTK).

To avoid ending up with a dysfunctional "zombie system" it is intended to stay
with GTK+ 2.0 while replacing most of the outdated dependencies (GConf,
GnomeVFS, etc.) and upgrading to GTK 3 and WebKitGTK afterwards. This should
allow to continually have a fully functional GnoTime build for testing.
During this time, Ubuntu 14.04 is the most recent version which allows
compiling and running GnoTime with its dependencies out of the box.

As of October 2023, the Gnome dependencies have been removed and more
modern alternatives have been used, while still on Gtk2. The autotools
build system has been updated and it no longer builds on Ubuntu 14.04.
The new baseline is Ubuntu 18.04.

## Coding Style

A `.clang-format` file has been added to this repository to enforce a consistent
coding style and alleviate the developers from taking care of formatting
manually. Any recent IDE should allow be able to format the code automatically.
The `.clang-format`  file aims at clang-format v15 currently.
