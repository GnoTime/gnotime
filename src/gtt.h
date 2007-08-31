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

#define GTT_APP_TITLE        "Gnome Time Tracker"
#define GTT_APP_PROPER_NAME  "GnoTime"
#define GTT_APP_NAME         "gnotime"

#define XML_DATA_FILENAME "gnotime.d/gnotime-data.xml"

/* err.c */

void err_init(void);

/* main.c */

/* The save_all() routine will write out all state to files.  
 *    If an error occurs, it returns an error message.
 */
char * save_all (void);

/* The save_properties() routine will write out the application
 * properties to the application file.  It will pop up a warning
 * gui window if the save fails for some reason.
 */
void save_properties (void);

/* The save_projects() routine will write out the project data
 * to the data file.  It will pop up a warning
 * gui window if the save fails for some reason.
 */
void save_projects (void);

/* The read_data() routine will load the project data file 
   and setup the interface with the new data
 */
void read_data (gboolean);

void unlock_gtt(void);
const char *gtt_gettext(const char *s);


#endif /* __GTT_H__ */
