/*   Keyboard inactivity timout dialog for GTimeTracker - a time tracker
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

#include "config.h"

#include <glade/glade.h>
#include <gnome.h>
#include <string.h>

#include <gnc-date.h>

#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "active-dialog.h"
#include "idle-timer.h"
#include "proj.h"
#include "util.h"


int config_active_timeout = -1;

struct GttActiveDialog_s 
{
	GladeXML    *gtxml;
	GtkDialog   *dlg;
	GtkButton   *yes_btn;
	GtkButton   *no_btn;
	
	GttProject  *prj;
	IdleTimeout *idt;
};


/* =========================================================== */

static void
dialog_close (GObject *obj, GttActiveDialog *dlg)
{
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
}

/* =========================================================== */

static void
dialog_kill (GObject *obj, GttActiveDialog *dlg)
{
	gtk_widget_destroy (GTK_WIDGET(dlg->dlg));
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
}

/* =========================================================== */

static void
start_proj (GObject *obj, GttActiveDialog *dlg)
{
		  printf ("duude start proj\n");
	dialog_kill (obj, dlg);
}

/* =========================================================== */
/* XXX the new GtkDialog is broken; it can't hide-on-close,
 * unlike to old, deprecated GnomeDialog.  Thus, we have to
 * do a heavyweight re-initialization each time.  Urgh.
 */

static void
active_dialog_realize (GttActiveDialog * id)
{
	GladeXML *gtxml;

	id->prj = NULL;

	gtxml = gtt_glade_xml_new ("glade/active.glade", "Active Dialog");
	id->gtxml = gtxml;

	id->dlg = GTK_DIALOG (glade_xml_get_widget (gtxml, "Active Dialog"));

	id->yes_btn = GTK_BUTTON(glade_xml_get_widget (gtxml, "yes button"));
	id->no_btn  = GTK_BUTTON(glade_xml_get_widget (gtxml, "no button"));

	g_signal_connect(G_OBJECT(id->dlg), "destroy",
	          G_CALLBACK(dialog_close), id);

	g_signal_connect(G_OBJECT(id->yes_btn), "clicked",
	          G_CALLBACK(start_proj), id);

	g_signal_connect(G_OBJECT(id->no_btn), "clicked",
	          G_CALLBACK(dialog_kill), id);

}

/* =========================================================== */

GttActiveDialog *
active_dialog_new (void)
{
	GttActiveDialog *id;

	id = g_new0 (GttActiveDialog, 1);
	id->idt = idle_timeout_new ();
	id->prj = NULL;

	id->gtxml = NULL;

	return id;
}

/* =========================================================== */

void 
show_active_dialog (GttActiveDialog *id)
{
	if (!id) return;

	/* Due to GtkDialog broken-ness, re-realize the GUI */
	if (NULL == id->gtxml)
	{
		active_dialog_realize (id);
	}

	gtk_widget_show (GTK_WIDGET(id->dlg));
}

/* =========================================================== */

void 
raise_active_dialog (GttActiveDialog *id)
{
	time_t now;
	time_t active_time;

	if (!id) return;
	if (NULL == id->gtxml) return;

	/* If there has not been any activity recently, then leave things
	 * alone. Otherwise, work real hard to put the dialog where the
	 * user will see it.
	 */
	now = time(0);
	active_time = now - poll_last_activity (id->idt);
	if (15 < active_time) return;

	/* The following will raise the window, and put it on the current
	 * workspace, at least if the metacity WM is used. Haven't tried
	 * other window managers.
	 */
	gtk_window_present (GTK_WINDOW (id->dlg));
}

/* =========================== END OF FILE ============================== */
