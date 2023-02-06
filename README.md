What is GnoTime?
================
The Gnome Time Tracker is a to-do list/diary/journal tool that can track
the amount of time spent on projects, and, among other things, generate
reports and invoices based on that time. I've used it to keep shopping
lists, organize ideas, track bug reports, keep a diary of activities,
provide weekly status reports to management, and even as a consultant
billing system.

HomePage
--------
 * http://gttr.sourceforge.net
 * http://www.linas.org/linux/gtt/gtt.html

Features
--------

 * TODO Lists
 * Diary/Journal
 * Running timer
 * Billing status
 * HTML Reports

Please see the [GnoTime web page](http://gttr.sourceforge.net) for a
detailed description of these features.

Status
------
Gnotime last saw significant updates in 2013. Maintenance is needed!

GnoTime has been ported to use Gnome2.  Note, however, that it still
uses some of the older, deprecated widgets; most notably gtkctree.
The new gnome2 treeview widget lacks many of the features needed to
support the keyboard/mouse mannerisms of GnoTime, thereby making
a full port impossible (although one is attempted in src/ctree-gnome2.c).
(The notes at the top of that file describe what's wrong.)

There are still numerous areas of gtt that lack polish.  Please
submit pull reqs to github that polish up those things that irk you the most.

Building
--------
Required packages:

Steps:
```
./autogen.sh --no-configure
mkdir build; cd build
../configure
```


Authors
-------
This program was originally written by Eckehard Berns <eb@berns.prima.de>,
but has been greatly expanded by Linas Vepstas ~~<linas@linas.org>~~
<linasvepstas@gmail.com>.

Thanks go out to many people who e-mailed me with suggestions and
bug fixes.  See the "about" window in the app for more details.


Required Packages for Building GnoTime
--------------------------------------
These instructions are out-of-date.

Besides the 'usual' Gnome2 development packages, the following are
some of the packages required to build GnoTime:

 * guile-1.6-dev
 * guile-1.6-slib
 * libgtkhtml3-dev
 * docbook-utils  (to build the documents subdirectory)


Copyright
---------
 * GnoTime - the Gnome Time Tracker
 * Copyright (C) 1997,98 Eckehard Berns
 * Copyright (C) 2001,2002,2003,2004 Linas Vepstas <linas@linas.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   For more details see the file [COPYING](COPYING).
