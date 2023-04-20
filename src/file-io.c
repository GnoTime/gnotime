/*   Config file input/output handling for GnoTime
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
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

#include <errno.h>
#include <glib.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "cur-proj.h"
#include "err-throw.h"
#include "file-io.h"
#include "gconf-io.h"
#include "gtt.h"
#include "menus.h"
#include "plug-in.h"
#include "prefs.h"
#include "proj.h"
#include "proj_p.h"
#include "timer.h"
#include "toolbar.h"

#ifdef USE_GTT_DEBUG_FILE
#define GTT_CONF "/gtt-DEBUG"
#else /* not DEBUG */
#define GTT_CONF "/" GTT_APP_NAME
#endif /* not DEBUG */

static const char *gtt_config_filepath = NULL;

int cur_proj_id = -1;
int run_timer = FALSE;
time_t last_timer = -1;
extern char *first_proj_title; /* command line flag */
int save_count = 0;

/* ============================================================= */
/* Configuration file I/O routines:
 * Note that this file supports reading from several old, 'obsolete' config file formats that
 * GTT has used over the years. It supports these reads so that users are not left out in the
 * cold when upgrading from old versions of GTT.  All 'saves' are in the new file format
 * (currently GSettings).
 *
 * 1) First try reading settings from GSettings
 * 2) Next try GConf
 * 3) Next try Gnome 2 Gnome-Config files in ~/.gnome2/GnoTime
 * 4) Last try Gnome 1 Gnome-Config files in ~/.gnome/gtt
 *
 * Note that some of the older config files also contained project
 * data in them.  The newer versions stored project data seperately
 * from the app config data.
 */

/* ======================================================= */

#define GET_INT(str)             \
    ({                           \
        strcpy(p, str);          \
        gnome_config_get_int(s); \
    })

#define GET_BOOL(str)             \
    ({                            \
        strcpy(p, str);           \
        gnome_config_get_bool(s); \
    })

#define GET_STR(str)                \
    ({                              \
        strcpy(p, str);             \
        gnome_config_get_string(s); \
    })

