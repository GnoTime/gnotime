/* idle-timer.h -- detect user inactivity 
 * Originally taken from:
 * xscreensaver, Copyright (c) 1993-2001 Jamie Zawinski <jwz@jwz.org>
 * hacked for use with gtt, Copyright (c) 2001 Linas Vepstas <linas@linas.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifndef __IDLE_TIMER_H__
#define __IDLE_TIMER_H__

#include <glib.h>
#include <sys/time.h>

typedef struct IdleTimeout_s IdleTimeout;

IdleTimeout * idle_timeout_new (void);

/* The poll_last_activity() routine returns the wall-clock-time of the last
 * user activity on this X server.  i.e. the number of seconds since
 * last activity is given by (time(0) - poll_last_activity())
 */
time_t poll_last_activity (IdleTimeout *si);


#endif /* __IDLE_TIMER_H__ */
