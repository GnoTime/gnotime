/*   file input/output handling for GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas
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

#include <errno.h>
#include <glib.h>
#include <gnome.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "cur-proj.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "plug-in.h"
#include "prefs.h"
#include "proj.h"
#include "timer.h"
#include "toolbar.h"

#ifdef DEBUG
#define GTT "/gtt-DEBUG/"
#else /* not DEBUG */
#define GTT "/gtt/"
#endif /* not DEBUG */


/* FIXME: we should not extern this; but for now its ok */
extern GList * plist;

static int cur_proj_id = -1;
static int run_timer = FALSE;
static time_t last_timer = -1;
extern char *first_proj_title;	/* command line flag */

/* ============================================================= */
/* file I/O routines */

/* RC_NAME is old depricated */
#define RC_NAME ".gtimetrackerrc"

static const char *
build_rc_name_old(void)
{
	static char *buf = NULL;

	if (buf != NULL) return buf;
	if (g_getenv("HOME") != NULL) {
		buf = g_concat_dir_and_file (g_getenv ("HOME"), RC_NAME);
	} else {
		buf = g_strdup (RC_NAME);
	}
	return buf;
}



static void
read_tb_sects_old(char *s)
{
	if (s[2] == 'n') {
		config_show_tb_new = (s[5] == 'n');
	} else if (s[2] == 'f') {
		config_show_tb_file = (s[5] == 'n');
	} else if (s[2] == 'c') {
		config_show_tb_ccp = (s[5] == 'n');
	} else if (s[2] == 'p') {
		config_show_tb_prop = (s[5] == 'n');
	} else if (s[2] == 't') {
		config_show_tb_timer = (s[5] == 'n');
	} else if (s[2] == 'o') {
		config_show_tb_pref = (s[5] == 'n');
	} else if (s[2] == 'h') {
		config_show_tb_help = (s[5] == 'n');
	} else if (s[2] == 'e') {
		config_show_tb_exit = (s[5] == 'n');
	}
}


