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

#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "idle-dialog.h"
#include "idle-timer.h"
#include "proj.h"
#include "util.h"


int config_idle_timeout = -1;

struct GttInactiveDialog_s 
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
dialog_close (GObject *obj, GttInactiveDialog *dlg)
{
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
}

/* =========================================================== */

static void
adjust_credit (GObject *obj, GttInactiveDialog *dlg)
{
	printf ("duude adjusting credit\n");
	gtk_widget_destroy (dlg->dlg);
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
}

/* =========================================================== */

static void
restart_proj (GObject *obj, GttInactiveDialog *dlg)
{
	adjust_credit (obj, dlg);
	ctree_start_timer (dlg->prj);
	dlg->prj = NULL;
}

/* =========================================================== */
/* XXX the new GtkDialog is broken; it can't hide-on-close,
 * unlike to old, deprecated GnomeDialog.  Thus, we have to
 * do a heavyweight re-initialization each time.  Urgh.
 */

static void
inactive_dialog_realize (GttInactiveDialog * id)
{
	GladeXML *gtxml;

	id->prj = NULL;

	gtxml = gtt_glade_xml_new ("glade/idle.glade", "Idle Dialog");
	id->gtxml = gtxml;

	id->dlg = GTK_DIALOG (glade_xml_get_widget (gtxml, "Idle Dialog"));

	id->yes_btn = GTK_BUTTON(glade_xml_get_widget (gtxml, "yes button"));
	id->no_btn  = GTK_BUTTON(glade_xml_get_widget (gtxml, "no button"));

	g_signal_connect(G_OBJECT(id->dlg), "destroy",
	          G_CALLBACK(dialog_close), id);

	g_signal_connect(G_OBJECT(id->yes_btn), "clicked",
	          G_CALLBACK(restart_proj), id);

	g_signal_connect(G_OBJECT(id->no_btn), "clicked",
	          G_CALLBACK(adjust_credit), id);

}

/* =========================================================== */

GttInactiveDialog *
inactive_dialog_new (void)
{
	GttInactiveDialog *id;
	GladeXML *gtxml;

	id = g_new0 (GttInactiveDialog, 1);
	id->idt = idle_timeout_new ();
	id->prj = NULL;

	id->gtxml = NULL;

	return id;
}

/* =========================================================== */

void 
show_inactive_dialog (GttInactiveDialog *id)
{
	time_t now;
	time_t idle_time;
	time_t stop;
	char *msg;
	GttInterval *ivl;
	GttProject *prj = cur_proj;

	if (!id) return;
	if (0 > config_idle_timeout) return;

	now = time(0);
	idle_time = now - poll_last_activity (id->idt);
	if (idle_time <= config_idle_timeout) return;

	/* stop the timer on the current project */
	ctree_stop_timer (cur_proj);

	/* The idle timer can trip because gtt was left running
	 * on a laptop, which was them put in suspend mode (i.e.
	 * by closing the cover).  When the laptop is resumed,
	 * the poll_last_activity will return the many hours/days
	 * tht the laptop has been shut down, and meremly stoping
	 * the timer (as above) will credit hours/days to the 
	 * current active project.  We don't want this, we need
	 * to undo this damage.
	 */
	ivl = gtt_project_get_first_interval (prj);
	stop = gtt_interval_get_stop (ivl);
	stop -= idle_time;
	stop += config_idle_timeout;
	gtt_interval_set_stop (ivl, stop);

	if (NULL == id->gtxml)
	{
		inactive_dialog_realize (id);
	}

	/* warn the user */
	msg = g_strdup_printf (
		_("The keyboard and mouse have been idle\n"
		  "for %d minutes.  The currently running\n"
		  "project (%s - %s)\n"
		  "has been stopped.\n"
		  "Do you want to restart it?"),
		(config_idle_timeout+30)/60,
		gtt_project_get_title(prj),
		gtt_project_get_desc(prj));

	id->prj = prj;
	gtk_widget_show (GTK_WIDGET(id->dlg));
}

/* =========================== END OF FILE ============================== */
