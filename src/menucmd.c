/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002 Linas Vepstas <linas@linas.org>
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
#include "ctree.h"
#include "cur-proj.h"
#include "dialog.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "proj.h"
#include "props-proj.h"
#include "timer.h"
#include "xml-gtt.h"


void
quit_app(GtkWidget *w, gpointer data)
{
	const char * errmsg;

	errmsg = save_all ();
	if (errmsg)
	{
		msgbox_ok(_("Warning"),
		     errmsg,
		     GTK_STOCK_OK,
		     GTK_SIGNAL_FUNC(gtk_main_quit));
		g_free ((gchar *) errmsg);
		return;
	}

	gtk_main_quit();
}



void
about_box(GtkWidget *w, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[] = {
		  "Eckehard Berns <eb@berns.i-s-o.net>",
		  "George Lebl <jirka@5z.com>",
		  "Linas Vepstas <linas@linas.org>",
		  "and dozens of bug-fixers",
		  NULL
	};
	const gchar *documentors[] = {
		"many documentors...",
		  NULL
	};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
	about = gnome_about_new(APP_NAME,
				    VERSION,
				    "Copyright (C) 1997,98 Eckehard Berns\n"
				    "Copyright (C) 2001,2002 Linas Vepstas",
#ifdef DEBUG
				    __DATE__ ", " __TIME__,
#else
 _("GTimeTracker is a combination stop-watch, diary, consultant billing "
   "system and project manager.  You can measure the amount of time you "
   "spend on a task, associate a memo with it, set a billing rate, print "
   "an invoice, as well as track that status of other projects."),

#endif
				    authors,
				    documentors,
				    "many translators",
				    NULL);
	gtk_signal_connect(GTK_OBJECT(about), "destroy",
		         GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
	gtk_widget_show(about);
}




static void
project_name_desc(GtkWidget *w, GtkEntry **entries)
{
	const char *name, *desc;
	GttProject *proj;
	GttProject *sib_prj;
	
	sib_prj = ctree_get_focus_project (global_ptw);

	if (!(name = gtk_entry_get_text(entries[0]))) return;
	if (!(desc = gtk_entry_get_text(entries[1]))) return;
	if (!name[0]) return;

	/* New project will have the same parent as the currently
	 * running project.  This seems like the sanest choice.
	 */
	proj = gtt_project_new_title_desc(name, desc);
	gtt_project_insert_after (proj, sib_prj);
	ctree_insert_after (global_ptw, proj, sib_prj);
}

static void
free_data(GtkWidget *dlg, gpointer data)
{
	g_free(data);
}


void
new_project(GtkWidget *widget, gpointer data)
{
	GtkWidget *dlg, *t, *title, *d, *desc;
	GtkBox *vbox;
	GtkWidget **entries = g_new0(GtkWidget *, 2);
	GtkWidget *table;

	title = gnome_entry_new("project_title");
	desc = gnome_entry_new("project_description");
	entries[0] = gnome_entry_gtk_entry(GNOME_ENTRY(title));
	entries[1] = gnome_entry_gtk_entry(GNOME_ENTRY(desc));

	new_dialog_ok_cancel(_("New Project..."), &dlg, &vbox,
			     GTK_STOCK_OK,
			     GTK_SIGNAL_FUNC(project_name_desc),
				 entries,
			     GTK_STOCK_CANCEL, NULL, NULL);

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
	gtk_signal_connect_object (GTK_OBJECT (entries[0]), "activate",
				   GTK_SIGNAL_FUNC (gtk_widget_grab_focus),
				   GTK_OBJECT (entries[1]));
	gnome_dialog_editable_enters(GNOME_DIALOG(dlg),
				     GTK_EDITABLE(entries[1]));

	gtk_signal_connect(GTK_OBJECT(dlg), "destroy",
			   GTK_SIGNAL_FUNC(free_data),
			   entries);
	
	gtk_widget_show(dlg);
}

/* ======================================================= */


GttProject *cutted_project = NULL;

void
cut_project(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (!prj) return;
	if (cutted_project)
	{
		gtt_project_destroy(cutted_project);
	}
	cutted_project = prj;
	prop_dialog_set_project(NULL);

	gtt_project_remove(cutted_project);
	if (cutted_project == cur_proj) cur_proj_set(NULL);
	ctree_remove(global_ptw, cutted_project);
}



void
paste_project(GtkWidget *w, gpointer data)
{
	GttProject *sib_prj;
	GttProject *p;
	
	sib_prj = ctree_get_focus_project (global_ptw);

	if (!cutted_project) return;
	p = cutted_project;

	/* if we paste a second time, we better paste a copy ... */
	cutted_project = gtt_project_dup(cutted_project);

	/* insert before the focus proj */
	gtt_project_insert_before (p, sib_prj);

	if (!sib_prj) 
	{
		/* top-level insert */
		ctree_add(global_ptw, p, NULL);
		return;
	}
	ctree_insert_before(global_ptw, p, sib_prj);
}



void
copy_project(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (!prj) return;

	if (cutted_project) 
	{
		gtt_project_destroy(cutted_project);
	}
	cutted_project = gtt_project_dup(prj);
	menu_set_states(); /* to enable paste */
}




/*
 * timer related menu functions
 */

void
menu_start_timer(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);
	cur_proj_set (prj);
}



void
menu_stop_timer(GtkWidget *w, gpointer data)
{
	cur_proj_set (NULL);
}


void
menu_toggle_timer(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	/* if (GTK_CHECK_MENU_ITEM(menus_get_toggle_timer())->active) { */
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
	prj = ctree_get_focus_project (global_ptw);

	if (prj) {
		prop_dialog_show(prj);
	}
}



void
menu_clear_daily_counter(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	gtt_clear_daily_counter (prj);
	ctree_update_label(global_ptw, prj);
}

/* ============================ END OF FILE ======================= */
