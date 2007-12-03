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

static gint main_timer = 0;
static gint file_save_timer = 0;
static gint config_save_timer = 0;
static GttIdleDialog *idle_dialog = NULL;
static GttActiveDialog *active_dialog = NULL;

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


static void schedule_zero_daily_counters_timer (void);

gint
zero_daily_counters (gpointer data)
{
	struct tm *t1;
	time_t now = time(0);

	/* zero out day counts */
	t1 = localtime(&now);
	if ((year_last_reset != t1->tm_year) ||
		(day_last_reset != t1->tm_yday))
	{
		gtt_project_list_compute_secs ();
		ctree_refresh(global_ptw);
		log_endofday();
		year_last_reset = t1->tm_year;
		day_last_reset = t1->tm_yday;
	}
	schedule_zero_daily_counters_timer ();
	return 0;
}

/* =========================================================== */

static gint
file_save_timer_func (gpointer data)
{
	save_projects ();
	return 1;
}

static gint
config_save_timer_func (gpointer data)
{
	save_properties ();
	return 1;
}

static gint
main_timer_func(gpointer data)
{
	time_t now = time(0);

	/* Wake up the notes area GUI, if needed. */
	gtt_notes_timer_callback (global_na);
	gtt_diary_timer_callback (NULL);

	if (!cur_proj)
	{
		main_timer = 0;
		return 0;
	}

	/* Update the data in the data engine. */
	gtt_project_timer_update (cur_proj);

	ctree_update_label(global_ptw, cur_proj);

	return 1;
}

static gboolean timer_inited = FALSE;

void
start_main_timer (void)
{
	if (main_timer)
	{
		g_source_remove (main_timer);
	}

	/* If we're showing seconds, call the timer routine once a second */
	/* else, do it once a minute */
	if (config_show_secs)
	{
		main_timer = g_timeout_add_seconds (1, main_timer_func, NULL);
	}
	else
	{
		main_timer = g_timeout_add_seconds (60, main_timer_func, NULL);
	}
}

static void
start_file_save_timer (void)
{
	g_return_if_fail (!file_save_timer);
	file_save_timer = g_timeout_add_seconds (config_autosave_period,
											 file_save_timer_func, NULL);
}

static void
start_config_save_timer (void)
{
	g_return_if_fail (!config_save_timer);
	config_save_timer = g_timeout_add_seconds (config_autosave_props_period,
											   config_save_timer_func, NULL);
}



void
stop_main_timer (void)
{
	if (cur_proj)
	{
		/* Update the data in the data engine. */
		gtt_project_timer_update (cur_proj);
		ctree_update_label(global_ptw, cur_proj);

	}
	g_return_if_fail (main_timer);
	g_source_remove (main_timer);
	main_timer = 0;
}

void
init_timer(void)
{
	g_return_if_fail (!timer_inited);
	timer_inited = TRUE;

	idle_dialog = idle_dialog_new();
	active_dialog = active_dialog_new();

	start_main_timer ();
	start_file_save_timer ();
	start_config_save_timer ();
}

gboolean
timer_is_running (void)
{
	return (NULL != cur_proj);
}

void
start_idle_timer (void)
{
	if (!timer_inited)
	{
		init_timer();
	}

	if (timer_is_running ())
	{
		idle_dialog_activate_timer (idle_dialog);
		active_dialog_deactivate_timer (active_dialog);
	}
}

void
start_no_project_timer (void)
{
	if (!timer_inited)
	{
		init_timer();
	}
	if (!idle_dialog_is_visible (idle_dialog) && timer_is_running ())
	{
		idle_dialog_deactivate_timer (idle_dialog);
		active_dialog_activate_timer (active_dialog);
	}
}

static void
schedule_zero_daily_counters_timer (void)
{
	time_t now = time(0);
	time_t timeout = 3600 - (now % 3600);
	g_timeout_add_seconds (timeout, zero_daily_counters, NULL);
}
/* ========================== END OF FILE ============================ */
