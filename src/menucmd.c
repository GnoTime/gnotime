/*   Main menu callbacks for app main menubar for GTimeTracker
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

#include <config.h>
#include <gnome.h>
#include <string.h>

#include "app.h"
#include "cur-proj.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "proj.h"
#include "props-proj.h"
#include "timer.h"
#include "toolbar.h"
#include "xml-gtt.h"


void
about_box(GtkWidget *w, gpointer data)
{
	const gchar *authors[] = {
		"Goedson Teixeira Paixão <goedson@debian.org>",
		"Linas Vepstas <linas@linas.org>",
		"Eckehard Berns <eb@berns.i-s-o.net>",
		"George Lebl <jirka@5z.com>",
		"Kip Warner <kip@thevertigo.com>",
		" ",
		_("Bug-fixes from:"),
		"Eric Anderson <eric.anderson@cordata.net>",
		"Derek Atkins <warlord@mit.edu>",
		"Jonathan Blandford  <jrb@redhat.com>",
		"Miguel de Icaza  <miguel@nuclecu.unam.mx>",
		"John Fleck <jfleck@inkstain.net>",
		"Nat Friedman  <nat@nat.org>",
		"Mark Galassi  <rosalia@cygnus.com>",
		"Jeff Garzik  <jgarzik@pobox.com>",
		"Sven M. Hallberg <pesco@gmx.de>",
		"Raja R Harinath  <harinath@cs.umn.edu>",
		"Peter Hawkins <peterhawkins@ozemail.com.au>",
		"Toshio Kuratomi <toshio@tiki-lounge.com>",
		"Egil Kvaleberg <egil@kvaleberg.no>",
		"Chris Lahey  <clahey@umich.edu>",
		"Gregory McLean <gregm@comstar.net>",
		"Kjartan Maraas  <kmaraas@gnome.org>",
		"Federico Mena Quintero  <federico@nuclecu.unam.mx>",
		"Tomas Ogren  <stric@ing.umu.se>",
		"Gediminas Paulauskas <menesis@delfi.lt>",
		"Havoc Pennington  <hp@pobox.com>",
		"Ettore Perazzoli  <ettore@comm2000.it>",
		"Changwoo Ryu  <cwryu@adam.kaist.ac.kr>",
		"Pablo Saratxaga <srtxg@chanae.alphanet.ch>",
		"Carsten Schaar  <nhadcasc@fs-maphy.uni-hannover.de>",
		"Mark Stosberg <mark@summersault.com>",
		"Tom Tromey  <tromey@cygnus.com>",
		"Sebastian Wilhelmi  <wilhelmi@ira.uka.de>",
		NULL
	};


	const gchar *documenters[] = {
		  "Eckehard Berns <eb@berns.i-s-o.net>",
		  "Linas Vepstas <linas@linas.org>",
		  "Goedson Teixeira Paixão <goedson@debian.org>",
		  NULL
	};

	const gchar *copyright = 
		"Copyright (C) 1997,98 Eckehard Berns\n"
		"Copyright (C) 2001-2004 Linas Vepstas\n"
		"Copyright (C) 2007-2008 Goedson Teixeira Paixão";

	const gchar *comments =  _("GnoTime is a combination of stop-watch, diary, "
							   " consultant billing system and todo-list manager.") ;
	gtk_show_about_dialog (w,
						   "version", VERSION,
						   "program-name", GTT_APP_TITLE,
						   "authors", authors,
						   "documenters", documenters,
						   "website", "http://gnotime.sourceforge.net/",
						   "translator-credits", _("translator-credits"),
						   "copyright", copyright,
						   "comments", comments, 
						   NULL);
}

/* =============================================================================== */
/* The below implements a new project popup dialog.
 * XXX It is HIG-deprecated and should be replaced by a
 * an editable c-tree entry line in the main window.
 * XXX FIXME remove this code ASAP ...
 */

