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

#include <stdlib.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "app.h"
#include "cur-proj.h"
#include "gconf-io-p.h"
#include "gconf-io.h"
#include "gtt.h"
#include "menus.h"
#include "plug-in.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"

/* XXX these should not be externs, they should be part of
 * some app-global structure.
 */
extern int save_count;         /* XXX */
extern char *first_proj_title; /* command line flag */
extern time_t last_timer;      /* XXX */
extern int cur_proj_id;
extern int run_timer;

#define GTT_GCONF "/apps/gnotime"

/* ======================================================= */

gboolean gtt_gconf_exists(void)
{
    GError *err_ret = NULL;
    GConfClient *client;
    GConfValue *gcv;

    /* Calling gconf_engine_dir_exists() on a non-existant directory
     * completely hoses that directory for future use. Its Badddd.
     * rc = gconf_engine_dir_exists (gengine, GTT_GCONF, &err_ret);
     * gconf_client_dir_exists() is no better.
     * Actually, the bug is that the dirs are unusable only while
     * gconf is still running. Upon reboot, its starts working OK.
     * Hack around it by trying to fetch a key.
     */

    client = gconf_client_get_default();
    gcv = gconf_client_get(client, GTT_GCONF "/dir_exists", &err_ret);
    if ((NULL == gcv) || (FALSE == GCONF_VALUE_TYPE_VALID(gcv->type)))
    {
        if (err_ret)
            printf("GTT: Error: gconf_exists XXX err %s\n", err_ret->message);
        return FALSE;
    }

    return TRUE;
}

/* ======================================================= */

void gtt_restore_reports_menu()
{
    int i, num;
    char s[120], *p;
    GttPlugin *reports_menu;
    GConfClient *client;

    client = gconf_client_get_default();

    /* Read in the user-defined report locations */
    num = GETINT("/Misc/NumReports", 0);
    reports_menu = g_new0(GttPlugin, num + 1);

    for (i = 0; i < num; i++)
    {
        GttPlugin *plg;
        const char *name, *path, *tip, *url;

        g_snprintf(s, sizeof(s), GTT_GCONF "/Reports/%d/", i);
        p = s + strlen(s);

        strcpy(p, "Name");
        name = F_GETSTR(s, "");

        strcpy(p, "Path");
        path = F_GETSTR(s, "");

        strcpy(p, "Tooltip");
        tip = F_GETSTR(s, "");

        strcpy(p, "LastSaveURL");
        url = F_GETSTR(s, "");

        plg = &reports_menu[i];
        plg->name = g_strdup(name);
        plg->path = g_strdup(path);
        plg->tooltip = g_strdup(tip);
        plg->last_url = g_strdup(url);

        *p = 0;
    }

    gtt_set_reports_menu(reports_menu);
}

/* ======================================================= */

