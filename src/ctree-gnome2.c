/*   GtkTreeView display of projects for the GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002 Linas Vepstas
 *
 *   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
 *   This is an attempted gnome-2 port of the ctree.c stuff, i.e.
 *   from the deprecated gtkctree to gtktreeview.  Its unfinished
 *   because gtktreeview seems to be missing a focus/cursor row,
 *   which is critical to the current operation of gtt.
 *
 *   Unfinished sections are marked by XXX's, and include:
 *   -- unfinished key/button bindings,
 *   -- unfinished drag-n-drop for reparenting
 *   -- unfinished expand/collapse
 *   -- unfinished column widths/text justification
 *
 *   Things I need to do, but can't figure out how:  
 *   -- I'd like to be able to draw a given row with different 
 *      colors/fonts/etc. i.e. to effectively highlight it 
 *      somehow to show its active.  How??
 *   -- Control left-right justification of column entries.
 *   -- Over-ride some the colors and arrow styles choosen by
 *      the gtk themes.
 *
 *   Things that we need that the old gtkctree does poorly:
 *   (reasons to abandon gtkctree):
 *   -- the drag-n-drop icon handling is atrocious.
 *   -- the drag-n-drop source & dest sometimes gets insanely
 *      confused.
 *   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
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

#ifdef GTT_CTREE_GNOME2

#include <config.h>
#include <gnome.h>

#include "app.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "gtt.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "props-proj.h"
#include "timer.h"
#include "util.h"

/* column types */
typedef enum {
	NULL_COL = 0,
	TIME_EVER_COL = 1,
	TIME_YEAR_COL,
	TIME_MONTH_COL,
	TIME_WEEK_COL,
	TIME_TODAY_COL,
	TIME_CURRENT_COL,
	TITLE_COL,
	DESC_COL,
	TASK_COL,
	START_COL,
	END_COL,
	DUE_COL,
	SIZING_COL,
	PERCENT_COL,
	URGENCY_COL,
	IMPORTANCE_COL,
	STATUS_COL,
} ColType;

#define NCOLS		20


/* this structure defines a specific 'row' in the 
 * tree view. */
typedef struct ProjTreeNode_s
{
	ProjTreeWindow *ptw;
	GttProject *prj;
	GtkTreeIter iter;

	char *col_values[NCOLS];

	char ever_timestr[24];
	char current_timestr[24];
	char day_timestr[24];
	char week_timestr[24];
	char month_timestr[24];
	char year_timestr[24];
	char start_timestr[24];
	char end_timestr[24];
	char due_timestr[24];
	char sizing_str[24];
	char percent_str[24];
} ProjTreeNode;

/* The object defining the overall view of projects */
struct ProjTreeWindow_s 
{
	GtkTreeStore *treestore;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;

	/* stuff that defines the column layout */
	ColType cols[NCOLS];
	char * col_titles[NCOLS];
	char * col_tooltips[NCOLS];
	GtkTooltips *col_tt_w[NCOLS];
	GtkJustification col_justify[NCOLS];
	gboolean col_width_set[NCOLS];
	int ncols;

	/* stuff we need to have handy while dragging */
	GdkDragContext *drag_context;
#if XXX
	GtkCTreeNode *source_ctree_node;
	GtkCTreeNode *parent_ctree_node;
	GtkCTreeNode *sibling_ctree_node;
#endif
};

static void stringify_col_values (ProjTreeNode *ptn, gboolean expand);
static void ctree_update_column_values (ProjTreeWindow *ptw, ProjTreeNode *ptn, gboolean expand);
static void ctree_update_row (ProjTreeWindow *ptw, ProjTreeNode *ptn);

/* ============================================================== */
/* Utility function; get the project associated with this row */

static ProjTreeNode *
get_ptn_from_row (ProjTreeWindow *ptw, GtkTreeIter *iter)
{
	GValue val =  {G_TYPE_INVALID};
	ProjTreeNode *ptn;

	gtk_tree_model_get_value (GTK_TREE_MODEL(ptw->treestore), 
	                iter, ptw->ncols, &val);
	ptn = (ProjTreeNode *) g_value_get_pointer (&val);
	return ptn;
}

static ProjTreeNode *
get_ptn_from_path (ProjTreeWindow *ptw, GtkTreePath *path)
{
	GValue val =  {G_TYPE_INVALID};
	ProjTreeNode *ptn;
	GtkTreeIter iter;
	gboolean rc;

	rc = gtk_tree_model_get_iter (GTK_TREE_MODEL(ptw->treestore),
	                &iter, path);					 
printf ("duuude iter for path=%d\n", rc);
	if (FALSE == rc) return NULL;
	gtk_tree_model_get_value (GTK_TREE_MODEL(ptw->treestore), 
	                &iter, ptw->ncols, &val);
	ptn = (ProjTreeNode *) g_value_get_pointer (&val);
	return ptn;
}

/* ============================================================== */
/* Simple functions encapsulating what happens when timer is 
 * started and stopped.   Takes as input a pointer to the 
 * row.
 */

static void
start_timer_for_row (ProjTreeWindow *ptw, GtkTreeIter *iter)
{
	ProjTreeNode *ptn;
	ptn = get_ptn_from_row (ptw, iter);
	cur_proj_set(ptn->prj);
	gtt_project_timer_update (ptn->prj);
	ctree_update_label (ptn->ptw, ptn->prj);
}

static void
stop_timer_for_row (ProjTreeWindow *ptw, GtkTreeIter *iter)
{
	ProjTreeNode *ptn;
	ptn = get_ptn_from_row (ptw, iter);
	if (ptn->prj != cur_proj) return;
	cur_proj_set(NULL);
	gtt_project_timer_update (ptn->prj);
	ctree_update_label (ptn->ptw, ptn->prj);
}

/* ============================================================== */
/* return value: true if we want widget to ignore this event,
 * false if we want the widget to handle it.
 */

