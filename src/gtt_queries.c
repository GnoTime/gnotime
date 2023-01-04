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
#include <limits.h>

#include "prefs.h"  /* XXX tmp hack for global config_daystart */
#include "proj.h"
#include "proj_p.h"
#include "query.h"

/* ========================================================== */

typedef struct DayArray_s
{
	int array_len;           /* same as number of days */
	GArray *buckets;         /* holds array of GttBucket */
	int start_cday;          /* day since 1900 */
	struct tm start_tm;      /* start time struct */
} DayArray;

static inline int
yearday_to_centuryday (int yday, int year)
{
	int cd;
	cd = 365*year + (year-1)/4 + yday;
  cd -= (year - 1)/100; /* year that are multiple of 100 are not leap years */
	return cd;
}

/* ========================================================== */
/** Sort list of tasks into daily bins */

static int
day_bin (GttInterval *ivl, gpointer data)
{
	DayArray *da = data;
	time_t start, start_off, stop, end_of_day;
	struct tm stm;
	int century_day, arr_day;
	GttTask *tsk;

	tsk = gtt_interval_get_parent (ivl);
	start = gtt_interval_get_start (ivl);
	stop = gtt_interval_get_stop (ivl);

	/* Get the starting point in array based on day-of-century */
	start_off = start - config_daystart_offset;
	localtime_r (&start_off, &stm);
	century_day = yearday_to_centuryday (stm.tm_yday, stm.tm_year);
	arr_day = century_day - da->start_cday;

	/* Loop over days until last day in interval */
	stm.tm_sec = 0;
	stm.tm_min = 0;
	stm.tm_hour = 0;

	while (1)
	{
		/* Check error bounds, should never happen */
		if ((0 > arr_day) || (arr_day >= da->array_len))
		{
			return 1;
		}
		GttBucket *bu;
		bu = &g_array_index (da->buckets, GttBucket, arr_day);

		stm.tm_mday ++;
		end_of_day = mktime (&stm);
		/* config_daystart_offset==3*3600 means new day starts at 3AM */
		end_of_day += config_daystart_offset;
		
		if (stop < end_of_day)
		{
			bu->total += stop - start;
			bu->intervals = g_list_append (bu->intervals, ivl);

			/* Avoid duplicate tasks by checking if same as last */
			if (!bu->tasks || (bu->tasks->data != tsk))
			{
				bu->tasks = g_list_prepend (bu->tasks, tsk);
			}
			return 1;
		}
		else
		{
			bu->total+= end_of_day - start;
			bu->intervals = g_list_append (bu->intervals, ivl);

			/* Avoid duplicate tasks by checking if same as last */
			if (!bu->tasks || (bu->tasks->data != tsk))
			{
				bu->tasks = g_list_prepend (bu->tasks, tsk);
			}
		}
		arr_day ++;
		start = end_of_day;
	}
	
	return 1;
}

/* ========================================================== */
/** Set up the DayArray object so that the length (in days) is known,
 *  and so that the start day is known
 */
static void
count_days (DayArray *da, GttProject *proj, gboolean include_subprojects)
{
	time_t start, stop;
	int num_days;

	/* Figure out how many days in the array */
	start = gtt_project_get_earliest_start (proj, include_subprojects);
	stop = gtt_project_get_latest_stop (proj, include_subprojects);
	num_days = (stop - start) / (24*3600);
	num_days += 2; /* two midnights might be crossed */

	da->array_len = num_days+1;
	
	localtime_r (&start, &da->start_tm);
	
	da->start_cday = yearday_to_centuryday (da->start_tm.tm_yday, da->start_tm.tm_year);
}


static void
run_daily_bins(DayArray *da, GttProject *proj, gboolean include_subprojects)
{
	int i;
	
	/* apply recursively */
	if (include_subprojects)
	{
		gtt_project_foreach_subproject_interval (proj, day_bin, da);
	}
	else
	{
		gtt_project_foreach_interval (proj, day_bin, da);
	}

	/* Clean up the taks lists by removing duplicates */
	for (i=0; i<da->array_len; i++)
	{
		GttBucket *bu;
		GList *node;
		bu = &g_array_index (da->buckets, GttBucket, i);

		/* Reverse the list, since they went in backwards */
		bu->tasks = g_list_reverse (bu->tasks);
		
		/* Scrub out duplicate task entries */
		for (node=bu->tasks; node; node=node->next)
		{
			GList *sode;
rerun:
			for (sode=node->next; sode; sode=sode->next)
			{
				if (sode->data == node->data)
				{
					sode = g_list_delete_link (node, sode);
					goto rerun;
				}
			}
		}
	}
}

