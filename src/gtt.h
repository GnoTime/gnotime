/*   GTimeTracker - a time tracker
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

#ifndef __GTT_H__
#define __GTT_H__


#include <gnome.h>

#ifdef DEBUG
#define APP_NAME "GTimeTracker DEBUG"
#else
#define APP_NAME "GTimeTracker"
#endif

#define XML_DATA_FILENAME "gtt-xml.gttml"

/* err.c */

void err_init(void);

/* main.c */

/* The save_all() routine will write out all state to files.  
 *    If an error occurs, it returns an error message.
 */
const char * save_all (void);
void unlock_gtt(void);
const char *gtt_gettext(const char *s);

#define gtt_sure_string(x) ((x)?(x):"")


#endif /* __GTT_H__ */
