/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002,2003,2004 Linas Vepstas <linas@linas.org>
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

#include <gnome.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <qof.h>

#include "active-dialog.h"
#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "gtt.h"
#include "log.h"
#include "menucmd.h"
#include "menus.h"
#include "notes-area.h"
#include "prefs.h"
#include "props-proj.h"
#include "timer.h"
#include "toolbar.h"
#include "util.h"


/* XXX Most of the globals below should be placed into a single
 * application-wide top-level structure, rather than being allowed
 * to be globals. But, for now, its OK ... 
 */
GttProject *cur_proj = NULL;

ProjTreeWindow *global_ptw = NULL;
NotesArea *global_na = NULL;
GtkWidget *app_window = NULL;
GtkWidget *status_bar = NULL;


static GtkLabel *status_project = NULL;
static GtkLabel *status_day_time = NULL;
static GtkWidget *status_timer = NULL;

char *config_shell_start = NULL;
char *config_shell_stop = NULL;

gboolean geom_place_override = FALSE;
gboolean geom_size_override = FALSE;


extern GttActiveDialog *act;

/* ============================================================= */

void 
update_status_bar(void)
{
  char day_total_str[25];
  char *s;
  
  if (!status_bar) return;
  
  /* Make the little clock item appear/disappear
   * when the project is started/stopped */
  if (status_timer)
  {
    if (timer_is_running())
      gtk_widget_show(status_timer);
    else
      gtk_widget_hide(status_timer);
  }
	
	/* update timestamp */
	qof_print_hours_elapsed_buff (day_total_str, 25,
                                gtt_project_list_total_secs_day(),
                                config_show_secs);

  if (0 != strcmp(day_total_str, gtk_label_get_text(status_day_time)))
  {
    gtk_label_set_text(status_day_time, day_total_str);
  }

	/* Display the project title */
	if (cur_proj)
	{
		s = g_strdup_printf ("%s - %s",
                         gtt_project_get_title(cur_proj),
                         gtt_project_get_desc(cur_proj));
	}
	else
	{
		s = g_strdup(_("Timer is not running"));
	}

  if (0 != strcmp(s, gtk_label_get_text(status_project)))
	{
		gtk_label_set_text(status_project, s);
	}
  g_free(s);
}


/* ============================================================= */
/* Handle shell commands */

void
do_run_shell_command (const char * str)
{
	pid_t pid;
	char *shell_path;
	int rc;
	struct stat shat;

	/* XXX This whole thing needs to be reviewewed for security */

   /* Provide minimal security by using only system shells */
	shell_path = "/bin/sh";
	rc = stat (shell_path, &shat);
	if ((0 == rc) && S_ISREG (shat.st_mode) && (S_IXUSR & shat.st_mode))
	{
		goto do_run_shell;
	}

	shell_path = "/usr/bin/sh";
	rc = stat (shell_path, &shat);
	if ((0 == rc) && S_ISREG (shat.st_mode) && (S_IXUSR & shat.st_mode))
	{
		goto do_run_shell;
	}
	g_warning("%s: %d: do_run_shell_command: can't find shell\n", __FILE__, __LINE__);
	return;

do_run_shell:
	pid = fork();
	if (pid == 0) 
	{
		execlp(shell_path, shell_path, "-c", str, NULL);
		g_warning("%s: %d: do_run_shell_command: couldn't exec\n", __FILE__, __LINE__);
		exit(1);
	}
	if (pid < 0) 
	{
		g_warning("%s: %d: do_run_shell_command: couldn't fork\n", __FILE__, __LINE__);
	}

	/* Note that the forked processes might be scheduled by the operating
	 * system 'out of order', if we've made rapid successive calls to this
	 * routine.  So we try to ensure in-order execution by trying to let
	 * the child process at least start running.  And we can do this by
	 * yielding our time-slice ... */
	sched_yield();
}

void
run_shell_command (GttProject *proj, gboolean do_start)
{
	char *cmd;
	const char *str;

	cmd = (do_start) ? config_shell_start : config_shell_stop;

	if (!cmd) return;

	/* Sometimes, we are called to stop a NULL project.
	 * We don't really want that (its a result be being called twice).
	 */ 
	if (!proj) return;
	
	str = printf_project (cmd, proj);
   do_run_shell_command (str);
	g_free ((gchar *) str);
}

/* ============================================================= */

void 
cur_proj_set (GttProject *proj)
{
	/* Due to the way the widget callbacks work, 
	 * we may be called recursively ... */
	if (cur_proj == proj) return;

	log_proj(NULL);
	gtt_project_timer_stop (cur_proj);
	run_shell_command (cur_proj, FALSE);

	if (proj)
	{
		if (timer_is_running ())
		{
			stop_main_timer ();
		}
		cur_proj = proj;
		gtt_project_timer_start (proj);
		run_shell_command (cur_proj, TRUE);
		start_idle_timer ();
		start_main_timer ();
	}
	else
	{
		if (timer_is_running ())
		{
			stop_main_timer ();
		}
		cur_proj = NULL;
		start_no_project_timer ();
	}
	log_proj(proj);

	/* update GUI elements */
	menu_set_states();
	toolbar_set_states();
	if (proj) 
	{
		prop_dialog_set_project(proj);
		notes_area_set_project (global_na, proj);
	}
	update_status_bar();
}


