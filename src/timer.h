/*   Low-level timer callbacks and timeouts for GTimeTracker
 *   Copyright (C) 2001, 2002, 2003 Linas Vepstas <linas@linas.org>
 *   Copyright (C) 1997,98 Eckehard Berns
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

#ifndef __GTT_TIMER_H__
#define __GTT_TIMER_H__

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#endif /* TM_IN_SYS_TIME */
#include <time.h>
#endif /* TIME_WITH_SYS_TIME */
#include "proj.h"

void init_timer(void);
gboolean timer_is_running (void);
gboolean timer_project_is_running (GttProject *prj);
/* The idle timeout is how long, in seconds, that the system seems idle
 * before the clock stops itself */
extern int config_idle_timeout;
extern int config_no_project_timeout;

/* The autosave period is how long, in seconds, we wait before doing a
 * periodic save-thyself. */
extern int config_autosave_period;

gint zero_daily_counters (gpointer data);
void set_last_reset (time_t last);

void gen_start_timer(void);
void gen_stop_timer(void);

void start_no_project_timer ();
void start_idle_timer ();

void start_main_timer ();
void stop_main_timer ();

#endif /* __GTT_TIMER_H__ */
