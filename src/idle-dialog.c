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

#include <config.h>
#include <gnome.h>
#include <string.h>

#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "idle-dialog.h"
#include "idle-timer.h"
#include "proj.h"


int config_idle_timeout = -1;

struct GttInactiveDialog_s 
{
	IdleTimeout *idt;
};


/* =========================================================== */

GttInactiveDialog *
inactive_dialog_new (void)
{
	GttInactiveDialog *id;

	id = g_new0 (GttInactiveDialog, 1);
	id ->idt = idle_timeout_new ();
	return id;
}

/* =========================================================== */

static void
restart_proj (GtkWidget *w, gpointer data)
{
	GttProject *prj = data;
	ctree_start_timer (prj);
}

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
	qbox_ok_cancel (_("System Idle"), msg,
		GTK_STOCK_YES, restart_proj, prj, 
		GTK_STOCK_NO, NULL, NULL);
}

/* =========================== END OF FILE ============================== */