static void
project_list_load_old(const char *fname)
{
	FILE *f;
	const char *realname;
	GList *pl, *t;
	GttProject *proj = NULL;
	char s[1024];
	int i;
	int _n, _f, _c, _p, _t, _o, _h, _e;

	if (fname != NULL)
	{
		realname = fname;
	}
	else
	{
		realname = build_rc_name_old();
	}

	if (NULL == (f = fopen(realname, "rt"))) 
	{
		gtt_err_set_code(GTT_CANT_OPEN_FILE);
#ifdef ENOENT
		if (errno == ENOENT) return;
#endif
		g_warning("could not open %s\n", realname);
		return;
	}
	pl = gtt_get_project_list();
	plist = NULL;

	_n = config_show_tb_new;
	_f = config_show_tb_file;
	_c = config_show_tb_ccp;
	_p = config_show_tb_prop;
	_t = config_show_tb_timer;
	_o = config_show_tb_pref;
	_h = config_show_tb_help;
	_e = config_show_tb_exit;
	errno = 0;
	while ((!feof(f)) && (!errno)) {
		if (!fgets(s, 1023, f)) continue;
		if (s[0] == '#') continue;
		if (s[0] == '\n') continue;
		if (s[0] == ' ') {
			/* desc for last project */
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			gtt_project_set_desc(proj, &s[1]);
		} else if (s[0] == 't') {
			/* last_timer */
			last_timer = (time_t)atol(&s[1]);
		} else if (s[0] == 's') {
			/* show seconds? */
			config_show_secs = (s[3] == 'n');
		} else if (s[0] == 'b') {
			if (s[1] == 'i') {
				/* show icons in the toolbar */
				config_show_tb_icons = (s[4] == 'n');
			} else if (s[1] == 't') {
				/* show text in the toolbar */
				config_show_tb_texts = (s[4] == 'n');
			} else if (s[1] == 'p') {
				/* show tooltips */
				config_show_tb_tips = (s[4] == 'n');
			} else if (s[1] == 'h') {
				/* show clist titles */
				config_show_clist_titles = (s[4] == 'n');
			} else if (s[1] == 's') {
				/* show status bar */
				if (s[4] == 'n') {
					config_show_statusbar = 1;
				} else {
					config_show_statusbar = 0;
				}
			} else if (s[1] == '_') {
				read_tb_sects_old(s);
			}
		} else if (s[0] == 'c') {
			/* switch project command */
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			if (config_command) g_free(config_command);
			config_command = g_strdup(&s[2]);
		} else if (s[0] == 'n') {
			/* no project command */
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			if (config_command_null) g_free(config_command_null);
			config_command_null = g_strdup(&s[2]);
		} else if (s[0] == 'l') {
			if (s[1] == 'u') {
				/* use logfile? */
				config_logfile_use = (s[4] == 'n');
			} else if (s[1] == 'n') {
				/* logfile name */
				while (s[strlen(s) - 1] == '\n')
					s[strlen(s) - 1] = 0;
				if (config_logfile_name) g_free(config_logfile_name);
				config_logfile_name = g_strdup(&s[3]);
			} else if (s[1] == 's') {
				/* minimum time for a project to get logged */
				config_logfile_min_secs = atoi(&s[3]);
			}
		} else if ((s[0] >= '0') && (s[0] <='9')) {
			time_t day_secs, ever_secs;

			/* new project */
			proj = gtt_project_new();
			gtt_project_list_append(proj);
			ever_secs = atol(s);
			for (i = 0; s[i] != ' '; i++) ;
			i++;
			day_secs = atol(&s[i]);
			gtt_project_compat_set_secs (proj, ever_secs, day_secs, last_timer);
			for (; s[i] != ' '; i++) ;
			i++;
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			gtt_project_set_title(proj, &s[i]);
		}
	}
	if ((errno) && (!feof(f))) goto err;
	fclose(f);

	t = plist;
	plist = pl;
	project_list_destroy();
	plist = t;
	gtt_project_list_compute_secs();

	if (config_show_statusbar)
	{
		gtk_widget_show(status_bar);
	}
	else
	{
		gtk_widget_hide(status_bar);
	}

	update_status_bar();
	if ((_n != config_show_tb_new) ||
	    (_f != config_show_tb_file) ||
	    (_c != config_show_tb_ccp) ||
	    (_p != config_show_tb_prop) ||
	    (_t != config_show_tb_timer) ||
	    (_o != config_show_tb_pref) ||
	    (_h != config_show_tb_help) ||
	    (_e != config_show_tb_exit)) {
		update_toolbar_sections();
	}
	return;

err:
	fclose(f);
	gtt_err_set_code (GTT_FILE_CORRUPT);
	g_warning("error reading %s\n", realname);
	project_list_destroy();
	plist = pl;
}


/* ======================================================= */

