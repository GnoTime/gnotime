/*   Keyboard inactivity timout dialog for GTimeTracker - a time tracker
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

#include <glade/glade.h>
#include <gnome.h>
#include <string.h>

#include <qof/gnc-date.h>

#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "active-dialog.h"
#include "idle-timer.h"
#include "proj.h"
#include "proj-query.h"
#include "util.h"


extern int config_idle_timeout;

struct GttActiveDialog_s 
{
	gboolean    armed;
	GladeXML    *gtxml;
	GtkDialog   *dlg;
	GtkButton   *yes_btn;
	GtkButton   *no_btn;
	GtkLabel    *active_label;
	GtkLabel    *credit_label;
	GtkOptionMenu  *project_menu;
	
	IdleTimeout *idt;
	time_t      time_armed;
};


/* =========================================================== */

static void
dialog_close (GObject *obj, GttActiveDialog *dlg)
{
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
	dlg->time_armed = time(0);
}

/* =========================================================== */

static void
dialog_kill (GObject *obj, GttActiveDialog *dlg)
{
	gtk_widget_destroy (GTK_WIDGET(dlg->dlg));
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
	dlg->time_armed = time(0);
}

/* =========================================================== */

static void
start_proj (GObject *obj, GttActiveDialog *dlg)
{
	GtkMenu *menu;
	GtkWidget *w;
	GttProject *prj;
	
	/* Start the project that the user has selected from the menu */
	menu = GTK_MENU (gtk_option_menu_get_menu (dlg->project_menu));
	w = gtk_menu_get_active (menu);
	prj = g_object_get_data (G_OBJECT (w), "prj");

	ctree_start_timer (prj);
	dlg->armed = FALSE;
	dialog_kill (obj, dlg);
}

/* =========================================================== */

static void
setup_menus (GttActiveDialog *dlg)
{
	GtkMenuShell *menushell;
	GList *prjlist, *node;
	char * msg;

	msg = _("No project timer is currently running in GnoTime.  "
	        "Do you want to start a project timer running?  "
	        "If so, you can select a project from the menu below, "
	        "and click 'Start' to start the project running.  "
	        "Otherwise, just click 'Cancel' to do nothing.");

	gtk_label_set_text (dlg->active_label, msg);

	msg = _("You can credit this project with the time that you worked "
	        "on it but were away from the keyboard.  Enter a time below, "
	        "the project will be credited when you click 'Start'");
						 
	gtk_label_set_text (dlg->credit_label, msg);

	menushell = GTK_MENU_SHELL (gtk_menu_new());

	/* Give user a list only of the unfinished projects, 
	 * so that there isn't too much clutter ... */
	prjlist = gtt_project_get_unfinished ();
	for (node = prjlist; node; node=node->next)
	{
		GttProject *prj = node->data;
		GtkWidget *item;
		item = gtk_menu_item_new_with_label (gtt_project_get_title (prj));
		g_object_set_data (G_OBJECT(item), "prj", prj);
		gtk_menu_shell_append (menushell, item);
		gtk_widget_show (item);
	}
	gtk_option_menu_set_menu (dlg->project_menu, GTK_WIDGET(menushell));
}

/* =========================================================== */
/* XXX the new GtkDialog is broken; it can't hide-on-close,
 * unlike to old, deprecated GnomeDialog.  Thus, we have to
 * do a heavyweight re-initialization each time.  Urgh.
 */

static void
active_dialog_realize (GttActiveDialog * id)
{
	GtkWidget *w;
	GladeXML *gtxml;

	gtxml = gtt_glade_xml_new ("glade/active.glade", "Active Dialog");
	id->gtxml = gtxml;

	id->dlg = GTK_DIALOG (glade_xml_get_widget (gtxml, "Active Dialog"));

	id->yes_btn = GTK_BUTTON(glade_xml_get_widget (gtxml, "yes button"));
	id->no_btn  = GTK_BUTTON(glade_xml_get_widget (gtxml, "no button"));
	id->active_label = GTK_LABEL (glade_xml_get_widget (gtxml, "active label"));
	id->credit_label = GTK_LABEL (glade_xml_get_widget (gtxml, "credit label"));
	w = glade_xml_get_widget (gtxml, "project option menu");
	id->project_menu = GTK_OPTION_MENU (w);

	g_signal_connect(G_OBJECT(id->dlg), "destroy",
	          G_CALLBACK(dialog_close), id);

	g_signal_connect(G_OBJECT(id->yes_btn), "clicked",
	          G_CALLBACK(start_proj), id);

	g_signal_connect(G_OBJECT(id->no_btn), "clicked",
	          G_CALLBACK(dialog_kill), id);

}

/* =========================================================== */

GttActiveDialog *
active_dialog_new (void)
{
	GttActiveDialog *ad;

	ad = g_new0 (GttActiveDialog, 1);
	ad->armed = TRUE;
	ad->time_armed = time(0);
	ad->idt = idle_timeout_new ();
	ad->gtxml = NULL;

	return ad;
}

/* =========================================================== */

void 
show_active_dialog (GttActiveDialog *ad)
{
	time_t now, idle_time;
	if (!ad) return;

	/* If there is a project currently running, or if the dialog
	 * isn't armed, or the timeout isn't configured, then do nothing.
	 */
	if (cur_proj) return;
	if (FALSE == ad->armed) return;
	if (0 > config_idle_timeout) return;

	/* If there hasn't been a project running in a while, then pop. */
	now = time(0);
	idle_time = now - ad->time_armed;
// printf ("duude armed, waiting %d %d\n", idle_time, config_idle_timeout);
	if (idle_time <= config_idle_timeout) return;
					
	/* Due to GtkDialog broken-ness, re-realize the GUI */
	if (NULL == ad->gtxml)
	{
		active_dialog_realize (ad);
		setup_menus (ad);
	
		gtk_widget_show (GTK_WIDGET(ad->dlg));
	}
	else
	{
		raise_active_dialog (ad);
	}
}

/* =========================================================== */

void 
raise_active_dialog (GttActiveDialog *ad)
{
	time_t now;
	time_t active_time;

	if (!ad) return;
	if (NULL == ad->gtxml) return;

	/* If there has not been any activity recently, then leave things
	 * alone. Otherwise, work real hard to put the dialog where the
	 * user will see it.
	 */
	now = time(0);
	active_time = now - poll_last_activity (ad->idt);
	if (15 < active_time) return;

	/* The following will raise the window, and put it on the current
	 * workspace, at least if the metacity WM is used. Haven't tried
	 * other window managers.
	 */
	gtk_window_present (GTK_WINDOW (ad->dlg));
}

/* =========================================================== */

void 
arm_active_dialog (GttActiveDialog *ad)
{
	if (!ad) return;
	ad->time_armed = time(0);
	ad->armed = TRUE;
}

/* =========================================================== */

void 
cancel_active_dialog (GttActiveDialog *ad)
{
	if (!ad) return;
	ad->armed = FALSE;
}

/* =========================== END OF FILE ============================== */
