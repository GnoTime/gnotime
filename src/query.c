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

#include "config.h"

#include <glib.h>

#include "proj.h"
#include "proj_p.h"
#include "query.h"


GArrary  
gtt_project_get_daily_time (GttProject *proj)
{
}

GArrary  
gtt_project_total_daily_time (GttProject *proj)
{
}

int   
gtt_project_foreach_interval (GttProject *proj, GttIntervalCB cb, gpointer data)
{
	int rc = 0;
	GList *tnode, *inode;
	
	/* Get the list of tasks, and walk the list.  We are not
	 * going to assume that the list is ordered in any way. 
	 */
	tnode = gtt_project_get_tasks (proj);
	for (; tnode; tnode=tnode->next)
	{
		inode = gtt_task_get_intervals (tnode->data);
		for (; inode; inode=inode->next)
		{
			GttInterval *iv = inode->data;
			rc = cb (iv, data);
			if (0 == rc) return rc;
		}
	}
	return rc;
}

int   
gtt_project_foreach_subproject_interval (GttProject *proj, 
					 GttIntervalCB cb, gpointer data)
{
	int rc = 0;
	GList *pnode;
	
	rc = gtt_project_foreach_interval (proj, cb, data);
	if (0 == rc) return 0;
	
	/* Apply to the list of subprojects. */
	pnode = gtt_project_get_children (proj);
	for (; pnode; pnode=pnode->next)
	{
		GttProject *p = pnode->data;
		rc = gtt_project_foreach_subproject_interval (p, cb, data);
		if (0 == rc) return 0;
	}
	return rc;
}

static int cmp_earliest (GttInterval *ivl, gpointer data)
{
	time_t * earliest = data;
	time_t s = gtt_interval_get_start_time (ivl);
	if (s < *earliest) *earliest = s;
	return 1;
}

static int cmp_latest (GttInterval *ivl, gpointer data)
{
	time_t * latest = data;
	time_t s = gtt_interval_get_stop_time (ivl);
	if (s > *latest) *latest = s;
	return 1;
}

time_t   
gtt_project_get_earliest_start (GttProject *proj)
{
	time_t earliest;
	
	earliest = now();
	if (!proj) return  earliest;
	
	gtt_project_foreach_interval (proj, cmp_earliest, &earliest);
	return earliest;
}


time_t   
gtt_project_total_earliest_start (GttProject *proj)
{
	time_t earliest;
	GList *pnode;
	
	earliest = now();
	if (!proj) return  earliest;
	
	gtt_project_foreach_subproject_interval (proj, cmp_earliest, &earliest);
	return earliest;
}

time_t   
gtt_project_get_latest_stop (GttProject *proj)
{
	time_t latest;
	
	latest = now();
	if (!proj) return  latest;
	
	gtt_project_foreach_interval (proj, cmp_latest, &latest);
	return latest;
}


time_t   
gtt_project_total_latest_stop (GttProject *proj)
{
	time_t latest;
	GList *pnode;
	
	latest = now();
	if (!proj) return  latest;
	
	gtt_project_foreach_subproject_interval (proj, cmp_latest, &latest);
	return latest;
}

/* =========================== END OF FILE ========================= */
