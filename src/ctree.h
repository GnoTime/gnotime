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

#ifndef GTT_CTREE_GNOME2

#include <gnome.h>
#include "proj.h"

typedef struct ProjTreeWindow_s ProjTreeWindow;

extern int clist_header_width_set;

/** The ctree_new() routine will create the widget that shows the 
 *    project tree.  It returns a pointer to an opaque handle to
 *    reference this widget.
 */
ProjTreeWindow * ctree_new(void);

/** The ctree_setup() routine will copy project data into the ctree
 *    window.  Call this routine after reading in project data, to 
 *    populate the window; or to redraw the window after a 'deep'
 *    data restructuring, such as change in the sort order of the 
 *    projects.  The order of the items displayed in the ctree will
 *    be the same as that presented in the project list.
 */
void ctree_setup (ProjTreeWindow *ptw, GttProjectList *);
void ctree_destroy(ProjTreeWindow *ptw);

/** The ctree_insert_before() routine inserts the new project p
 *   before the project "before me".
 */
void ctree_add(ProjTreeWindow *, GttProject *p);
void ctree_insert_before(ProjTreeWindow *, GttProject *p, GttProject *insert_before_me);
void ctree_insert_after(ProjTreeWindow *, GttProject *p, GttProject *insert_after_me);

/** The ctree_start_timer() routine handles all of the ctree-related
 *    work for starting a project running.  This will not only color
 *    the project as 'active', but it will also actually start the 
 *    project timer ticking.
 *
 * The ctree_stop_timer() routine stops the project timer.
 */
void ctree_start_timer (GttProject *prj);
void ctree_stop_timer (GttProject *prj);

void ctree_remove(ProjTreeWindow *, GttProject *p);
void ctree_update_label(ProjTreeWindow *, GttProject *p);
void ctree_update_title(ProjTreeWindow *, GttProject *p);
void ctree_update_desc(ProjTreeWindow *, GttProject *p);

void ctree_set_col_width (ProjTreeWindow *ptw, int col, int width);
int  ctree_get_col_width (ProjTreeWindow *ptw, int col);

/** The ctree_refresh() routine redraws the entire ctree window.
 *    It will update the column visibility according to the current
 *    defaults, then grab all data out of the top project list,
 *    reformat its internal representation of the data (e.g. date, 
 *    time strings) , and then redraw everything so that it becomes 
 *    visible to the user.
 */
void ctree_refresh (ProjTreeWindow *ptw);

/** The ctree_update_column_visibility() routine sets/changes
 *    which columns will be visible in the ctree window.  Note
 *    that it does *not* redraw the data in the columns: use
 *    ctree_refresh() for that.
 */
void ctree_update_column_visibility (ProjTreeWindow *ptw);
void ctree_titles_show (ProjTreeWindow *ptw);
void ctree_titles_hide (ProjTreeWindow *ptw);
void ctree_subproj_show (ProjTreeWindow *ptw);
void ctree_subproj_hide (ProjTreeWindow *ptw);

GtkWidget * ctree_get_widget(ProjTreeWindow *);

/** The 'focus project' corresponds to the 'focus row' in the ctree:
 *    its the project that corresponds to where the keyboard events
 *    are directed.
 */
GttProject *ctree_get_focus_project (ProjTreeWindow *);

/** Return true if the ctree widget has input focus. This can 
 *  be used to test the validity of the ctree object as the
 *  target of a cut & paste operation.
 */
gboolean ctree_has_focus (ProjTreeWindow *);

/** The ctree_get_expander_state() routine returns the state of
 *    the row expanders as an ascii string.
 * The ctree_set_expander_state() routine takes this ascii string,
 *    and sets the expanders to match.
 */
const char * ctree_get_expander_state (ProjTreeWindow *);
void  ctree_set_expander_state (ProjTreeWindow *ptw, const char *expn);

#endif /* GTT_CTREE_GNOME2 */
#endif /* __GTT_CTREE_H__ */
