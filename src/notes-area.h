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

typedef struct NotesArea_s NotesArea;

NotesArea * notes_area_new (void);

/* bind a project to the notes area */
void notes_area_set_project (NotesArea *na, GttProject *proj);

/* returns the vpaned widget at the top of the notes area heirarchy */
GtkWidget * notes_area_get_widget (NotesArea *na);

/* add the ctree widget to the appropriate location */
void notes_area_add_ctree (NotesArea *na, GtkWidget *ctree);

/* Set the position of the two divideders in the notes area:
 * the vertical divider between the ctree and the notes,
 * and the horiz divider between the proj on left and diary on right
 */
void notes_area_get_pane_sizes (NotesArea *na, int *vp, int *hp);
void notes_area_set_pane_sizes (NotesArea *na, int vp, int hp);


#endif /* GTT_NOTES_AREA_H */
