# Hacking

This document is intended to give a short outline of the current development
goal and approach. See further information in the following sections.

## Current Goal And Approach

GnoTime fell victim to heavy bit rot and was for this reason not able to be
compiled on modern day Linux distributions. During 2023, many updates have
been made.

As of October 2023, the Gnome dependencies have been removed and more
modern alternatives have been used, GTK upgraded to version 3, GtkHTML
replaced with WebKitGTK, and the autotools build system has been updated.
It is known to build on Ubuntu 18.04 and Fedora 37. It no longer builds onUbuntu 14.04.

## Coding Style

A `.clang-format` file has been added to this repository to enforce a consistent
coding style and alleviate the developers from taking care of formatting
manually. Any recent IDE should allow be able to format the code automatically.
The `.clang-format`  file aims at clang-format v15 currently.
