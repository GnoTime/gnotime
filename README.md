What is GnoTime?
================
The Gnome Time Tracker is a to-do list/diary/journal tool that can track
the amount of time spent on projects, and, among other things, generate
reports and invoices based on that time. It can be used to keep shopping
lists, organize ideas, track bug reports, keep a diary of activities,
provide weekly status reports to management, and even works as a consultant
billing system.

HomePage
--------
 * http://gttr.sourceforge.net
 * http://www.linas.org/linux/gtt/gtt.html

Features
--------
 * TODO Lists
 * Diary/Journal
 * Multiple task timers
 * Billing subsystem
 * (Configurable) HTML Reports

Please see the [GnoTime web page](http://gttr.sourceforge.net) for a
detailed description of these features.

Status
------
After being stale since about 2013 there is renewed development of GnoTime.
This is being managed on [GitHub](https://github.com/GnoTime/gnotime).

The dev branch is currently known to build on Ubuntu 18.04.

Building
--------
The current dependencies of GnoTime are heavily outdated. The current aim is to
update them to the state of current Debian oldstable (i.e. _Debian Buster_).
As of October 2023, the dev branch has been updated to build on Ubuntu 18.04
as the oldest platform known to work. (It no longer builds on the previous
reference platform Ubuntu 14.04.) The below listed packages are
valid in the realm of this reference platform and should allow an issue-less
compilation.

### Required packages
```
guile-2.0-dev (or 2.2)
libdbus-glib-1-dev
libgconf2-dev
libglib2.0-dev
libgtk2.0-dev
libgtkhtml3.14-dev
libqof-dev
libxss-dev
libxml2-dev
scrollkeeper
```

Developers and maintainers also need:
```
glade-gtk2
```

### QOF Query Object Framework
One pre-requiste to building this is the `qof` package.
It is not distributed by distros, by default.

QOF on github:
 * https://github.com/codehelp/qof

QOF Documentation:
 * http://qof.sourceforge.net/doxy/main.html

It's not hard to build:
```
git clone https://github.com/codehelp/qof
./autogen.sh
./configure
make
sudo make install
```


### Build steps:
```
NOCONFIGURE=true ./autogen.sh
mkdir build; cd build
../configure
```

Authors
-------
This program was originally written by Eckehard Berns <eb@berns.prima.de>,
but has been greatly expanded by Linas Vepstas ~~<linas@linas.org>~~
<linasvepstas@gmail.com>. Modernized in 2023 by Markus Prasser and Oskar
Berggren.

Thanks go out to many people who e-mailed me with suggestions and
bug fixes.  See the "about" window in the app for more details.


Copyright
---------
 * GnoTime - the Gnome Time Tracker
 * Copyright (C) 1997,98 Eckehard Berns
 * Copyright (C) 2001,2002,2003,2004 Linas Vepstas <linas@linas.org>
 * Copyright (C) 2023 Markus Prasser
 * Copyright (C) 2023 Oskar Berggren <oskar.berggren@gmail.com>

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
