/*   Catch/block signals, x11 errors, clean up gracefully.
 *   A part of GTimeTracker - a time tracker
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

#include <config.h>
#include <gnome.h>

#include "gtt.h"
 
#include <X11/Xlib.h>
#include <signal.h>


#undef DIE_ON_NORMAL_ERROR


/* =================================================== */
/* error handlers for 'unavoidable' system errors */

static void die(void)
{
	fprintf(stderr, " - saving and dying\n");
	save_all();
	unlock_gtt();
	exit(1);
}



static void sig_handler(int signum)
{
	fprintf(stderr, "%s: Signal %d caught", APP_NAME, signum);
	die();
}


#ifdef DIE_ON_NORMAL_ERROR
static int x11_error_handler(Display *d, XErrorEvent *e)
{
	fprintf(stderr, "%s: X11 error caight", APP_NAME);
	die();
	return 0; /* keep the compiler happy */
}
#endif

static int x11_io_error_handler(Display *d)
{
	fprintf(stderr, "%s: fatal X11 io error caight", APP_NAME);
	die();
	return 0; /* keep the compiler happy */
}

void err_init(void)
{
	static int inited = 0;
	
	if (inited) return;
#ifdef DIE_ON_NORMAL_ERROR
	XSetErrorHandler(x11_error_handler);
#endif
	signal(SIGINT, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGPIPE, sig_handler);
	XSetIOErrorHandler(x11_io_error_handler);
}

/* =========================== END OF FILE ======================== */
