/*   GUI dialog for global application preferences for GTimeTracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
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

#include <glade/glade.h>
#include <gnome.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "dialog.h"
#include "gtt.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"
#include "util.h"


/* globals */
int config_show_secs = 0;
int config_show_statusbar = 1;
int config_show_clist_titles = 1;
int config_show_subprojects = 1;
int config_show_title_ever = 1;
int config_show_title_year = 0;
int config_show_title_month = 0;
int config_show_title_week = 0;
int config_show_title_lastweek = 0;
int config_show_title_day = 1;
int config_show_title_yesterday = 0;
int config_show_title_current = 0;
int config_show_title_desc = 1;
int config_show_title_task = 1;
int config_show_title_estimated_start = 0;
int config_show_title_estimated_end = 0;
int config_show_title_due_date = 0;
int config_show_title_sizing = 0;
int config_show_title_percent_complete = 0;
int config_show_title_urgency = 1;
int config_show_title_importance = 1;
int config_show_title_status = 0;

int config_show_tb_icons = 1;
int config_show_tb_texts = 1;
int config_show_tb_tips = 1;
int config_show_tb_new = 1;
int config_show_tb_ccp = 0;
int config_show_tb_journal = 1;
int config_show_tb_calendar = 0;
int config_show_tb_prop = 1;
int config_show_tb_timer = 1;
int config_show_tb_pref = 0;
int config_show_tb_help = 1;
int config_show_tb_exit = 1;

char *config_logfile_name = NULL;
char *config_logfile_start = NULL;
char *config_logfile_stop = NULL;
int config_logfile_use = 0;
int config_logfile_min_secs = 0;

char * config_data_url = NULL;

typedef struct _PrefsDialog 
{
	GladeXML *gtxml;
	GnomePropertyBox *dlg;
	GtkCheckButton *show_secs;
	GtkCheckButton *show_statusbar;
	GtkCheckButton *show_clist_titles;
	GtkCheckButton *show_subprojects;

	GtkCheckButton *show_title_importance;
	GtkCheckButton *show_title_urgency;
	GtkCheckButton *show_title_status;
	GtkCheckButton *show_title_ever;
	GtkCheckButton *show_title_year;
	GtkCheckButton *show_title_month;
	GtkCheckButton *show_title_week;
	GtkCheckButton *show_title_lastweek;
	GtkCheckButton *show_title_day;
	GtkCheckButton *show_title_yesterday;
	GtkCheckButton *show_title_current;
	GtkCheckButton *show_title_desc;
	GtkCheckButton *show_title_task;
	GtkCheckButton *show_title_estimated_start;
	GtkCheckButton *show_title_estimated_end;
	GtkCheckButton *show_title_due_date;
	GtkCheckButton *show_title_sizing;
	GtkCheckButton *show_title_percent_complete;

	GtkCheckButton *logfileuse;
	GtkWidget      *logfilename_l;
	GtkEntry       *logfilename;
	GtkWidget      *logfilestart_l;
	GtkEntry       *logfilestart;
	GtkWidget      *logfilestop_l;
	GtkEntry       *logfilestop;
	GtkWidget      *logfileminsecs_l;
	GtkEntry       *logfileminsecs;

	GtkEntry       *shell_start;
	GtkEntry       *shell_stop;

	GtkCheckButton *show_tb_icons;
	GtkCheckButton *show_tb_texts;
	GtkCheckButton *show_tb_tips;
	GtkCheckButton *show_tb_new;
	GtkCheckButton *show_tb_ccp;
	GtkCheckButton *show_tb_journal;
	GtkCheckButton *show_tb_pref;
	GtkCheckButton *show_tb_timer;
	GtkCheckButton *show_tb_prop;
	GtkCheckButton *show_tb_help;
	GtkCheckButton *show_tb_exit;

	GtkEntry       *idle_secs;
} PrefsDialog;


/* ============================================================== */

#define ENTRY_TO_CHAR(a, b) { 			\
	const char *s = gtk_entry_get_text(a); 	\
	if (s[0]) {				\
		if (b) g_free(b); 		\
		b = g_strdup(s); 		\
	} else { 				\
		if (b) g_free(b); 		\
		b = NULL; 			\
	} 					\
}

#define SHOW_CHECK(TOK) {					\
	int state = GTK_TOGGLE_BUTTON(odlg->show_##TOK)->active;\
	if (config_show_##TOK != state) {			\
		change = 1;					\
		config_show_##TOK = state;			\
	}							\
}

