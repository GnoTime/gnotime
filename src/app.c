/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
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
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "gtt.h"
#include "log.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "props-proj.h"
#include "timer.h"
#include "toolbar.h"
#include "util.h"


/* I had some problems with the GtkStatusbar (frame and label didn't
   get shown). So I defined this here */
#define GTK_USE_STATUSBAR

/* Due to the same problems I define this, if I want to include the
   gtk_widget_show for the frame and the label of the statusbar */
#ifdef GTK_USE_STATUSBAR
#undef SB_USE_HACK
#endif


GttProject *cur_proj = NULL;

ProjTreeWindow *global_ptw;
GtkWidget *window;
GtkWidget *glist;
GtkWidget *status_bar;

#ifdef GTK_USE_STATUSBAR
static GtkStatusbar *status_project = NULL;
static GtkStatusbar *status_day_time = NULL;
static GtkWidget *status_timer = NULL;
static gint status_project_id = 1, status_day_time_id = 2;
#else /* GTK_USE_STATUSBAR */
static GtkLabel *status_project = NULL;
static GtkLabel *status_day_time = NULL;
#endif /* GTK_USE_STATUSBAR */

char *config_command = NULL;
char *config_command_null = NULL;

gboolean geom_place_override = FALSE;
gboolean geom_size_override = FALSE;

/* ============================================================= */

void 
update_status_bar(void)
{
	char day_total_str[25];
	static char *old_day_time = NULL;
	static char *old_project = NULL;
	char *s;

	if (!status_bar) return;
	if (status_timer) {
		if (timer_is_running())
			gtk_widget_show(status_timer);
		else
			gtk_widget_hide(status_timer);
	}
	if (!old_day_time) old_day_time = g_strdup("");
	if (!old_project) old_project = g_strdup("");

	print_hours_elapsed (day_total_str, 25, 
	gtt_project_list_total_secs_day(), config_show_secs);

	s = g_strdup(day_total_str);
	if (0 != strcmp(s, old_day_time)) {
#ifdef GTK_USE_STATUSBAR
		gtk_statusbar_remove(status_day_time, 2, status_day_time_id);
		status_day_time_id = gtk_statusbar_push(status_day_time, 2, s);
#else /* not GTK_USE_STATUSBAR */
		gtk_label_set(status_day_time, s);
#endif /* not GTK_USE_STATUSBAR */
		g_free(old_day_time);
		old_day_time = s;
	} else {
		g_free(s);
	}
	if (cur_proj) {
		s = g_strdup_printf ("%s - %s", 
		gtt_project_get_title(cur_proj),
		gtt_project_get_desc(cur_proj));
	} else {
		s = g_strdup(_("no project selected"));
	}
	if (0 != strcmp(s, old_project)) {
#ifdef GTK_USE_STATUSBAR
		gtk_statusbar_remove(status_project, 1, status_project_id);
		status_project_id = gtk_statusbar_push(status_project, 1, s);
#else /* not GTK_USE_STATUSBAR */
		gtk_label_set(status_project, s);
#endif /* not GTK_USE_STATUSBAR */
		g_free(old_project);
		old_project = s;
	} else {
		g_free(s);
	}
}



void 
cur_proj_set(GttProject *proj)
{
	GttProject *prev_proj = NULL;
	pid_t pid;
	char *cmd;
	const char *str;

	/* Due to the way the widget callbacks work, 
	 * we may be called recursively ... */
	if (cur_proj == proj) return;

	log_proj(NULL);
	gtt_project_timer_stop (cur_proj);
	if (proj) 
	{
		cur_proj = proj;
		gtt_project_timer_start (proj); 
	}
	else
	{
		prev_proj = cur_proj;
		cur_proj = NULL;
	}
	log_proj(proj);
	menu_set_states();
	if (proj) prop_dialog_set_project(proj);
	update_status_bar();
	cmd = (proj) ? config_command : config_command_null;

	/* handle commands */
	if (!cmd) return;
	str = printf_project (cmd, (proj) ? cur_proj : prev_proj);
	pid = fork();
	if (pid == 0) 
	{
		execlp("sh", "sh", "-c", str, NULL);
		g_warning("%s: %d: cur_proj_set: couldn't exec\n", __FILE__, __LINE__);
		exit(1);
	}
	if (pid < 0) {
		g_warning("%s: %d: cur_proj_set: couldn't fork\n", __FILE__, __LINE__);
	}

	/* Note that the forked processes might be scheduled by the operating
	 * system 'out of order', if we've made rapid successive calls to this
	 * routine.  So we try to ensure in-order execution by trying to let
	 * the child process at least start running.  And we can do this by
	 * yielding our time-slice ... */
	sched_yield();
}



