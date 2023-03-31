/*   string/int GtkListStore for GtkComboBox (display name plus value)
 *   Copyright (C) 2023 Oskar Berggren
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

#ifndef GTT_SELECT_LIST_H
#define GTT_SELECT_LIST_H

#include <gtk/gtk.h>

GtkListStore *gtt_select_list_new();

void gtt_select_list_append(GtkListStore *store, const gchar *text, gint value);
gboolean gtt_select_list_find_value(GtkListStore *store, GtkTreeIter *iter, gint value);

void gtt_combo_select_list_init(GtkComboBox *combo_box);
void gtt_combo_select_list_append(GtkComboBox *combo_box, const gchar *text, gint value);
void gtt_combo_select_list_set_active_by_value(GtkComboBox *combo_box, gint value);
gint gtt_combo_select_list_get_value(GtkComboBox *combo_box);

#endif /* GTT_SELECT_LIST_H */
