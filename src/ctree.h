/*   ctree implementation of main window for GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas
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

#ifndef __GTT_CTREE_H__
#define __GTT_CTREE_H__

#include <gnome.h>
#include "proj.h"

typedef struct ProjTreeWindow_s ProjTreeWindow;

extern int clist_header_width_set;

/* create the window that shows the project tree */
ProjTreeWindow * ctree_new(void);
void ctree_setup(ProjTreeWindow *ptw);
void ctree_destroy(ProjTreeWindow *ptw);

/* The ctree_insert_before() routine inserts the new project p
 *   before the project "before me".
 *
 * The ctree_update_column_visibility() routine sets/changes
 *    which columns will be visible in the ctree window.  Note
 *    that it does *not* redraw the data in the columns: use
 *    ctree_refresh() for that.
 *
 * The ctree_refresh() routine redraws the entire ctree window.
 *    It will update the column visibility according to the current
 *    defaults, then grab all data out of the top project list,
 *    and then redraw everything.
 */
void ctree_add(ProjTreeWindow *, GttProject *p, GtkCTreeNode *parent);
void ctree_insert_before(ProjTreeWindow *, GttProject *p, GttProject *insert_before_me);
void ctree_insert_after(ProjTreeWindow *, GttProject *p, GttProject *insert_after_me);

void ctree_remove(ProjTreeWindow *, GttProject *p);
void ctree_update_label(ProjTreeWindow *, GttProject *p);
void ctree_update_title(ProjTreeWindow *, GttProject *p);
void ctree_update_desc(ProjTreeWindow *, GttProject *p);
void ctree_unselect(ProjTreeWindow *, GttProject *p);
void ctree_select(ProjTreeWindow *, GttProject *p);

void ctree_set_col_width (ProjTreeWindow *ptw, int col, int width);
int  ctree_get_col_width (ProjTreeWindow *ptw, int col);

void ctree_refresh (ProjTreeWindow *ptw);
void ctree_update_column_visibility (ProjTreeWindow *ptw);
void ctree_titles_show (ProjTreeWindow *ptw);
void ctree_titles_hide (ProjTreeWindow *ptw);
void ctree_subproj_show (ProjTreeWindow *ptw);
void ctree_subproj_hide (ProjTreeWindow *ptw);

GtkWidget * ctree_get_widget(ProjTreeWindow *);

/* The 'focus project' corresponds to the 'focus row' in the ctree:
 *    its the project that corresponds to where the keyboard events
 *    are directed.
 */
GttProject *ctree_get_focus_project (ProjTreeWindow *);

#endif /* __GTT_CTREE_H__ */
