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
#include "prefs.h"
#include "toolbar.h"

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

gchar *gtt_gsettings_get_expander(void)
{
    GSettings *display = g_settings_get_child(settings_obj, "display");

    gchar *ret = NULL;

    gtt_gsettings_get_maybe_str(display, "expander-state", &ret);

    g_object_unref(display);
    display = NULL;

    return ret;
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

    // Display -----------------------------------------------------------------
    {
        GSettings *display = g_settings_get_child(settings_obj, "display");

        config_show_secs = g_settings_get_boolean(display, "show-secs");

        prefs_set_show_secs();

        config_show_clist_titles = g_settings_get_boolean(display, "show-table-header");
        config_show_subprojects = g_settings_get_boolean(display, "show-sub-projects");
        config_show_statusbar = g_settings_get_boolean(display, "show-statusbar");

        config_show_title_ever = g_settings_get_boolean(display, "show-time-ever");
        config_show_title_day = g_settings_get_boolean(display, "show-time-day");
        config_show_title_yesterday = g_settings_get_boolean(display, "show-time-yesterday");
        config_show_title_week = g_settings_get_boolean(display, "show-time-week");
        config_show_title_lastweek = g_settings_get_boolean(display, "show-time-last-week");
        config_show_title_month = g_settings_get_boolean(display, "show-time-month");
        config_show_title_year = g_settings_get_boolean(display, "show-time-year");
        config_show_title_current = g_settings_get_boolean(display, "show-time-current");
        config_show_title_desc = g_settings_get_boolean(display, "show-desc");
        config_show_title_task = g_settings_get_boolean(display, "show-task");
        config_show_title_estimated_start
            = g_settings_get_boolean(display, "show-estimated-start");
        config_show_title_estimated_end = g_settings_get_boolean(display, "show-estimated-end");
        config_show_title_due_date = g_settings_get_boolean(display, "show-due-date");
        config_show_title_sizing = g_settings_get_boolean(display, "show-sizing");
        config_show_title_percent_complete
            = g_settings_get_boolean(display, "show-percent-complete");
        config_show_title_urgency = g_settings_get_boolean(display, "show-urgency");
        config_show_title_importance = g_settings_get_boolean(display, "show-importance");
        config_show_title_status = g_settings_get_boolean(display, "show-status");

        g_object_unref(display);
        display = NULL;
    }

    const gboolean _c = config_show_tb_ccp;
    const gboolean _e = config_show_tb_exit;
    const gboolean _h = config_show_tb_help;
    const gboolean _j = config_show_tb_journal;
    const gboolean _n = config_show_tb_new;
    const gboolean _o = config_show_tb_pref;
    const gboolean _p = config_show_tb_prop;
    const gboolean _t = config_show_tb_timer;

    // Toolbar -----------------------------------------------------------------
    {
        GSettings *toolbar = g_settings_get_child(settings_obj, "toolbar");

        config_show_tb_ccp = g_settings_get_boolean(toolbar, "show-ccp");
        config_show_tb_exit = g_settings_get_boolean(toolbar, "show-exit");
        config_show_tb_help = g_settings_get_boolean(toolbar, "show-help");
        config_show_tb_journal = g_settings_get_boolean(toolbar, "show-journal");
        config_show_tb_new = g_settings_get_boolean(toolbar, "show-new");
        config_show_tb_pref = g_settings_get_boolean(toolbar, "show-pref");
        config_show_tb_prop = g_settings_get_boolean(toolbar, "show-prop");
        config_show_tb_tips = g_settings_get_boolean(toolbar, "show-tips");
        config_show_tb_timer = g_settings_get_boolean(toolbar, "show-timer");
        config_show_toolbar = g_settings_get_boolean(toolbar, "show-toolbar");

        g_object_unref(toolbar);
        toolbar = NULL;
    }

    // Report ------------------------------------------------------------------
    {
        GSettings *report = g_settings_get_child(settings_obj, "report");

        gtt_gsettings_get_str(report, "currency-symbol", &config_currency_symbol);
        config_currency_use_locale = g_settings_get_boolean(report, "currency-use-locale");

        g_object_unref(report);
        report = NULL;
    }

    // Redraw the GUI
    if (config_show_statusbar)
    {
        gtk_widget_show(status_bar);
    }
    else
    {
        gtk_widget_hide(status_bar);
    }

    update_status_bar();

    if ((_n != config_show_tb_new) || (_c != config_show_tb_ccp)
        || (_j != config_show_tb_journal) || (_p != config_show_tb_prop)
        || (_t != config_show_tb_timer) || (_o != config_show_tb_pref)
        || (_h != config_show_tb_help) || (_e != config_show_tb_exit))
    {
        update_toolbar_sections();
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

    // Display -----------------------------------------------------------------
    {
        GSettings *display = g_settings_get_child(settings_obj, "display");

        gtt_gsettings_set_bool(display, "show-secs", config_show_secs);
        gtt_gsettings_set_bool(display, "show-statusbar", config_show_statusbar);
        gtt_gsettings_set_bool(display, "show-sub-projects", config_show_subprojects);
        gtt_gsettings_set_bool(display, "show-table-header", config_show_clist_titles);
        gtt_gsettings_set_bool(display, "show-time-current", config_show_title_current);
        gtt_gsettings_set_bool(display, "show-time-day", config_show_title_day);
        gtt_gsettings_set_bool(display, "show-time-yesterday", config_show_title_yesterday);
        gtt_gsettings_set_bool(display, "show-time-week", config_show_title_week);
        gtt_gsettings_set_bool(display, "show-time-last-week", config_show_title_lastweek);
        gtt_gsettings_set_bool(display, "show-time-month", config_show_title_month);
        gtt_gsettings_set_bool(display, "show-time-year", config_show_title_year);
        gtt_gsettings_set_bool(display, "show-time-ever", config_show_title_ever);
        gtt_gsettings_set_bool(display, "show-desc", config_show_title_desc);
        gtt_gsettings_set_bool(display, "show-task", config_show_title_task);
        gtt_gsettings_set_bool(
            display, "show-estimated-start", config_show_title_estimated_start
        );
        gtt_gsettings_set_bool(display, "show-estimated-end", config_show_title_estimated_end);
        gtt_gsettings_set_bool(display, "show-due-date", config_show_title_due_date);
        gtt_gsettings_set_bool(display, "show-sizing", config_show_title_sizing);
        gtt_gsettings_set_bool(
            display, "show-percent-complete", config_show_title_percent_complete
        );
        gtt_gsettings_set_bool(display, "show-urgency", config_show_title_urgency);
        gtt_gsettings_set_bool(display, "show-importance", config_show_title_importance);
        gtt_gsettings_set_bool(display, "show-status", config_show_title_status);

        const gchar *xpn = gtt_projects_tree_get_expander_state(projects_tree);
        gtt_gsettings_set_maybe_str(display, "expander-state", xpn);

        g_object_unref(display);
        display = NULL;
    }

    // Toolbar -----------------------------------------------------------------
    {
        GSettings *toolbar = g_settings_get_child(settings_obj, "toolbar");

        gtt_gsettings_set_bool(toolbar, "show-ccp", config_show_tb_ccp);
        gtt_gsettings_set_bool(toolbar, "show-exit", config_show_tb_exit);
        gtt_gsettings_set_bool(toolbar, "show-help", config_show_tb_help);
        gtt_gsettings_set_bool(toolbar, "show-journal", config_show_tb_journal);
        gtt_gsettings_set_bool(toolbar, "show-new", config_show_tb_new);
        gtt_gsettings_set_bool(toolbar, "show-pref", config_show_tb_pref);
        gtt_gsettings_set_bool(toolbar, "show-prop", config_show_tb_prop);
        gtt_gsettings_set_bool(toolbar, "show-timer", config_show_tb_timer);
        gtt_gsettings_set_bool(toolbar, "show-tips", config_show_tb_tips);
        gtt_gsettings_set_bool(toolbar, "show-toolbar", config_show_toolbar);

        g_object_unref(toolbar);
        toolbar = NULL;
    }

    // Report ------------------------------------------------------------------
    {
        GSettings *report = g_settings_get_child(settings_obj, "report");

        gtt_gsettings_set_str(report, "currency-symbol", config_currency_symbol);
        gtt_gsettings_set_bool(report, "currency-use-locale", config_currency_use_locale);

        g_object_unref(report);
        report = NULL;
    }
}
