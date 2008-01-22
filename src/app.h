/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
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

#ifndef __GTT_APP_H__
#define __GTT_APP_H__

#include <gnome.h>
#include "notes-area.h"
#include "proj.h"
#include "status-icon.h"


extern NotesArea *global_na;        /* global ptr to notes GUI area */
extern GttProjectsTree *projects_tree;

extern GtkWidget *app_window;  /* global top-level window */
extern GtkWidget *status_bar;

/* true if command line over-rides geometry */
extern gboolean geom_size_override;
extern gboolean geom_place_override;

void update_status_bar(void);

void app_new(int argc, char *argv[], const char *geometry_string);

void app_show(void);
void app_quit(GtkWidget *w, gpointer data);
		  
/* The ctree will call 'focus_row_set' whenever the focus row changes.
 * This is used in turn as a cheesey way  to re-distribute this event
 * to other subsystems.  Should be replaced ultimately by g_signals.
 */

void focus_row_set (GttProject *);

/** Run the start/stop shell command for the indicated project.
 *  boolean is true for start, false for stop.  We export this
 *  function only to handle application shutdown when getting signal.
 */
void run_shell_command (GttProject *, gboolean do_start);

/** Run the shell command. */
void do_run_shell_command (const char * str);

#endif /* __GTT_APP_H__ */
