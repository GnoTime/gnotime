/*   GConf2 input/output handling for GTimeTracker - a time tracker
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

#include "config.h"

#include "gtt-gsettings-io.h"

#include "gtt-gsettings-gnomeui.h"
#include "gtt-gsettings-io-p.h"

#include "app.h"
#include "cur-proj.h"
#include "menus.h"
#include "plug-in.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"

static void gtt_gsettings_ensure_initialized(void);
static void gtt_gsettings_restore_reports_menu(GnomeApp *app);

// XXX these should not be externs, they should be part of some app-global structure.
extern int save_count;         // XXX
extern char *first_proj_title; // Commandline flag
extern time_t last_timer;      // XXX
extern int cur_proj_id;
extern int run_timer;

static GSettings *settings_obj = NULL;

/**
 * @brief Save reports menu attributes to the GSettings system
 */
void gtt_gsettings_save_reports_menu(void)
{
    gtt_gsettings_ensure_initialized();

    GnomeUIInfo *const reports_menu = gtt_get_reports_menu();

    // Write out the customer report info
    int i = 0;
    for (i = 0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++)
    {
        gchar *settings_path = g_strdup_printf("/org/gnotime/app/reports/report-%d/", i);

        GSettings *settings
            = g_settings_new_with_path("org.gnotime.app.reports", settings_path);

        g_settings_delay(settings);

        GttPlugin *plg = reports_menu[i].user_data;

        gtt_gsettings_set_string(settings, "name", plg->name);
        gtt_gsettings_set_string(settings, "path", plg->path);
        gtt_gsettings_set_string(settings, "tooltip", plg->tooltip);
        gtt_gsettings_set_string(settings, "last-save-url", plg->last_url);

        gtt_save_gnomeui_to_gsettings(settings, &reports_menu[i]);

        g_settings_apply(settings);

        g_object_unref(settings);
        settings = NULL;

        g_free(settings_path);
        settings_path = NULL;
    }

    GSettings *misc = g_settings_get_child(settings_obj, "misc");
    gtt_gsettings_set_int(misc, "num-reports", i);
    g_object_unref(misc);
    misc = NULL;
}

/**
 * @brief Save all GnoTime attributes into the GSettings application settings system
 */
