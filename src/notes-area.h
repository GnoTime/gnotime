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

#ifndef GTT_NOTES_AREA_H
#define GTT_NOTES_AREA_H

#include <gnome.h>
#include "projects-tree.h"

typedef struct NotesArea_s NotesArea;

NotesArea * notes_area_new (void);

/* The notes_area_set_project() routine binds a project to the 
 *    notes area.  That is, the notes area will display (and edit)
 *    the indicated project.
 */
void notes_area_set_project (NotesArea *na, GttProject *proj);

/* returns the vpaned widget at the top of the notes area heirarchy */
GtkWidget * notes_area_get_widget (NotesArea *na);

/* add the GttProjectsTree widget to the appropriate location */
void notes_area_add_projects_tree (NotesArea *na, GttProjectsTree *projects_tree);

/* Set the position of the two divideders in the notes area:
 * the vertical divider between the ctree and the notes,
 * and the horiz divider between the proj on left and diary on right
 */
void notes_area_get_pane_sizes (NotesArea *na, int *vp, int *hp);
void notes_area_set_pane_sizes (NotesArea *na, int vp, int hp);

/* The gtt_notes_timer_callback() routine is a 'private' routine, 
 * a timeout callback that is called by the timer.
 */
void gtt_notes_timer_callback (NotesArea *na);

#endif /* GTT_NOTES_AREA_H */
