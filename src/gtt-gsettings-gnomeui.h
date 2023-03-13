/* GnomeUI to GSettings input/output handling for GnoTime - a time tracker
 * Copyright (C) 2023      Markus Prasser
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */

#ifndef GTT_GSETTINGS_GNOMEUI_H
#define GTT_GSETTINGS_GNOMEUI_H

#include <gnome.h>

#include <gio/gio.h>

/* These routines provide some simplisitic save/restore
 * functions for GnomeUIInfo structures, so that we can
 * save and restore menu structures.  An alternative,
 * possibly prefered way of doing this would be to
 * output glade XML, and manage the menus with glade.
 * No big deal either way.
 */

void gtt_save_gnomeui_to_gsettings(GSettings *settings, GnomeUIInfo *gui);

void gtt_restore_gnomeui_from_gsettings(GSettings *settings, GnomeUIInfo *gui);

#endif // GTT_GSETTINGS_GNOMEUI_H