/* ============================================================= */

void 
focus_row_set (GttProject *proj)
{
	/* update GUI elements */
	
	prop_dialog_set_project(proj);
	notes_area_set_project (global_na, proj);
}


/* ============================================================= */

void 
app_new(int argc, char *argv[], const char *geometry_string)
{
	GtkWidget *ctree;
	GtkWidget *vbox;
	GtkWidget *widget;
	GtkWidget *vpane;
    GtkWidget *separator;
    GtkLabel *filler;
    GtkHBox *labels;
    GtkVBox *status_vbox;
    GtkStatusbar *grip;

	app_window = gnome_app_new(GTT_APP_NAME, GTT_APP_TITLE " " VERSION);
	gtk_window_set_wmclass(GTK_WINDOW(app_window),
                           GTT_APP_NAME, GTT_APP_PROPER_NAME);

	/* 485 x 272 seems to be a good size to default to */
	gtk_window_set_default_size(GTK_WINDOW(app_window), 485, 272);
	gtk_window_set_resizable (GTK_WINDOW(app_window), TRUE);
	
	/* build menus */
	menus_create(GNOME_APP(app_window));

	/* build toolbar */
	widget = build_toolbar();
	gtk_widget_show(widget);
	gnome_app_set_toolbar(GNOME_APP(app_window), GTK_TOOLBAR(widget));
	
	/* container holds status bar, main ctree widget */
	vbox = gtk_vbox_new(FALSE, 0);

	/* build statusbar */

    status_vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));
    gtk_widget_show(GTK_WIDGET(status_vbox));

    labels = GTK_HBOX(gtk_hbox_new(FALSE, 0));
    gtk_widget_show(GTK_WIDGET(labels));
    
	status_bar = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(status_bar);
    separator = gtk_hseparator_new();
    gtk_widget_show(separator);
    gtk_box_pack_start(GTK_BOX(status_vbox), separator, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(status_vbox), labels, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(status_bar), status_vbox, TRUE, TRUE, 0);

    grip = GTK_STATUSBAR(gtk_statusbar_new());
    gtk_statusbar_set_has_resize_grip(grip, TRUE);
    gtk_widget_show(GTK_WIDGET(grip));
    gtk_box_pack_start(GTK_BOX(status_bar), GTK_WIDGET(grip), FALSE, FALSE, 0);

	/* put elapsed time into statusbar */
	status_day_time = GTK_LABEL(gtk_label_new(_("00:00")));
	gtk_widget_show(GTK_WIDGET(status_day_time));

	gtk_box_pack_start(GTK_BOX(labels), GTK_WIDGET(status_day_time),
                       FALSE, TRUE, 0);
	
	/* put project name into statusbar */
	status_project = GTK_LABEL(gtk_label_new( _("Timer is not running")));
	gtk_widget_show(GTK_WIDGET(status_project));
	
	gtk_box_pack_start(GTK_BOX(labels), GTK_WIDGET(status_project),
                       FALSE, TRUE, 10);

    filler = GTK_LABEL(gtk_label_new(""));
    gtk_widget_show(GTK_WIDGET(filler));
    gtk_box_pack_start(GTK_BOX(labels), GTK_WIDGET(filler), TRUE, TRUE, 1);

	/* put timer icon into statusbar */
	status_timer = gtk_image_new_from_stock (GNOME_STOCK_TIMER,
                                             GTK_ICON_SIZE_MENU);
	gtk_widget_show(status_timer);
	gtk_box_pack_end(GTK_BOX(status_bar), GTK_WIDGET(status_timer),
                     FALSE, FALSE, 1);

	/* create the main columned tree for showing projects */
	global_ptw = ctree_new();
	ctree = ctree_get_widget(global_ptw);

	/* create the notes area */
	global_na = notes_area_new();
	vpane = notes_area_get_widget (global_na);

	/* Need to reparent, to get rid of glade parent-window hack.
	 * But gtk_widget_reparent (vpane); causes  a "Gtk-CRITICAL" 
	 * to occur.  So we need a fancier move.
	 */
	gtk_widget_ref (vpane);
	gtk_container_remove(GTK_CONTAINER(vpane->parent), vpane);
	gtk_box_pack_start(GTK_BOX(vbox), vpane, TRUE, TRUE, 0);
	gtk_widget_unref (vpane);

    gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, FALSE, 2);

	notes_area_add_ctree (global_na, ctree);
	
	/* we are done building it, make it visible */
	gtk_widget_show(vbox);
	gnome_app_set_contents(GNOME_APP(app_window), vbox);

	gtt_status_icon_create();
	if (!geometry_string) return;
	
	if (gtk_window_parse_geometry(GTK_WINDOW(app_window),geometry_string))
	{
		geom_size_override=TRUE;
	}
	else 
	{
		gnome_app_error(GNOME_APP(app_window),
			_("Couldn't understand geometry (position and size)\n"
			  " specified on command line"));
	}
}

void 
app_show (void)
{
	if (!GTK_WIDGET_MAPPED(app_window)) 
	{
		gtk_widget_show(app_window);
	}
}



void
app_quit(GtkWidget *w, gpointer data)
{
	save_properties ();
	save_projects ();
	gtt_status_icon_destroy ();
	gtk_main_quit();
}

/* ============================== END OF FILE ===================== */
