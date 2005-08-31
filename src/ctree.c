/*   GtkCTree display of projects for GTimeTracker 
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

#ifndef GTT_CTREE_GNOME2

#include <config.h>
#include <gnome.h>

#include <qof/gnc-date.h>

#include "app.h"
#include "ctree.h"
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
	TIME_LASTWEEK_COL,
	TIME_YESTERDAY_COL,
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

#define NCOLS		21


typedef struct ProjTreeNode_s
{
	ProjTreeWindow *ptw;
	GtkCTreeNode *ctnode;
	GttProject *prj;

	char save_expander_state;

	char *col_values[NCOLS];

	char ever_timestr[24];
	char current_timestr[24];
	char day_timestr[24];
	char yesterday_timestr[24];
	char week_timestr[24];
	char lastweek_timestr[24];
	char month_timestr[24];
	char year_timestr[24];
	char start_timestr[24];
	char end_timestr[24];
	char due_timestr[24];
	char sizing_str[24];
	char percent_str[24];
} ProjTreeNode;

struct ProjTreeWindow_s 
{
	GtkCTree *ctree;

	/* List of projects we are currently displaying in this tree */
	GList *proj_list;
	
	/* Stuff that defines the column layout */
	ColType cols[NCOLS];
	char * col_titles[NCOLS];
	char * col_tooltips[NCOLS];
	GtkTooltips *col_tt_w[NCOLS];
	GtkJustification col_justify[NCOLS];
	gboolean col_width_set[NCOLS];
	int ncols;

	/* Stuff we need to have handy while dragging */
	GdkDragContext *drag_context;
	GtkCTreeNode *source_ctree_node;
	GtkCTreeNode *parent_ctree_node;
	GtkCTreeNode *sibling_ctree_node;

	/* Colors to use to indicate active project */
	GdkColor active_bgcolor;
	GdkColor neutral_bgcolor;

	/* Should the select row be shown? */
	gboolean show_select_row;

	/* Total number of rows/projects, and string showing 
	 * expander state.  Used to save and restore expanders.
	 */
	int num_rows;
	char * expander_state;
};

static void stringify_col_values (ProjTreeNode *ptn, gboolean expand);
static void ctree_update_column_values (ProjTreeWindow *ptw, ProjTreeNode *ptn, gboolean expand);
static void ctree_update_row (ProjTreeWindow *ptw, ProjTreeNode *ptn);
		  
		  

/* ============================================================== */

static void
start_timer_for_row (ProjTreeWindow *ptw, ProjTreeNode *ptn)
{
	GtkCTree *ctree = ptw->ctree;
	GttProject *prj = ptn->prj;
	
	cur_proj_set(prj);
	gtt_project_timer_update (prj);
	
	ctree_update_label (ptw, prj);

	gtk_ctree_node_set_background (ctree, ptn->ctnode, &ptw->active_bgcolor);
	
	/* unselect so that select color doesn't block the active color */
	/* toggle to make it the focus row */
	gtk_ctree_select (ctree, ptn->ctnode);
	gtk_ctree_unselect (ctree, ptn->ctnode);
}



static void
stop_timer_for_row (ProjTreeWindow *ptw, ProjTreeNode *ptn)
{
	GtkCTree *ctree = ptw->ctree;
	GttProject *prj = ptn->prj;
	
	if (prj != cur_proj) return;
	cur_proj_set(NULL);
	
	gtt_project_timer_update (prj);
	ctree_update_label (ptw, prj);
	gtk_ctree_node_set_background (ctree, ptn->ctnode, &ptw->neutral_bgcolor);
	
	/* Use the select color, if its desired */
	if (ptn->ptw->show_select_row)
	{
		gtk_ctree_select (ctree, ptn->ctnode);
	}
}

static void
toggle_timer_for_row (ProjTreeWindow *ptw, ProjTreeNode *ptn)
{
	gboolean running = timer_is_running();
	if (!ptn) return;
	if ((ptn->prj == cur_proj) && running)
	{
		stop_timer_for_row (ptw, ptn);
	}
	else
	{
		if (running)
		{
			ProjTreeNode *curr_ptn;
			curr_ptn = gtt_project_get_private_data (cur_proj);
			stop_timer_for_row (ptw, curr_ptn);
		}
		start_timer_for_row (ptw, ptn);
	}
}

void 
ctree_start_timer (GttProject *prj)
{
	ProjTreeNode *ptn;
	if (!prj) return;
	ptn = gtt_project_get_private_data (prj);
	if (!ptn) return;
	start_timer_for_row (ptn->ptw, ptn);
}

void 
ctree_stop_timer (GttProject *prj)
{
	ProjTreeNode *ptn;
	if (!prj) return;
	ptn = gtt_project_get_private_data (prj);
	if (!ptn) return;
	stop_timer_for_row (ptn->ptw, ptn);
}

/* ============================================================== */

gboolean 
ctree_has_focus (ProjTreeWindow *ptw)
{
	return GTK_WIDGET_HAS_FOCUS (GTK_WIDGET(ptw->ctree));
}

/* ============================================================== */