static void 
prefs_set(GnomePropertyBox * pb, gint page, PrefsDialog *odlg)
{
	int state;

	if (0 == page)
	{
		int change = 0;

		SHOW_CHECK (title_importance);
		SHOW_CHECK (title_urgency);
		SHOW_CHECK (title_status);
		SHOW_CHECK (title_ever);
		SHOW_CHECK (title_year);
		SHOW_CHECK (title_month);
		SHOW_CHECK (title_week);
		SHOW_CHECK (title_lastweek);
		SHOW_CHECK (title_day);
		SHOW_CHECK (title_yesterday);
		SHOW_CHECK (title_current);
		SHOW_CHECK (title_desc);
		SHOW_CHECK (title_task);
		SHOW_CHECK (title_estimated_start);
		SHOW_CHECK (title_estimated_end);
		SHOW_CHECK (title_due_date);
		SHOW_CHECK (title_sizing);
		SHOW_CHECK (title_percent_complete);

		if (change)
		{
			ctree_refresh (global_ptw);
		}
	
	}
	if (1 == page)
	{

		/* display options */
		state = GTK_TOGGLE_BUTTON(odlg->show_secs)->active;
		if (state != config_show_secs) {
			config_show_secs = state;
			ctree_setup (global_ptw, gtt_get_project_list());
			update_status_bar();
			if (status_bar)
			gtk_widget_queue_resize(status_bar);
		}
		if (GTK_TOGGLE_BUTTON(odlg->show_statusbar)->active) {
			gtk_widget_show(GTK_WIDGET(status_bar));
			config_show_statusbar = 1;
		} else {
			gtk_widget_hide(GTK_WIDGET(status_bar));
			config_show_statusbar = 0;
		}
		if (GTK_TOGGLE_BUTTON(odlg->show_clist_titles)->active) {
			config_show_clist_titles = 1;
			ctree_titles_show (global_ptw);
		} else {
			config_show_clist_titles = 0;
			ctree_titles_hide (global_ptw);
		}
	
		if (GTK_TOGGLE_BUTTON(odlg->show_subprojects)->active) {
			config_show_subprojects = 1;
			ctree_subproj_show (global_ptw);
		} else {
			config_show_subprojects = 0;
			ctree_subproj_hide (global_ptw);
		}

	}

	if (2 == page)
	{
		/* shell command options */
		ENTRY_TO_CHAR(odlg->shell_start, config_shell_start);
		ENTRY_TO_CHAR(odlg->shell_stop, config_shell_stop);
	}

	if (3 == page)
	{
		/* log file options */
		config_logfile_use = GTK_TOGGLE_BUTTON(odlg->logfileuse)->active;
		ENTRY_TO_CHAR(odlg->logfilename, config_logfile_name);
		ENTRY_TO_CHAR(odlg->logfilestart, config_logfile_start);
		ENTRY_TO_CHAR(odlg->logfilestop, config_logfile_stop);
		config_logfile_min_secs = atoi (gtk_entry_get_text(odlg->logfileminsecs));
	}

	if (4 == page)
	{
		int change = 0;

		/* toolbar */
		config_show_tb_icons = GTK_TOGGLE_BUTTON(odlg->show_tb_icons)->active;
		config_show_tb_texts = GTK_TOGGLE_BUTTON(odlg->show_tb_texts)->active;
		config_show_tb_tips = GTK_TOGGLE_BUTTON(odlg->show_tb_tips)->active;
	
		/* toolbar sections */
		SHOW_CHECK (tb_new);
		SHOW_CHECK (tb_ccp);
		SHOW_CHECK (tb_journal);
		SHOW_CHECK (tb_prop);
		SHOW_CHECK (tb_timer);
		SHOW_CHECK (tb_pref);
		SHOW_CHECK (tb_help);
		SHOW_CHECK (tb_exit);

		if (change) 
		{
			update_toolbar_sections();
		}

		toolbar_set_states();
	}

	if (5 == page)
	{
		config_idle_timeout = atoi(gtk_entry_get_text(GTK_ENTRY(odlg->idle_secs)));
	}

	/* Also save them the to file at this point */
	save_properties();
}

/* ============================================================== */

static void 
logfile_sensitive_cb(GtkWidget *w, PrefsDialog *odlg)
{
	int state;
	
	state = GTK_TOGGLE_BUTTON(odlg->logfileuse)->active;
	gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilename), state);
	gtk_widget_set_sensitive(odlg->logfilename_l, state);
	gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilestart), state);
	gtk_widget_set_sensitive(odlg->logfilestart_l, state);
	gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilestop), state);
	gtk_widget_set_sensitive(odlg->logfilestop_l, state);
	gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfileminsecs), state);
	gtk_widget_set_sensitive(odlg->logfileminsecs_l, state);
}