void
gtt_load_config (const char *fname)
{
	char s[256];
	int i, num;
	int _n, _f, _c, _j, _p, _t, _o, _h, _e;
	gboolean got_default;

	/* The old file type doesn't have numprojets in it */
	gnome_config_get_int_with_default(GTT"Misc/NumProjects=0", &got_default);
	if (got_default) {
		project_list_load_old(fname);
		if (NULL == config_data_url) {
			config_data_url = XML_DATA_FILENAME;
		}
		return;
	}

	/* If already running, and we are over-loading a new file,
	 * then save the currently running project, and try to set it
	 * running again ... */
	if (gtt_project_get_title(cur_proj) && (!first_proj_title)) 
	{
		/* we need to strdup because title is freed when 
		 * the project list is destroyed ... */
		first_proj_title = g_strdup (gtt_project_get_title (cur_proj));
	}

	_n = config_show_tb_new;
	_f = config_show_tb_file;
	_c = config_show_tb_ccp;
	_j = config_show_tb_journal;
	_p = config_show_tb_prop;
	_t = config_show_tb_timer;
	_o = config_show_tb_pref;
	_h = config_show_tb_help;
	_e = config_show_tb_exit;

	/* get last running project */
       	cur_proj_id = gnome_config_get_int(GTT"Misc/CurrProject=-1");

       	config_idle_timeout = gnome_config_get_int(GTT"Misc/IdleTimeout=-1");

	/* Reset the main window width and height to the values 
	 * last stored in the config file.  Note that if the user 
	 * specified command-line flags, then the command line 
	 * over-rides the config file. */
	if (!geom_place_override) 
	{
		int x, y;
		x = gnome_config_get_int(GTT"Geometry/X=10");
		y = gnome_config_get_int(GTT"Geometry/Y=10");
		gtk_widget_set_uposition(GTK_WIDGET(window), x, y);
	}
	if (!geom_size_override)
	{
		int w, h;
		w = gnome_config_get_int(GTT"Geometry/Width=320");
		h = gnome_config_get_int(GTT"Geometry/Height=220");

		gtk_window_set_default_size(GTK_WINDOW(window), w, h);
	}

	config_show_secs = gnome_config_get_bool(GTT"Display/ShowSecs=false");
	config_show_clist_titles = gnome_config_get_bool(GTT"Display/ShowTableHeader=false");
	config_show_subprojects = gnome_config_get_bool(GTT"Display/ShowSubProjects=true");
	config_show_statusbar = gnome_config_get_bool(GTT"Display/ShowStatusbar=true");

	config_show_title_ever = gnome_config_get_bool(GTT"Display/ShowTimeEver=true");
	config_show_title_day = gnome_config_get_bool(GTT"Display/ShowTimeDay=true");
	config_show_title_week = gnome_config_get_bool(GTT"Display/ShowTimeWeek=false");
	config_show_title_month = gnome_config_get_bool(GTT"Display/ShowTimeMonth=false");
	config_show_title_year = gnome_config_get_bool(GTT"Display/ShowTimeYear=false");
	config_show_title_current = gnome_config_get_bool(GTT"Display/ShowTimeCurrent=false");
	config_show_title_desc = gnome_config_get_bool(GTT"Display/ShowDesc=true");
	config_show_title_task = gnome_config_get_bool(GTT"Display/ShowTask=true");
	config_show_title_estimated_start = gnome_config_get_bool(GTT"Display/ShowEstimatedStart=false");
	config_show_title_estimated_end = gnome_config_get_bool(GTT"Display/ShowEstimatedEnd=false");
	config_show_title_due_date = gnome_config_get_bool(GTT"Display/ShowDueDate=false");
	config_show_title_sizing = gnome_config_get_bool(GTT"Display/ShowSizing=false");
	config_show_title_percent_complete = gnome_config_get_bool(GTT"Display/ShowPercentComplete=false");
	config_show_title_urgency = gnome_config_get_bool(GTT"Display/ShowUrgency=true");
	config_show_title_importance = gnome_config_get_bool(GTT"Display/ShowImportance=true");
	config_show_title_status = gnome_config_get_bool(GTT"Display/ShowStatus=false");
	ctree_update_column_visibility (global_ptw);


	/* ------------ */
	config_show_tb_icons = gnome_config_get_bool(GTT"Toolbar/ShowIcons=true");
	config_show_tb_texts = gnome_config_get_bool(GTT"Toolbar/ShowTexts=true");
	config_show_tb_tips = gnome_config_get_bool(GTT"Toolbar/ShowTips=true");
	config_show_tb_new = gnome_config_get_bool(GTT"Toolbar/ShowNew=true");
	config_show_tb_file = gnome_config_get_bool(GTT"Toolbar/ShowFile=false");
	config_show_tb_ccp = gnome_config_get_bool(GTT"Toolbar/ShowCCP=false");
	config_show_tb_journal = gnome_config_get_bool(GTT"Toolbar/ShowJournal=true");
	config_show_tb_prop = gnome_config_get_bool(GTT"Toolbar/ShowProp=true");
	config_show_tb_timer = gnome_config_get_bool(GTT"Toolbar/ShowTimer=true");
	config_show_tb_pref = gnome_config_get_bool(GTT"Toolbar/ShowPref=false");
	config_show_tb_help = gnome_config_get_bool(GTT"Toolbar/ShowHelp=true");
	config_show_tb_exit = gnome_config_get_bool(GTT"Toolbar/ShowExit=true");

	/* ------------ */
	config_command = gnome_config_get_string(GTT"Actions/ProjCommand");
	config_command_null = gnome_config_get_string(GTT"Actions/NullCommand");

	/* ------------ */
	config_logfile_use = gnome_config_get_bool(GTT"LogFile/Use=false");
	config_logfile_name = gnome_config_get_string(GTT"LogFile/Filename");
	config_logfile_start = gnome_config_get_string(GTT"LogFile/Entry");
	if (!config_logfile_start)
		config_logfile_start = g_strdup(_("project %t started"));
	config_logfile_stop = gnome_config_get_string(GTT"LogFile/EntryStop");
	if (!config_logfile_stop)
		config_logfile_stop = g_strdup(_("stopped project %t"));
	config_logfile_min_secs = gnome_config_get_int(GTT"LogFile/MinSecs");

	/* ------------ */
	config_data_url = gnome_config_get_string(GTT"Data/URL=" XML_DATA_FILENAME);
	if (NULL == config_data_url) 
	{
		config_data_url = XML_DATA_FILENAME;
	}

	/* ------------ */
	num = 0;
	for (i = 0; -1 < num; i++) {
		g_snprintf(s, sizeof (s), GTT"CList/ColumnWidth%d=-1", i);
		num = gnome_config_get_int(s);
		if (-1 < num) 
		{
			ctree_set_col_width (global_ptw, i, num);
		}
	}

	/* Read in the user-defined report locations */
	num = gnome_config_get_int(GTT"Misc/NumReports=0");
	if (0 < num)
	{
		for (i = num-1; i >= 0 ; i--) 
		{
			GttPlugin *plg;
			char * name, *path, *tip;
			g_snprintf(s, sizeof (s), GTT"Report%d/Name", i);
			name = gnome_config_get_string(s);
			g_snprintf(s, sizeof (s), GTT"Report%d/Path", i);
			path = gnome_config_get_string(s);
			g_snprintf(s, sizeof (s), GTT"Report%d/Tooltip", i);
			tip = gnome_config_get_string(s);
			plg = gtt_plugin_new (name, path);
			plg->tooltip = g_strdup (tip);
		}
	} 

	/* The old-style config file also contained project data
	 * in it. Read this data, if present.  The new config file
	 * format has num-projects set to -1.
	 */
	run_timer = gnome_config_get_int(GTT"Misc/TimerRunning=0");
	last_timer = atol(gnome_config_get_string(GTT"Misc/LastTimer=-1"));
	num = gnome_config_get_int(GTT"Misc/NumProjects=0");
	if (0 < num)
	{
		/* start with a clean slate */
		project_list_destroy();

		for (i = 0; i < num; i++) 
		{
			GttProject *proj;
			time_t ever_secs, day_secs;

			proj = gtt_project_new();
			gtt_project_list_append(proj);
			g_snprintf(s, sizeof (s), GTT"Project%d/Title", i);
			gtt_project_set_title(proj, gnome_config_get_string(s));
	
			/* Match the last running project */
			if (i == cur_proj_id) {
				cur_proj_set(proj);
			}
	
			g_snprintf(s, sizeof (s), GTT"Project%d/Desc", i);
			gtt_project_set_desc(proj, gnome_config_get_string(s));
			g_snprintf(s, sizeof (s), GTT"Project%d/SecsEver=0", i);
			ever_secs = gnome_config_get_int(s);
			g_snprintf(s, sizeof (s), GTT"Project%d/SecsDay=0", i);
			day_secs = gnome_config_get_int(s);
			gtt_project_compat_set_secs (proj, ever_secs, day_secs, last_timer);
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
	if ((_n != config_show_tb_new) ||
	    (_f != config_show_tb_file) ||
	    (_c != config_show_tb_ccp) ||
	    (_j != config_show_tb_journal) ||
	    (_p != config_show_tb_prop) ||
	    (_t != config_show_tb_timer) ||
	    (_o != config_show_tb_pref) ||
	    (_h != config_show_tb_help) ||
	    (_e != config_show_tb_exit)) 
	{
		update_toolbar_sections();
	}
}

/* ======================================================= */

void 
gtt_post_data_config (void)
{
	/* Assume we've already read the XML data, and just 
	 * set the current project */
	cur_proj_set (gtt_project_locate_from_id (cur_proj_id));

	/* Over-ride the current project based on the 
	 * command-line setting */
	if (first_proj_title)
	{
		GList *node;
		for (node = gtt_get_project_list(); node; node = node->next) 
		{
			GttProject *prj = node->data;
			if (!gtt_project_get_title(prj)) continue;

			/* set project based on command line */
			if (0 == strcmp(gtt_project_get_title(prj), 
					first_proj_title)) 
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
		set_last_reset (last_timer);
		zero_on_rollover (time(0));
	}

	/* if a project is running, then set it running again,
	 * otherwise be sure to stop the clock. */
	if (FALSE == run_timer) 
	{
		cur_proj_set (NULL);
	}
}

/* ======================================================= */
/* save only the GUI configuration info, not the actual data */

void
gtt_save_config(const char *fname)
{
	GList *node;
	char s[120];
	int i, old_num;
	int x, y, w, h;

	old_num = gnome_config_get_int(GTT"Misc/NumProjects=0");

	/* ------------- */
	/* save the window location and size */
	gdk_window_get_origin(window->window, &x, &y);
	gdk_window_get_size(window->window, &w, &h);
	gnome_config_set_int(GTT"Geometry/Width", w);
	gnome_config_set_int(GTT"Geometry/Height", h);
	gnome_config_set_int(GTT"Geometry/X", x);
	gnome_config_set_int(GTT"Geometry/Y", y);

	/* ------------- */
	/* save the configure dialog values */
	gnome_config_set_bool(GTT"Display/ShowSecs", config_show_secs);
	gnome_config_set_bool(GTT"Display/ShowStatusbar", config_show_statusbar);
	gnome_config_set_bool(GTT"Display/ShowSubProjects", config_show_subprojects);
	gnome_config_set_bool(GTT"Display/ShowTableHeader", config_show_clist_titles);
	gnome_config_set_bool(GTT"Display/ShowTimeCurrent", config_show_title_current);
	gnome_config_set_bool(GTT"Display/ShowTimeDay", config_show_title_day);
	gnome_config_set_bool(GTT"Display/ShowTimeWeek", config_show_title_week);
	gnome_config_set_bool(GTT"Display/ShowTimeMonth", config_show_title_month);
	gnome_config_set_bool(GTT"Display/ShowTimeYear", config_show_title_year);
	gnome_config_set_bool(GTT"Display/ShowTimeEver", config_show_title_ever);
	gnome_config_set_bool(GTT"Display/ShowDesc", config_show_title_desc);
	gnome_config_set_bool(GTT"Display/ShowTask", config_show_title_task);
	gnome_config_set_bool(GTT"Display/ShowEstimatedStart", config_show_title_estimated_start);
	gnome_config_set_bool(GTT"Display/ShowEstimatedEnd", config_show_title_estimated_end);
	gnome_config_set_bool(GTT"Display/ShowDueDate", config_show_title_due_date);
	gnome_config_set_bool(GTT"Display/ShowSizing", config_show_title_sizing);
	gnome_config_set_bool(GTT"Display/ShowPercentComplete", config_show_title_percent_complete);
	gnome_config_set_bool(GTT"Display/ShowUrgency", config_show_title_urgency);
	gnome_config_set_bool(GTT"Display/ShowImportance", config_show_title_importance);
	gnome_config_set_bool(GTT"Display/ShowStatus", config_show_title_status);

	/* ------------- */
	gnome_config_set_bool(GTT"Toolbar/ShowIcons", config_show_tb_icons);
	gnome_config_set_bool(GTT"Toolbar/ShowTexts", config_show_tb_texts);
	gnome_config_set_bool(GTT"Toolbar/ShowTips", config_show_tb_tips);
	gnome_config_set_bool(GTT"Toolbar/ShowNew", config_show_tb_new);
	gnome_config_set_bool(GTT"Toolbar/ShowFile", config_show_tb_file);
	gnome_config_set_bool(GTT"Toolbar/ShowCCP", config_show_tb_ccp);
	gnome_config_set_bool(GTT"Toolbar/ShowJournal", config_show_tb_journal);
	gnome_config_set_bool(GTT"Toolbar/ShowProp", config_show_tb_prop);
	gnome_config_set_bool(GTT"Toolbar/ShowTimer", config_show_tb_timer);
	gnome_config_set_bool(GTT"Toolbar/ShowPref", config_show_tb_pref);
	gnome_config_set_bool(GTT"Toolbar/ShowHelp", config_show_tb_help);
	gnome_config_set_bool(GTT"Toolbar/ShowExit", config_show_tb_exit);

	/* ------------- */
	if (config_command)
		gnome_config_set_string(GTT"Actions/ProjCommand", config_command);
	else
		gnome_config_clean_key(GTT"Actions/ProjCommand");
	if (config_command_null)
		gnome_config_set_string(GTT"Actions/NullCommand", config_command_null);
	else
		gnome_config_clean_key(GTT"Actions/NullCommand");

	/* ------------- */
	gnome_config_set_bool(GTT"LogFile/Use", config_logfile_use);
	if (config_logfile_name)
		gnome_config_set_string(GTT"LogFile/Filename", config_logfile_name);
	else
		gnome_config_clean_key(GTT"LogFile/Filename");
	if (config_logfile_start)
		gnome_config_set_string(GTT"LogFile/Entry", config_logfile_start);
	else
		gnome_config_set_string(GTT"LogFile/Entry", "");
	if (config_logfile_stop)
		gnome_config_set_string(GTT"LogFile/EntryStop",
					config_logfile_stop);
	else
		gnome_config_set_string(GTT"LogFile/EntryStop", "");
	gnome_config_set_int(GTT"LogFile/MinSecs", config_logfile_min_secs);

	/* ------------- */
	gnome_config_set_string(GTT"Data/URL", config_data_url);

	/* ------------- */
	w = 0;
	for (i = 0; -1< w; i++) 
	{
		g_snprintf(s, sizeof (s), GTT"CList/ColumnWidth%d", i);
		w = ctree_get_col_width (global_ptw, i);
		if (0 > w) break;
		gnome_config_set_int(s, w);
	}

	/* ------------- */
	g_snprintf(s, sizeof (s), "%ld", time(0));
	gnome_config_set_string(GTT"Misc/LastTimer", s);
	gnome_config_set_int(GTT"Misc/IdleTimeout", config_idle_timeout);
	gnome_config_set_int(GTT"Misc/TimerRunning", timer_is_running());
	gnome_config_set_int(GTT"Misc/CurrProject", gtt_project_get_id (cur_proj));
	gnome_config_set_int(GTT"Misc/NumProjects", -1);

	/* delete all project information */
	for (i=0; i < old_num; i++) {
		g_snprintf(s, sizeof (s), GTT"Project%d", i);
		gnome_config_clean_section(s);
	}

	/* write out the customer report info */
	i = 0;
	for (node = gtt_plugin_get_list(); node; node=node->next)
	{
		GttPlugin *plg = node->data;
	       	g_snprintf(s, sizeof (s), GTT"Report%d/Name", i);
		gnome_config_set_string(s, plg->name);
	       	g_snprintf(s, sizeof (s), GTT"Report%d/Path", i);
		gnome_config_set_string(s, plg->path);
	       	g_snprintf(s, sizeof (s), GTT"Report%d/Tooltip", i);
		gnome_config_set_string(s, plg->tooltip);
		i++;
	}
	gnome_config_set_int(GTT"Misc/NumReports", i);

	gnome_config_sync();
}

/* ======================================================= */

const char * 
gtt_get_config_filepath (void)
{
	return gnome_config_get_real_path (GTT);
}

/* ======================================================= */
/* project list export */

static char *
get_time (int secs)
{
	/* Translators: This is a "time format", that is
	 * format on how to print the elapsed time with
	 * hours:minutes:seconds. */
	return g_strdup_printf (_("%d:%02d:%02d"),
				secs / (60*60),
				(secs / 60) % 60,
				secs % 60);
}

gboolean
project_list_export (const char *fname)
{
	FILE *fp;
	GList *node;

	fp = fopen (fname, "w");
	if (fp == NULL)
		return FALSE;

	/* Translators: this is the header of a table separated file,
	 * it should really be all ASCII, or at least not multibyte,
	 * I don't think most spreadsheets would handle that well. */
	fprintf (fp, "Title\tDescription\tTotal time\tTime today\n");

	for (node = gtt_get_project_list(); node; node = node->next) 
	{
		GttProject *prj = node->data;
		char *total_time, *time_today;
		if (!gtt_project_get_title(prj)) continue;
		total_time = get_time (gtt_project_total_secs_ever(prj));
		time_today = get_time (gtt_project_total_secs_day(prj));
		fprintf (fp, "%s\t%s\t%s\t%s\n",
			 gtt_sure_string (gtt_project_get_title(prj)),
			 gtt_sure_string (gtt_project_get_desc(prj)),
			 total_time,
			 time_today);
		g_free (total_time);
		g_free (time_today);
	}

	fclose (fp);

	return TRUE;
}

/* =========================== END OF FILE ========================= */
