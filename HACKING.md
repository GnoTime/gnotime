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

Development can be conducted with any modern system but compatibility with
Ubuntu 14.04 as a baseline should be kept. Ubuntu 14.04 is the most recent
version which allows compiling and running GnoTime with its dependencies out of
the box.

Sidenote: My personal approach is to develop GnoTime utilizing the modern
tooling of _Fedora 37_ and having a _Ubuntu 14.04_ VM in which I track my
development efforts utilizing the current HEAD of this repository.

## Coding Style

A `.clang-format` file has been added to this repository to enforce a consistent
coding style and alleviate the developers from taking care of formatting
manually. Any recent IDE should allow be able to format the code automatically.
The `.clang-format`  file aims at clang-format v15 currently.
