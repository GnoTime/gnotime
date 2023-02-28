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
