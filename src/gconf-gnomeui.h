/*   GnomeUI to GConf2 input/output handling for GTimeTracker - a time tracker
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

#ifndef GTT_GCONF_GNOMEUI_H_
#define GTT_GCONF_GNOMEUI_H_

#include <gconf/gconf-client.h>
#include <gnome.h>

/* These routines provide some simplisitic save/restore
 * functions for GnomeUIInfo structures, so that we can 
 * save and restore menu structures.
 */

/* Save the contents of a GnomeUIInfo structure with GConf 
 * to the indicated path. */
void gtt_save_gnomeui_in_gconf (GConfClient *client, 
                const char * path, GnomeUIInfo *gui);

/* Restore from GConf path into the designated GnomeUIInfo struct */
void gtt_restore_gnomeui_from_gconf (GConfClient *client,
                const char * path, GnomeUIInfo *gui);

#endif
