/*   Timer functions for GTimeTracker - a time tracker
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

void init_timer(void);
gboolean timer_is_running (void);
extern int config_idle_timeout;

void zero_on_rollover (time_t now);
void set_last_reset (time_t last);

#endif /* __GTT_TIMER_H__ */