static GtkCTreeNode *
get_focus_row (GtkCTree *ctree)
{
	GtkCTreeNode *rownode;
	rownode = gtk_ctree_node_nth (ctree,  GTK_CLIST(ctree)->focus_row);
	return rownode;
}

/* ============================================================== */
/* Pseudo focus-row-changed callback. Things inside of ctree
 * should call this, which in turn calls out to distribute
 * an event to other subsystems, telling them that the focus 
 * has changed. 
 */

static void
focus_row_changed_cb (GtkCTree *ctree, int row)
{
	GtkCTreeNode *rownode;
	GttProject *proj = NULL;

	rownode = get_focus_row(ctree);
	if (rownode)
	{
		ProjTreeNode *ptn;
		ptn = gtk_ctree_node_get_row_data(ctree, rownode);
		proj = ptn->prj;
	}
	
	/* Call the event-redistributor in app.c.  This should
	 * be replaced by g_object/g_signal for a better implementation. */
	focus_row_set (proj);
}

/* ============================================================== */

GttProject *
ctree_get_focus_project (ProjTreeWindow *ptw)
{
	GttProject * proj = NULL;
	GtkCTreeNode *rownode;
	if (!ptw) return NULL;

	rownode = gtk_ctree_node_nth (ptw->ctree,  GTK_CLIST(ptw->ctree)->focus_row);
	if (rownode)
	{
		ProjTreeNode *ptn;
		ptn = gtk_ctree_node_get_row_data(ptw->ctree, rownode);
		if (ptn) proj = ptn->prj;
	}

	return proj;
}


static void
set_focus_project (ProjTreeWindow *ptw, GttProject *prj)
{
	GtkCTree *ctree;
	ProjTreeNode *ptn;
	int i;
	
	ptn = gtt_project_get_private_data (prj);
	if (!ptn) return;
		
	/* Set the focus row.  Seems like the only way
	 * to get the row number is to search for it. */
	ctree = ptw->ctree;
	i = 0;
	while (1) 
	{
		GtkCTreeNode * ctn = gtk_ctree_node_nth (ctree, i);
		if (!ctn) break;
		if (gtk_ctree_node_get_row_data(ctree, ctn) == ptn)
		{
			/* The freeze-draw cycle forces a redraw */
			gtk_clist_freeze (GTK_CLIST(ctree));
    		GTK_CLIST(ctree)->focus_row = i;
			gtk_clist_thaw (GTK_CLIST(ctree));
			break;
		}
		i++;
	}
	focus_row_changed_cb (ctree, GTK_CLIST(ctree)->focus_row);
}

/* ============================================================== */

static int
widget_key_event(GtkCTree *ctree, GdkEvent *event, gpointer data)
{
	static int return_key_pressed = FALSE;
	ProjTreeNode *ptn;
	GtkCTreeNode *rownode;
	GdkEventKey *kev = (GdkEventKey *)event;

	if ((GDK_KEY_PRESS == event->type)  &&
	     gtk_widget_is_focus (GTK_WIDGET(ctree)) &&
	    (kev->keyval == GDK_Return)) 
	{ 
		/* Avoid toggling timer when receiving a KEY_RELEASE
		 * if the KEY_PRESS was not received previously. 
		 * (because KEY_PRESS went to another window.) */
		return_key_pressed = TRUE;
		return TRUE;
	}

	if (GDK_KEY_RELEASE != event->type) return FALSE;
	if (FALSE == gtk_widget_is_focus (GTK_WIDGET(ctree))) return FALSE;
	
	switch (kev->keyval)
	{
		case GDK_Return:
			rownode = get_focus_row(ctree);
			if (rownode && return_key_pressed)
			{
				ptn = gtk_ctree_node_get_row_data(ctree, rownode);
				toggle_timer_for_row (ptn->ptw, ptn);
				return_key_pressed = FALSE;
			}
			return TRUE;
		case GDK_Up:
		case GDK_Page_Up:
		case GDK_Down:
		case GDK_Page_Down:
			rownode = get_focus_row(ctree);
			if (rownode)
			{
				ptn = gtk_ctree_node_get_row_data(ctree, rownode);
			   if (ptn->ptw->show_select_row) gtk_ctree_select (ctree, rownode);
			}
			focus_row_changed_cb (ctree, GTK_CLIST(ctree)->focus_row);
			return FALSE;
		case GDK_Left:
			rownode = get_focus_row(ctree);
			gtk_ctree_collapse (ctree, rownode);
			return TRUE;
		case GDK_Right:
			rownode = get_focus_row(ctree);
			gtk_ctree_expand (ctree, rownode);
			return TRUE;
			
		case 'j':
			if(GTK_CLIST(ctree)->focus_row < GTK_CLIST(ctree)->rows - 1)
			{
				/* The freeze-draw cycle forces a redraw */
				gtk_clist_freeze(GTK_CLIST(ctree));
				GTK_CLIST(ctree)->focus_row += 1;
				gtk_clist_thaw(GTK_CLIST(ctree));
				focus_row_changed_cb (ctree, GTK_CLIST(ctree)->focus_row);
			}
			return TRUE;

		case 'k':
			if(GTK_CLIST(ctree)->focus_row > 0)
			{
				/* The freeze-draw cycle forces a redraw */
				gtk_clist_freeze(GTK_CLIST(ctree));
				GTK_CLIST(ctree)->focus_row -= 1;
				gtk_clist_thaw(GTK_CLIST(ctree));
				focus_row_changed_cb (ctree, GTK_CLIST(ctree)->focus_row);
			}
			return TRUE;

		case 'q':
			app_quit (NULL, NULL);
			return TRUE;

		default:
			return FALSE;
	}
	return FALSE;
}