void gtt_gsettings_save(void)
{
    gtt_gsettings_ensure_initialized();

    g_settings_delay(settings_obj);

    // Geometry --------------------------------------------------------------------------------
    {
        GSettings *geometry = g_settings_get_child(settings_obj, "geometry");

        GdkWindow *const win = gtk_widget_get_window(app_window);

        // Save the window location and size
        gtt_gsettings_set_int(geometry, "width", gdk_window_get_width(win));
        gtt_gsettings_set_int(geometry, "height", gdk_window_get_height(win));

        gint x, y;
        gdk_window_get_origin(win, &x, &y);
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

    // Display ---------------------------------------------------------------------------------
    {
        GSettings *display = g_settings_get_child(settings_obj, "display");

        // Save the configure dialog values
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

        const char *xpn = gtt_projects_tree_get_expander_state(projects_tree);
        gtt_gsettings_set_maybe_string(display, "expander-state", xpn);

        g_object_unref(display);
        display = NULL;
    }

    // Toolbar ---------------------------------------------------------------------------------
    {
        GSettings *toolbar = g_settings_get_child(settings_obj, "toolbar");

        gtt_gsettings_set_bool(toolbar, "show-toolbar", config_show_toolbar);
        gtt_gsettings_set_bool(toolbar, "show-tips", config_show_tb_tips);
        gtt_gsettings_set_bool(toolbar, "show-new", config_show_tb_new);
        gtt_gsettings_set_bool(toolbar, "show-ccp", config_show_tb_ccp);
        gtt_gsettings_set_bool(toolbar, "show-journal", config_show_tb_journal);
        gtt_gsettings_set_bool(toolbar, "show-prop", config_show_tb_prop);
        gtt_gsettings_set_bool(toolbar, "show-timer", config_show_tb_timer);
        gtt_gsettings_set_bool(toolbar, "show-pref", config_show_tb_pref);
        gtt_gsettings_set_bool(toolbar, "show-help", config_show_tb_help);
        gtt_gsettings_set_bool(toolbar, "show-exit", config_show_tb_exit);

        g_object_unref(toolbar);
        toolbar = NULL;
    }

    // Actions ---------------------------------------------------------------------------------
    {
        GSettings *actions = g_settings_get_child(settings_obj, "actions");

        gtt_gsettings_set_maybe_string(actions, "start-command", config_shell_start);
        gtt_gsettings_set_maybe_string(actions, "stop-command", config_shell_stop);

        g_object_unref(actions);
        actions = NULL;
    }

    // Log-File --------------------------------------------------------------------------------
    {
        GSettings *log_file = g_settings_get_child(settings_obj, "log-file");

        gtt_gsettings_set_bool(log_file, "use", config_logfile_use);
        gtt_gsettings_set_maybe_string(log_file, "filename", config_logfile_name);
        gtt_gsettings_set_string(
            log_file, "entry-start", (NULL != config_logfile_start) ? config_logfile_start : ""
        );
        gtt_gsettings_set_string(
            log_file, "entry-stop", (NULL != config_logfile_stop) ? config_logfile_stop : ""
        );
        gtt_gsettings_set_int(log_file, "min-secs", config_logfile_min_secs);

        g_object_unref(log_file);
        log_file = NULL;
    }

    // Data ------------------------------------------------------------------------------------
    {
        GSettings *data = g_settings_get_child(settings_obj, "data");

        gtt_gsettings_set_string(data, "url", config_data_url);
        gtt_gsettings_set_int(data, "save-count", save_count);

        g_object_unref(data);
        data = NULL;
    }

    // C-List ----------------------------------------------------------------------------------
    {
        GSettings *c_list = g_settings_get_child(settings_obj, "c-list");

        long i, w;
        GSList *list = NULL;
        for (i = 0, w = 0; -1 < w; i++)
        {
            w = gtt_projects_tree_get_col_width(projects_tree, i);
            if (0 > w)
                break;
            list = g_slist_prepend(list, GINT_TO_POINTER(w));
        }
        list = g_slist_reverse(list);

        gtt_gsettings_set_array_int(c_list, "column-widths", list);

        g_slist_free(list);
        list = NULL;

        g_object_unref(c_list);
        c_list = NULL;
    }

    // Misc ------------------------------------------------------------------------------------
    {
        GSettings *misc = g_settings_get_child(settings_obj, "misc");

        // Use string for time, to avoid integer conversion problems
        gchar s[120];
        g_snprintf(s, sizeof(s), "%ld", time(0));
        gtt_gsettings_set_string(misc, "last-timer", s);
        gtt_gsettings_set_int(misc, "idle-timeout", config_idle_timeout);
        gtt_gsettings_set_int(misc, "no-project-timeout", config_no_project_timeout);
        gtt_gsettings_set_int(misc, "autosave-period", config_autosave_period);
        gtt_gsettings_set_int(misc, "timer-running", timer_is_running());
        gtt_gsettings_set_int(misc, "curr-project", gtt_project_get_id(cur_proj));
        gtt_gsettings_set_int(misc, "num-projects", -1);

        gtt_gsettings_set_int(misc, "day-start-offset", config_daystart_offset);
        gtt_gsettings_set_int(misc, "week-start-offset", config_weekstart_offset);

        g_object_unref(misc);
        misc = NULL;
    }

    gtt_gsettings_set_int(settings_obj, "time-format", config_time_format);

    // Report ----------------------------------------------------------------------------------
    {
        GSettings *report = g_settings_get_child(settings_obj, "report");

        gtt_gsettings_set_string(report, "currency-symbol", config_currency_symbol);
        gtt_gsettings_set_bool(report, "currency-use-locale", config_currency_use_locale);

        g_object_unref(report);
        report = NULL;
    }

    // Write out the user's report menu structure
    gtt_gsettings_save_reports_menu();

    // Signal to future GnoTime application runs that the GSettings system is initialized and
    // to be used as the primary attribute storage system.
    gtt_gsettings_set_bool(settings_obj, "initialized", TRUE);

    g_settings_apply(settings_obj);
}

/**
 * @brief Check if a write to GSettings has occurred already or if this is the initial access
 *
 * The purpose behind this method is to grant the possibility to retrieve settings from previous
 * configuration systems. TODO: As soon as these old configuration systems have been dropped
 * this function can be removed too.
 *
 * @return `TRUE` if settings have been written to GSettings already, `FALSE` otherwise
 */
gboolean gtt_gsettings_initial_access(void)
{
    gtt_gsettings_ensure_initialized();

    return (FALSE == g_settings_get_boolean(settings_obj, "initialized"));
}

/**
 * @brief Fetch all GnoTime attributes from the GSettings application settings system
 */
void gtt_gsettings_load(void)
{
    gtt_gsettings_ensure_initialized();

    // If already running, and we are over-loading a new file, then save the currently running
    // project, and try to set it running again ...
    if (gtt_project_get_title(cur_proj) && (!first_proj_title))
    {
        // ` strdup` is needed because title is freed when the project list is destroyed ...
        first_proj_title = g_strdup(gtt_project_get_title(cur_proj));
    }

    const int _n = config_show_tb_new;
    const int _c = config_show_tb_ccp;
    const int _j = config_show_tb_journal;
    const int _p = config_show_tb_prop;
    const int _t = config_show_tb_timer;
    const int _o = config_show_tb_pref;
    const int _h = config_show_tb_help;
    const int _e = config_show_tb_exit;

    // Misc ------------------------------------------------------------------------------------
    {
        GSettings *misc = g_settings_get_child(settings_obj, "misc");

        // Get last running project
        cur_proj_id = g_settings_get_int(misc, "curr-project");

        config_idle_timeout = g_settings_get_int(misc, "idle-timeout");
        config_no_project_timeout = g_settings_get_int(misc, "no-project-timeout");
        config_autosave_period = g_settings_get_int(misc, "autosave-period");
        config_daystart_offset = g_settings_get_int(misc, "day-start-offset");
        config_weekstart_offset = g_settings_get_int(misc, "week-start-offset");

        g_object_unref(misc);
        misc = NULL;
    }

    // Geometry --------------------------------------------------------------------------------
    {
        GSettings *geometry = g_settings_get_child(settings_obj, "geometry");

        /* Reset the main window width and height to the values
         * last stored in the config file.  Note that if the user
         * specified command-line flags, then the command line
         * over-rides the config file. */
        if (!geom_place_override)
        {
            const gint x = g_settings_get_int(geometry, "x");
            const gint y = g_settings_get_int(geometry, "y");
            gtk_window_move(GTK_WINDOW(app_window), x, y);
        }
        if (!geom_size_override)
        {
            const gint w = g_settings_get_int(geometry, "width");
            const gint h = g_settings_get_int(geometry, "height");

            gtk_window_set_default_size(GTK_WINDOW(app_window), w, h);
        }

        {
            const gint vp = g_settings_get_int(geometry, "v-paned");
            const gint hp = g_settings_get_int(geometry, "h-paned");
            notes_area_set_pane_sizes(global_na, vp, hp);
        }

        g_object_unref(geometry);
        geometry = NULL;
    }

    // Display ---------------------------------------------------------------------------------
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

    prefs_update_projects_view();

    // Toolbar ---------------------------------------------------------------------------------
    {
        GSettings *toolbar = g_settings_get_child(settings_obj, "toolbar");

        config_show_toolbar = g_settings_get_boolean(toolbar, "show-toolbar");
        config_show_tb_tips = g_settings_get_boolean(toolbar, "show-tips");
        config_show_tb_new = g_settings_get_boolean(toolbar, "show-new");
        config_show_tb_ccp = g_settings_get_boolean(toolbar, "show-ccp");
        config_show_tb_journal = g_settings_get_boolean(toolbar, "show-journal");
        config_show_tb_prop = g_settings_get_boolean(toolbar, "show-prop");
        config_show_tb_timer = g_settings_get_boolean(toolbar, "show-timer");
        config_show_tb_pref = g_settings_get_boolean(toolbar, "show-pref");
        config_show_tb_help = g_settings_get_boolean(toolbar, "show-help");
        config_show_tb_exit = g_settings_get_boolean(toolbar, "show-exit");

        g_object_unref(toolbar);
        toolbar = NULL;
    }

    // Actions ---------------------------------------------------------------------------------
    {
        GSettings *actions = g_settings_get_child(settings_obj, "actions");

        gtt_gsettings_get_maybe_string(actions, "start-command", &config_shell_start);
        gtt_gsettings_get_maybe_string(actions, "stop-command", &config_shell_stop);

        g_object_unref(actions);
        actions = NULL;
    }

    // Log-File --------------------------------------------------------------------------------
    {
        GSettings *log_file = g_settings_get_child(settings_obj, "log-file");

        config_logfile_use = g_settings_get_boolean(log_file, "use");
        gtt_gsettings_get_maybe_string(log_file, "filename", &config_logfile_name);
        gtt_gsettings_get_string(log_file, "entry-start", &config_logfile_start);
        gtt_gsettings_get_string(log_file, "entry-stop", &config_logfile_stop);
        config_logfile_min_secs = g_settings_get_int(log_file, "min-secs");

        g_object_unref(log_file);
        log_file = NULL;
    }

    config_time_format = g_settings_get_int(settings_obj, "time-format");

    // Report ----------------------------------------------------------------------------------
    {
        GSettings *report = g_settings_get_child(settings_obj, "report");

        gtt_gsettings_get_string(report, "currency-symbol", &config_currency_symbol);
        config_currency_use_locale = g_settings_get_boolean(report, "currency-use-locale");

        g_object_unref(report);
        report = NULL;
    }

    // Data ------------------------------------------------------------------------------------
    {
        GSettings *data = g_settings_get_child(settings_obj, "data");

        save_count = g_settings_get_int(data, "save-count");
        gtt_gsettings_get_string(data, "url", &config_data_url);

        g_object_unref(data);
        data = NULL;
    }

    // CList -----------------------------------------------------------------------------------
    {
        GSettings *c_list = g_settings_get_child(settings_obj, "c-list");

        int i;
        GSList *list = gtt_gsettings_get_array_int(c_list, "column-widths");
        GSList *node;
        for (i = 0, node = list; node != NULL; node = node->next, i++)
        {
            const int num = GPOINTER_TO_INT(node->data);
            if (-1 < num)
            {
                gtt_projects_tree_set_col_width(projects_tree, i, num);
            }
        }

        g_slist_free(list);
        list = NULL;

        g_object_unref(c_list);
        c_list = NULL;
    }

    // Read in the user-defined report locations
    gtt_gsettings_restore_reports_menu(GNOME_APP(app_window));

    // Misc ------------------------------------------------------------------------------------
    {
        GSettings *misc = g_settings_get_child(settings_obj, "misc");

        run_timer = g_settings_get_int(misc, "timer-running");
        // Use string for time, to avoid unsigned-long problems
        gchar *tmp_last_timer = g_settings_get_string(misc, "last-timer");
        last_timer = (time_t) atol(tmp_last_timer);
        g_free(tmp_last_timer);
        tmp_last_timer = NULL,

        g_object_unref(misc);
        misc = NULL;
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

gchar *gtt_gsettings_get_expander(void)
{
    gtt_gsettings_ensure_initialized();

    GSettings *display = g_settings_get_child(settings_obj, "display");

    gchar *res = NULL;
    gtt_gsettings_get_maybe_string(display, "expander-state", &res);

    g_object_unref(display);
    display = NULL;

    return res;
}

static void gtt_gsettings_ensure_initialized(void)
{
    if (G_LIKELY(NULL != settings_obj))
    {
        return;
    }

    settings_obj = g_settings_new("org.gnotime.app");
}

static void gtt_gsettings_restore_reports_menu(GnomeApp *const app)
{
    gtt_gsettings_ensure_initialized();

    // Read in the user-defined report locations
    GSettings *misc = g_settings_get_child(settings_obj, "misc");
    const gint num = g_settings_get_int(misc, "num-reports");
    g_object_unref(misc);
    misc = NULL;

    GnomeUIInfo *reports_menu = g_new0(GnomeUIInfo, num + 1);

    int i = 0;
    for (i = 0; i < num; i++)
    {
        gchar *settings_path = g_strdup_printf("/org/gnotime/app/reports/report-%d/", i);

        GSettings *settings
            = g_settings_new_with_path("org.gnotime.app.reports", settings_path);

        gchar *name = g_settings_get_string(settings, "name");
        gchar *path = g_settings_get_string(settings, "path");

        GttPlugin *plg = gtt_plugin_new(name, path);

        g_free(path);
        path = NULL;
        g_free(name);
        name = NULL;

        plg->tooltip = g_settings_get_string(settings, "tooltip");
        plg->last_url = g_settings_get_string(settings, "last-save-url");

        gtt_restore_gnomeui_from_gsettings(settings, &reports_menu[i]);

        // Fixup
        reports_menu[i].user_data = plg;

        g_object_unref(settings);
        settings = NULL;

        g_free(settings_path);
        settings_path = NULL;
    }
    reports_menu[i].type = GNOME_APP_UI_ENDOFINFO;

    gtt_set_reports_menu(app, reports_menu);
}
