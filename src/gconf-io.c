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

#include <gconf/gconf-client.h>
#include <gconf/gconf.h>
#include <glib.h>
#include <gnome.h>

#include "app.h"
#include "cur-proj.h"
#include "gconf-gnomeui.h"
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

void gtt_save_reports_menu(void)
{
    int i;
    char s[120], *p;
    GnomeUIInfo *reports_menu;
    GConfClient *client;

    client = gconf_client_get_default();
    reports_menu = gtt_get_reports_menu();

    /* Write out the customer report info */
    for (i = 0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++)
    {
        GttPlugin *plg = reports_menu[i].user_data;
        g_snprintf(s, sizeof(s), GTT_GCONF "/Reports/%d/", i);
        p = s + strlen(s);
        strcpy(p, "Name");
        F_SETSTR(s, plg->name);

        strcpy(p, "Path");
        F_SETSTR(s, plg->path);

        strcpy(p, "Tooltip");
        F_SETSTR(s, plg->tooltip);

        strcpy(p, "LastSaveURL");
        F_SETSTR(s, plg->last_url);

        *p = 0;
        gtt_save_gnomeui_to_gconf(client, s, &reports_menu[i]);
    }
    SETINT("/Misc/NumReports", i);
}

/* ======================================================= */
/* Save only the GUI configuration info, not the actual data */
/* XXX fixme -- this should really use GConfChangeSet */

void gtt_gconf_save(void)
{
    char s[120];

    GConfEngine *gengine;
    GConfClient *client;

    gengine = gconf_engine_get_default();
    client = gconf_client_get_for_engine(gengine);
    SETINT("/dir_exists", 1);

    /* ------------- */
    if (config_shell_start)
    {
        SETSTR("/Actions/StartCommand", config_shell_start);
    }
    else
    {
        UNSET("/Actions/StartCommand");
    }

    if (config_shell_stop)
    {
        SETSTR("/Actions/StopCommand", config_shell_stop);
    }
    else
    {
        UNSET("/Actions/StopCommand");
    }

    /* ------------- */
    SETBOOL("/LogFile/Use", config_logfile_use);
    if (config_logfile_name)
    {
        SETSTR("/LogFile/Filename", config_logfile_name);
    }
    else
    {
        UNSET("/LogFile/Filename");
    }

    if (config_logfile_start)
    {
        SETSTR("/LogFile/EntryStart", config_logfile_start);
    }
    else
    {
        SETSTR("/LogFile/EntryStart", "");
    }

    if (config_logfile_stop)
    {
        SETSTR("/LogFile/EntryStop", config_logfile_stop);
    }
    else
    {
        SETSTR("/LogFile/EntryStop", "");
    }

    SETINT("/LogFile/MinSecs", config_logfile_min_secs);

    /* ------------- */
    SETSTR("/Data/URL", config_data_url);
    SETINT("/Data/SaveCount", save_count);

    /* ------------- */
    {
        long i, w;
        GSList *list = NULL;
        for (i = 0, w = 0; -1 < w; i++)
        {
            w = gtt_projects_tree_get_col_width(projects_tree, i);
            if (0 > w)
                break;
            list = g_slist_prepend(list, (gpointer) w);
        }
        list = g_slist_reverse(list);
        SETLIST("/CList/ColumnWidths", GCONF_VALUE_INT, list);
        g_slist_free(list);
    }

    /* ------------- */
    /* Use string for time, to avoid integer conversion problems */
    g_snprintf(s, sizeof(s), "%ld", time(0));
    SETSTR("/Misc/LastTimer", s);
    SETINT("/Misc/IdleTimeout", config_idle_timeout);
    SETINT("/Misc/NoProjectTimeout", config_no_project_timeout);
    SETINT("/Misc/AutosavePeriod", config_autosave_period);
    SETINT("/Misc/TimerRunning", timer_is_running());
    SETINT("/Misc/CurrProject", gtt_project_get_id(cur_proj));
    SETINT("/Misc/NumProjects", -1);

    SETINT("/Misc/DayStartOffset", config_daystart_offset);
    SETINT("/Misc/WeekStartOffset", config_weekstart_offset);

    SETINT("/time_format", config_time_format);

    /* Write out the user's report menu structure */
    gtt_save_reports_menu();

    /* Sync to file.
     * XXX if this fails, the error is serious, and there should be a
     * graphical popup.
     */
    {
        GError *err_ret = NULL;
        gconf_client_suggest_sync(client, &err_ret);
        if (NULL != err_ret)
        {
            printf("GTT: GConf: Sync Failed\n");
        }
    }
}

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

void gtt_restore_reports_menu(GnomeApp *app)
{
    int i, num;
    char s[120], *p;
    GnomeUIInfo *reports_menu;
    GConfClient *client;

    client = gconf_client_get_default();

    /* Read in the user-defined report locations */
    num = GETINT("/Misc/NumReports", 0);
    reports_menu = g_new0(GnomeUIInfo, num + 1);

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

        plg = gtt_plugin_new(name, path);
        plg->tooltip = g_strdup(tip);
        plg->last_url = g_strdup(url);

        *p = 0;
        gtt_restore_gnomeui_from_gconf(client, s, &reports_menu[i]);

        /* fixup */
        reports_menu[i].user_data = plg;
    }
    reports_menu[i].type = GNOME_APP_UI_ENDOFINFO;

    gtt_set_reports_menu(app, reports_menu);
}

/* ======================================================= */

void gtt_gconf_load(void)
{
    int i, num;
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

    /* Get last running project */
    cur_proj_id = GETINT("/Misc/CurrProject", -1);

    config_idle_timeout = GETINT("/Misc/IdleTimeout", 300);
    config_no_project_timeout = GETINT("/Misc/NoProjectTimeout", 300);
    config_autosave_period = GETINT("/Misc/AutosavePeriod", 60);
    config_daystart_offset = GETINT("/Misc/DayStartOffset", 0);
    config_weekstart_offset = GETINT("/Misc/WeekStartOffset", 0);

    prefs_update_projects_view();

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
    gtt_restore_reports_menu(GNOME_APP(app_window));

    run_timer = GETINT("/Misc/TimerRunning", 0);
    /* Use string for time, to avoid unsigned-long problems */
    last_timer = (time_t) atol(GETSTR("/Misc/LastTimer", "-1"));
}

/* =========================== END OF FILE ========================= */
