/* GSettings configuration handling for GnoTime - a time tracker
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

#include "gtt-gsettings-io.h"

#include "app.h"
#include "gtt-gsettings-io-p.h"

#include <gio/gio.h>

static GSettings *settings_obj = NULL;

/**
 * @brief Free the internally held GSettings object
 *
 * This should be called once on application shutdown AFTER all settings have been stored.
 */
void gtt_gsettings_deinit(void)
{
    if (G_UNLIKELY(NULL == settings_obj))
    {
        g_warning("GSettings object is not initialized unexpectedly");

        return;
    }

    g_clear_object(&settings_obj);
}

/**
 * @brief Initialize the primary GSettings object of GnoTime
 *
 * This should be called once on application startup BEFORE any settings can be queried.
 */
void gtt_gsettings_init(void)
{
    if (G_UNLIKELY(NULL != settings_obj))
    {
        g_warning("GSettings object is initialized unexpectedly");

        return;
    }

    settings_obj = g_settings_new("org.gnotime.app");
}

void gtt_gsettings_load(void)
{
    // Geometry ----------------------------------------------------------------
    {
        GSettings *geometry = g_settings_get_child(settings_obj, "geometry");

        // Reset the main window width and height to the values last stored in the config file.
        // Note that if the user specified commandline flags, then the command line over-rides
        // the config file.
        if (FALSE == geom_place_override)
        {
            const gint x = g_settings_get_int(geometry, "x");
            const gint y = g_settings_get_int(geometry, "y");
            gtk_widget_set_uposition(GTK_WIDGET(app_window), x, y);
        }

        if (FALSE == geom_size_override)
        {
            const gint w = g_settings_get_int(geometry, "width");
            const gint h = g_settings_get_int(geometry, "height");
            gtk_window_set_default_size(GTK_WINDOW(app_window), w, h);
        }

        const gint vp = g_settings_get_int(geometry, "v-paned");
        const gint hp = g_settings_get_int(geometry, "h-paned");
        notes_area_set_pane_sizes(global_na, vp, hp);

        g_object_unref(geometry);
        geometry = NULL;
    }
}

void gtt_gsettings_save(void)
{
    // Geometry ----------------------------------------------------------------
    {
        GSettings *geometry = g_settings_get_child(settings_obj, "geometry");

        // Save the window location and size
        gint x, y;
        gdk_window_get_origin(app_window->window, &x, &y);
        gint w, h;
        gdk_window_get_size(app_window->window, &w, &h);
        gtt_gsettings_set_int(geometry, "width", w);
        gtt_gsettings_set_int(geometry, "height", h);
        gtt_gsettings_set_int(geometry, "x", x);
        gtt_gsettings_set_int(geometry, "y", y);

        {
            int vp, hp;
            notes_area_get_pane_sizes(global_na, &vp, &hp);
            gtt_gsettings_set_int(geometry, "v-paned", vp);
            gtt_gsettings_set_int(geometry, "h-paned", hp);
        }

        g_object_unref(geometry);
        geometry = NULL;
    }
}