static int
widget_key_event(GtkTreeView *ctree, GdkEvent *event, gpointer data)
{
	GdkEventKey *kev = (GdkEventKey *)event;

printf ("duuude key eve\n");
#if XXX
	GtkCTreeNode *rownode;
	if (event->type != GDK_KEY_RELEASE) return FALSE;
	switch (kev->keyval)
	{
		case GDK_Return:
			rownode = gtk_ctree_node_nth (ctree,  GTK_CLIST(ctree)->focus_row);
			if (rownode)
			{
				ProjTreeNode *ptn;
				ptn = gtk_ctree_node_get_row_data(ctree, rownode);
				if ((ptn->prj == cur_proj) && timer_is_running())
				{
					gtk_ctree_unselect (ctree, rownode);
					cur_proj_set (NULL);
				}
				else
				{
					gtk_ctree_select (ctree, rownode);
					cur_proj_set (ptn->prj);
				}
				gtt_project_timer_update (ptn->prj);
				ctree_update_label (ptn->ptw, ptn->prj);
			}
			return TRUE;
		case GDK_Up:
		case GDK_Down:
			return FALSE;
		case GDK_Left:
			rownode = gtk_ctree_node_nth (ctree,  GTK_CLIST(ctree)->focus_row);
			gtk_ctree_collapse (ctree, rownode);
			return TRUE;
		case GDK_Right:
			rownode = gtk_ctree_node_nth (ctree,  GTK_CLIST(ctree)->focus_row);
			gtk_ctree_expand (ctree, rownode);
			return TRUE;
		default:
			return FALSE;
	}
#endif // XXX
	return FALSE;
}

/* ============================================================== */
/* This function handles raw mouse-button press events. 
 *
 * Here's the desired user interface behaviour:
 * Left-click on a project starts or stops the timer for that project.
 */ 

static int
widget_button_event(GtkTreeView *ctree, GdkEvent *event, gpointer data)
{
	ProjTreeWindow *ptw = data;
	GdkEventButton *bevent = (GdkEventButton *)event;
	GtkWidget *menu;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean have_selection;
	
	if ((GDK_BUTTON_PRESS == event->type) && (1 == bevent->button))
	{
printf ("duude left button\n");
		selection = gtk_tree_view_get_selection (ptw->treeview);
		have_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
		if (have_selection)
		{
			stop_timer_for_row (ptw, &iter);
			gtk_tree_selection_unselect_all (selection);
			
			/* Don't let GtkTreeSelection get its mits on this one. */
			return TRUE;   
		}

		/* Do let GtkTreeSelection do its thing with this event */
		return FALSE;
	}
printf ("duuude other button eve\n");

#if XXX
	/* The only button event we handle are right-mouse-button,
	 * end double-click-left-mouse-button. */
	if (!((event->type == GDK_2BUTTON_PRESS && bevent->button==1) ||
	      (event->type == GDK_BUTTON_PRESS && bevent->button==3)))
		return FALSE;

	gtk_clist_get_selection_info(clist,bevent->x,bevent->y,&row,&column);
	if (0 > row) return FALSE;
	
	/* change the focus row */
	gtk_clist_freeze(clist);
	clist->focus_row = row;
	gtk_clist_thaw(clist);

	if (event->type == GDK_2BUTTON_PRESS) 
	{
		/* double-click left mouse edits the project.
		 * but maybe we want to change double-click to 
		 * something more useful ... */
		GttProject *prj;
		prj = ctree_get_focus_project (ptw);
		prop_dialog_show (prj);
	} 
	else 
	{
		/* right mouse button brings up popup menu */
		menu = menus_get_popup();
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, bevent->time);
	}
#endif XXX
	return FALSE;
}

/* ============================================================== */
/* If the selection changed (i.e. it was a single left-mouse click)
 * we will start the project timer.  Left-mouse clicks that stop
 * the project are handled by the raw-button-press handler.
 */

static void 
tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{ 
	ProjTreeWindow *ptw = data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean have_selection;
	
	have_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
	if (have_selection)
	{
		start_timer_for_row (ptw, &iter);
	}
}

/* ============================================================== */

GttProject *
ctree_get_focus_project (ProjTreeWindow *ptw)
{
	GttProject * proj = NULL;
	if (!ptw) return NULL;

#if XXX
	rownode = gtk_ctree_node_nth (ptw->ctree,  GTK_CLIST(ptw->ctree)->focus_row);
	if (rownode)
	{
		ProjTreeNode *ptn;
		ptn = gtk_ctree_node_get_row_data(ptw->ctree, rownode);
		if (ptn) proj = ptn->prj;
	}
#endif
	{
		ProjTreeNode *ptn;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor (ptw->treeview, &path, &col);
printf ("duuude path is %s\n", gtk_tree_path_to_string(path));
ptn = get_ptn_from_path (ptw, path);
		if (ptn) proj = ptn->prj;
	}

	return proj;
}

/* ============================================================== */

#if XXX
static void 
tree_expand (GtkCTree *ctree, GtkCTreeNode *row)
{
	int i;
	ProjTreeNode *ptn = gtk_ctree_node_get_row_data(ctree, row);
	ctree_update_column_values (ptw, ptn, TRUE);
}

static void 
tree_collapse (GtkCTree *ctree, GtkCTreeNode *row)
{
	int i;
	ProjTreeNode *ptn = gtk_ctree_node_get_row_data(ctree, row);
	ctree_update_column_values (ptw, ptn, FALSE);
}
#endif

/* ============================================================== */

#if XXX

static void
click_column(GtkCList *clist, gint col, gpointer data)
{
	ProjTreeWindow *ptw = data;
	ColType ct;

	if ((0 > col) || (ptw->ncols <= col)) return;
	ct = ptw->cols[col];
	switch (ct)
	{
		case TIME_EVER_COL:
			project_list_sort_ever();
			break;
		case TIME_CURRENT_COL:
			project_list_sort_current();
			break;
		case TIME_TODAY_COL:
			project_list_sort_day();
			break;
		case TIME_WEEK_COL:
			project_list_sort_week();
			break;
		case TIME_MONTH_COL:
			project_list_sort_month();
			break;
		case TIME_YEAR_COL:
			project_list_sort_year();
			break;
		case TITLE_COL:
			project_list_sort_title();
			break;
		case DESC_COL:
			project_list_sort_desc();
			break;
		case TASK_COL:
			break;
		case NULL_COL:
			break;
		case START_COL:
			project_list_sort_start();
			break;
		case END_COL:
			project_list_sort_end();
			break;
		case DUE_COL:
			project_list_sort_due();
			break;
		case SIZING_COL:
			project_list_sort_sizing();
			break;
		case PERCENT_COL:
			project_list_sort_percent();
			break;
		case URGENCY_COL:
			project_list_sort_urgency();
			break;
		case IMPORTANCE_COL:
			project_list_sort_importance();
			break;
		case STATUS_COL:
			project_list_sort_status();
			break;
	}
	
	ctree_setup(ptw);
}
#endif

/* ============================================================== */
/* Attempt to change pixmaps to indicate whether dragged project
 * will be a peer, or a child, of the project its dragged onto.
 * Right now, this code ain't pretty; there must be a better way ...  
 */
