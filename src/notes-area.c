/*   Notes Area display of project notes for GTimeTracker
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

#include <glade/glade.h>
#include <gnome.h>

#include "proj.h"
#include "notes-area.h"
#include "util.h"
#include "menus.h"

struct NotesArea_s
{
	GladeXML *gtxml;
	GtkPaned *vpane;   /* top level pane */
	GtkContainer *projects_tree_holder;   /* scrolled widget that holds the projects tree */

	GtkPaned *hpane;   /* left-right divider */

	GtkEntry *proj_title;
	GtkEntry *proj_desc;
	GtkTextView *proj_notes;
	
	GtkEntry *task_memo;
	GtkTextView *task_notes;

	GtkButton *close_proj;
	GtkButton *close_task;
	GtkButton *new_task;

	GttProject *proj;

	/* The goal of 'ignore events' is to prevent an inifinite
	 * loop of cascading events as we modify the project and the GUI.
	 */
	gboolean ignore_events;

	/* The goal of the freezes is to prevent more than one update
	 * of windows per second.  The problem is that without this,
	 * there would be one event per keystroke, which could cause
	 * a redraw of e.g. the journal window.  In such a case, even
	 * moderate typists on a slow CPU could saturate the CPU entirely.
	 */
	gboolean proj_freeze;
	GttTask * task_freeze;
};


/* ============================================================== */

#define TSK_SETUP                               \
	GttTask *tsk;                                \
	const char * str;                            \
	if (NULL == na->proj) return;                \
	if (na->ignore_events) return;               \
	                                             \
	na->ignore_events = TRUE;                    \
	tsk = gtt_project_get_first_task (na->proj); \
	if (NULL == tsk)                             \
	{                                            \
		tsk = gtt_task_new();                     \
		gtt_project_prepend_task (na->proj, tsk); \
	}                                            \
	if (tsk != na->task_freeze)                  \
	{                                            \
		/* Try to avoid race condition if another task */  \
		/* is created while this task is frozen. */        \
		if (NULL != na->task_freeze) gtt_task_thaw (na->task_freeze); \
		na->task_freeze = tsk;                    \
	}                                            \


static void
task_memo_changed (GtkEntry *entry, NotesArea *na)
{
	TSK_SETUP;
	str = gtk_entry_get_text (entry);
	gtt_task_set_memo (tsk, str);
	na->ignore_events = FALSE;
}

/* ============================================================== */

static void
task_notes_changed (GtkTextBuffer *entry, NotesArea *na)
{
	TSK_SETUP;
	str = xxxgtk_textview_get_text (na->task_notes);
	gtt_task_set_notes (tsk, str);
	na->ignore_events = FALSE;
}

/* ============================================================== */

#define PRJ_SETUP                        \
	const char * str;                     \
	if (NULL == na->proj) return;         \
	if (na->ignore_events) return;        \
	                                      \
	if (FALSE == na->proj_freeze)         \
	{                                     \
		na->proj_freeze = TRUE;            \
	}                                     \
	na->ignore_events = TRUE;             \

	
static void
proj_title_changed (GtkEntry *entry, NotesArea *na)
{
	PRJ_SETUP
	str = gtk_entry_get_text (entry);
	gtt_project_set_title (na->proj, str);
	na->ignore_events = FALSE;
}


/* ============================================================== */

static void
proj_desc_changed (GtkEntry *entry, NotesArea *na)
{
	PRJ_SETUP
	str = gtk_entry_get_text (entry);
	gtt_project_set_desc (na->proj, str);
	na->ignore_events = FALSE;
}


/* ============================================================== */

static void
proj_notes_changed (GtkTextBuffer *entry, NotesArea *na)
{
	PRJ_SETUP
	str = xxxgtk_textview_get_text (na->proj_notes);
	gtt_project_set_notes (na->proj, str);
	na->ignore_events = FALSE;
}

/* ============================================================== */
/* This routine will cause pending events to get delivered. */

void 
gtt_notes_timer_callback (NotesArea *na)
{
	if (!na) return;
	na->ignore_events = TRUE;
	if (na->task_freeze)
	{
		gtt_task_thaw (na->task_freeze);
		na->task_freeze = NULL;
	}
	if (na->proj_freeze)
	{
		na->proj_freeze = FALSE;
		gtt_project_thaw (na->proj);
	}
	na->ignore_events = FALSE;
}

/* ============================================================== */
/* These are some strange routines used to close off paned areas.
 * They are weird, they need to guess at sizes to close off the
 * panes.  This should really be a gkpaned built-in function.
 */

#define CLOSED_MARGIN 10