static void gtt_load_gnome_config(const char *prefix)
{
    char *s, *p;
    int prefix_len;
    int i, num;
    int _n, _c, _j, _p, _t, _o, _h, _e;

#define TOKLEN 120
    prefix_len = 0;
    if (prefix)
        prefix_len = strlen(prefix);
    s = g_new(char, prefix_len + TOKLEN);
    if (!s)
        return;
    s[0] = 0;
    if (prefix)
        strcpy(s, prefix);
    p = &s[prefix_len];

    /* If already running, and we are over-loading a new file,
     * then save the currently running project, and try to set it
     * running again ... */
    if (gtt_project_get_title(cur_proj) && (!first_proj_title))
    {
        /* we need to strdup because title is freed when
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

    /* get last running project */
    cur_proj_id = GET_INT("/Misc/CurrProject=-1");

    config_idle_timeout = GET_INT("/Misc/IdleTimeout=300");
    config_autosave_period = GET_INT("/Misc/AutosavePeriod=60");

    /* Reset the main window width and height to the values
     * last stored in the config file.  Note that if the user
     * specified command-line flags, then the command line
     * over-rides the config file. */
    if (!geom_place_override)
    {
        int x, y;
        x = GET_INT("/Geometry/X=10");
        y = GET_INT("/Geometry/Y=10");
        gtk_widget_set_uposition(GTK_WIDGET(app_window), x, y);
    }
    if (!geom_size_override)
    {
        int w, h;
        w = GET_INT("/Geometry/Width=442");
        h = GET_INT("/Geometry/Height=272");

        gtk_window_set_default_size(GTK_WINDOW(app_window), w, h);
    }

    {
        int vp, hp;
        vp = GET_INT("/Geometry/VPaned=250");
        hp = GET_INT("/Geometry/HPaned=220");
        notes_area_set_pane_sizes(global_na, vp, hp);
    }

    config_show_secs = GET_BOOL("/Display/ShowSecs=false");
    config_show_clist_titles = GET_BOOL("/Display/ShowTableHeader=false");
    config_show_subprojects = GET_BOOL("/Display/ShowSubProjects=true");
    config_show_statusbar = GET_BOOL("/Display/ShowStatusbar=true");

    config_show_title_ever = GET_BOOL("/Display/ShowTimeEver=true");
    config_show_title_day = GET_BOOL("/Display/ShowTimeDay=true");
    config_show_title_yesterday = GET_BOOL("/Display/ShowTimeYesterday=false");
    config_show_title_week = GET_BOOL("/Display/ShowTimeWeek=false");
    config_show_title_lastweek = GET_BOOL("/Display/ShowTimeLastWeek=false");
    config_show_title_month = GET_BOOL("/Display/ShowTimeMonth=false");
    config_show_title_year = GET_BOOL("/Display/ShowTimeYear=false");
    config_show_title_current = GET_BOOL("/Display/ShowTimeCurrent=false");
    config_show_title_desc = GET_BOOL("/Display/ShowDesc=true");
    config_show_title_task = GET_BOOL("/Display/ShowTask=true");
    config_show_title_estimated_start = GET_BOOL("/Display/ShowEstimatedStart=false");
    config_show_title_estimated_end = GET_BOOL("/Display/ShowEstimatedEnd=false");
    config_show_title_due_date = GET_BOOL("/Display/ShowDueDate=false");
    config_show_title_sizing = GET_BOOL("/Display/ShowSizing=false");
    config_show_title_percent_complete = GET_BOOL("/Display/ShowPercentComplete=false");
    config_show_title_urgency = GET_BOOL("/Display/ShowUrgency=true");
    config_show_title_importance = GET_BOOL("/Display/ShowImportance=true");
    config_show_title_status = GET_BOOL("/Display/ShowStatus=false");
    prefs_update_projects_view();

    /* ------------ */
    config_show_tb_tips = GET_BOOL("/Toolbar/ShowTips=true");
    config_show_tb_new = GET_BOOL("/Toolbar/ShowNew=true");
    config_show_tb_ccp = GET_BOOL("/Toolbar/ShowCCP=false");
    config_show_tb_journal = GET_BOOL("/Toolbar/ShowJournal=true");
    config_show_tb_prop = GET_BOOL("/Toolbar/ShowProp=true");
    config_show_tb_timer = GET_BOOL("/Toolbar/ShowTimer=true");
    config_show_tb_pref = GET_BOOL("/Toolbar/ShowPref=false");
    config_show_tb_help = GET_BOOL("/Toolbar/ShowHelp=true");
    config_show_tb_exit = GET_BOOL("/Toolbar/ShowExit=true");

    /* ------------ */
    config_shell_start = GET_STR("/Actions/StartCommand=echo start id=%D "
                                 "\\\"%t\\\"-\\\"%d\\\" %T  %H-%M-%S hours=%h min=%m secs=%s");
    config_shell_stop = GET_STR("/Actions/StopCommand=echo stop id=%D \\\"%t\\\"-\\\"%d\\\" "
                                "%T  %H-%M-%S hours=%h min=%m secs=%s");

    /* ------------ */
    config_logfile_use = GET_BOOL("/LogFile/Use=false");
    config_logfile_name = GET_STR("/LogFile/Filename");
    config_logfile_start = GET_STR("/LogFile/Entry");
    if (!config_logfile_start)
        config_logfile_start = g_strdup(_("project %t started"));
    config_logfile_stop = GET_STR("/LogFile/EntryStop");
    if (!config_logfile_stop)
        config_logfile_stop = g_strdup(_("stopped project %t"));
    config_logfile_min_secs = GET_INT("/LogFile/MinSecs");

    /* ------------ */
    save_count = GET_INT("/Data/SaveCount=0");
    config_data_url = GET_STR("/Data/URL=" XML_DATA_FILENAME);
    if (NULL == config_data_url)
    {
        config_data_url = XML_DATA_FILENAME;
    }

    /* ------------ */
    num = 0;
    for (i = 0; -1 < num; i++)
    {
        g_snprintf(p, TOKLEN, "/CList/ColumnWidth%d=-1", i);
        num = gnome_config_get_int(s);
        if (-1 < num)
        {
            //			ctree_set_col_width (global_ptw, i, num);
        }
    }

    /* Read in the user-defined report locations */
    num = GET_INT("/Misc/NumReports=0");
    if (0 < num)
    {
        GnomeUIInfo *reports_menu;
        reports_menu = g_new0(GnomeUIInfo, num + 1);
        for (i = num - 1; i >= 0; i--)
        {
            GttPlugin *plg;
            char *name, *path, *tip;
            g_snprintf(p, TOKLEN, "/Report%d/Name", i);
            name = gnome_config_get_string(s);
            g_snprintf(p, TOKLEN, "/Report%d/Path", i);
            path = gnome_config_get_string(s);
            g_snprintf(p, TOKLEN, "/Report%d/Tooltip", i);
            tip = gnome_config_get_string(s);
            plg = gtt_plugin_new(name, path);
            plg->tooltip = g_strdup(tip);

            /* fixup */
            reports_menu[i].type = GNOME_APP_UI_ITEM;
            reports_menu[i].user_data = plg;
            reports_menu[i].label = name;
            reports_menu[i].hint = tip;
            reports_menu[i].unused_data = NULL;
            reports_menu[i].pixmap_type = GNOME_APP_PIXMAP_STOCK;
            reports_menu[i].pixmap_info = GNOME_STOCK_BLANK;
            reports_menu[i].accelerator_key = 0;
            reports_menu[i].ac_mods = (GdkModifierType) 0;
        }
        reports_menu[num].type = GNOME_APP_UI_ENDOFINFO;
        gtt_set_reports_menu(GNOME_APP(app_window), reports_menu);
    }

    /* The old-style config file also contained project data
     * in it. Read this data, if present.  The new config file
     * format has num-projects set to -1.
     */
    run_timer = GET_INT("/Misc/TimerRunning=0");
    last_timer = atol(GET_STR("/Misc/LastTimer=-1"));
    num = GET_INT("/Misc/NumProjects=0");
    if (0 < num)
    {
        for (i = 0; i < num; i++)
        {
            GttProject *proj;
            time_t ever_secs, day_secs;

            proj = gtt_project_new();
            gtt_project_list_append(master_list, proj);
            g_snprintf(p, TOKLEN, "/Project%d/Title", i);
            gtt_project_set_title(proj, gnome_config_get_string(s));

            /* Match the last running project */
            if (i == cur_proj_id)
            {
                cur_proj_set(proj);
            }

            g_snprintf(p, TOKLEN, "/Project%d/Desc", i);
            gtt_project_set_desc(proj, gnome_config_get_string(s));
            g_snprintf(p, TOKLEN, "/Project%d/SecsEver=0", i);
            ever_secs = gnome_config_get_int(s);
            g_snprintf(p, TOKLEN, "/Project%d/SecsDay=0", i);
            day_secs = gnome_config_get_int(s);
            gtt_project_compat_set_secs(proj, ever_secs, day_secs, last_timer);
        }
        gtt_project_list_compute_secs();
    }

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

    g_free(s);
}

/* ======================================================= */

void gtt_load_config(void)
{
    const char *h;
    char *s;

    // Check if GSettings has been written to at least once and use it if so (this check is
    // conducted to allow the loading of attributes from previous configuration systems)
    if (FALSE == gtt_gsettings_initial_access())
    {
        gtt_gsettings_load();
        gtt_config_filepath = NULL;
        return;
    }

    /* Check for gconf2, and use that if it exists */
    if (gtt_gconf_exists())
    {
        gtt_gconf_load();
        gtt_config_filepath = NULL;
        return;
    }

    /* gnotime breifly used the gnome2 gnome_config file */
    if (gnome_config_has_section(GTT_CONF "/Misc"))
    {
        printf("GTT: Info: Importing ~/.gnome2/" GTT_CONF " file\n");
        gtt_load_gnome_config(GTT_CONF);
        gtt_config_filepath = gnome_config_get_real_path(GTT_CONF);

        /* The data file will be in the same directory ...
         * so prune filename to get the directory */
        char *p = strrchr(gtt_config_filepath, '/');
        if (p)
            *p = 0x0;
        return;
    }

    /* Look for a gnome-1.4 era gnome_config file */
    h = g_get_home_dir();
    s = g_new(char, strlen(h) + 120);
    strcpy(s, "=");
    strcat(s, h);
    strcat(s, "/.gnome/gtt=/Misc");
    if (gnome_config_has_section(s))
    {
        strcpy(s, "=");
        strcat(s, h);
        strcat(s, "/.gnome/gtt=");
        printf("GTT: Info: Importing ~/.gnome/gtt file\n");
        gtt_load_gnome_config(s);
        strcpy(s, h);
        strcat(s, "/.gnome");
        gtt_config_filepath = s;
        return;
    }
    g_free(s);

    config_data_url = XML_DATA_FILENAME;
}

/* ======================================================= */

void gtt_post_data_config(void)
{
    /* Assume we've already read the XML data, and just
     * set the current project */
    cur_proj_set(gtt_project_locate_from_id(cur_proj_id));

    /* Over-ride the current project based on the
     * command-line setting */
    if (first_proj_title)
    {
        GList *node;
        node = gtt_project_list_get_list(master_list);
        for (; node; node = node->next)
        {
            GttProject *prj = node->data;
            if (!gtt_project_get_title(prj))
                continue;

            /* set project based on command line */
            if (0 == strcmp(gtt_project_get_title(prj), first_proj_title))
            {
                cur_proj_set(prj);
                break;
            }
        }
    }

    /* FIXME: this is a mem leak, depending on usage in main.c */
    first_proj_title = NULL;

    /* reset the clocks, if needed */
    if (0 < last_timer)
    {
        set_last_reset(last_timer);
        zero_daily_counters(NULL);
    }

    /* if a project is running, then set it running again,
     * otherwise be sure to stop the clock. */
    if (FALSE == run_timer)
    {
        cur_proj_set(NULL);
    }
}

void gtt_post_ctree_config(void)
{
    gchar *xpn = NULL;

    /* Assume the ctree has been set up.  Now punch in the final
     * bit of ctree state.
     */

    /* Restore the expander state */
    if (FALSE == gtt_gsettings_initial_access())
    {
        xpn = gtt_gsettings_get_expander();
    }
    else if (gtt_gconf_exists())
    {
        xpn = gtt_gconf_get_expander();
    }
    else
    {
        xpn = gnome_config_get_string(GTT_CONF "/Display/ExpanderState");
    }
    if (xpn)
    {
        gtt_projects_tree_set_expander_state(projects_tree, xpn);
    }
}

/* ======================================================= */
/* Save only the GUI configuration info, not the actual data */

void gtt_save_config(void)
{
    gtt_gsettings_save();
}

/* ======================================================= */

const char *gtt_get_config_filepath(void)
{
    return gtt_config_filepath;
}

/* =========================== END OF FILE ========================= */
