/*   file input/output handling for GTimeTracker - a time tracker
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

#include <errno.h>
#include <glib.h>
#include <gnome.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
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
#define GTT_CONF "/gtt-DEBUG"
#else /* not DEBUG */
#define GTT_CONF "/" GTT_APP_NAME
#endif /* not DEBUG */


/* FIXME: we should not extern plist; but for now its ok */
extern GList * plist;


static int cur_proj_id = -1;
static int run_timer = FALSE;
static time_t last_timer = -1;
extern char *first_proj_title;	/* command line flag */
int save_count = 0;

/* ============================================================= */
/* file I/O routines */

/* RC_NAME is old, depricated; stays here for backwards compat. */
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
	int _n, _c, _p, _t, _o, _h, _e;

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
			/* start project command */
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			if (config_shell_start) g_free(config_shell_start);
			config_shell_start = g_strdup(&s[2]);
		} else if (s[0] == 'n') {
			/* stop project command */
			while (s[strlen(s) - 1] == '\n')
				s[strlen(s) - 1] = 0;
			if (config_shell_stop) g_free(config_shell_stop);
			config_shell_stop = g_strdup(&s[2]);
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
	int _n, _c, _j, _p, _t, _o, _h, _e;
	gboolean got_default = FALSE;

	/* The old file type doesn't have numprojets in it */
	gnome_config_get_int_with_default(GTT_CONF"/Misc/NumProjects=0", &got_default);
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
	_c = config_show_tb_ccp;
	_j = config_show_tb_journal;
	_p = config_show_tb_prop;
	_t = config_show_tb_timer;
	_o = config_show_tb_pref;
	_h = config_show_tb_help;
	_e = config_show_tb_exit;

	/* get last running project */
   cur_proj_id = gnome_config_get_int(GTT_CONF"/Misc/CurrProject=-1");

   config_idle_timeout = gnome_config_get_int(GTT_CONF"/Misc/IdleTimeout=300");
   config_autosave_period = gnome_config_get_int(GTT_CONF"/Misc/AutosavePeriod=60");

	/* Reset the main window width and height to the values 
	 * last stored in the config file.  Note that if the user 
	 * specified command-line flags, then the command line 
	 * over-rides the config file. */
	if (!geom_place_override) 
	{
		int x, y;
		x = gnome_config_get_int(GTT_CONF"/Geometry/X=10");
		y = gnome_config_get_int(GTT_CONF"/Geometry/Y=10");
		gtk_widget_set_uposition(GTK_WIDGET(app_window), x, y);
	}
	if (!geom_size_override)
	{
		int w, h;
		w = gnome_config_get_int(GTT_CONF"/Geometry/Width=442");
		h = gnome_config_get_int(GTT_CONF"/Geometry/Height=272");

		gtk_window_set_default_size(GTK_WINDOW(app_window), w, h);
	}

	{
		int vp, hp;
		vp = gnome_config_get_int(GTT_CONF"/Geometry/VPaned=250");
		hp = gnome_config_get_int(GTT_CONF"/Geometry/HPaned=220");
		notes_area_set_pane_sizes (global_na, vp, hp);
	}

	config_show_secs = gnome_config_get_bool(GTT_CONF"/Display/ShowSecs=false");
	config_show_clist_titles = gnome_config_get_bool(GTT_CONF"/Display/ShowTableHeader=false");
	config_show_subprojects = gnome_config_get_bool(GTT_CONF"/Display/ShowSubProjects=true");
	config_show_statusbar = gnome_config_get_bool(GTT_CONF"/Display/ShowStatusbar=true");

	config_show_title_ever = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeEver=true");
	config_show_title_day = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeDay=true");
	config_show_title_week = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeWeek=false");
	config_show_title_month = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeMonth=false");
	config_show_title_year = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeYear=false");
	config_show_title_current = gnome_config_get_bool(GTT_CONF"/Display/ShowTimeCurrent=false");
	config_show_title_desc = gnome_config_get_bool(GTT_CONF"/Display/ShowDesc=true");
	config_show_title_task = gnome_config_get_bool(GTT_CONF"/Display/ShowTask=true");
	config_show_title_estimated_start = gnome_config_get_bool(GTT_CONF"/Display/ShowEstimatedStart=false");
	config_show_title_estimated_end = gnome_config_get_bool(GTT_CONF"/Display/ShowEstimatedEnd=false");
	config_show_title_due_date = gnome_config_get_bool(GTT_CONF"/Display/ShowDueDate=false");
	config_show_title_sizing = gnome_config_get_bool(GTT_CONF"/Display/ShowSizing=false");
	config_show_title_percent_complete = gnome_config_get_bool(GTT_CONF"/Display/ShowPercentComplete=false");
	config_show_title_urgency = gnome_config_get_bool(GTT_CONF"/Display/ShowUrgency=true");
	config_show_title_importance = gnome_config_get_bool(GTT_CONF"/Display/ShowImportance=true");
	config_show_title_status = gnome_config_get_bool(GTT_CONF"/Display/ShowStatus=false");
	ctree_update_column_visibility (global_ptw);


	/* ------------ */
	config_show_tb_icons = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowIcons=true");
	config_show_tb_texts = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowTexts=true");
	config_show_tb_tips = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowTips=true");
	config_show_tb_new = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowNew=true");
	config_show_tb_ccp = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowCCP=false");
	config_show_tb_journal = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowJournal=true");
	config_show_tb_prop = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowProp=true");
	config_show_tb_timer = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowTimer=true");
	config_show_tb_pref = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowPref=false");
	config_show_tb_help = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowHelp=true");
	config_show_tb_exit = gnome_config_get_bool(GTT_CONF"/Toolbar/ShowExit=true");

	/* ------------ */
	config_shell_start = gnome_config_get_string(GTT_CONF"/Actions/StartCommand=echo start id=%D \\\"%t\\\"-\\\"%d\\\" %T  %H-%M-%S hours=%h min=%m secs=%s");
	config_shell_stop = gnome_config_get_string(GTT_CONF"/Actions/StopCommand=echo stop id=%D \\\"%t\\\"-\\\"%d\\\" %T  %H-%M-%S hours=%h min=%m secs=%s");

	/* ------------ */
	config_logfile_use = gnome_config_get_bool(GTT_CONF"/LogFile/Use=false");
	config_logfile_name = gnome_config_get_string(GTT_CONF"/LogFile/Filename");
	config_logfile_start = gnome_config_get_string(GTT_CONF"/LogFile/Entry");
	if (!config_logfile_start)
		config_logfile_start = g_strdup(_("project %t started"));
	config_logfile_stop = gnome_config_get_string(GTT_CONF"/LogFile/EntryStop");
	if (!config_logfile_stop)
		config_logfile_stop = g_strdup(_("stopped project %t"));
	config_logfile_min_secs = gnome_config_get_int(GTT_CONF"/LogFile/MinSecs");

	/* ------------ */
	save_count = gnome_config_get_int(GTT_CONF"/Data/SaveCount=0");
	config_data_url = gnome_config_get_string(GTT_CONF"/Data/URL=" XML_DATA_FILENAME);
	if (NULL == config_data_url) 
	{
		config_data_url = XML_DATA_FILENAME;
	}

	/* ------------ */
	num = 0;
	for (i = 0; -1 < num; i++) {
		g_snprintf(s, sizeof (s), GTT_CONF"/CList/ColumnWidth%d=-1", i);
		num = gnome_config_get_int(s);
		if (-1 < num) 
		{
			ctree_set_col_width (global_ptw, i, num);
		}
	}

	/* Read in the user-defined report locations */
	num = gnome_config_get_int(GTT_CONF"/Misc/NumReports=0");
	if (0 < num)
	{
		for (i = num-1; i >= 0 ; i--) 
		{
			GttPlugin *plg;
			char * name, *path, *tip;
			g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Name", i);
			name = gnome_config_get_string(s);
			g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Path", i);
			path = gnome_config_get_string(s);
			g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Tooltip", i);
			tip = gnome_config_get_string(s);
			plg = gtt_plugin_new (name, path);
			plg->tooltip = g_strdup (tip);
		}
	} 

	/* The old-style config file also contained project data
	 * in it. Read this data, if present.  The new config file
	 * format has num-projects set to -1.
	 */
	run_timer = gnome_config_get_int(GTT_CONF"/Misc/TimerRunning=0");
	last_timer = atol(gnome_config_get_string(GTT_CONF"/Misc/LastTimer=-1"));
	num = gnome_config_get_int(GTT_CONF"/Misc/NumProjects=0");
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
			g_snprintf(s, sizeof (s), GTT_CONF"/Project%d/Title", i);
			gtt_project_set_title(proj, gnome_config_get_string(s));
	
			/* Match the last running project */
			if (i == cur_proj_id) {
				cur_proj_set(proj);
			}
	
			g_snprintf(s, sizeof (s), GTT_CONF"/Project%d/Desc", i);
			gtt_project_set_desc(proj, gnome_config_get_string(s));
			g_snprintf(s, sizeof (s), GTT_CONF"/Project%d/SecsEver=0", i);
			ever_secs = gnome_config_get_int(s);
			g_snprintf(s, sizeof (s), GTT_CONF"/Project%d/SecsDay=0", i);
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
	
void 
gtt_post_ctree_config (void)
{
	char * xpn;

	/* Assume the ctree has been set up.  Now punch in the final
	 * bit of ctree state.
	 */

	/* Restore the expander state */
	xpn = gnome_config_get_string(GTT_CONF"/Display/ExpanderState");
	ctree_set_expander_state (global_ptw, xpn);
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
	const char *xpn;

	old_num = gnome_config_get_int(GTT_CONF"/Misc/NumProjects=0");

	/* ------------- */
	/* save the window location and size */
	gdk_window_get_origin(app_window->window, &x, &y);
	gdk_window_get_size(app_window->window, &w, &h);
	gnome_config_set_int(GTT_CONF"/Geometry/Width", w);
	gnome_config_set_int(GTT_CONF"/Geometry/Height", h);
	gnome_config_set_int(GTT_CONF"/Geometry/X", x);
	gnome_config_set_int(GTT_CONF"/Geometry/Y", y);

	{
		int vp, hp;
		notes_area_get_pane_sizes (global_na, &vp, &hp);
		gnome_config_set_int(GTT_CONF"/Geometry/VPaned", vp);
		gnome_config_set_int(GTT_CONF"/Geometry/HPaned", hp);
	}
	/* ------------- */
	/* save the configure dialog values */
	gnome_config_set_bool(GTT_CONF"/Display/ShowSecs", config_show_secs);
	gnome_config_set_bool(GTT_CONF"/Display/ShowStatusbar", config_show_statusbar);
	gnome_config_set_bool(GTT_CONF"/Display/ShowSubProjects", config_show_subprojects);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTableHeader", config_show_clist_titles);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeCurrent", config_show_title_current);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeDay", config_show_title_day);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeWeek", config_show_title_week);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeMonth", config_show_title_month);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeYear", config_show_title_year);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTimeEver", config_show_title_ever);
	gnome_config_set_bool(GTT_CONF"/Display/ShowDesc", config_show_title_desc);
	gnome_config_set_bool(GTT_CONF"/Display/ShowTask", config_show_title_task);
	gnome_config_set_bool(GTT_CONF"/Display/ShowEstimatedStart", config_show_title_estimated_start);
	gnome_config_set_bool(GTT_CONF"/Display/ShowEstimatedEnd", config_show_title_estimated_end);
	gnome_config_set_bool(GTT_CONF"/Display/ShowDueDate", config_show_title_due_date);
	gnome_config_set_bool(GTT_CONF"/Display/ShowSizing", config_show_title_sizing);
	gnome_config_set_bool(GTT_CONF"/Display/ShowPercentComplete", config_show_title_percent_complete);
	gnome_config_set_bool(GTT_CONF"/Display/ShowUrgency", config_show_title_urgency);
	gnome_config_set_bool(GTT_CONF"/Display/ShowImportance", config_show_title_importance);
	gnome_config_set_bool(GTT_CONF"/Display/ShowStatus", config_show_title_status);

	xpn = ctree_get_expander_state (global_ptw);
	gnome_config_set_string(GTT_CONF"/Display/ExpanderState", xpn);

	/* ------------- */
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowIcons", config_show_tb_icons);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowTexts", config_show_tb_texts);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowTips", config_show_tb_tips);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowNew", config_show_tb_new);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowCCP", config_show_tb_ccp);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowJournal", config_show_tb_journal);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowProp", config_show_tb_prop);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowTimer", config_show_tb_timer);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowPref", config_show_tb_pref);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowHelp", config_show_tb_help);
	gnome_config_set_bool(GTT_CONF"/Toolbar/ShowExit", config_show_tb_exit);

	/* ------------- */
	if (config_shell_start)
		gnome_config_set_string(GTT_CONF"/Actions/StartCommand", config_shell_start);
	else
		gnome_config_clean_key(GTT_CONF"/Actions/StartCommand");
	if (config_shell_stop)
		gnome_config_set_string(GTT_CONF"/Actions/StopCommand", config_shell_stop);
	else
		gnome_config_clean_key(GTT_CONF"/Actions/StopCommand");

	/* ------------- */
	gnome_config_set_bool(GTT_CONF"/LogFile/Use", config_logfile_use);
	if (config_logfile_name)
		gnome_config_set_string(GTT_CONF"/LogFile/Filename", config_logfile_name);
	else
		gnome_config_clean_key(GTT_CONF"/LogFile/Filename");
	if (config_logfile_start)
		gnome_config_set_string(GTT_CONF"/LogFile/Entry", config_logfile_start);
	else
		gnome_config_set_string(GTT_CONF"/LogFile/Entry", "");
	if (config_logfile_stop)
		gnome_config_set_string(GTT_CONF"/LogFile/EntryStop",
					config_logfile_stop);
	else
		gnome_config_set_string(GTT_CONF"/LogFile/EntryStop", "");
	gnome_config_set_int(GTT_CONF"/LogFile/MinSecs", config_logfile_min_secs);

	/* ------------- */
	gnome_config_set_string(GTT_CONF"/Data/URL", config_data_url);
	gnome_config_set_int(GTT_CONF"/Data/SaveCount", save_count);

	/* ------------- */
	w = 0;
	for (i = 0; -1< w; i++) 
	{
		g_snprintf(s, sizeof (s), GTT_CONF"/CList/ColumnWidth%d", i);
		w = ctree_get_col_width (global_ptw, i);
		if (0 > w) break;
		gnome_config_set_int(s, w);
	}

	/* ------------- */
	g_snprintf(s, sizeof (s), "%ld", time(0));
	gnome_config_set_string(GTT_CONF"/Misc/LastTimer", s);
	gnome_config_set_int(GTT_CONF"/Misc/IdleTimeout", config_idle_timeout);
	gnome_config_set_int(GTT_CONF"/Misc/AutosavePeriod", config_autosave_period);
	gnome_config_set_int(GTT_CONF"/Misc/TimerRunning", timer_is_running());
	gnome_config_set_int(GTT_CONF"/Misc/CurrProject", gtt_project_get_id (cur_proj));
	gnome_config_set_int(GTT_CONF"/Misc/NumProjects", -1);

	/* Delete all project information (this file shouldn't have any,
	 * unless its very very old.  Projects are now stored in xml file. */
	for (i=0; i < old_num; i++) {
		g_snprintf(s, sizeof (s), GTT_CONF"/Project%d", i);
		gnome_config_clean_section(s);
	}

	/* write out the customer report info */
	i = 0;
	for (node = gtt_plugin_get_list(); node; node=node->next)
	{
		GttPlugin *plg = node->data;
	       	g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Name", i);
		gnome_config_set_string(s, plg->name);
	       	g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Path", i);
		gnome_config_set_string(s, plg->path);
	       	g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Tooltip", i);
		gnome_config_set_string(s, plg->tooltip);
		i++;
	}
	gnome_config_set_int(GTT_CONF"/Misc/NumReports", i);

	gnome_config_sync();
}

/* ======================================================= */

const char * 
gtt_get_config_filepath (void)
{
	return gnome_config_get_real_path (GTT_CONF);
}

/* =========================== END OF FILE ========================= */