static void
close_proj_area (GtkButton *but, NotesArea *na)
{
	int hpane_width;
	int hpane_div;
	
	hpane_width = GTK_WIDGET(na->hpane)->allocation.width;
	hpane_div = gtk_paned_get_position (na->hpane);
	
	if (hpane_div > hpane_width -CLOSED_MARGIN)
	{
		int vpane_height;
		vpane_height = GTK_WIDGET(na->vpane)->allocation.height;
		gtk_paned_set_position (na->vpane, vpane_height);
	}
	else
	{
		gtk_paned_set_position (na->hpane, 0);
	}
}

static void
close_task_area (GtkButton *but, NotesArea *na)
{
	int hpane_width;
	int hpane_div;
	
	hpane_width = GTK_WIDGET(na->hpane)->allocation.width;
	hpane_div = gtk_paned_get_position (na->hpane);
	
	/* XXX we really need only the first test, but the second 
	 * one deals iwth a freaky gtk vpaned bug that makes this 
	 * hidden button active.  Whatever.
	 */ 
	if ((hpane_div < CLOSED_MARGIN) ||
	    (hpane_div > hpane_width -CLOSED_MARGIN))
	{
		int vpane_height;
		vpane_height = GTK_WIDGET(na->vpane)->allocation.height;
		gtk_paned_set_position (na->vpane, vpane_height);
	}
	else
	{
		gtk_paned_set_position (na->hpane, hpane_width);
	}
}

/* ============================================================== */

static void
new_task_cb (GtkButton *but, NotesArea *na)
{
	GttTask *tsk;
	if (NULL == na->proj) return;
	// if (na->ignore_events) return;
	
	// na->ignore_events = TRUE;
	tsk = gtt_task_new();
	gtt_project_prepend_task (na->proj, tsk);
	if (NULL != na->task_freeze) gtt_task_thaw (na->task_freeze); 
	gtt_task_freeze (tsk);
	na->task_freeze = tsk;

	// na->ignore_events = FALSE;
}

/* ============================================================== */

#define CONNECT_ENTRY(GLADE_NAME,CB)  ({                           \
	GtkEntry * entry;                                               \
	entry = GTK_ENTRY(glade_xml_get_widget (gtxml, GLADE_NAME));    \
	g_signal_connect (G_OBJECT (entry), "changed",                  \
	                G_CALLBACK (CB), dlg);                          \
   entry; })

#define CONNECT_TEXT(GLADE_NAME,CB)  ({                            \
	GtkTextView *tv;                                                \
	GtkTextBuffer *buff;                                            \
	tv = GTK_TEXT_VIEW(glade_xml_get_widget (gtxml, GLADE_NAME));   \
	buff = gtk_text_view_get_buffer (tv);                           \
	g_signal_connect (G_OBJECT (buff), "changed",                   \
	                G_CALLBACK (CB), dlg);                          \
   tv; })


NotesArea *
notes_area_new (void)
{
	NotesArea *dlg;
	GladeXML *gtxml;

	dlg = g_new0 (NotesArea, 1);

	gtxml = gtt_glade_xml_new ("glade/notes.glade", "top window");
	dlg->gtxml = gtxml;
	
	dlg->vpane = GTK_PANED(glade_xml_get_widget (gtxml, "notes vpane"));
	dlg->projects_tree_holder = GTK_CONTAINER(glade_xml_get_widget (gtxml, "ctree holder"));
	dlg->hpane = GTK_PANED(glade_xml_get_widget (gtxml, "leftright hpane"));
	dlg->close_proj = GTK_BUTTON(glade_xml_get_widget (gtxml, "close proj button"));
	dlg->close_task = GTK_BUTTON(glade_xml_get_widget (gtxml, "close diary button"));
	dlg->new_task = GTK_BUTTON(glade_xml_get_widget (gtxml, "new diary button"));
	
	dlg->proj_title = CONNECT_ENTRY ("proj title entry", proj_title_changed);
	dlg->proj_desc = CONNECT_ENTRY ("proj desc entry", proj_desc_changed);
	dlg->task_memo = CONNECT_ENTRY ("diary entry", task_memo_changed);

	dlg->proj_notes = CONNECT_TEXT ("proj notes textview", proj_notes_changed);
	dlg->task_notes = CONNECT_TEXT ("diary notes textview", task_notes_changed);
	

	g_signal_connect (G_OBJECT (dlg->close_proj), "clicked",
	                G_CALLBACK (close_proj_area), dlg);

	g_signal_connect (G_OBJECT (dlg->close_task), "clicked",
	                G_CALLBACK (close_task_area), dlg);

	g_signal_connect (G_OBJECT (dlg->new_task), "clicked",
	                G_CALLBACK (new_task_cb), dlg);

	gtk_widget_show (GTK_WIDGET(dlg->vpane));

	dlg->proj = NULL;
	dlg->ignore_events = FALSE;
	dlg->proj_freeze = FALSE;
	dlg->task_freeze = NULL;

	return dlg;
}