static int
widget_button_event(GtkCList *clist, GdkEvent *event, gpointer data)
{
	ProjTreeWindow *ptw = data;
	int row,column;
	GdkEventButton *bevent = (GdkEventButton *)event;
	GtkWidget *menu;
	ProjTreeNode *ptn;
	GtkCTreeNode *rownode;
	
	/* The only button event we handle are right-mouse-button,
	 * end double-click-left-mouse-button. */
	if (!((event->type == GDK_2BUTTON_PRESS && bevent->button==1) ||
	      (event->type == GDK_BUTTON_PRESS && bevent->button==3)))
		return FALSE;

	gtk_clist_get_selection_info(clist,bevent->x,bevent->y,&row,&column);
	if (0 > row) return FALSE;
	
	/* Change the focus row */
	/* The freeze-draw cycle forces a redraw */
	gtk_clist_freeze(clist);
	clist->focus_row = row;
	gtk_clist_thaw(clist);

	rownode = get_focus_row(ptw->ctree);
	if (event->type == GDK_2BUTTON_PRESS) 
	{
		/* double-click left mouse toggles the project timer. */
		ptn = gtk_ctree_node_get_row_data(ptw->ctree, rownode);
		toggle_timer_for_row (ptw, ptn);
	} 
	else 
	{
		/* right mouse button brings up popup menu */
		menu = menus_get_popup();
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, bevent->time);
	   if (ptw->show_select_row) gtk_ctree_select (GTK_CTREE(clist), rownode);
	}
	return TRUE;
}

/* ============================================================== */

static void
tree_select_row(GtkCTree *ctree, GtkCTreeNode* rownode, gint column)
{
	ProjTreeNode *ptn;
	ptn = gtk_ctree_node_get_row_data(ctree, rownode);
	
	/* Make sure that the blue of the select doesn't clobber
	 * the active color */
	if ((ptn->prj == cur_proj) && timer_is_running())
	{
		gtk_ctree_unselect (ctree, ptn->ctnode);
	}

	/* don't show the row no matter what, if its not desired */
	if (FALSE == ptn->ptw->show_select_row)
	{
		gtk_ctree_unselect (ctree, ptn->ctnode);
	}
}



static void
tree_unselect_row(GtkCTree *ctree, GtkCTreeNode* rownode, gint column)
{
	/* nothing in this incarnation */
}

static void 
tree_expand (GtkCTree *ctree, GtkCTreeNode *row)
{
	ProjTreeNode *ptn = gtk_ctree_node_get_row_data(ctree, row);
   ctree_update_column_values (ptn->ptw, ptn, TRUE);
}

static void 
tree_collapse (GtkCTree *ctree, GtkCTreeNode *row)
{
	ProjTreeNode *ptn = gtk_ctree_node_get_row_data(ctree, row);
   ctree_update_column_values (ptn->ptw, ptn, FALSE);
}

/* ============================================================== */

static void 
ctree_save_expander_state (ProjTreeWindow *ptw)
{
	int i = 0;
	GtkCTree *ctree;
	GtkCTreeNode *ctn;
	ProjTreeNode *ptn;
	gboolean expanded, is_leaf;

	if (!ptw) return;
	ctree = ptw->ctree;

	while (1) 
	{
		ctn = gtk_ctree_node_nth (ctree, i);
		if (!ctn) break;
		ptn = gtk_ctree_node_get_row_data(ctree, ctn);
		gtk_ctree_get_node_info (ctree, ctn, 
		                NULL, NULL, NULL, NULL, NULL, NULL,
		                &is_leaf, &expanded);

		if (expanded)
		{
			ptn->save_expander_state = 'y';
		}
		else
		{
			ptn->save_expander_state = 'n';
		}
		i++;
	}
}

/* ============================================================== */