static GtkWidget *parent_pixmap = NULL;
static GtkWidget *sibling_pixmap = NULL;

#if XXX
/* we need to drag context to flip the pixmaps */
static void
drag_begin (GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	ProjTreeWindow *ptw = data;
	ptw->drag_context = context;
}

static void 
drag_drop (GtkWidget *widget, GdkDragContext *context,
           gint x, gint y, guint time, gpointer data)
{
	ProjTreeWindow *ptw = data;
	ProjTreeNode *ptn;
	GttProject *src_prj, *par_prj=NULL, *sib_prj=NULL;
	GttProject *old_parent=NULL;
	GtkCTree *ctree = GTK_CTREE(widget);

	if (!ptw->source_ctree_node) return;

	/* sanity check. We expect a source node, and either 
	 * a parent, or a sibling */
	ptn = gtk_ctree_node_get_row_data(ctree, ptw->source_ctree_node);
	src_prj = ptn->prj;
	if (!src_prj) return;

	if (ptw->parent_ctree_node)
	{
		ptn = gtk_ctree_node_get_row_data(ctree, ptw->parent_ctree_node);
		par_prj = ptn->prj;
	}
	if (ptw->sibling_ctree_node)
	{
		ptn = gtk_ctree_node_get_row_data(ctree, ptw->sibling_ctree_node);
		sib_prj = ptn->prj;
	}

	if (!par_prj && !sib_prj) return;

	old_parent = gtt_project_get_parent (src_prj);

	if (sib_prj)
	{
		gtt_project_insert_before (src_prj, sib_prj);

		/* if collapsed we need to update the time */
		ctree_update_label (ptw, gtt_project_get_parent(src_prj));
	}
	else if (par_prj)
	{
		gtt_project_append_project (par_prj, src_prj);
		ctree_update_label (ptw, par_prj);
	}

	/* Make sure we update the timestamps for the old parent
	 * from which we were cutted. */
	ctree_update_label (ptw, old_parent);

	ptw->source_ctree_node = NULL;
	ptw->parent_ctree_node = NULL;
	ptw->sibling_ctree_node = NULL;
	ptw->drag_context = NULL;
}

/* ============================================================== */

static gboolean 
ctree_drag (GtkCTree *ctree, GtkCTreeNode *source_node, 
                             GtkCTreeNode *new_parent,
                             GtkCTreeNode *new_sibling)
{
	ProjTreeWindow *ptw = g_object_get_data (G_OBJECT(ctree), "ptw");

	if (!source_node) return FALSE;

	if (!parent_pixmap->window) 
	{
		gtk_widget_realize (parent_pixmap);
		gtk_widget_realize (sibling_pixmap);
	}

	/* Record the values. We don't actually reparent anything
	 * until the drag has completed. */
	ptw->source_ctree_node = source_node;
	ptw->parent_ctree_node = new_parent;
	ptw->sibling_ctree_node = new_sibling;

#if 0
{
printf ("draging %s\n", gtt_project_get_desc(
	gtk_ctree_node_get_row_data(ctree, source_ctree_node)->prj));
printf ("to parent %s\n", gtt_project_get_desc(
	gtk_ctree_node_get_row_data(ctree, parent_ctree_node)->prj));
printf ("before sibl %s\n\n", gtt_project_get_desc(
	gtk_ctree_node_get_row_data(ctree, sibling_ctree_node)->prj));
}
#endif
	/* Note, we must test for new sibling before new parent,
	 * since there can be a sibling *and* parent */
	if (new_sibling) 
	{
		/* if we have a sibling and a parent, and the parent
		 * is not expanded, then the visually correct thing 
		 * is to show 'subproject of parent; (i.e. downarrow)
		 */
		if (new_parent && (0 == GTK_CTREE_ROW(new_parent)->expanded)) 
		{
			/* down arrow means 'child of parent' */
			gtk_drag_set_icon_widget (ptw->drag_context, parent_pixmap, 10, 10);
			return TRUE;
		}

		/* left arrow means 'peer' (specifcally, 'insert before
		 * sibling') */
		gtk_drag_set_icon_widget (ptw->drag_context, sibling_pixmap, 10, 10);
		return TRUE;
	}

	if (new_parent) 
	{
		/* down arrow means 'child of parent' */
		gtk_drag_set_icon_widget (ptw->drag_context, parent_pixmap, 10, 10);
		return TRUE;
	}

	return FALSE;
}
#endif

/* ============================================================== */
/* note about column layout:
 * We have two ways of doing this: create a column-cell object,
 * and program in an object-oriented style.  The other way is to
 * set a column type, and use a big switch statement.  Beacuse this
 * is a small project, we'll just use a switch statement.  Seems
 * easier.
 *
 * Note also we have implemented a general system here, that 
 * allows us to show columns in any order, or the same column
 * more than once.   However, we do not actually make use of
 * this anywhere; at the moment, the column locations are fixed.
 */ 

