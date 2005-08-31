/* Low-level timer callbacks & timeout handlers for GTimeTracker 
 * Copyright (C) 1997,98 Eckehard Berns
 * Copyright (C) 2001,2002, 2003 Linas Vepstas <linas@linas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <gnome.h>
#include <string.h>

#include "active-dialog.h"
#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "gtt.h"
#include "idle-dialog.h"
#include "log.h"
#include "notes-area.h"
#include "prefs.h"
#include "proj.h"
#include "props-task.h"
#include "timer.h"


int config_autosave_period = 60;
int config_autosave_props_period = (4*3600);
GttActiveDialog *act = NULL;

static gint main_timer = 0;
static GttIdleDialog *idt = NULL;

/* =========================================================== */
/* zero out day counts if rolled past midnight */

static int day_last_reset = -1;
static int year_last_reset = -1;

void
set_last_reset (time_t last)
{
	struct tm *t0;
	t0 = localtime (&last);
	day_last_reset = t0->tm_yday;
	year_last_reset = t0->tm_year;
}


void
zero_on_rollover (time_t when)
{
	struct tm *t1;

	/* zero out day counts */
	t1 = localtime(&when);
	if ((year_last_reset != t1->tm_year) ||
	    (day_last_reset != t1->tm_yday)) 
	{
		gtt_project_list_compute_secs ();
		ctree_refresh(global_ptw);
		log_endofday();
		year_last_reset = t1->tm_year;
		day_last_reset = t1->tm_yday;
	}
}

/* =========================================================== */

static gint 
timer_func(gpointer data)
{
	time_t now = time(0);

	/* Even if there is no active project, we still have to zero out 
	 * the day/week/month counters periodically. */
	if (0 == now%60) zero_on_rollover (now);

	/* Periodically save data to file -- this is how we avoid data loss
	 * in case Gnotime or X11 or the OS crashes. */
	if (0 == now%config_autosave_period)
	{
		save_projects ();
	}

	/* Save current configuration, but less often */
	if (0 == now%config_autosave_props_period)
	{
		save_properties ();
	}

	/* Wake up the notes area GUI, if needed. */
	gtt_notes_timer_callback (global_na);
	gtt_diary_timer_callback (NULL);
	
	/* If no project is running, but there is keyboard/mouse activity, 
	 * remind user to either restart the timer on an expired project,
	 * or to pick a new project, as appropriate.
	 */
	if (!cur_proj) 
	{
		if (0 < config_no_project_timeout)
		{
			/* Make sure the idle dialog is visible */
			raise_idle_dialog (idt);
			show_active_dialog (act);
		}
		return 1;
	}

	/* Update the data in the data engine. */
	gtt_project_timer_update (cur_proj);

	/* Update the GUI display, once a minute or once a second,
	 * depending on the user preferences */
	if (config_show_secs) 
	{
		ctree_update_label(global_ptw, cur_proj);
	} 
	else if (1 == gtt_project_get_secs_day(cur_proj) % 5) 
	{
		ctree_update_label(global_ptw, cur_proj);
	}

	/* Look for keyboard/mouse inactivity, and expire (stop) 
	 * the timer if needed. */
	if (0 < config_idle_timeout) 
	{
		show_idle_dialog (idt);
		cancel_active_dialog (act);
	}
	return 1;
}


static gboolean timer_inited = FALSE;

void 
init_timer(void)
{
	if (timer_inited) return;
	timer_inited = TRUE;

	idt = idle_dialog_new();
	act = active_dialog_new();
	
	/* The timer is measured in milliseconds, so 1000
	 * means it pops once a second. */
	main_timer = gtk_timeout_add(1000, timer_func, NULL);
}

gboolean 
timer_is_running (void)
{
	return (NULL != cur_proj);
}

/* ========================== END OF FILE ============================ */