static void
project_name_desc(GtkDialog *w, gint response_id, GtkEntry **entries)
{
	const char *name, *desc;
	GttProject *proj;
	GttProject *sib_prj;

	if (GTK_RESPONSE_OK != response_id)
	{
		gtk_widget_destroy (GTK_WIDGET (w));
		return;
	}

	sib_prj = gtt_projects_tree_get_selected_project (projects_tree);

	if (!(name = gtk_entry_get_text(entries[0]))) return;
	if (!(desc = gtk_entry_get_text(entries[1]))) return;
	if (!name[0]) return;

	/* New project will have the same parent as the currently
	 * running project.  This seems like the sanest choice.
	 */
	proj = gtt_project_new_title_desc(name, desc);
	gtt_project_insert_after (proj, sib_prj);
	gtt_projects_tree_append_project (projects_tree, proj, gtt_project_get_parent (sib_prj));

	gtk_widget_destroy (GTK_WIDGET (w));
}

static void
free_data(GtkWidget *dlg, gpointer data)
{
	g_free(data);
}


void
new_project(GtkWidget *widget, gpointer data)
{
	GtkWidget *w, *t, *title, *d, *desc;
	GtkDialog *dlg;
	GtkBox *vbox;
	GtkWidget **entries = g_new0(GtkWidget *, 2);
	GtkWidget *table;

	title = gnome_entry_new("project_title");
	desc = gnome_entry_new("project_description");
	entries[0] = gnome_entry_gtk_entry(GNOME_ENTRY(title));
	entries[1] = gnome_entry_gtk_entry(GNOME_ENTRY(desc));

	/* Create new dialog box */
	w = gtk_dialog_new_with_buttons (
	             _("New Project..."),
	            // GTK_WINDOW (widget),
	            NULL,
		         GTK_DIALOG_MODAL,
		         NULL);
	g_signal_connect (G_OBJECT(w), "response",
	         G_CALLBACK (project_name_desc), entries);
	dlg = GTK_DIALOG(w);
	gtk_dialog_add_button (dlg, GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_add_button (dlg, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	vbox = GTK_BOX(dlg->vbox);

	/* Put stuff into the dialog box */
	t = gtk_label_new(_("Project Title"));
	d = gtk_label_new(_("Description"));

	table = gtk_table_new(2,2, FALSE);
	gtk_table_attach(GTK_TABLE(table), t,     0,1, 0,1,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), title, 1,2, 0,1,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), d,     0,1, 1,2,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), desc,  1,2, 1,2,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);

	gtk_box_pack_start(vbox, table, FALSE, FALSE, 2);
	gtk_widget_show(t);
	gtk_widget_show(title);
	gtk_widget_show(d);
	gtk_widget_show(desc);
	gtk_widget_show(table);

	gtk_widget_grab_focus(entries[0]);

	/* enter in first entry goes to next */
	g_signal_connect_object (G_OBJECT (entries[0]), "activate",
				   G_CALLBACK (gtk_widget_grab_focus),
				   GTK_OBJECT (entries[1]), 0);
	// gnome_dialog_editable_enters(GNOME_DIALOG(dlg),
	// 			     GTK_EDITABLE(entries[1]));

	g_signal_connect(G_OBJECT(dlg), "destroy",
			   G_CALLBACK(free_data),
			   entries);

	gtk_widget_show(GTK_WIDGET(dlg));
}

/* ======================================================= */
/* Project cut-n-paste GUI interactions. Support infinite
 * undo by using glist to store list of cuts. */

/* XXX hack alert -- we should delete this list during shutdown. */
static GList *cutted_project_list = NULL;

gboolean
have_cutted_project (void)
{
	return (NULL==cutted_project_list) ? FALSE : TRUE;
}

static inline void
debug_print_cutted_proj_list (char * str)
{
#ifdef DEBUG
	GList *n;
	printf ("proj list --- \n");
	if (NULL == cutted_project_list)
	{
		printf ("%s: project list is empty\n", str);
	}
	for (n=cutted_project_list; n; n=n->next)
	{
		GttProject *p = n->data;
		printf ("%s: n=%p prj=%p title=%s\n", str,n,p,gtt_project_get_title(p));
	}
	printf ("\n");
#endif
}