static void 
ctree_init_cols (ProjTreeWindow *ptw)
{
	int i;

	/* init column types */
	i=0; ptw->cols[i] = IMPORTANCE_COL;
	i++; ptw->cols[i] = URGENCY_COL;
	i++; ptw->cols[i] = STATUS_COL;
	i++; ptw->cols[i] = TIME_EVER_COL;
	i++; ptw->cols[i] = TIME_YEAR_COL;
	i++; ptw->cols[i] = TIME_MONTH_COL;
	i++; ptw->cols[i] = TIME_WEEK_COL;
	i++; ptw->cols[i] = TIME_TODAY_COL;
	i++; ptw->cols[i] = TIME_CURRENT_COL;
	i++; ptw->cols[i] = TITLE_COL;
	i++; ptw->cols[i] = DESC_COL;
	i++; ptw->cols[i] = TASK_COL;
	i++; ptw->cols[i] = START_COL;
	i++; ptw->cols[i] = END_COL;
	i++; ptw->cols[i] = DUE_COL;
	i++; ptw->cols[i] = SIZING_COL;
	i++; ptw->cols[i] = PERCENT_COL;
	i++; ptw->cols[i] = NULL_COL;
	ptw->ncols = i;

	/* set column titles */
	for (i=0; NULL_COL != ptw->cols[i]; i++)
	{
		switch (ptw->cols[i])
		{
			case TIME_EVER_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Total");
				ptw->col_tooltips[i] = 
					_("Total time spent on this project.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_CURRENT_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Entry");
				ptw->col_tooltips[i] = 
					_("Time spent under the current "
					"diary entry.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_TODAY_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Today");
				ptw->col_tooltips[i] =  
					_("Time spent on this project today.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_WEEK_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Week");
				ptw->col_tooltips[i] = 
					_("Time spent on this project "
					"this week.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_MONTH_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Month");
				ptw->col_tooltips[i] =  
					_("Time spent on this project "
					"this month.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_YEAR_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Year");
				ptw->col_tooltips[i] = 
					_("Time spent on this project "
					"this year.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TITLE_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] =  _("Title");
				ptw->col_tooltips[i] =  _("Project Title");
				ptw->col_width_set[i] = FALSE;
				break;
			case DESC_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] =  _("Description");
				ptw->col_tooltips[i] =  _("Description");
				ptw->col_width_set[i] = FALSE;
				break;
			case TASK_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] =  _("Diary Entry");
				ptw->col_tooltips[i] = 
					_("The current diary entry");
				ptw->col_width_set[i] = FALSE;
				break;
			case START_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Start");
				ptw->col_tooltips[i] =  
					_("Estimated Project Start Date");
				ptw->col_width_set[i] = FALSE;
				break;
			case END_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("End");
				ptw->col_tooltips[i] =  
					_("Estimated Project Completion Date");
				ptw->col_width_set[i] = FALSE;
				break;
			case DUE_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Due");
				ptw->col_tooltips[i] =  
					_("Due Date: the date by which "
					"the project absolutely must be "
					"completed by.");
				ptw->col_width_set[i] = FALSE;
				break;
			case SIZING_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_RIGHT;
				ptw->col_titles[i] = _("Size");
				ptw->col_tooltips[i] = 
					_("Sizing: How much work it will "
					"take to finish this project.");
				ptw->col_width_set[i] = FALSE;
				break;
			case PERCENT_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_RIGHT;
				ptw->col_titles[i] =  _("Done");
				ptw->col_tooltips[i] = 
					_("Percent Complete: How much "
					"of this project has been "
					"finished till now.");
				ptw->col_width_set[i] = FALSE;
				break;
			case URGENCY_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] = _("U");
				ptw->col_tooltips[i] = 
					_("Urgency: Does this project need "
					"to be done soon, done quickly?");
				ptw->col_width_set[i] = FALSE;
				break;
			case IMPORTANCE_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] = _("Imp");
				ptw->col_tooltips[i] = 
					_("Importance: How important "
					"is this project?");
				ptw->col_width_set[i] = FALSE;
				break;
			case STATUS_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] = _("Status");
				ptw->col_tooltips[i] = 
					_("The current status of this project.");
				ptw->col_width_set[i] = FALSE;
				break;
			case NULL_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_LEFT;
				ptw->col_titles[i] =  "";
				ptw->col_tooltips[i] =  "";
				ptw->col_width_set[i] = FALSE;
				break;
		}
	}
}

/* ============================================================== */

static int
string_width(GtkWidget *w, const char * str)
{
	GtkStyle *style;
	GdkFont *font;
	GdkGC *gc;
	GdkGCValues vals;
	int width;

	style = gtk_widget_get_style(w);

	/* Fallback if the GC still isn't available */
	if (NULL == style) 
	{
		style = gtk_widget_get_default_style ();
		if (NULL == style) return 49;
	}

	/* Try really hard to find a font */
	gc = style->text_gc[0];
	if (NULL == gc) gc = style->fg_gc[0];
	if (NULL == gc) gc = style->bg_gc[0];
	if (NULL == gc) gc = style->base_gc[0];
	if (NULL == gc)
	{
		style = gtk_widget_get_default_style ();
		if (NULL == style) return 50;

		gc = style->text_gc[0];
		if (NULL == gc) gc = style->fg_gc[0];
		if (NULL == gc) gc = style->bg_gc[0];
		if (NULL == gc) gc = style->base_gc[0];
		if (NULL == gc) return 51;
	}
	
	gdk_gc_get_values(gc, &vals);
	font = vals.font;

	/* ha. Finally. Get the width. */
	width = gdk_string_width(font, str);

	return width;
}

/* Initialize to a reasonable column width, but only 
 * if it hasn't been set already.  */

static void
default_col_width (ProjTreeWindow *ptw, int col, const char * str)
{
	int width;
	if (TRUE == ptw->col_width_set[col]) return;

	width = string_width (GTK_WIDGET(ptw->treeview), str);
	ctree_set_col_width (ptw, col, width);
}

/* Note: we don't need i18n/l7n translations for most of the strings;
 * they are used only for column widths ... */
void 
ctree_update_column_visibility (ProjTreeWindow *ptw)
{
	int i;

	/* set column visibility */
	for (i=0; i <= ptw->ncols; i++)
	{
		GtkTreeViewColumn *col = gtk_tree_view_get_column (ptw->treeview, i);
		switch (ptw->cols[i])
		{
		case TITLE_COL:
			default_col_width (ptw, i, "-00:00:00");
			break;
		case TIME_EVER_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_ever);
			break;
		case TIME_CURRENT_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_current);
			break;
		case TIME_TODAY_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				 config_show_title_day);
			break;
		case TIME_WEEK_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_week);
			break;
		case TIME_MONTH_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col, 
				config_show_title_month);
			break;
		case TIME_YEAR_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_year);
			break;
		case DESC_COL:
			default_col_width (ptw, i, "Not too long");
			gtk_tree_view_column_set_visible (col, 
				config_show_title_desc);
			break;
		case TASK_COL:
			default_col_width (ptw, i, "Some longer string");
			gtk_tree_view_column_set_visible (col,
				config_show_title_task);
			break;
		case START_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_estimated_start);
			break;
		case END_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_estimated_end);
			break;
		case DUE_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_tree_view_column_set_visible (col,
				config_show_title_due_date);
			break;
		case SIZING_COL:
			default_col_width (ptw, i, "XXX.XX");
			gtk_tree_view_column_set_visible (col,
				config_show_title_sizing);
			break;
		case PERCENT_COL:
			default_col_width (ptw, i, "100%");
			gtk_tree_view_column_set_visible (col,
				config_show_title_percent_complete);
			break;
		case URGENCY_COL:
			default_col_width (ptw, i, "X");
			gtk_tree_view_column_set_visible (col,
				config_show_title_urgency);
			break;
		case IMPORTANCE_COL:
			default_col_width (ptw, i, "XXX");
			gtk_tree_view_column_set_visible (col,
				config_show_title_importance);
			break;
		case STATUS_COL:
			default_col_width (ptw, i, "abcedfg");
			gtk_tree_view_column_set_visible (col,
				config_show_title_status);
			break;
		case NULL_COL:
			gtk_tree_view_column_set_visible (col, FALSE);
			break;
		}

	}
}

