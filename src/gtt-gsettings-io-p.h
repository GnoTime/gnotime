/* GSettings input/output handling for GnoTime - a time tracker
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

// This is a PRIVATE header file for GSettings users only! Do not use!

#ifndef GTT_GSETTINGS_IO_P_H
#define GTT_GSETTINGS_IO_P_H

#include <gio/gio.h>
#include <glib.h>

GSList *gtt_gsettings_get_array_int(GSettings *settings, const gchar *key);
void gtt_gsettings_get_maybe_string(GSettings *settings, const gchar *key, gchar **value);
void gtt_gsettings_get_string(GSettings *settings, const gchar *key, gchar **value);

void gtt_gsettings_set_array_int(GSettings *settings, const gchar *key, GSList *value);
void gtt_gsettings_set_bool(GSettings *settings, const gchar *key, gboolean value);
void gtt_gsettings_set_int(GSettings *settings, const gchar *key, gint value);
void gtt_gsettings_set_maybe_string(GSettings *settings, const gchar *key, const gchar *value);
void gtt_gsettings_set_string(GSettings *settings, const gchar *key, const gchar *value);

#endif // GTT_GSETTINGS_IO_P_H