static void
click_column(GtkCList *clist, gint col, gpointer data)
{
	ProjTreeWindow *ptw = data;
	GList *prlist;
	ColType ct;

	prlist = ptw->proj_list;
	if ((0 > col) || (ptw->ncols <= col)) return;
	ct = ptw->cols[col];
	switch (ct)
	{
		case TIME_EVER_COL:
			prlist = project_list_sort_ever (prlist);
			break;
		case TIME_CURRENT_COL:
			prlist = project_list_sort_current (prlist);
			break;
		case TIME_YESTERDAY_COL:
			prlist = project_list_sort_yesterday (prlist);
			break;
		case TIME_TODAY_COL:
			prlist = project_list_sort_day (prlist);
			break;
		case TIME_WEEK_COL:
			prlist = project_list_sort_week (prlist);
			break;
		case TIME_LASTWEEK_COL:
			prlist = project_list_sort_lastweek (prlist);
			break;
		case TIME_MONTH_COL:
			prlist = project_list_sort_month (prlist);
			break;
		case TIME_YEAR_COL:
			prlist = project_list_sort_year (prlist);
			break;
		case TITLE_COL:
			prlist = project_list_sort_title (prlist);
			break;
		case DESC_COL:
			prlist = project_list_sort_desc (prlist);
			break;
		case TASK_COL:
			break;
		case NULL_COL:
			break;
		case START_COL:
			prlist = project_list_sort_start (prlist);
			break;
		case END_COL:
			prlist = project_list_sort_end (prlist);
			break;
		case DUE_COL:
			prlist = project_list_sort_due (prlist);
			break;
		case SIZING_COL:
			prlist = project_list_sort_sizing (prlist);
			break;
		case PERCENT_COL:
			prlist = project_list_sort_percent (prlist);
			break;
		case URGENCY_COL:
			prlist = project_list_sort_urgency (prlist);
			break;
		case IMPORTANCE_COL:
			prlist = project_list_sort_importance (prlist);
			break;
		case STATUS_COL:
			prlist = project_list_sort_status (prlist);
			break;
	}
	
	ctree_setup(ptw, prlist);
}

/* ============================================================== */
/* Attempt to change pixmaps to indicate whether dragged project
 * will be a peer, or a child, of the project its dragged onto.
 * Right now, this code ain't pretty; there must be a better way ...  
 */
static GtkWidget *parent_pixmap = NULL;
static GtkWidget *sibling_pixmap = NULL;

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

	/* Set the focus row to the newly inserted project. */
	set_focus_project (ptw, src_prj);
	
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
	ProjTreeNode *ptn;
	ptn = gtk_ctree_node_get_row_data(ctree, source_node);
	printf ("draging     \"%s\"\n", gtt_project_get_title(ptn?ptn->prj:NULL));
	ptn = gtk_ctree_node_get_row_data(ctree, new_parent);
	printf ("\tto parent   \"%s\"\n", gtt_project_get_title(ptn?ptn->prj:NULL));
	ptn = gtk_ctree_node_get_row_data(ctree, new_sibling);
	printf ("\tbefore sibl \"%s\"\n\n", gtt_project_get_title(ptn?ptn->prj:NULL));
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

/* ============================================================== */
/* note about column layout:
 * We have two ways of doing this: create a column-cell object,
 * and program in an object-oriented styles.  The othe way is to
 * set a column type, and use a big switch statement.  Beacuse this
 * is a small prioject, we'll just use a switch statement.  Seems
 * easier.
 *
 * Note also we have implemented a general system here, that 
 * allows us to show columns in any order, or the same column
 * more than once.   However, we do not actually make use of
 * this anywhere; at the moment, the column locatins are fixed.
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
	i++; ptw->cols[i] = TIME_LASTWEEK_COL;
	i++; ptw->cols[i] = TIME_YESTERDAY_COL;
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
					_("Time spent under the current diary entry.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_YESTERDAY_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Yesterday");
				ptw->col_tooltips[i] =  
					_("Time spent on this project yesterday.");
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
					_("Time spent on this project this week.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_LASTWEEK_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] = _("Last Week");
				ptw->col_tooltips[i] = 
					_("Time spent on this project last week.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_MONTH_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Month");
				ptw->col_tooltips[i] =  
					_("Time spent on this project this month.");
				ptw->col_width_set[i] = FALSE;
				break;
			case TIME_YEAR_COL:
				ptw->col_justify[i] = GTK_JUSTIFY_CENTER;
				ptw->col_titles[i] =  _("Year");
				ptw->col_tooltips[i] = 
					_("Time spent on this project this year.");
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
		if (NULL == style) return -49;
	}

	/* Try really hard to find a font */
	gc = style->text_gc[0];
	if (NULL == gc) gc = style->fg_gc[0];
	if (NULL == gc) gc = style->bg_gc[0];
	if (NULL == gc) gc = style->base_gc[0];
	if (NULL == gc)
	{
		style = gtk_widget_get_default_style ();
		if (NULL == style) return -50;

		gc = style->text_gc[0];
		if (NULL == gc) gc = style->fg_gc[0];
		if (NULL == gc) gc = style->bg_gc[0];
		if (NULL == gc) gc = style->base_gc[0];
		if (NULL == gc) return -51;
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

	/* Size the column width based on the font size times
	 * length of string.  Get the font from the GC. */
	width = string_width (GTK_WIDGET(ptw->ctree), str);

	/* Add special handling for certain columns, so that we get
	 * a good first-time default column width.
	 */
	if (0 > width)
	{
		width = -width;
		switch (ptw->cols[col])
		{
			case IMPORTANCE_COL: width = 16; break;
			case URGENCY_COL:    width = 16; break;	
			case DESC_COL:       width = 62; break;	
			default: break;
		}
	}
	ctree_set_col_width (ptw, col, width);
}

/* Note: we don't need i18n/l7n translations for most of the strings;
 * they are used only for column widths ... */