/* ============================================================== */
/* The stringify_col_values() routine will "print" or convert to
 * strings the various parts of the project.  These strings are then
 * directly suitable for passing into the tree-store widget for
 * subsequent display.
 */

#define PRT_TIME(SLOT) {						\
	ptn->col_values[i] =  ptn->SLOT##_timestr;			\
	if (config_show_title_##SLOT) {					\
		time_t secs;						\
		if (expand) {						\
			secs = gtt_project_get_secs_##SLOT(prj);	\
		} else {						\
			secs = gtt_project_total_secs_##SLOT(prj); 	\
		}							\
		if (0 < secs || ((0 == secs) && (prj == cur_proj))) { 	\
			print_hours_elapsed (ptn->SLOT##_timestr, 24,	\
				secs, config_show_secs);		\
		} else {						\
			ptn->SLOT##_timestr[0] = '-';			\
			ptn->SLOT##_timestr[1] = 0x0;			\
		}							\
	}								\
}

static void 
stringify_col_values (ProjTreeNode *ptn, gboolean expand)
{
	GttProject *prj = ptn->prj;
	ProjTreeWindow *ptw = ptn->ptw;
	int i;

	/* set column values */
	for (i=0; NULL_COL != ptw->cols[i]; i++)
	{
		switch (ptw->cols[i])
		{
		case TIME_EVER_COL:
			PRT_TIME(ever);
			break;
		case TIME_CURRENT_COL:
			PRT_TIME(current);
			break;
		case TIME_TODAY_COL:
			PRT_TIME(day);
			break;
		case TIME_WEEK_COL:
			PRT_TIME(week);
			break;
		case TIME_MONTH_COL:
			PRT_TIME(month);
			break;
		case TIME_YEAR_COL:
			PRT_TIME(year);
			break;
		case TITLE_COL:
			ptn->col_values[i] = 
				(char *) gtt_project_get_title(prj);
			break;
		case DESC_COL:
			if (config_show_title_desc) {
				ptn->col_values[i] = 
					(char *) gtt_project_get_desc(prj);
			}
			break;
		case TASK_COL:
			if (config_show_title_task) {
				GList *leadnode;
				leadnode = gtt_project_get_tasks (prj);
				if (leadnode)
				{
					ptn->col_values[i] =  
					(char *) gtt_task_get_memo (leadnode->data);
				}
				else
				{
					ptn->col_values[i] = "";
				}
			}
			break;
		case START_COL:
			ptn->col_values[i] =  ptn->start_timestr;
			if (config_show_title_estimated_start) {
				time_t secs;
				secs = gtt_project_get_estimated_start(prj);
				if (0 < secs) {
					print_date (ptn->start_timestr, 24, secs);
				} else {
					ptn->start_timestr[0] = '-';
					ptn->start_timestr[1] = 0x0;
				}
			}
			break;
		case END_COL:
			ptn->col_values[i] =  ptn->end_timestr;
			if (config_show_title_estimated_end) {
				time_t secs;
				secs = gtt_project_get_estimated_end(prj);
				if (0 < secs) {
					print_date (ptn->end_timestr, 24, secs);
				} else {
					ptn->end_timestr[0] = '-';
					ptn->end_timestr[1] = 0x0;
				}
			}
			break;
		case DUE_COL:
			ptn->col_values[i] =  ptn->due_timestr;
			if (config_show_title_due_date) {
				time_t secs;
				secs = gtt_project_get_due_date(prj);
				if (0 < secs) {
					print_date (ptn->due_timestr, 24, secs);
				} else {
					ptn->due_timestr[0] = '-';
					ptn->due_timestr[1] = 0x0;
				}
			}
			break;
		case SIZING_COL:
			ptn->col_values[i] =  ptn->sizing_str;
			if (config_show_title_sizing) {
				int sz;
				sz = gtt_project_get_sizing(prj);
				if (0 < sz)
				{
					snprintf (ptn->sizing_str, 24, 
						"%.2f", (((double)sz)/3600.0));
				} else {
					ptn->sizing_str[0] = '-';
					ptn->sizing_str[1] = 0x0;
				}
			}
			break;
		case PERCENT_COL:
			ptn->col_values[i] =  ptn->percent_str;
			if (config_show_title_percent_complete) {
				int sz;
				sz = gtt_project_get_percent_complete(prj);
				snprintf (ptn->percent_str, 24, 
					"%d%%",sz);
			}
			break;
		case URGENCY_COL:
			if (config_show_title_urgency) {
				GttRank rank;
				rank = gtt_project_get_urgency (prj);
				switch (rank) {
			case GTT_UNDEFINED: ptn->col_values[i] = "-"; break;
			case GTT_LOW:    ptn->col_values[i] = _("3"); break;
			case GTT_MEDIUM: ptn->col_values[i] = _("2"); break;
			case GTT_HIGH:   ptn->col_values[i] = _("1"); break;
				}
			}
			break;
		case IMPORTANCE_COL:
			if (config_show_title_importance) {
				GttRank rank;
				rank = gtt_project_get_importance (prj);
				switch (rank) {
			case GTT_UNDEFINED: ptn->col_values[i] = "-"; break;
			case GTT_LOW:    ptn->col_values[i] = _("Low"); break;
			case GTT_MEDIUM: ptn->col_values[i] = _("Med"); break;
			case GTT_HIGH:   ptn->col_values[i] = _("High"); break;
				}
			}
			break;
		case STATUS_COL:
			if (config_show_title_status) {
				GttProjectStatus status;
				status = gtt_project_get_status (prj);
				switch (status) {
			case GTT_NO_STATUS:   ptn->col_values[i] = "-"; break;
			case GTT_NOT_STARTED: ptn->col_values[i] = _("Not Started"); break;
			case GTT_IN_PROGRESS: ptn->col_values[i] = _("In Progress"); break;
			case GTT_ON_HOLD:     ptn->col_values[i] = _("On Hold"); break;
			case GTT_CANCELLED:   ptn->col_values[i] = _("Cancelled"); break;
			case GTT_COMPLETED:   ptn->col_values[i] = _("Completed"); break;
				}
			}
			break;
		case NULL_COL:
			ptn->col_values[i] =  "";
			break;
		}
	}
}

/* ============================================================== */
/* update the values in the treestore/treeview for this project */

