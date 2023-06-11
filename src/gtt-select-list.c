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

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "gtt-select-list.h"

#define TEXT_COLUMN_ID 0
#define VALUE_COLUMN_ID 1

/************************************************************
 * A "select-list" is a specialized GtkListStore holding
 * a (display) text in the first column and some arbitrary
 * integer identifying key in the second column.
 *
 * It can be used with GtkComboBox to display user-friendly
 * strings, while allowing the program to directly set and
 * retrieve a numerical constant (e.g. an enum value).
 *
 * Several convenience methods that operate directly on a
 * GtkComboBox are offered.
 ************************************************************/

/**
 * @brief Create a new select-list GtkListStore
 */
GtkListStore *gtt_select_list_new()
{
    GtkListStore *store;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    return store;
}

/**
 * @brief Append an item to a select-list GtkListStore
 * @param text The display text for the item
 * @param value An identifying value associated with the item
 */
void gtt_select_list_append(GtkListStore *store, const gchar *text, gint value)
{
    gtk_list_store_insert_with_values(
        store, NULL, G_MAXINT, TEXT_COLUMN_ID, text, VALUE_COLUMN_ID, value, -1
    );
}

/**
 * @brief Find a select-list item based on the value property
 * @param store The select-list GtkListStore
 * @param iter A GtkTreeIter that will be set to the matching item, if found.
 * @param value The value to search for.
 */
gboolean gtt_select_list_find_value(GtkListStore *store, GtkTreeIter *iter, gint value)
{
    GtkTreeModel *model = GTK_TREE_MODEL(store);
    gint candidate;

    if (!gtk_tree_model_get_iter_first(model, iter))
    {
        return FALSE;
    }

    do
    {
        gtk_tree_model_get(model, iter, 1, &candidate, -1);

        if (candidate == value)
            return TRUE;
    }
    while (gtk_tree_model_iter_next(model, iter));

    return FALSE;
}

/**
 * @brief Initialize a GtkComboBox with a new select-list GtkListStore
 */
void gtt_combo_select_list_init(GtkComboBox *combo_box)
{
    GtkListStore *store = gtt_select_list_new();
    gtk_combo_box_set_model(combo_box, GTK_TREE_MODEL(store));

    // Currently, we assume that the combo box is already set up
    // with a text cell rendered for column 0.
}

/**
 * @brief Append an item to the select-list GtkListStore use by the combo box
 * @param text The display text for the item
 * @param value An identifying value associated with the item
 */
void gtt_combo_select_list_append(GtkComboBox *combo_box, const gchar *text, gint value)
{
    GtkListStore *store;

    store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));

    gtt_select_list_append(store, text, value);
}

/**
 * @brief Set the active item of the combo box based on item value
 */
void gtt_combo_select_list_set_active_by_value(GtkComboBox *combo_box, gint value)
{
    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));

    if (gtt_select_list_find_value(store, &iter, value))
        gtk_combo_box_set_active_iter(combo_box, &iter);
}

/**
 * @brief Get the value of the currently selected item
 * @return The identifier value associated with the current item, or -1 if no item was active
 */
gint gtt_combo_select_list_get_value(GtkComboBox *combo_box)
{
    GtkListStore *store;
    GtkTreeIter iter;
    int value = -1;

    store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));

    if (gtk_combo_box_get_active_iter(combo_box, &iter))
    {
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, VALUE_COLUMN_ID, &value, -1);
    }

    return value;
}