/* ========================================================== */
/** Initialize array of buckets */

static void
init_bins (DayArray *da)
{
	time_t start_of_day, end_of_day;
	struct tm stm;
	int i;

	/* Use system routines to get things like day-light savings
	 * correct.  Otherwise, we could just have += 24*3600 */
	stm = da->start_tm;
	stm.tm_sec = 0;
	stm.tm_min = 0;
	stm.tm_hour = 0;

	end_of_day = mktime (&stm);
	/* config_daystart_offset==3*3600 means new day starts at 3AM */
	end_of_day += config_daystart_offset;
	for (i=0; i<da->array_len; i++)
	{
		GttBucket *bu;
		bu = &g_array_index (da->buckets, GttBucket, i);
		
		start_of_day = end_of_day;
		stm.tm_mday ++;
		end_of_day = mktime (&stm);
		end_of_day += config_daystart_offset;
		
		bu->start = start_of_day;
		bu->end = end_of_day;
		bu->total = 0;
		bu->tasks = NULL;
		bu->intervals = NULL;
	}
}

/* ========================================================== */

GArray *
gtt_project_get_daily_buckets (GttProject *proj, gboolean include_subprojects)
{
	DayArray da;
	GArray *arr = NULL;

	if (!proj) return NULL;
	
	/* Figure out how many days in the array */
	count_days (&da, proj, include_subprojects);
	if (0 > da.array_len) return NULL;

	/* Alloc the array, fill it in, return the results */
	arr = g_array_new (FALSE, TRUE, sizeof (GttBucket));
	g_array_set_size (arr, da.array_len);

	da.buckets = arr;
	init_bins (&da);
	run_daily_bins (&da, proj, include_subprojects);

	return arr;
}

/* ========================================================== */

int
gtt_project_foreach_interval (GttProject *proj, GttIntervalCB cb, gpointer data)
{
	int rc = 1;
	GList *tnode, *inode;
	
	/* Get the list of tasks, and walk the list.  We are not
	 * going to assume that the list is ordered in any way.
	 */
	tnode = gtt_project_get_tasks (proj);
	for (; tnode; tnode=tnode->next)
	{
		GttTask *tsk = tnode->data;
		inode = gtt_task_get_intervals (tsk);
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

/* ========================================================== */

static int cmp_earliest (GttInterval *ivl, gpointer data)
{
	time_t * earliest = data;
	time_t s = gtt_interval_get_start (ivl);
	if (s < *earliest) *earliest = s;
	return 1;
}

static int cmp_latest (GttInterval *ivl, gpointer data)
{
	time_t * latest = data;
	time_t s = gtt_interval_get_stop (ivl);
	if (s > *latest) *latest = s;
	return 1;
}

time_t
gtt_project_get_earliest_start (GttProject *proj, gboolean include_subprojects)
{
	time_t earliest;
	
	earliest = INT_MAX;
	if (!proj) return  earliest;
	
	if (include_subprojects)
	{
		gtt_project_foreach_subproject_interval (proj, cmp_earliest, &earliest);
	}
	else
	{
		gtt_project_foreach_interval (proj, cmp_earliest, &earliest);
	}
	return earliest;
}


time_t
gtt_project_get_latest_stop (GttProject *proj, gboolean include_subprojects)
{
	time_t latest;
	
	latest = 0;
	if (!proj) return  latest;
	
	if (include_subprojects)
	{
		gtt_project_foreach_subproject_interval (proj, cmp_latest, &latest);
	}
	else
	{
		gtt_project_foreach_interval (proj, cmp_latest, &latest);
	}
	return latest;
}

/* =========================== END OF FILE ========================= */