static void
ctree_update_column_values (ProjTreeWindow *ptw, ProjTreeNode *ptn, gboolean expand)
{
	int i;

	/* Generate strings for this project */
	stringify_col_values (ptn, expand);

	/* Copy the values into to treestore */
	for (i=0; i<ptw->ncols; i++)
	{
		GValue val = {G_TYPE_INVALID};
	 	g_value_init(&val, G_TYPE_STRING);
		g_value_set_string (&val, ptn->col_values[i]);
		gtk_tree_store_set_value (ptw->treestore, &ptn->iter, i, &val);
	}
}

/* ============================================================== */
/* Redraw one row, and one row only.  Expansion automatically determined. */

static void
ctree_update_row (ProjTreeWindow *ptw, ProjTreeNode *ptn)
{
	GtkTreePath * path;
	gboolean expand;

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(ptw->treestore), &ptn->iter);
	expand = gtk_tree_view_row_expanded (ptw->treeview, path);

	ctree_update_column_values (ptw, ptn, expand);
}

/* ============================================================== */
/* Redraw utility, recursively walks visibile project tree */

static void
refresh_list (ProjTreeWindow *ptw, GList *prjlist)
{
	GList *node;

	/* now, draw each project */
	for (node = prjlist; node; node = node->next) 
	{
		int i;
		gboolean expand;
		ProjTreeNode *ptn;
		GttProject *prj = node->data;
		GtkTreePath * path;

		ptn = gtt_project_get_private_data (prj);

		/* Determine if project is expanded -- this affects display of time totals */
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(ptw->treestore), &ptn->iter);
		expand = gtk_tree_view_row_expanded (ptw->treeview, path);
		
		ctree_update_column_values (ptw, ptn, expand);

		/* Should we show sub-projects? recurse */
		if (expand)
		{
			prjlist = gtt_project_get_children (prj);
			refresh_list (ptw, prjlist);
		}
	}
}

/* ============================================================== */
/* Reinsert new values for all visible projects in the project
 * window.  */

void
ctree_refresh (ProjTreeWindow *ptw)
{
	GList *prjlist;

	if (!ptw) return;

	/* freeze, in prep for a massive update */
	gtk_widget_freeze_child_notify (GTK_WIDGET(ptw->treeview));

	/* make sure the right set of columns are visibile */
	ctree_update_column_visibility (ptw);

	/* now, draw each project */
	prjlist = gtt_get_project_list();
	refresh_list (ptw, prjlist);

	gtk_widget_thaw_child_notify (GTK_WIDGET(ptw->treeview));
}

/* ============================================================== */
/* Redraw handler, redraws just one particular project/row */

static void
redraw (GttProject *prj, gpointer data)
{
	ProjTreeNode *ptn = data;
	ctree_update_row (ptn->ptw, ptn);
}

/* ============================================================== */

GtkWidget *
ctree_get_widget (ProjTreeWindow *ptw)
{
	if (!ptw) return NULL;
	return (GTK_WIDGET (ptw->treeview));
}

ProjTreeWindow *
ctree_new(void)
{
	ProjTreeWindow *ptw;
	GtkWidget *wimg;
	GtkWidget *w, *sw;
	int i;

	ptw = g_new0 (ProjTreeWindow, 1);
#if XXX
	ptw->source_ctree_node = NULL;
	ptw->parent_ctree_node = NULL;
	ptw->sibling_ctree_node = NULL;
	ptw->drag_context = NULL;
#endif

	ctree_init_cols (ptw);

	/* Set the column types.  We also need to store a bit of private
	 * data with each tree node; we will cheat our way into this by
	 * storing a pointer in a bogus column that is never displayed.
	 */
	{
		GType col_type[NCOLS];
		for (i=0;i<ptw->ncols;i++) { col_type[i] = G_TYPE_STRING; }
		col_type[ptw->ncols] = G_TYPE_POINTER;
		ptw->treestore = gtk_tree_store_newv (ptw->ncols+1, col_type);
	}
	w = gtk_tree_view_new_with_model (GTK_TREE_MODEL(ptw->treestore));
	ptw->treeview = GTK_TREE_VIEW (w);

	g_object_set_data (G_OBJECT(w), "ptw", ptw);

	/* Set up the columns in the treeview widget */
	for (i=0; i<ptw->ncols; i++)
	{
      GtkTreeViewColumn *col;
		GtkCellRenderer *renderer;
		
		renderer = gtk_cell_renderer_text_new ();

		col = gtk_tree_view_column_new_with_attributes (
							 ptw->col_titles[i],
							 renderer, "text", i, NULL);

		gtk_tree_view_insert_column (ptw->treeview, col, i);

		/* Find the column into which to put the expander */
		if (TITLE_COL == ptw->cols[i])
		{
			gtk_tree_view_set_expander_column (ptw->treeview, col);
		}
		/* XXX
		 * there seems to be no way to set left/right justification in 
		 * this widget set ?? 
		gtk_clist_set_column_justification(GTK_CLIST(w), 
			i, ptw->col_justify[i]);
		*/
	}

	/* Set up a bogus column to hold private data;
	 * this column is never actually drawn.
	 * We need to do this because there is currently 
	 * no other way to store private data together with
	 * a node in the tree.
	 */
	{
      GtkTreeViewColumn *col = gtk_tree_view_column_new ();
		gtk_tree_view_insert_column (ptw->treeview, col, ptw->ncols);
	}
	
	/* some columns are quite narrow, so put tooltips over them. */
	/* XXX
	for (i=0; i<ptw->ncols; i++)
	{
		GtkTooltips *tt;
		tt = gtk_tooltips_new();
		gtk_tooltips_enable (tt);
		gtk_tooltips_set_tip (tt, GTK_CLIST(w)->column[i].button,
			ptw->col_tooltips[i], NULL);
		ptw->col_tt_w[i] = tt;
	}
	*/

	/* Not all of the columns are visible; only some are.  Set this */
	ctree_update_column_visibility (ptw);
	
	/*   XXX  uhh, isn't this overriding some other geom hint?? thats bad */
	gtk_widget_set_size_request(w, -1, 120);

	/* create the top-level window to hold the c-tree */
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (sw), w);
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (sw),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show_all (sw);

	/* Only one row can be highlighted at a time */
	ptw->selection = gtk_tree_view_get_selection (ptw->treeview);
	gtk_tree_selection_set_mode (ptw->selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (ptw->selection), "changed", 
		                G_CALLBACK (tree_selection_changed_cb), ptw);

	
	g_signal_connect(G_OBJECT(w), "button_press_event",
			   G_CALLBACK(widget_button_event), ptw);
	g_signal_connect(G_OBJECT(w), "key_release_event",
			   G_CALLBACK(widget_key_event), ptw);
