/*   GConf2 input/output handling for GTimeTracker - a time tracker
 *   Copyright (C) 2003 Linas Vepstas <linas@linas.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef GTT_GCONF_IO_H_
#define GTT_GCONF_IO_H_

#include <glib.h>

/** 
 * The gtt_gconf_save() routine will save all off the GTT attributes
 * into the Gnome2 Gconf attribute system.
 */
void gtt_gconf_save (void);


void gtt_gconf_load (void);
gboolean gtt_gconf_exists (void);

#endif /* GTT_GCONF_IO_H_ */