void 
ctree_update_column_visibility (ProjTreeWindow *ptw)
{
	int i;

	/* set column visibility */
	for (i=0; NULL_COL != ptw->cols[i]; i++)
	{
		switch (ptw->cols[i])
		{
		case TITLE_COL:
			default_col_width (ptw, i, "-00:00:00");
			break;
		case TIME_EVER_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree), 
				i, config_show_title_ever);
			break;
		case TIME_CURRENT_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_current);
			break;
		case TIME_YESTERDAY_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				 i, config_show_title_yesterday);
			break;
		case TIME_TODAY_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				 i, config_show_title_day);
			break;
		case TIME_WEEK_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_week);
			break;
		case TIME_LASTWEEK_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_lastweek);
			break;
		case TIME_MONTH_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree), 
				i, config_show_title_month);
			break;
		case TIME_YEAR_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_year);
			break;
		case DESC_COL:
			default_col_width (ptw, i, "Not too long");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree), 
				i, config_show_title_desc);
			break;
		case TASK_COL:
			default_col_width (ptw, i, "Some longer string");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_task);
			break;
		case START_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_estimated_start);
			break;
		case END_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_estimated_end);
			break;
		case DUE_COL:
			default_col_width (ptw, i, "-00:00:00");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_due_date);
			break;
		case SIZING_COL:
			default_col_width (ptw, i, "XXX.XX");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_sizing);
			break;
		case PERCENT_COL:
			default_col_width (ptw, i, "100%");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_percent_complete);
			break;
		case URGENCY_COL:
			default_col_width (ptw, i, "X");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_urgency);
			break;
		case IMPORTANCE_COL:
			default_col_width (ptw, i, "XXX");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_importance);
			break;
		case STATUS_COL:
			default_col_width (ptw, i, "abcedfg");
			gtk_clist_set_column_visibility (GTK_CLIST(ptw->ctree),
				i, config_show_title_status);
			break;
		case NULL_COL:
			break;
		}

	}
}

/* ============================================================== */

#define PRT_TIME(SLOT) {                                        \
   ptn->col_values[i] =  ptn->SLOT##_timestr;                   \
   if (config_show_title_##SLOT) {                              \
      time_t secs;                                              \
      if (expand) {                                             \
         secs = gtt_project_get_secs_##SLOT(prj);               \
      } else {                                                  \
         secs = gtt_project_total_secs_##SLOT(prj);             \
      }                                                         \
      if (0 < secs || ((0 == secs) && (prj == cur_proj))) {     \
         qof_print_hours_elapsed_buff (ptn->SLOT##_timestr, 24, \
                 secs, config_show_secs);                       \
      } else {                                                  \
         ptn->SLOT##_timestr[0] = '-';                          \
         ptn->SLOT##_timestr[1] = 0x0;                          \
      }                                                         \
   }                                                            \
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
		case TIME_YESTERDAY_COL:
			PRT_TIME(yesterday);
			break;
		case TIME_TODAY_COL:
			PRT_TIME(day);
			break;
		case TIME_WEEK_COL:
			PRT_TIME(week);
			break;
		case TIME_LASTWEEK_COL:
			PRT_TIME(lastweek);
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
					qof_print_date_buff (ptn->start_timestr, 24, secs);
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
					qof_print_date_buff (ptn->end_timestr, 24, secs);
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
					qof_print_date_buff (ptn->due_timestr, 24, secs);
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
	
	stringify_col_values (ptn, expand);
	for (i=0; i<ptw->ncols; i++)
	{
		gtk_ctree_node_set_text(ptw->ctree, 
			ptn->ctnode, i, ptn->col_values[i]);
	}
}

/* ============================================================== */
/* Redraw one row, and one row only.  Expansion automatically determined. */

static void
ctree_update_row (ProjTreeWindow *ptw, ProjTreeNode *ptn)
{
	int i;
	gboolean expand;
	
	expand = GTK_CTREE_ROW(ptn->ctnode)->expanded;
	stringify_col_values (ptn, expand);
	for (i=0; i<ptw->ncols; i++)
	{
		gtk_ctree_node_set_text(ptw->ctree, 
			ptn->ctnode, i, ptn->col_values[i]);
	}
}
		  
/* ============================================================== */
/* redraw utility, recursively walks visibile project tree */