#define SET_ACTIVE(TOK) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->show_##TOK), \
		config_show_##TOK);

static void 
options_dialog_set(PrefsDialog *odlg)
{
	char s[30];

	SET_ACTIVE(secs);
	SET_ACTIVE(statusbar);
	SET_ACTIVE(clist_titles);
	SET_ACTIVE(subprojects);

	SET_ACTIVE(title_importance);
	SET_ACTIVE(title_urgency);
	SET_ACTIVE(title_status);
	SET_ACTIVE(title_ever);
	SET_ACTIVE(title_year);
	SET_ACTIVE(title_month);
	SET_ACTIVE(title_week);
	SET_ACTIVE(title_lastweek);
	SET_ACTIVE(title_day);
	SET_ACTIVE(title_yesterday);
	SET_ACTIVE(title_current);
	SET_ACTIVE(title_desc);
	SET_ACTIVE(title_task);
	SET_ACTIVE(title_estimated_start);
	SET_ACTIVE(title_estimated_end);
	SET_ACTIVE(title_due_date);
	SET_ACTIVE(title_sizing);
	SET_ACTIVE(title_percent_complete);

	if (config_shell_start)
		gtk_entry_set_text(odlg->shell_start, config_shell_start);
	else
		gtk_entry_set_text(odlg->shell_start, "");
	
	if (config_shell_stop)
		gtk_entry_set_text(odlg->shell_stop, config_shell_stop);
	else
		gtk_entry_set_text(odlg->shell_stop, "");
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->logfileuse), config_logfile_use);
	if (config_logfile_name)
		gtk_entry_set_text(odlg->logfilename, config_logfile_name);
	else
		gtk_entry_set_text(odlg->logfilename, "");
	
	if (config_logfile_start)
		gtk_entry_set_text(odlg->logfilestart, config_logfile_start);
	else
		gtk_entry_set_text(odlg->logfilestart, "");
	
	if (config_logfile_stop)
		gtk_entry_set_text(odlg->logfilestop, config_logfile_stop);
	else
		gtk_entry_set_text(odlg->logfilestop, "");

	g_snprintf(s, sizeof (s), "%d", config_logfile_min_secs);
	gtk_entry_set_text(GTK_ENTRY(odlg->logfileminsecs), s);

	logfile_sensitive_cb(NULL, odlg);

	/* toolbar sections */
	SET_ACTIVE(tb_icons);
	SET_ACTIVE(tb_texts);
	SET_ACTIVE(tb_tips);
	SET_ACTIVE(tb_new);
	SET_ACTIVE(tb_ccp);
	SET_ACTIVE(tb_journal);
	SET_ACTIVE(tb_prop);
	SET_ACTIVE(tb_timer);
	SET_ACTIVE(tb_pref);
	SET_ACTIVE(tb_help);
	SET_ACTIVE(tb_exit);

	/* misc section */
	g_snprintf(s, sizeof (s), "%d", config_idle_timeout);
	gtk_entry_set_text(GTK_ENTRY(odlg->idle_secs), s);

	/* set to unmodified as it reflects the current state of the app */
	gnome_property_box_set_modified(GNOME_PROPERTY_BOX(odlg->dlg), FALSE);
}

/* ============================================================== */

#define GETWID(strname) 						\
({									\
	GtkWidget *e;							\
	e = glade_xml_get_widget (gtxml, strname);			\
	gtk_signal_connect_object(GTK_OBJECT(e), "changed",		\
			  GTK_SIGNAL_FUNC(gnome_property_box_changed), 	\
			  GTK_OBJECT(dlg->dlg));			\
	e;								\
})

#define GETCHWID(strname) 						\
({									\
	GtkWidget *e;							\
	e = glade_xml_get_widget (gtxml, strname);			\
	gtk_signal_connect_object(GTK_OBJECT(e), "toggled",		\
			  GTK_SIGNAL_FUNC(gnome_property_box_changed), 	\
			  GTK_OBJECT(dlg->dlg));			\
	e;								\
})

static void 
display_options(PrefsDialog *dlg)
{
	GtkWidget *w;
	GladeXML *gtxml = dlg->gtxml;

	w = GETCHWID ("show secs");
	dlg->show_secs = GTK_CHECK_BUTTON(w);

	w = GETCHWID ("show statusbar");
	dlg->show_statusbar = GTK_CHECK_BUTTON(w);

	w = GETCHWID ("show header");
	dlg->show_clist_titles = GTK_CHECK_BUTTON(w);

	w = GETCHWID ("show sub");
	dlg->show_subprojects = GTK_CHECK_BUTTON(w);
}

