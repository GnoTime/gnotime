/*   History for GtkComboBox (stored in GSettings)
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

#ifndef GTT_HISTORY_LIST_H
#define GTT_HISTORY_LIST_H

#include <gtk/gtk.h>

void gtt_combo_history_list_init(GtkComboBox *combo_box, const gchar *history_id);
void gtt_combo_history_list_save(GtkComboBox *combo_box, const gchar *history_id, gint max_items);

#endif /* GTT_HISTORY_LIST_H */