static void
refresh_list (ProjTreeWindow *ptw, GList *prjlist)
{
	GList *node;

	/* now, draw each project */
	for (node = prjlist; node; node = node->next) 
	{
		gboolean expand;
		ProjTreeNode *ptn;
		GttProject *prj = node->data;

		ptn = gtt_project_get_private_data (prj);

		/* Under rare circumstances, e.g. when run after midnight,
		 * th GUI might not yet be initialized, so ptn is zero. 
		 * We sorta shouldn't even be here in that case, but oh well. */
		if (!ptn) continue;
		
		/* Determine if project is expanded -- this affects 
		 * display of time totals */
		expand = GTK_CTREE_ROW(ptn->ctnode)->expanded;
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
/* redraw all */

void
ctree_refresh (ProjTreeWindow *ptw)
{
	GtkCTree *tree_w;
	GList *prjlist;

	if (!ptw) return;
	tree_w = ptw->ctree;

	/* Freeze, in prep for a massive update */
	/* XXX As of gtk 2.2.1, doing a freeze-thaw cycle
	 * fails to redraw the window correctly.  
	 * Actually, if the window gets an expose events,
	 * then it will show the right thing. Until then, work
	 * around by not calling freeze-thaw.
	 * Glurg. Failing to freeze-thaw causes some pretty
	 * nasty cpu-cycle sucking sucks. Ughh.
	 */
	gtk_clist_freeze(GTK_CLIST(tree_w)); 

	/* Make sure the right set of columns are visibile */
	ctree_update_column_visibility (ptw);

	/* Now, draw each project */
	prjlist = gtt_get_project_list();
	refresh_list (ptw, prjlist);

	gtk_clist_thaw(GTK_CLIST(tree_w));
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

static void
select_row_cb (GtkCTree *ctree, gint row, gint col, 
					 GdkEvent *ev, ProjTreeWindow *ptw)
{
	/* shim to artificial callback */
	focus_row_changed_cb (ctree, row);
}

/* ============================================================== */

GtkWidget *
ctree_get_widget (ProjTreeWindow *ptw)
{
	if (!ptw) return NULL;
	return (GTK_WIDGET (ptw->ctree));
}

ProjTreeWindow *
ctree_new(void)
{
	ProjTreeWindow *ptw;
	GtkWidget *wimg;
	GtkWidget *w;
	int i;

	ptw = g_new0 (ProjTreeWindow, 1);
	ptw->proj_list = NULL;

	ptw->source_ctree_node = NULL;
	ptw->parent_ctree_node = NULL;
	ptw->sibling_ctree_node = NULL;
	ptw->drag_context = NULL;

	ctree_init_cols (ptw);

	for (i=0; NULL_COL != ptw->cols[i]; i++) {
		if (TITLE_COL == ptw->cols[i]) break;
	}

	w = gtk_ctree_new_with_titles(ptw->ncols, i, ptw->col_titles);
	ptw->ctree = GTK_CTREE(w);

	g_object_set_data (G_OBJECT(w), "ptw", ptw);

	for (i=0; i<ptw->ncols; i++)
	{
		gtk_clist_set_column_justification(GTK_CLIST(w), 
			i, ptw->col_justify[i]);
	}
	gtk_clist_column_titles_active(GTK_CLIST(w));
	gtk_clist_set_selection_mode(GTK_CLIST(w), GTK_SELECTION_SINGLE);

	/* some columns are quite narrow, so put tooltips over them. */
	for (i=0; i<ptw->ncols; i++)
	{
		GtkTooltips *tt;
		tt = gtk_tooltips_new();
		gtk_tooltips_enable (tt);
		gtk_tooltips_set_tip (tt, GTK_CLIST(w)->column[i].button,
			ptw->col_tooltips[i], NULL);
		ptw->col_tt_w[i] = tt;
	}

	/* set miinimum size, set stubbie on the tree display */
	gtk_widget_set_size_request (w, -1, 120);
	ctree_update_column_visibility (ptw);
	gtk_ctree_set_show_stub(GTK_CTREE(w), FALSE);

	/* Grab initial focus for hot-key events */
	gtk_widget_grab_focus (w);
	
	/* connect various signals */
	g_signal_connect(G_OBJECT(w), "button_press_event",
			   G_CALLBACK(widget_button_event), ptw);
	g_signal_connect(G_OBJECT(w), "key_release_event",
			   G_CALLBACK(widget_key_event), ptw);
	g_signal_connect(G_OBJECT(w), "key_press_event",
			   G_CALLBACK(widget_key_event), ptw);
	g_signal_connect(G_OBJECT(w), "tree_select_row",
			   G_CALLBACK(tree_select_row), NULL);
	g_signal_connect(G_OBJECT(w), "click_column",
			   G_CALLBACK(click_column), ptw);
	g_signal_connect(G_OBJECT(w), "tree_unselect_row",
			   G_CALLBACK(tree_unselect_row), NULL);
	g_signal_connect(G_OBJECT(w), "tree_expand",
			   G_CALLBACK(tree_expand), NULL);
	g_signal_connect(G_OBJECT(w), "tree_collapse",
			   G_CALLBACK(tree_collapse), NULL);

	g_signal_connect(G_OBJECT(w), "drag_begin",
			   G_CALLBACK(drag_begin), ptw);

	g_signal_connect(G_OBJECT(w), "drag_drop",
			   G_CALLBACK(drag_drop), ptw);

	g_signal_connect(G_OBJECT(w), "select_row",
			   G_CALLBACK(select_row_cb), ptw);

	/* allow projects to be re-arranged by dragging */
	gtk_clist_set_reorderable(GTK_CLIST(w), TRUE);
	gtk_clist_set_use_drag_icons (GTK_CLIST(w), TRUE);
	gtk_ctree_set_drag_compare_func (GTK_CTREE(w), ctree_drag);

	/* Desperate attempt to clue the user into how the 
	 * dragged project will be reparented. We show left or
	 * down arrows, respecitvely.  If anyone can come up with
	 * something more elegant, then please ...  */
	parent_pixmap = gtk_window_new (GTK_WINDOW_POPUP);
	sibling_pixmap = gtk_window_new (GTK_WINDOW_POPUP);

	/* XXX gnome_pixmap_new_from_xpm_d is deprecated, but I cannot
	 * figure out how to use the new, recommended interfaces. Ugh. */
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

	/* Set up background colors that will be used to mark the active
	 * project */
	gdk_color_parse("green", &ptw->active_bgcolor);
	gdk_color_alloc(gdk_colormap_get_system(), &ptw->active_bgcolor);

	gdk_color_parse("white", &ptw->neutral_bgcolor);
	gdk_color_alloc(gdk_colormap_get_system(), &ptw->neutral_bgcolor);

	ptw->show_select_row = FALSE;

	/* total number of rows and expander state */
	ptw->num_rows = 0;
	ptw->expander_state = NULL;

	return ptw;
}


/* ========================================================= */
/* The ctree_setup routine runs during initialization,
 * and also whenever the window is 'deeply' restructured:
 * e.g. when the projects are sorted into a new order.
 * 
 * It sets up the state of the ctree widget infrastructure.
 * It can't be run until after the project data has been
 * read in.
 */

void
ctree_setup (ProjTreeWindow *ptw, GList *prjlist)
{
	GtkCTree *tree_w;
	GList *node;

	if (!ptw) return;
	tree_w = ptw->ctree;
	

	/* Save expander state before doing anything.  */
	ctree_save_expander_state (ptw);

	/* First, add all projects to the ctree. */
	ptw->proj_list = prjlist;
	if (prjlist) 
	{
		gtk_clist_freeze(GTK_CLIST(tree_w));
		gtk_clist_clear(GTK_CLIST(tree_w));
		for (node = prjlist; node; node = node->next) 
		{
			GttProject *prj = node->data;
			ctree_add(ptw, prj);
		}
		gtk_clist_thaw(GTK_CLIST(tree_w));
	} else {
		gtk_clist_clear(GTK_CLIST(tree_w));
	}

	/* Next, highlight the current project, and expand 
	 * the tree branches to it, if needed */
	if (cur_proj) 
	{
		GttProject *parent;
		ProjTreeNode *ptn;

		ptn = gtt_project_get_private_data (cur_proj);
		
		/* Select to set initial focus row */
		gtk_ctree_select (tree_w, ptn->ctnode);  
		
		start_timer_for_row (ptn->ptw, ptn);
		parent = gtt_project_get_parent (cur_proj);
		while (parent) 
		{
			ptn = gtt_project_get_private_data (parent);
			gtk_ctree_expand(tree_w, ptn->ctnode);
			parent = gtt_project_get_parent (parent);
		}
	}

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

		/* Make sure that the active row is in view */
		gtk_ctree_node_moveto(tree_w, ptn->ctnode, -1, 0.5, 0.0);

		/* Set the focus row here as well. */
		set_focus_project (ptw, cur_proj);
	}

	gtk_widget_grab_focus (GTK_WIDGET(tree_w));
	gtk_widget_queue_draw (GTK_WIDGET(tree_w));
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

	gtk_widget_destroy (GTK_WIDGET(ptw->ctree));
	ptw->ctree = NULL;
	ptw->ncols = 0;
	g_free (ptw);
}

/* ============================================================== */

#if 0
/* we don't want to do this willy-nilly with a GtkDestroyNotify, 
 * because it will erase things like expander state whenever we clear
 * and redraw the window, e.g. during column sorts.
 * We need some smarter way of freeing things.  */
static void
ptn_destroy (ProjTreeNode *ptn)
{
	gtt_project_set_private_data (ptn->prj, NULL);
	gtt_project_remove_notifier (ptn->prj, redraw, ptn);
	ptn->ptw = NULL;
	ptn->prj = NULL;
	g_free (ptn);
}
#endif

static void
do_ctree_add (ProjTreeWindow *ptw, GttProject *p, GtkCTreeNode *parent)
{
	ProjTreeNode *ptn;
	GList *n;

	ptn = gtt_project_get_private_data (p);
	if (!ptn)
	{
		ptn = g_new0 (ProjTreeNode, 1);
		ptn->ptw = ptw;
		ptn->prj = p;
		ptn->save_expander_state = 0;
		gtt_project_set_private_data (p, ptn);
		gtt_project_add_notifier (p, redraw, ptn);
	}
	stringify_col_values (ptn, FALSE);
	ptn->ctnode = gtk_ctree_insert_node (ptw->ctree,  parent, NULL,
                               ptn->col_values, 0, NULL, NULL, NULL, NULL,
                               FALSE, FALSE);

	gtk_ctree_node_set_row_data (ptw->ctree, ptn->ctnode, ptn);

	/* Restore expander state, if possible */
	if ('y' == ptn->save_expander_state)
	{
		gtk_ctree_expand (ptw->ctree, ptn->ctnode);
	}
	else if ('n' == ptn->save_expander_state)
	{
		gtk_ctree_collapse (ptw->ctree, ptn->ctnode);
	}
	
	/* Make sure children get moved over also */
	for (n=gtt_project_get_children(p); n; n=n->next)
	{
		GttProject *sub_prj = n->data;
		do_ctree_add (ptw, sub_prj, ptn->ctnode);
	}
	
	/* Set the focus row to the newly inserted project. */
	set_focus_project (ptw, p);
}

void
ctree_add (ProjTreeWindow *ptw, GttProject *p)
{
	do_ctree_add (ptw, p, NULL);
}

/* ============================================================== */

void
ctree_insert_before (ProjTreeWindow *ptw, GttProject *p, GttProject *sibling)
{
	ProjTreeNode *ptn;
	GtkCTreeNode *sibnode=NULL;
	GtkCTreeNode *parentnode=NULL;
	GList *n;

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

	/* Make sure children get moved over also */
	for (n=gtt_project_get_children(p); n; n=n->next)
	{
		GttProject *sub_prj = n->data;
		do_ctree_add (ptw, sub_prj, ptn->ctnode);
	}
	
	/* Set the focus row to the newly inserted project. */
	set_focus_project (ptw, p);
}

/* ============================================================== */

void
ctree_insert_after (ProjTreeWindow *ptw, GttProject *p, GttProject *sibling)
{
	ProjTreeNode *ptn;
	GtkCTreeNode *parentnode=NULL;
	GtkCTreeNode *next_sibling=NULL;
	GList *n;

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

	/* Make sure children get moved over also */
	for (n=gtt_project_get_children(p); n; n=n->next)
	{
		GttProject *sub_prj = n->data;
		do_ctree_add (ptw, sub_prj, ptn->ctnode);
	}
	
	/* Set the focus row to the newly inserted project. */
	set_focus_project (ptw, p);
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
	gtk_ctree_node_set_row_data(ptw->ctree, ptn->ctnode, NULL);
	gtk_ctree_remove_node(ptw->ctree, ptn->ctnode);
	ptn->prj = NULL;
	ptn->ctnode = NULL;
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
	gtk_clist_column_titles_show(GTK_CLIST(ptw->ctree));
}

void 
ctree_titles_hide (ProjTreeWindow *ptw)
{
	if (!ptw) return;
	gtk_clist_column_titles_hide(GTK_CLIST(ptw->ctree));
}

void
ctree_subproj_show (ProjTreeWindow *ptw)
{
	if (!ptw) return;
	gtk_ctree_set_show_stub(ptw->ctree, FALSE);
	gtk_ctree_set_line_style(ptw->ctree, GTK_CTREE_LINES_SOLID);
	gtk_ctree_set_expander_style(ptw->ctree,GTK_CTREE_EXPANDER_SQUARE);
}

void
ctree_subproj_hide (ProjTreeWindow *ptw)
{
	if (!ptw) return;
	gtk_ctree_set_show_stub(ptw->ctree, FALSE);
	gtk_ctree_set_line_style(ptw->ctree, GTK_CTREE_LINES_NONE);
	gtk_ctree_set_expander_style(ptw->ctree,GTK_CTREE_EXPANDER_NONE);
}

/* ============================================================== */

void
ctree_set_col_width (ProjTreeWindow *ptw, int col, int width)
{
	if (!ptw) return;
	gtk_clist_set_column_width(GTK_CLIST(ptw->ctree), col, width);
	ptw->col_width_set[col] = TRUE;
}

int
ctree_get_col_width (ProjTreeWindow *ptw, int col)
{
	int width;
	if (!ptw) return -1;
	if (0 > col) return -1;
	if (col >= GTK_CLIST(ptw->ctree)->columns) return -1;
	width = GTK_CLIST(ptw->ctree)->column[col].width;
	return width;
}

/* ============================================================== */

const char *
ctree_get_expander_state (ProjTreeWindow *ptw)
{
	int i = 0;
	GtkCTree *ctree;
	GtkCTreeNode *ctn;
	ProjTreeNode *ptn;
	gboolean expanded, is_leaf;

	if (!ptw) return NULL;
	ctree = ptw->ctree;

	ptw->num_rows = gtt_project_list_total();
	if (ptw->expander_state) g_free (ptw->expander_state);
	ptw->expander_state = g_new0 (char, ptw->num_rows+1);
	
	for (i=0; i<ptw->num_rows; i++)
	{
		ctn = gtk_ctree_node_nth (ctree, i);
		if (!ctn) break;
		ptn = gtk_ctree_node_get_row_data(ctree, ctn);
		gtk_ctree_get_node_info (ctree, ctn, 
		                NULL, NULL, NULL, NULL, NULL, NULL,
		                &is_leaf, &expanded);

		if (expanded)
		{
			ptw->expander_state[i] = 'y';
		}
		else
		{
			ptw->expander_state[i] = 'n';
		}
	}
	ptw->expander_state[i] = 0x0;
	return ptw->expander_state;
}

/* ============================================================== */

void
ctree_set_expander_state (ProjTreeWindow *ptw, const char *expn)
{
	GtkCTree *ctree;
	GtkCTreeNode *ctn;
	int i = 0;

	if (!ptw || !expn) return;
	
	ctree = ptw->ctree;

	while (expn[i])
	{
		ctn = gtk_ctree_node_nth (ctree, i);
		if (!ctn) break;

		if ('y' == expn[i])
		{
			gtk_ctree_expand (ctree, ctn);
		}
		else if ('n' == expn[i])
		{
			gtk_ctree_collapse (ctree, ctn);
		}
				  
		i++;
	}
}

#endif /* GTT_CTREE_GNOME2 */
/* ===================== END OF FILE ==============================  */
