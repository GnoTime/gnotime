/*   Project data query for GTimeTracker
 *   Copyright (C) 2003 Linas Vepstas <linas@linas.org>
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

#ifndef __GTT_QUERY_H__
#define __GTT_QUERY_H__

#include <glib.h>
#include "proj.h"

/* This file contains routines that return various info about
 * the data in the system.  In some fancier world, these would
 * be replaced by a generic query mechanism; but for right now,
 * these are some hard-coded routines that return what we need.
 */


/* The following routines are needed to implement a 
 *    calendar report in GTT.  
 *
 * The gtt_project_get_earliest_start() routine returns
 *    the earliest start time for this project.  In other words,
 *    it returns the date of the earliest activity on this project.
 *    If 'include_subprojects' is TRUE, then subprojects are
 *    included in the search for the earliest start.
 *
 * The gtt_project_get_latest_stop() routine returns
 *    the latest stop time for this project.  In other words,
 *    it returns the date of the last activity on this project.
 *    If 'include_subprojects' is TRUE, then subprojects are
 *    included in the search for the latest stop.
 *
 * The gtt_project_get_daily_time() routine returns 
 *    a GArray containing the total number of seconds 
 *    spent on the project, day by day.  Each element
 *    of the array corresponds to one day.  Day 0 
 *    corresponds to the earliest day for which there
 *    is data for this project.  The length of the array
 *    is sufficient to hold data for all non-zero days.
 *    The array should be freed when it is no longer needed.
 *    Use the gtt_project_get_earliest_start() routine to
 *    find out what day 0 correpinds to in calendar time.
 *    If 'include_subprojects' is TRUE, then subprojects are
 *    included in the day totals.
 */

GArray * gtt_project_get_daily_time (GttProject *proj, 
					      gboolean include_subprojects);

time_t   gtt_project_get_earliest_start (GttProject *proj, 
					      gboolean include_subprojects);

time_t   gtt_project_get_latest_stop (GttProject *proj,
					      gboolean include_subprojects);

#endif /* __GTT_QUERY_H__ */