void gtt_gconf_load(void)
{
    int i, num;
    int _n, _c, _j, _p, _t, _o, _h, _e;
    GConfClient *client;

    client = gconf_client_get_default();
    gconf_client_add_dir(client, GTT_GCONF, GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

    /* If already running, and we are over-loading a new file,
     * then save the currently running project, and try to set it
     * running again ... */
    if (gtt_project_get_title(cur_proj) && (!first_proj_title))
    {
        /* We need to strdup because title is freed when
         * the project list is destroyed ... */
        first_proj_title = g_strdup(gtt_project_get_title(cur_proj));
    }

    _n = config_show_tb_new;
    _c = config_show_tb_ccp;
    _j = config_show_tb_journal;
    _p = config_show_tb_prop;
    _t = config_show_tb_timer;
    _o = config_show_tb_pref;
    _h = config_show_tb_help;
    _e = config_show_tb_exit;

    /* Get last running project */
    cur_proj_id = GETINT("/Misc/CurrProject", -1);

    config_idle_timeout = GETINT("/Misc/IdleTimeout", 300);
    config_no_project_timeout = GETINT("/Misc/NoProjectTimeout", 300);
    config_autosave_period = GETINT("/Misc/AutosavePeriod", 60);
    config_daystart_offset = GETINT("/Misc/DayStartOffset", 0);
    config_weekstart_offset = GETINT("/Misc/WeekStartOffset", 0);

    /* Reset the main window width and height to the values
     * last stored in the config file.  Note that if the user
     * specified command-line flags, then the command line
     * over-rides the config file. */
    if (!geom_place_override)
    {
        int x, y;
        x = GETINT("/Geometry/X", 10);
        y = GETINT("/Geometry/Y", 10);
        gtk_window_move(GTK_WINDOW(app_window), x, y);
    }
    if (!geom_size_override)
    {
        int w, h;
        w = GETINT("/Geometry/Width", 442);
        h = GETINT("/Geometry/Height", 272);

        gtk_window_set_default_size(GTK_WINDOW(app_window), w, h);
    }

    {
        int vp, hp;
        vp = GETINT("/Geometry/VPaned", 250);
        hp = GETINT("/Geometry/HPaned", 220);
        notes_area_set_pane_sizes(global_na, vp, hp);
    }

    config_show_secs = GETBOOL("/Display/ShowSecs", FALSE);

    prefs_set_show_secs();

    config_show_clist_titles = GETBOOL("/Display/ShowTableHeader", FALSE);
    config_show_subprojects = GETBOOL("/Display/ShowSubProjects", TRUE);
    config_show_statusbar = GETBOOL("/Display/ShowStatusbar", TRUE);

    config_show_title_ever = GETBOOL("/Display/ShowTimeEver", TRUE);
    config_show_title_day = GETBOOL("/Display/ShowTimeDay", TRUE);
    config_show_title_yesterday = GETBOOL("/Display/ShowTimeYesterday", FALSE);
    config_show_title_week = GETBOOL("/Display/ShowTimeWeek", FALSE);
    config_show_title_lastweek = GETBOOL("/Display/ShowTimeLastWeek", FALSE);
    config_show_title_month = GETBOOL("/Display/ShowTimeMonth", FALSE);
    config_show_title_year = GETBOOL("/Display/ShowTimeYear", FALSE);
    config_show_title_current = GETBOOL("/Display/ShowTimeCurrent", FALSE);
    config_show_title_desc = GETBOOL("/Display/ShowDesc", TRUE);
    config_show_title_task = GETBOOL("/Display/ShowTask", TRUE);
    config_show_title_estimated_start = GETBOOL("/Display/ShowEstimatedStart", FALSE);
    config_show_title_estimated_end = GETBOOL("/Display/ShowEstimatedEnd", FALSE);
    config_show_title_due_date = GETBOOL("/Display/ShowDueDate", FALSE);
    config_show_title_sizing = GETBOOL("/Display/ShowSizing", FALSE);
    config_show_title_percent_complete = GETBOOL("/Display/ShowPercentComplete", FALSE);
    config_show_title_urgency = GETBOOL("/Display/ShowUrgency", TRUE);
    config_show_title_importance = GETBOOL("/Display/ShowImportance", TRUE);
    config_show_title_status = GETBOOL("/Display/ShowStatus", FALSE);

    prefs_update_projects_view();

    /* ------------ */
    config_show_toolbar = GETBOOL("/Toolbar/ShowToolbar", TRUE);
    config_show_tb_tips = GETBOOL("/Toolbar/ShowTips", TRUE);
    config_show_tb_new = GETBOOL("/Toolbar/ShowNew", TRUE);
    config_show_tb_ccp = GETBOOL("/Toolbar/ShowCCP", FALSE);
    config_show_tb_journal = GETBOOL("/Toolbar/ShowJournal", TRUE);
    config_show_tb_prop = GETBOOL("/Toolbar/ShowProp", TRUE);
    config_show_tb_timer = GETBOOL("/Toolbar/ShowTimer", TRUE);
    config_show_tb_pref = GETBOOL("/Toolbar/ShowPref", FALSE);
    config_show_tb_help = GETBOOL("/Toolbar/ShowHelp", TRUE);
    config_show_tb_exit = GETBOOL("/Toolbar/ShowExit", TRUE);

    /* ------------ */
    config_shell_start = GETSTR(
        "/Actions/StartCommand",
        "echo start id=%D \\\"%t\\\"-\\\"%d\\\" %T  %H-%M-%S hours=%h min=%m secs=%s"
    );
    config_shell_stop = GETSTR(
        "/Actions/StopCommand",
        "echo stop id=%D \\\"%t\\\"-\\\"%d\\\" %T  %H-%M-%S hours=%h min=%m secs=%s"
    );

    /* ------------ */
    config_logfile_use = GETBOOL("/LogFile/Use", FALSE);
    config_logfile_name = GETSTR("/LogFile/Filename", NULL);
    config_logfile_start = GETSTR("/LogFile/EntryStart", _("project %t started"));
    config_logfile_stop = GETSTR("/LogFile/EntryStop", _("stopped project %t"));
    config_logfile_min_secs = GETINT("/LogFile/MinSecs", 3);

    /* ------------ */
    config_time_format = GETINT("/time_format", 3);

    config_currency_symbol = GETSTR("/Report/CurrencySymbol", "$");
    config_currency_use_locale = GETBOOL("/Report/CurrencyUseLocale", TRUE);
    /* ------------ */
    save_count = GETINT("/Data/SaveCount", 0);
    config_data_url = GETSTR("/Data/URL", XML_DATA_FILENAME);

    /* ------------ */
    {
        GSList *node, *list = GETINTLIST("/CList/ColumnWidths");
        for (i = 0, node = list; node != NULL; node = node->next, i++)
        {
            num = (long) (node->data);
            if (-1 < num)
            {
                gtt_projects_tree_set_col_width(projects_tree, i, num);
            }
        }
    }

    /* Read in the user-defined report locations */
    gtt_restore_reports_menu();

    run_timer = GETINT("/Misc/TimerRunning", 0);
    /* Use string for time, to avoid unsigned-long problems */
    last_timer = (time_t) atol(GETSTR("/Misc/LastTimer", "-1"));

    /* redraw the GUI */
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

gchar *gtt_gconf_get_expander(void)
{
    GConfClient *client = gconf_client_get_default();
    return GETSTR("/Display/ExpanderState", NULL);
}

/* =========================== END OF FILE ========================= */
