/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
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

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "dialog.h"
#include "idle-timer.h"
#include "log.h"
#include "prefs.h"
#include "proj.h"
#include "timer.h"


static gint main_timer = 0;
static IdleTimeout *idt = NULL;

int config_idle_timeout = -1;
int config_autosave_period = 60;

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
		log_endofday();
		year_last_reset = t1->tm_year;
	    	day_last_reset = t1->tm_yday;
	}
}

/* =========================================================== */

static void
restart_proj (GtkWidget *w, gpointer data)
{
	GttProject *prj = data;
	ctree_start_timer (prj);
}

static gint 
timer_func(gpointer data)
{
	time_t now = time(0);

	/* Even if there is no active project,
	 * we still have to zero out the counters periodically. */
	if (0 == now%60) zero_on_rollover (now);

	/* Periodically save data to file -- not strictly neccessary,
	 * but real nice way of avoiding data loss in case something 
	 * core dumps. */
	if (0 == now%config_autosave_period)
	{
		save_projects ();
	}

	if (!cur_proj) return 1;

	gtt_project_timer_update (cur_proj);

	if (config_show_secs) 
	{
		ctree_update_label(global_ptw, cur_proj);
	} 
	else if (1 == gtt_project_get_secs_day(cur_proj) % 5) 
	{
		ctree_update_label(global_ptw, cur_proj);
	}

	if (0 < config_idle_timeout) 
	{
		int idle_time;
		idle_time = now - poll_last_activity (idt);
		/*
printf ("duude idle for =%d\n", idle_time);
*/
		if (idle_time > config_idle_timeout) 
		{
			time_t stop;
			char *msg;
			GttInterval *ivl;
			GttProject *prj = cur_proj;

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
			return 1;
		}
	}
	return 1;
}


static int timer_inited = 0;

void 
init_timer(void)
{
	if (timer_inited) return;
	timer_inited = 1;

	idt = idle_timeout_new();

	/* the timer is measured in milliseconds, so 1000
	 * means it pops once a second. */
	main_timer = gtk_timeout_add(1000, timer_func, NULL);
}

gboolean 
timer_is_running (void)
{
	return (NULL != cur_proj);
}

/* ================================= END OF FILE ============================== */