#define DLGWID(strname)					\
	w = GETCHWID ("show " #strname);		\
	dlg->show_title_##strname = GTK_CHECK_BUTTON(w);


static void 
field_options(PrefsDialog *dlg)
{
	GtkWidget *w;
	GladeXML *gtxml = dlg->gtxml;

	DLGWID (importance);
	DLGWID (urgency);
	DLGWID (status);
	DLGWID (ever);
	DLGWID (year);
	DLGWID (month);
	DLGWID (week);
	DLGWID (lastweek);
	DLGWID (day);
	DLGWID (yesterday);
	DLGWID (current);
	DLGWID (desc);
	DLGWID (task);
	DLGWID (estimated_start);
	DLGWID (estimated_end);
	DLGWID (due_date);
	DLGWID (sizing);
	DLGWID (percent_complete);
}


static void 
shell_command_options (PrefsDialog *dlg)
{
	GtkWidget *e;
	GladeXML *gtxml = dlg->gtxml;

	e = GETWID ("start project");
	dlg->shell_start = GTK_ENTRY(e);

	e = GETWID ("stop project");
	dlg->shell_stop = GTK_ENTRY(e);
}

static void 
logfile_options(PrefsDialog *dlg)
{
	GtkWidget *w;
	GladeXML *gtxml = dlg->gtxml;

	w = GETCHWID ("use logfile");
	dlg->logfileuse = GTK_CHECK_BUTTON(w);
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
		   GTK_SIGNAL_FUNC(logfile_sensitive_cb),
		   (gpointer *)dlg);

	w = glade_xml_get_widget (gtxml, "filename label");
	dlg->logfilename_l = w;

	w = GETWID ("filename combo");
	dlg->logfilename = GTK_ENTRY(w);

	w = glade_xml_get_widget (gtxml, "fstart label");
	dlg->logfilestart_l = w;

	w = GETWID ("fstart combo");
	dlg->logfilestart = GTK_ENTRY(w);

	w = glade_xml_get_widget (gtxml, "fstop label");
	dlg->logfilestop_l = w;

	w = GETWID ("fstop combo");
	dlg->logfilestop = GTK_ENTRY(w);

	w = glade_xml_get_widget (gtxml, "fmin label");
	dlg->logfileminsecs_l = w;

	w = GETWID ("fmin combo");
	dlg->logfileminsecs = GTK_ENTRY(w);
}

#define TBWID(strname)					\
	w = GETCHWID ("show " #strname);		\
	dlg->show_tb_##strname = GTK_CHECK_BUTTON(w);

static void 
toolbar_options(PrefsDialog *dlg)
{
	GtkWidget *w;
	GladeXML *gtxml = dlg->gtxml;

	TBWID (icons);
	TBWID (texts);
	TBWID (tips);
	TBWID (new);
	TBWID (ccp);
	TBWID (journal);
	TBWID (prop);
	TBWID (timer);
	TBWID (pref);
	TBWID (help);
	TBWID (exit);
}

static void 
misc_options(PrefsDialog *dlg)
{
	GtkWidget *w;
	GladeXML *gtxml = dlg->gtxml;

	w = GETWID ("idle secs");
	dlg->idle_secs = GTK_ENTRY(w);
}

/* ============================================================== */

static PrefsDialog *
prefs_dialog_new (void)
{
	PrefsDialog *dlg;
	GladeXML *gtxml;

	dlg = g_malloc(sizeof(PrefsDialog));

	gtxml = gtt_glade_xml_new ("glade/prefs.glade", "Global Preferences");
	dlg->gtxml = gtxml;

	dlg->dlg = GNOME_PROPERTY_BOX (glade_xml_get_widget (gtxml,  "Global Preferences"));

	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "help",
			   GTK_SIGNAL_FUNC(gtt_help_popup),
			   "gnotime.xml#preferences");

	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "apply",
			   GTK_SIGNAL_FUNC(prefs_set), dlg);

	/* ------------------------------------------------------ */
	/* grab the various entry boxes and hook them up */
	display_options (dlg);
	field_options (dlg);
	shell_command_options (dlg);
	logfile_options (dlg);
	toolbar_options (dlg);
	misc_options (dlg);

	gnome_dialog_close_hides(GNOME_DIALOG(dlg->dlg), TRUE);
	return dlg;
}


/* ============================================================== */

static PrefsDialog *dlog = NULL;

void 
prefs_dialog_show(void)
{
	if (!dlog) dlog = prefs_dialog_new();
 
	options_dialog_set (dlog);
	gtk_widget_show(GTK_WIDGET(dlog->dlg));
}

/* ==================== END OF FILE ============================= */