#if XXX
	g_signal_connect(G_OBJECT(w), "click_column",
			   G_CALLBACK(click_column), ptw);
	g_signal_connect(G_OBJECT(w), "tree_expand",
			   G_CALLBACK(tree_expand), NULL);
	g_signal_connect(G_OBJECT(w), "tree_collapse",
			   G_CALLBACK(tree_collapse), NULL);

	g_signal_connect(G_OBJECT(w), "drag_begin",
			   G_CALLBACK(drag_begin), ptw);

	g_signal_connect(G_OBJECT(w), "drag_drop",
			   G_CALLBACK(drag_drop), ptw);
#endif

	/* allow projects to be re-arranged by dragging */
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(w), TRUE);

	/* XXX
	gtk_clist_set_use_drag_icons (GTK_CLIST(w), TRUE);
	gtk_ctree_set_drag_compare_func (GTK_CTREE(w), ctree_drag);
	*/

#if XXX
	/* Desperate attempt to clue the user into how the 
	 * dragged project will be reparented. We show left or
	 * down arrows, respecitvely.  If anyone can come up with
	 * something more elegant, then please ...  */
	parent_pixmap = gtk_window_new (GTK_WINDOW_POPUP);
	sibling_pixmap = gtk_window_new (GTK_WINDOW_POPUP);

	{
		#include "down.xpm"
		wimg = gnome_pixmap_new_from_xpm_d (down);
		gtk_widget_ref(wimg);
		gtk_widget_show(wimg);
		gtk_container_add (GTK_CONTAINER(parent_pixmap), wimg);
	}
	{
		#include "left.xpm"
		wimg = gnome_pixmap_new_from_xpm_d (left);
		gtk_widget_ref(wimg);
		gtk_widget_show(wimg);
		gtk_container_add (GTK_CONTAINER(sibling_pixmap), wimg);
	}
#endif

	return ptw;
}


/* ========================================================= */

void
ctree_setup (ProjTreeWindow *ptw)
{
	GList *node, *prjlist;
	GttProject *running_project = cur_proj;

	if (!ptw) return;
	cur_proj_set (NULL);

	/* Start with a clean slate */
	gtk_tree_store_clear (ptw->treestore);

	/* Next, add all projects to the tree store */
	prjlist = gtt_get_project_list();
	if (prjlist) 
	{
		gtk_widget_freeze_child_notify (GTK_WIDGET(ptw->treeview));
		for (node = prjlist; node; node = node->next) 
		{
			GttProject *prj = node->data;
			ctree_add(ptw, prj, NULL);
		}
		gtk_widget_thaw_child_notify (GTK_WIDGET(ptw->treeview));
	}

	/* Next, highlight the current project, and expand 
	 * the tree branches to it, if needed, so that its visibile. */
	if (running_project) cur_proj_set (running_project);
	if (cur_proj) 
	{
		GttProject *parent;
		ProjTreeNode *ptn;

		parent = gtt_project_get_parent (cur_proj);
		while (parent) 
		{
			GtkTreePath * path;
			ptn = gtt_project_get_private_data (parent);

			path = gtk_tree_model_get_path(GTK_TREE_MODEL(ptw->treestore), &ptn->iter);
			gtk_tree_view_expand_row (ptw->treeview, path, FALSE);
			
			parent = gtt_project_get_parent (parent);
		}

		ptn = gtt_project_get_private_data (cur_proj);
		gtk_tree_selection_select_iter (ptw->selection, &ptn->iter);
	}

	menu_set_states();

#if XXX
	if (config_show_clist_titles)
	{
		gtk_clist_column_titles_show(GTK_CLIST(tree_w));
	}
	else
	{
		gtk_clist_column_titles_hide(GTK_CLIST(tree_w));
	}

	if (config_show_subprojects)
	{
		gtk_ctree_set_line_style(tree_w, GTK_CTREE_LINES_SOLID);
		gtk_ctree_set_expander_style(tree_w, GTK_CTREE_EXPANDER_SQUARE);
	}
	else
	{
		gtk_ctree_set_line_style(tree_w, GTK_CTREE_LINES_NONE);
		gtk_ctree_set_expander_style(tree_w, GTK_CTREE_EXPANDER_NONE);
	}

	if (cur_proj) 
	{
		ProjTreeNode *ptn;
		ptn = gtt_project_get_private_data (cur_proj);
		gtk_ctree_node_moveto(tree_w, ptn->ctnode, -1,
				 0.5, 0.0);
		/* hack alert -- we should set the focus row 
		 * here as well */
	}
#endif 
	gtk_widget_queue_draw(GTK_WIDGET(ptw->treeview));
}

/* ========================================================= */

void
ctree_destroy (ProjTreeWindow *ptw)
{

	/* hack alert -- this function is not usable in its
	 * current form */
	g_warning ("incompletely implemented.  should probably "
		"traverse and destroy ptn's\n");
	// ctree_remove (ptw, ptn->prj);

	gtk_widget_destroy (GTK_WIDGET(ptw->treeview));
	gtk_widget_destroy (GTK_WIDGET(ptw->treestore));
	ptw->treeview = NULL;
	ptw->treestore = NULL;
	ptw->selection = NULL;
	ptw->ncols = 0;
	g_free (ptw);
}

/* ============================================================== */
/* This routine adds the indicated project to the GtkTree store/view
 * window. */

void
ctree_add (ProjTreeWindow *ptw, GttProject *p, GtkTreeIter *parent)
{
	int i;
	ProjTreeNode *ptn;
	GList *n;

	/* Setup a back-pointer between the project and the 
	 * row widget, so that we can later find ourselves */
	ptn = gtt_project_get_private_data (p);
	if (!ptn)
	{
		ptn = g_new0 (ProjTreeNode, 1);
		ptn->ptw = ptw;
		ptn->prj = p;
		gtt_project_set_private_data (p, ptn);
		gtt_project_add_notifier (p, redraw, ptn);
	}

	/* Add the project to the tail end of the list */
	gtk_tree_store_append (ptw->treestore, &ptn->iter, parent);

	/* Copy project data into the treestore columns */
	ctree_update_column_values (ptw, ptn, FALSE);

	/* Store the back pointer to the row data in a bogus column
	 * (the NULL column, which is always the last column) */
	{
		GValue val = {G_TYPE_INVALID};
	 	g_value_init(&val, G_TYPE_POINTER);
		g_value_set_pointer (&val, ptn);
		gtk_tree_store_set_value (ptw->treestore, &ptn->iter, ptw->ncols, &val);
	}
	
	/* Make sure children get moved over also */
	for (n=gtt_project_get_children(p); n; n=n->next)
	{
		GttProject *sub_prj = n->data;
		ctree_add (ptw, sub_prj, &ptn->iter);
	}
}

