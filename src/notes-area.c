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

#include "notes-area.h"
#include "util.h"

typedef struct _NotesArea_s
{
	GladeXML *gtxml;
	GtkWidget *vpane;

} NotesArea;

/* ============================================================== */

static NotesArea *
notes_area_new (void)
{
	NotesArea *dlg;
	GladeXML *gtxml;

	dlg = g_new0 (NotesArea, 1);

	gtxml = gtt_glade_xml_new ("glade/notes.glade", "top window");
	dlg->gtxml = gtxml;
	
	dlg->vpane = glade_xml_get_widget (gtxml, "notes vpane");

	gtk_widget_show (dlg->vpane);

	return dlg;
}

/* ============================================================== */

static NotesArea *nadlg = NULL;

void 
notes_area_init (void)
{
	if (!nadlg) nadlg = notes_area_new ();
}


GtkWidget *
notes_area_get_widget (void)
{
	if (!nadlg) nadlg = notes_area_new ();
	return nadlg->vpane;
}

/* ========================= END OF FILE ======================== */