/* ============================================================== */
/* This routine copies data from the data engine, and pushes it 
 * into the GUI.  
 */ 

static void
notes_area_do_set_project (NotesArea *na, GttProject *proj)
{
	const char * str;
	GttTask *tsk;
	
	if (!na) return;
	if (na->ignore_events) return;

	/* Calling gtk_entry_set_text makes 'changed' events happen,
	 * which causes us to get the entry text, which exposes a gtk
	 * bug.  So we work around the bug and save cpu time by ignoring
	 * change events during a mass update. */
	na->ignore_events = TRUE;
	
	/* Note Bene its OK to have the proj be null: this has the
	 * effect of clearing all the fields out.
	 */
	na->proj = proj;
	
	/* Fetch data from the data engine, stuff it into the GUI. */
	str = gtt_project_get_title (proj);
	if (!str) str = "";
	gtk_entry_set_text (na->proj_title, str);
	
	str = gtt_project_get_desc (proj);
	if (!str) str = "";
	gtk_entry_set_text (na->proj_desc, str);

	str = gtt_project_get_notes (proj);
	if (!str) str = "";
	xxxgtk_textview_set_text (na->proj_notes, str);

	tsk = gtt_project_get_first_task (proj);
	str = gtt_task_get_memo (tsk);
	if (!str) str = "";
	gtk_entry_set_text (na->task_memo, str);
	
	str = gtt_task_get_notes (tsk);
	if (!str) str = "";
	xxxgtk_textview_set_text (na->task_notes, str);

	na->ignore_events = FALSE;
}

/* ============================================================== */

static void
redraw (GttProject *prj, gpointer data)
{
	NotesArea *na = data;
	if (na->ignore_events) return;
	notes_area_do_set_project (na, prj);
}

/* ============================================================== */

void
notes_area_set_project (NotesArea *na, GttProject *proj)
{
  if (na->proj != NULL) {
    gtt_project_remove_notifier (na->proj, redraw, na);
    na->proj = NULL;
  }
  if (proj != NULL) {
    gtt_project_add_notifier (proj, redraw, na);
  }

  notes_area_do_set_project (na, proj);
}

/* ============================================================== */

GtkWidget *
notes_area_get_widget (NotesArea *nadlg)
{
	if (!nadlg) return NULL;
	return GTK_WIDGET(nadlg->vpane);
}

static void
projects_tree_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
	NotesArea *nadlg = (NotesArea *) user_data;
	GttProjectsTree *gpt = GTT_PROJECTS_TREE (gtk_tree_selection_get_tree_view (selection));
	GttProject *prj = gtt_projects_tree_get_selected_project (gpt);

	notes_area_set_project (nadlg, prj);

}

static int
projects_tree_clicked (GtkWidget *ptree, GdkEvent *event, gpointer data)
{
    GdkEventButton *bevent = (GdkEventButton *) event;
	GttProjectsTree *projects_tree = GTT_PROJECTS_TREE (ptree);
	GtkMenuShell *menu;

    if (!(event->type == GDK_BUTTON_PRESS && bevent->button == 3))
    {
		return FALSE;
    }

	menu = menus_get_popup ();
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 3, bevent->time);

	return FALSE;

}

void 
notes_area_add_projects_tree (NotesArea *nadlg, GttProjectsTree *ptree)
{
	if (!nadlg) return;

	GtkTreeSelection *tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ptree));
	g_signal_connect (tree_selection, "changed", G_CALLBACK (projects_tree_selection_changed), nadlg);

	gtk_container_add (nadlg->projects_tree_holder, GTK_WIDGET(ptree));
	gtk_widget_show_all (GTK_WIDGET(nadlg->projects_tree_holder));

	g_signal_connect (GTK_WIDGET (ptree), "button_press_event",
			  G_CALLBACK (projects_tree_clicked), NULL);
}


void
notes_area_get_pane_sizes (NotesArea *na, int *vp, int *hp)
{
	if (!na) return;
	if(vp) *vp = gtk_paned_get_position (na->vpane);
	if(hp) *hp = gtk_paned_get_position (na->hpane);
}

void
notes_area_set_pane_sizes (NotesArea *na, int vp, int hp)
{
	if (!na) return;
	gtk_paned_set_position (na->vpane, vp);
	gtk_paned_set_position (na->hpane, hp);
}

/* ========================= END OF FILE ======================== */