/* ============================================================== */

void
ctree_insert_before (ProjTreeWindow *ptw, GttProject *p, GttProject *sibling)
{
	ProjTreeNode *ptn;
#if XXX
	GtkCTreeNode *sibnode=NULL;
	GtkCTreeNode *parentnode=NULL;

	if (sibling)
	{
		GttProject *parent = gtt_project_get_parent (sibling);
		if (parent) 
		{
			ptn = gtt_project_get_private_data (parent);
			parentnode = ptn->ctnode;
		}
		ptn = gtt_project_get_private_data (sibling);
		sibnode = ptn->ctnode;
	}

	ptn = gtt_project_get_private_data (p);
	if (!ptn)
	{
		ptn = g_new0 (ProjTreeNode, 1);
		ptn->ptw = ptw;
		ptn->prj = p;
		gtt_project_set_private_data (p, ptn);
		gtt_project_add_notifier (p, redraw, ptn);
	}
	stringify_col_values (ptn, FALSE);
	ptn->ctnode = gtk_ctree_insert_node (ptw->ctree,  
                               parentnode, sibnode,
                               ptn->col_values, 0, NULL, NULL, NULL, NULL,
                               FALSE, FALSE);

	gtk_ctree_node_set_row_data(ptw->ctree, ptn->ctnode, ptn);
#endif
}

/* ============================================================== */

void
ctree_insert_after (ProjTreeWindow *ptw, GttProject *p, GttProject *sibling)
{
	ProjTreeNode *ptn;
#if XXX
	GtkCTreeNode *parentnode=NULL;
	GtkCTreeNode *next_sibling=NULL;

	if (sibling)
	{
		GttProject *parent = gtt_project_get_parent (sibling);
		if (parent) 
		{
			ptn = gtt_project_get_private_data (parent);
			parentnode = ptn->ctnode;
		}
		ptn = gtt_project_get_private_data (sibling);
		next_sibling = GTK_CTREE_NODE_NEXT(ptn->ctnode);

		/* weird math: if this is the last leaf on this
		 * branch, then next_sibling must be null to become 
		 * the new last leaf. Unfortunately, GTK_CTREE_NODE_NEXT
		 * doesn't give null, it just gives the next branch */
		if (next_sibling &&  
		   (GTK_CTREE_ROW(next_sibling)->parent != parentnode)) next_sibling = NULL;
	}

	ptn = gtt_project_get_private_data (p);
	if (!ptn)
	{
		ptn = g_new0 (ProjTreeNode, 1);
		ptn->ptw = ptw;
		ptn->prj = p;
		gtt_project_set_private_data (p, ptn);
		gtt_project_add_notifier (p, redraw, ptn);
	}
	stringify_col_values (ptn, FALSE);
	ptn->ctnode = gtk_ctree_insert_node (ptw->ctree,  
                               parentnode, next_sibling,
                               ptn->col_values, 0, NULL, NULL, NULL, NULL,
                               FALSE, FALSE);

	gtk_ctree_node_set_row_data(ptw->ctree, ptn->ctnode, ptn);
#endif
}

/* ============================================================== */

void
ctree_remove(ProjTreeWindow *ptw, GttProject *p)
{
	ProjTreeNode *ptn;
	if (!ptw || !p) return;

	ptn = gtt_project_get_private_data (p);
	g_return_if_fail (NULL != ptn);
	gtt_project_remove_notifier (p, redraw, ptn);
#if XXX
	gtk_ctree_node_set_row_data(ptw->ctree, ptn->ctnode, NULL);
	gtk_ctree_remove_node(ptw->ctree, ptn->ctnode);
	ptn->prj = NULL;
	ptn->ctnode = NULL;
#endif
	ptn->ptw = NULL;
	g_free (ptn);
	gtt_project_set_private_data (p, NULL);
}

/* ============================================================== */

void
ctree_update_label(ProjTreeWindow *ptw, GttProject *p)
{
	ProjTreeNode *ptn;
	if (!ptw || !p) return;
	ptn = gtt_project_get_private_data (p);
	g_return_if_fail (NULL != ptn);
	
	ctree_update_row (ptw, ptn);
	update_status_bar();
}

/* ============================================================== */

void 
ctree_titles_show (ProjTreeWindow *ptw)
{
	if (!ptw) return;
	// XXX
	// gtk_clist_column_titles_show(GTK_CLIST(ptw->ctree));
}

void 
ctree_titles_hide (ProjTreeWindow *ptw)
{
	if (!ptw) return;
	// XXX
	// gtk_clist_column_titles_hide(GTK_CLIST(ptw->ctree));
}

void
ctree_subproj_show (ProjTreeWindow *ptw)
{
	if (!ptw) return;
#if XXX
	gtk_ctree_set_show_stub(ptw->ctree, FALSE);
	gtk_ctree_set_line_style(ptw->ctree, GTK_CTREE_LINES_SOLID);
	gtk_ctree_set_expander_style(ptw->ctree,GTK_CTREE_EXPANDER_SQUARE);
#endif
}

void
ctree_subproj_hide (ProjTreeWindow *ptw)
{
	if (!ptw) return;
#if XXX
	gtk_ctree_set_show_stub(ptw->ctree, FALSE);
	gtk_ctree_set_line_style(ptw->ctree, GTK_CTREE_LINES_NONE);
	gtk_ctree_set_expander_style(ptw->ctree,GTK_CTREE_EXPANDER_NONE);
#endif
}

/* ============================================================== */

void
ctree_set_col_width (ProjTreeWindow *ptw, int col, int width)
{
	if (!ptw) return;
	// XXX
	// gtk_clist_set_column_width(GTK_CLIST(ptw->ctree), col, width);
	ptw->col_width_set[col] = TRUE;
}

int
ctree_get_col_width (ProjTreeWindow *ptw, int col)
{
	int width = -1;
	if (!ptw) return -1;
	if (0 > col) return -1;
#if XXX
	if (col >= GTK_CLIST(ptw->ctree)->columns) return -1;
	width = GTK_CLIST(ptw->ctree)->column[col].width;
#endif
	return width;
}

#endif /* GTT_CTREE_GNOME2 */
/* ===================== END OF FILE ==============================  */