void
cut_project(GtkWidget *w, gpointer data)
{
	GttProject *cut_prj;

	cut_prj = gtt_projects_tree_get_selected_project (projects_tree);
	if (!cut_prj) return;

	cutted_project_list = g_list_prepend (cutted_project_list, cut_prj);
	debug_print_cutted_proj_list ("cut");

	/* Clear out relevent GUI elements. */
	prop_dialog_set_project(NULL);

	if (cut_prj == cur_proj) gen_stop_timer ();
	gtt_project_remove(cut_prj);
	gtt_projects_tree_remove_project (projects_tree, cut_prj);

	menu_set_states();      /* To enable paste menu item */
	toolbar_set_states();
}



void
paste_project(GtkWidget *w, gpointer data)
{
	GttProject *sib_prj;
	GttProject *p;

	sib_prj = gtt_projects_tree_get_selected_project (projects_tree);

	debug_print_cutted_proj_list ("pre paste");

	if (!cutted_project_list) return;
	p = cutted_project_list->data;

	if (NULL == cutted_project_list->next)
	{
		/* If we paste a second time, we better paste a copy ... */
		cutted_project_list->data = gtt_project_dup(p);
	}
	else
	{
		/* Pop element off the top. */
		cutted_project_list->data = NULL;
		cutted_project_list =
		     g_list_delete_link(cutted_project_list, cutted_project_list);
	}
	debug_print_cutted_proj_list ("post paste");

	/* Insert before the focus proj */
	gtt_project_insert_before (p, sib_prj);
	gtt_projects_tree_insert_project_before (projects_tree, p, sib_prj);
}



void
copy_project(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = gtt_projects_tree_get_selected_project (projects_tree);

	if (!prj) return;

	prj = gtt_project_dup(prj);

	/* Hitting copy has effect of completely trashing
	 * the list of earlier cut projects.  We do this in order
	 * to allow the most recently copied project to be pasted
	 * multiple times.  */
	GList *n = cutted_project_list;
	for (n=cutted_project_list; n; n=n->next)
	{
		GttProject *p = n->data;
		gtt_project_destroy (p);
	}
	g_list_free (cutted_project_list);

	cutted_project_list = g_list_prepend (NULL, prj);
	debug_print_cutted_proj_list ("copy");

	/* Update various subsystems */
	menu_set_states();      /* to enable paste menu item */
	toolbar_set_states();
}


/* ======================================================= */
/* Timer related menu functions */

void
gen_start_timer(void)
{
	GttProject *prj;
	prj = gtt_projects_tree_get_selected_project (projects_tree);
	cur_proj_set (prj);
}


void
gen_stop_timer(void)
{
	cur_proj_set (NULL);
}

void
menu_start_timer(GtkWidget *w, gpointer data)
{
	gen_start_timer();
}


void
menu_stop_timer(GtkWidget *w, gpointer data)
{
	gen_stop_timer();
}


void
menu_toggle_timer(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = gtt_projects_tree_get_selected_project (projects_tree);

	if (timer_is_running()) {
		cur_proj_set (NULL);
	} else {
		cur_proj_set (prj);
	}
}


void
menu_options(GtkWidget *w, gpointer data)
{
	prefs_dialog_show();
}



void
menu_properties(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = gtt_projects_tree_get_selected_project (projects_tree);

	if (prj) {
		prop_dialog_show(prj);
	}
	else
	{
		GtkWidget *dlg = gtk_message_dialog_new (GTK_WINDOW (app_window),
												 GTK_DIALOG_MODAL,
												 GTK_MESSAGE_INFO,
												 GTK_BUTTONS_OK,
												 _("You must select the project of which you want to edit the properties"));
		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);
	}
}


/* Cheesey usability hack to tell the user how to edit the timer
 * intervals.  Replace with something intuitive at earliest convenience.
 */

void
menu_howto_edit_times (GtkWidget *w,gpointer data)
{
	char * msg;

	msg = _("To edit the timer interval for this project,\n"
	        "open the Activity window and click on a link.\n"
	        "This will bring up a menu of time editing options.\n");

	GtkWidget *mb;
	mb = gtk_message_dialog_new (NULL,
	         GTK_DIALOG_MODAL,
	         GTK_MESSAGE_INFO,
	         GTK_BUTTONS_OK,
		      msg);
	gtk_dialog_run (GTK_DIALOG (mb));
	gtk_widget_destroy (mb);
	show_report (NULL, ACTIVITY_REPORT);
}


/* ============================ END OF FILE ======================= */
