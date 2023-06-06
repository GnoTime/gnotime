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

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "gtt-gsettings-io-p.h"
#include "gtt-history-list.h"
#include "util.h"

#define TEXT_COLUMN_ID 0


static GSettings *settings_open_history(const gchar *const history_id)
{
    gchar *path = g_strdup_printf("/org/gnotime/app/gtt-entry-histories/history-%s/", history_id);

    GSettings *settings = g_settings_new_with_path("org.gnotime.app.gtt-entry-histories", path);

    g_free(path);

    return settings;
}

static gboolean is_present_in_list(GSList *gsettings_items, const gchar *item)
{
    for (; gsettings_items; gsettings_items = gsettings_items->next)
    {

        if (strcmp((gchar *) gsettings_items->data, item) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static GSList *copy_model_to_gslist(GtkComboBox *combo_box, GSList *items, gint max_items)
{
    GtkTreeModel *model = gtk_combo_box_get_model(combo_box);
    GtkTreeIter iter;
    gchar *item;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first(model, &iter);

    while (valid && max_items > 0)
    {
        gtk_tree_model_get(model, &iter, 0, &item, -1);

        if (!is_present_in_list(items, item))
        {
            items = g_slist_prepend(items, item);
            max_items--;
        }

        valid = gtk_tree_model_iter_next(model, &iter);
    }

    return items;
}

static gchar *copy_and_trim(const gchar *str)
{
    gchar *result = g_strdup(str);

    g_strstrip(result);

    return result;
}

/**
 * @brief Load history item from GSettings as a new model for the combo_box.
 */
void gtt_combo_history_list_init(GtkComboBox *combo_box, const gchar *history_id)
{
    GtkListStore *store;

    store = gtk_list_store_new(1, G_TYPE_STRING);

    /* Load up items from GSettings into our list model.*/
    GSettings *settings = settings_open_history(history_id);

    GSList *items = gtt_gsettings_get_array_string(settings, "history");
    GSList *node = NULL;
    for (node = items; NULL != node; node = node->next)
    {
        gtk_list_store_insert_with_values(
            store, NULL, G_MAXINT, TEXT_COLUMN_ID, node->data, -1);
    }

    g_slist_free(items);
    g_object_unref(settings);

    /* Attach list model. The combo box should be set to display column 0 as text. */
    gtk_combo_box_set_model(combo_box, GTK_TREE_MODEL(store));
}

/**
 * @brief Save combo_box items (and current text) as history in GSettings.
 */
void gtt_combo_history_list_save(GtkComboBox *combo_box, const gchar *history_id, gint max_items)
{
    GSList *gsettings_items = NULL;
    gchar *item;

    if (max_items < 1)
        max_items = 10;

    /* The text from the combo box text entry, which may not be in the model.*/
    item = copy_and_trim(gtt_combo_entry_get_text(combo_box));
    if (item && strcmp(item, "") != 0)
    {
        gsettings_items = g_slist_prepend(gsettings_items, (gpointer)item);
        max_items--;
    }

    /* Then add everything from the model. */
    gsettings_items = copy_model_to_gslist(combo_box, gsettings_items, max_items);

    gsettings_items = g_slist_reverse(gsettings_items);

    /* Save as array in GSettings. */
    GSettings *settings = settings_open_history(history_id);
    gtt_gsettings_set_array_string(settings, "history", gsettings_items);

    g_free(item);
    g_slist_free(gsettings_items);
    g_object_unref(settings);
}