void app_new(int argc, char *argv[], const char *geometry_string)
{
	GtkWidget *vbox;
	GtkWidget *widget;
	gint x, y, w, h;

	window = gnome_app_new("gtt", APP_NAME " " VERSION);
	gtk_window_set_wmclass(GTK_WINDOW(window),
			       "gtt", "GTimeTracker");
	/* 320 x 220 seems to be a good size to default to */
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 220);
	gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);
	menus_create(GNOME_APP(window));
	widget = build_toolbar();
	gtk_widget_show(widget);
	gnome_app_set_toolbar(GNOME_APP(window), GTK_TOOLBAR(widget));
	vbox = gtk_vbox_new(FALSE, 0);

	status_bar = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(status_bar);
	gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, FALSE, 2);
	status_day_time = GTK_STATUSBAR(gtk_statusbar_new());
#ifdef SB_USE_HACK
	gtk_widget_show(GTK_WIDGET(status_day_time->frame));
	gtk_widget_show(GTK_WIDGET(status_day_time->label));
#endif /* SB_USE_HACK */
	gtk_widget_show(GTK_WIDGET(status_day_time));
	status_day_time_id = gtk_statusbar_push(status_day_time,
						2, _("00:00"));
	gtk_box_pack_start(GTK_BOX(status_bar), GTK_WIDGET(status_day_time),
			   FALSE, FALSE, 1);
	status_project = GTK_STATUSBAR(gtk_statusbar_new());
#ifdef SB_USE_HACK
	gtk_widget_show(GTK_WIDGET(status_project->frame));
	gtk_widget_show(GTK_WIDGET(status_project->label));
#endif /* SB_USE_HACK */
	gtk_widget_show(GTK_WIDGET(status_project));
	status_project_id = gtk_statusbar_push(status_project,
					       1, _("no project selected"));
	gtk_box_pack_start(GTK_BOX(status_bar), GTK_WIDGET(status_project),
			   TRUE, TRUE, 1);
	status_timer = gtk_image_new_from_stock (GNOME_STOCK_TIMER, 
			GTK_ICON_SIZE_MENU);
	gtk_widget_show(status_timer);
	gtk_box_pack_end(GTK_BOX(status_bar), GTK_WIDGET(status_timer),
			 FALSE, FALSE, 1);

	global_ptw = ctree_new();
	glist = ctree_get_widget(global_ptw);

	gtk_box_pack_end(GTK_BOX(vbox), glist->parent, TRUE, TRUE, 0);

	gtk_widget_show(vbox);
	gnome_app_set_contents(GNOME_APP(window), vbox);

	if (!geometry_string) {
		return;
	}
#if OLD_GNOME_12_CODE
	/* XXX get rid of this at earliest convenience, 
	 * after testing, of course ...  */
	if (gnome_parse_geometry(geometry_string, &x, &y, &w, &h)) {
		if ((x != -1) && (y != -1)) {
			gtk_widget_set_uposition(GTK_WIDGET(window), x, y);
			geom_place_override=TRUE;
		}
		if ((w != -1) && (h != -1)) {
			gtk_window_set_default_size(GTK_WINDOW(window), w, h);
			geom_size_override=TRUE;
		}
	} 
#else
	if (gtk_window_parse_geometry(GTK_WINDOW(window),geometry_string))
	{
		geom_size_override=TRUE;
	}
#endif
	else 
	{
		gnome_app_error(GNOME_APP(window),
			_("Couldn't understand geometry (position and size)\n"
			  " specified on command line"));
	}
}

void 
app_show (void)
{
	if (!GTK_WIDGET_MAPPED(window)) 
	{
		gtk_widget_show(window);
	}
}

/* ============================== END OF FILE ===================== */
