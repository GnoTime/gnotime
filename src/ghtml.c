/*   Generate gtt-parsed html output for GnoTime - a time tracker
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
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

#define _GNU_SOURCE
#include <glib.h>
#include <guile/gh.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "gtt.h"
#include "ghtml.h"
#include "ghtml-deprecated.h"
#include "proj.h"
#include "query.h"
#include "util.h"


/* Design problems:
 * The way this is currently defined, there is no type safety, and
 * we could easily be slipping the wrong addresses to the wrong places.
 * We really should add a type identifier to the the project,
 * task and interval objects.  This type should be in the cdr
 * part of the object, so that it will stop recursion and list-walking.
 * (The utility routines in gtt.scm walk lists and trees; thus,
 * putting a non-pointer in the cdr is an effective way to stop the 
 * recursion.)  The 'daily-totals' object already follows this 
 * convention.
 */

/* ============================================================== */
/* Seems to me like guile screwed the pooch; we need a global! */

GttGhtml *ghtml_guile_global_hack = NULL;   

/* ============================================================== */
/* This routine will reverse the order of a scheme list */

static SCM
reverse_list (SCM node_list)
{
	SCM rc, node;
	rc = SCM_EOL;

	while (FALSE == SCM_NULLP(node_list))
	{
		node = gh_car (node_list);
		rc = gh_cons (node, rc);
		node_list = gh_cdr (node_list);
	}
	return rc;
}

/* ============================================================== */
/* A routine to recursively apply a scheme form to a hierarchical 
 * list of gtt projects.  It returns the result of the apply. */

static SCM
do_apply_on_project (GttGhtml *ghtml, SCM project, 
             SCM (*func)(GttGhtml *, GttProject *))
{
	GttProject * prj;
	SCM rc;

	/* If its a number, its in fact a pointer to the C struct. */
	if (SCM_NUMBERP(project))
	{
		prj = (GttProject *) gh_scm2ulong (project);
		rc = func (ghtml, prj);
		return rc;
	}

	/* If its a list, then process the list */
	else if (SCM_CONSP(project))
	{
		SCM proj_list = project;
		
		/* Get a pointer to null */
		rc = SCM_EOL;
	
		while (FALSE == SCM_NULLP(proj_list))
		{
			SCM evl;
			project = gh_car (proj_list);
			evl = do_apply_on_project (ghtml, project, func);
			if (FALSE == SCM_NULLP (evl))
			{
				rc = gh_cons (evl, rc);
			}
			proj_list = gh_cdr (proj_list);
		}

		/* reverse the list. Ughh */
		/* gh_reverse (rc);  this doesn't work, to it manually */
		rc = reverse_list (rc);
		
		return rc;
	}

	/* If its a null list, do nothing */
	else if (SCM_NULLP (project))
	{ 
		return project;
	}
	
	g_warning ("expecting gtt project as argument, got something else\n");
	return SCM_EOL;
}

/* ============================================================== */
/* A routine to recursively apply a scheme form to a flat 
 * list of gtt tasks.  It returns the result of the apply. */

static SCM
do_apply_on_task (GttGhtml *ghtml, SCM task, 
             SCM (*func)(GttGhtml *, GttTask *))
{
	GttTask * tsk;
	SCM rc;

	/* If its a number, its in fact a pointer to the C struct. */
	if (SCM_NUMBERP(task))
	{
		tsk = (GttTask *) gh_scm2ulong (task);
		rc = func (ghtml, tsk);
		return rc;
	}

	/* If its a list, then process the list */
	else if (SCM_CONSP(task))
	{
		SCM task_list = task;
		
		/* Get a pointer to null */
		rc = SCM_EOL;
	
		while (FALSE == SCM_NULLP(task_list))
		{
			SCM evl;
			task = gh_car (task_list);
			evl = do_apply_on_task (ghtml, task, func);
			if (FALSE == SCM_NULLP (evl))
			{
				rc = gh_cons (evl, rc);
			}
			task_list = gh_cdr (task_list);
		}

		/* reverse the list. Ughh */
		/* gh_reverse (rc);  this doesn't work, to it manually */
		rc = reverse_list (rc);
		
		return rc;
	}

	/* If its a null list, do nothing */
	else if (SCM_NULLP (task))
	{ 
		return task;
	}
	
	g_warning ("expecting gtt task as argument, got something else\n");
	return SCM_EOL;
}

/* ============================================================== */
/* A routine to recursively apply a scheme form to a flat 
 * list of gtt intervals.  It returns the result of the apply. */

static SCM
do_apply_on_interval (GttGhtml *ghtml, SCM invl, 
             SCM (*func)(GttGhtml *, GttInterval *))
{
	GttInterval * ivl;
	SCM rc;

	/* If its a number, its in fact a pointer to the C struct. */
	if (SCM_NUMBERP(invl))
	{
		ivl = (GttInterval *) gh_scm2ulong (invl);
		rc = func (ghtml, ivl);
		return rc;
	}

	/* If its a list, then process the list */
	else if (SCM_CONSP(invl))
	{
		SCM invl_list = invl;
		
		/* Get a pointer to null */
		rc = SCM_EOL;
	
		while (FALSE == SCM_NULLP(invl_list))
		{
			SCM evl;
			invl = gh_car (invl_list);
			evl = do_apply_on_interval (ghtml, invl, func);
			if (FALSE == SCM_NULLP (evl))
			{
				rc = gh_cons (evl, rc);
			}
			invl_list = gh_cdr (invl_list);
		}

		/* reverse the list. Ughh */
		/* gh_reverse (rc);  this doesn't work, to it manually */
		rc = reverse_list (rc);
		
		return rc;
	}

	/* If its a null list, do nothing */
	else if (SCM_NULLP (invl))
	{ 
		return invl;
	}
	
	
	g_warning ("expecting gtt interval as argument, got something else\n");
	return SCM_EOL;
}

/* ============================================================== */
/* This routine accepts an SCM node, and 'prints' it out.
 * (or tries to). It knows how to print numbers strings and lists.
 */

static SCM
do_show_scm (GttGhtml *ghtml, SCM node)
{
	size_t len;
	char * str = NULL;

	if (NULL == ghtml->write_stream) return SCM_EOL;

	/* Need to test for numbers first, since later tests 
	 * may core-dump guile-1.3.4 if the node was in fact a number. */
	if (SCM_NUMBERP(node))
	{
		char buf[132];
		double x;
		long  ix;

		x = gh_scm2double (node);
		ix = (long) x;

		/* If the number is representable in 32 bits,
		 * and if the fractional part is so small its
		 * not representable as a double, then print it
		 * as an integer. */
		if ((INT_MAX > x) && (-INT_MAX < x) &&
			 ((x-(double)ix) < 4.0*x*DBL_EPSILON) )
		{
			sprintf (buf, "%d", ix);
		}
		else
		{
			sprintf (buf, "%26.18g", x);
		}
		(ghtml->write_stream) (ghtml, buf, strlen(buf), ghtml->user_data);
	}
	else
	/* either a 'symbol or a "quoted string" */
	if (SCM_SYMBOLP(node) || SCM_STRINGP (node))
	{
		str = gh_scm2newstr (node, &len);
		if (0<len) (ghtml->write_stream) (ghtml, str, len, ghtml->user_data);
		free (str);
	}
	else
	if (SCM_CONSP(node))
	{
		SCM node_list = node;
		while (FALSE == SCM_NULLP(node_list))
		{
			node = gh_car (node_list);
			do_show_scm (ghtml, node);
			node_list = gh_cdr (node_list);
		}
	}
	else
	if (SCM_BOOLP(node))
	{
		const char *str;
		if (SCM_FALSEP(node)) str = _("False");
		else str = _("True");
		(ghtml->write_stream) (ghtml, str, strlen(str), ghtml->user_data);
	}
	else
	if (SCM_NULLP(node))  
	{
		/* No op; maybe this should be a warning? */
	}
	else
	{
		g_warning ("Don't know how to gtt-show this type\n");
	}

	/* We could return the printed string, but I'm not sure why.. */
	return SCM_EOL;
}

static SCM
show_scm (SCM node_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_show_scm (ghtml, node_list);
}

/* ============================================================== */
/* Cheesy hack, this returns a pointer to the currently
 * selected project as a ulong.  Its baaaad,  but acheives its 
 * purpose for now.   Its baad because the C pointer is not
 * currently checked before use.  This could lead to core dumps
 * if the scheme code was bad.  It sure would be nice to be
 * able to check that the pointer is a valid pointer to a gtt
 * project.  For example, maybe projects and tasks should be
 * GObjects, and then would could check the cast.  Later. 
 * --linas
 */

/* The 'selected project' is the project highlighted by the 
 * focus row in the main window.
 */

static SCM
do_ret_selected_project (GttGhtml *ghtml)
{
	SCM rc;
	GttProject *prj = ctree_get_focus_project (global_ptw);
	rc = gh_ulong2scm ((unsigned long) prj);
	return rc;
}

static SCM
ret_selected_project (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_ret_selected_project (ghtml);
}

static SCM
do_ret_linked_project (GttGhtml *ghtml)
{
	SCM rc;
	rc = gh_ulong2scm ((unsigned long) ghtml->prj);
	return rc;
}

static SCM
ret_linked_project (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_ret_linked_project (ghtml);
}

/* ============================================================== */
/* Control printing of internal links */

static SCM
do_set_links_on (GttGhtml *ghtml)
{
	if (FALSE == ghtml->really_hide_links)
	{
		ghtml->show_links = TRUE;
	}
	return SCM_EOL;
}

static SCM
set_links_on (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_set_links_on (ghtml);
}

static SCM
do_set_links_off (GttGhtml *ghtml)
{
	ghtml->show_links = FALSE;
	return SCM_EOL;
}

static SCM
set_links_off (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_set_links_off (ghtml);
}

/* ============================================================== */
/* Return a list of all of the projects */

static SCM
do_ret_projects (GttGhtml *ghtml, GList *proj_list)
{
	SCM rc;
	GList *n;

	/* Get a pointer to null */
	rc = SCM_EOL;
	if (!proj_list) return rc;
	
	/* find the tail */
	for (n= proj_list; n->next; n=n->next) {}
	proj_list = n;
	
	/* Walk backwards, creating a scheme list */
	for (n= proj_list; n; n=n->prev)
	{
		GttProject *prj = n->data;
      SCM node;
		GList *subprjs;
		
		/* handle sub-projects, if any, before the project itself */
		subprjs = gtt_project_get_children (prj);
		if (subprjs)
		{
			node = do_ret_projects (ghtml, subprjs);
			rc = gh_cons (node, rc);
		}

		node = gh_ulong2scm ((unsigned long) prj);
		rc = gh_cons (node, rc);
	}
	return rc;
}

static SCM
ret_projects (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	
	/* Get list of all top-level projects */
	GList *proj_list = gtt_get_project_list();
	return do_ret_projects (ghtml, proj_list);
}

/* ============================================================== */
/* Return a list of all of the tasks of a project */

static SCM
do_ret_tasks (GttGhtml *ghtml, GttProject *prj)
{
	SCM rc;
	GList *n, *task_list;

	/* Get a pointer to null */
	rc = SCM_EOL;
	if (!prj) return rc;
	
	/* Get list of tasks, then get tail */
	task_list = gtt_project_get_tasks (prj);
	if (!task_list) return rc;
	
	for (n= task_list; n->next; n=n->next) {}
	task_list = n;
	
	/* Walk backwards, creating a scheme list */
	for (n= task_list; n; n=n->prev)
	{
		GttTask *tsk = n->data;
      SCM node;
		
		node = gh_ulong2scm ((unsigned long) tsk);
		rc = gh_cons (node, rc);
	}
	return rc;
}

static SCM
ret_tasks (SCM proj_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_apply_on_project (ghtml, proj_list, do_ret_tasks);
}

/* ============================================================== */
/* Return a list of all of the intervals of a task */

static SCM
do_ret_intervals (GttGhtml *ghtml, GttTask *tsk)
{
	SCM rc;
	GList *n, *ivl_list;

	/* Oddball hack to make interval datestamp printing work nicely */
	ghtml->last_ivl_time = 0;

	/* Get a pointer to null */
	rc = SCM_EOL;
	if (!tsk) return rc;
	
	/* Get list of intervals, then get tail */
	ivl_list = gtt_task_get_intervals (tsk);
	if (!ivl_list) return rc;
	
	for (n= ivl_list; n->next; n=n->next) {}
	ivl_list = n;
	
	/* Walk backwards, creating a scheme list */
	for (n= ivl_list; n; n=n->prev)
	{
		GttInterval *ivl = n->data;
		SCM node;
		
		node = gh_ulong2scm ((unsigned long) ivl);
		rc = gh_cons (node, rc);
	}
	return rc;
}

static SCM
ret_intervals (SCM task_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_apply_on_task (ghtml, task_list, do_ret_intervals);
}


/* ============================================================== */
/* Return a list of date handles for accessing daily totals */

static SCM
do_ret_daily_totals (GttGhtml *ghtml, GttProject *prj)
{
	SCM rc, rpt;
	int i;
	GArray *arr;  
	time_t earliest;
	struct tm tday;
	
	/* Get a pointer to null */
	rc = SCM_EOL;
	if (!prj) return rc;
	
	/* Get the project data */
	arr = gtt_project_get_daily_buckets (prj, TRUE);
	earliest = gtt_project_get_earliest_start (prj, TRUE);
	
	/* Format the start date */
	localtime_r (&earliest, &tday);
	tday.tm_mday --;

	for (i=0; i< arr->len; i++)
	{
		GttBucket *bu;
		char buff[100];
		SCM node;
		time_t rptdate, secs;
		
		tday.tm_mday ++;
		bu = & g_array_index (arr, GttBucket, i);
		secs = bu->total;

		/* Skip days for which no time has been spent */
		if (0 == secs) continue;

		/* Print time spent on project this day */
		print_hours_elapsed (buff, 100, secs, TRUE);
		node = gh_str2scm (buff, strlen (buff));
		rpt = gh_cons (node, SCM_EOL);
		
		/* XXX report date should be time_t in the middle of the interval */
		/* Print date */
		rptdate = mktime (&tday);
		print_date (buff, 100, rptdate);
		node = gh_str2scm (buff, strlen (buff));
		rpt = gh_cons (node, rpt);

		/* Put a data type in the cdr slot */
		node = gh_str2scm ("daily", 5);
		rpt = gh_cons (rpt, node);
		
		rc = gh_cons (rpt, rc);
	}
	g_array_free (arr, TRUE);

	return rc;
}

static SCM
ret_daily_totals (SCM proj_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_apply_on_project (ghtml, proj_list, do_ret_daily_totals);
}


/* ============================================================== */
/* Define a set of subroutines that accept a scheme list of projects,
 * applies the gtt_project function on each, and then returns a 
 * scheme list containing the results.   
 *
 * For example, ret_project_title() takes a scheme list of gtt
 * projects, gets the project title for each, and then returns
 * a scheme list of the project titles.
 */

#define RET_PROJECT_SIMPLE(RET_FUNC,DO_SIMPLE)                      \
static SCM                                                          \
RET_FUNC (SCM proj_list)                                            \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_project (ghtml, proj_list, DO_SIMPLE);        \
}


#define RET_PROJECT_STR(RET_FUNC,GTT_GETTER)                        \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttProject *prj)                 \
{                                                                   \
	const char * str = GTT_GETTER (prj);                             \
	if (NULL == str) return SCM_EOL;                             \
	return gh_str2scm (str, strlen (str));                           \
}                                                                   \
RET_PROJECT_SIMPLE(RET_FUNC,GTT_GETTER##_scm)
		  

#define RET_PROJECT_LONG(RET_FUNC,GTT_GETTER)                       \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttProject *prj)                 \
{                                                                   \
	long i = GTT_GETTER (prj);                                       \
	return gh_long2scm (i);                                          \
}                                                                   \
RET_PROJECT_SIMPLE(RET_FUNC,GTT_GETTER##_scm)

                                                                    \
#define RET_PROJECT_ULONG(RET_FUNC,GTT_GETTER)                      \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttProject *prj)                 \
{                                                                   \
	unsigned long i = GTT_GETTER (prj);                              \
	return gh_ulong2scm (i);                                         \
}                                                                   \
RET_PROJECT_SIMPLE(RET_FUNC,GTT_GETTER##_scm)



RET_PROJECT_STR   (ret_project_title, gtt_project_get_title)
RET_PROJECT_STR   (ret_project_desc,  gtt_project_get_desc)
RET_PROJECT_STR   (ret_project_notes, gtt_project_get_notes)
RET_PROJECT_ULONG (ret_project_est_start, gtt_project_get_estimated_start)
RET_PROJECT_ULONG (ret_project_est_end,   gtt_project_get_estimated_end)
RET_PROJECT_ULONG (ret_project_due_date,  gtt_project_get_due_date)

RET_PROJECT_LONG  (ret_project_sizing,  gtt_project_get_sizing)
RET_PROJECT_LONG  (ret_project_percent, gtt_project_get_percent_complete)


/* ============================================================== */
/* Handle ret_project_title_link in the almost-standard way,
 * i.e. as
 * RET_PROJECT_STR (ret_project_title, gtt_project_get_title) 
 * except that we need to handle url links as well. 
 */
static SCM
get_project_title_link_scm (GttGhtml *ghtml, GttProject *prj)
{
	if (ghtml->show_links)
	{
		GString *str;
		str = g_string_new (NULL);
		g_string_append_printf (str, "<a href=\"gtt:proj:0x%x\">", prj);
		g_string_append (str, gtt_project_get_title (prj)); 
		g_string_append (str, "</a>");
		return gh_str2scm (str->str, str->len);
	}
	else 
	{
		const char * str = gtt_project_get_title (prj); 
		return gh_str2scm (str, strlen (str));
	}
}

RET_PROJECT_SIMPLE (ret_project_title_link, get_project_title_link_scm)

/* ============================================================== */

static const char * 
get_urgency (GttProject *prj) 
{
	GttRank rank = gtt_project_get_urgency (prj);
	switch (rank)
	{
		case GTT_LOW:       return _("Low"); 
		case GTT_MEDIUM:    return _("Normal");
		case GTT_HIGH:      return _("Urgent"); 
	}
	return _("Undefined");
}

static const char * 
get_importance (GttProject *prj) 
{
	GttRank rank = gtt_project_get_importance (prj);
	switch (rank)
	{
		case GTT_LOW:       return _("Low"); 
		case GTT_MEDIUM:    return _("Medium");
		case GTT_HIGH:      return _("Important"); 
	}
	return _("Undefined");
}

static const char * 
get_status (GttProject *prj) 
{
	GttProjectStatus status = gtt_project_get_status (prj);
	switch (status)
	{
		case GTT_NOT_STARTED:  return _("Not Started"); 
		case GTT_IN_PROGRESS:  return _("In Progress"); 
		case GTT_ON_HOLD:      return _("On Hold"); 
		case GTT_CANCELLED:    return _("Cancelled"); 
		case GTT_COMPLETED:    return _("Completed"); 
	}
	return _("Undefined");
}

RET_PROJECT_STR (ret_project_urgency,    get_urgency)
RET_PROJECT_STR (ret_project_importance, get_importance)
RET_PROJECT_STR (ret_project_status,     get_status)
		  
/* ============================================================== */
/* Define a set of subroutines that accept a scheme list of tasks,
 * applies the gtt_task function on each, and then returns a 
 * scheme list containing the results.   
 *
 * For example, ret_task_memo() takes a scheme list of gtt
 * tasks, gets the task memo for each, and then returns
 * a scheme list of the task memos.
 */

#define RET_TASK_SIMPLE(RET_FUNC,GTT_GETTER)                        \
static SCM                                                          \
RET_FUNC (SCM task_list)                                            \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_task (ghtml, task_list, GTT_GETTER##_scm);    \
}

#define RET_TASK_STR(RET_FUNC,GTT_GETTER)                           \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttTask *tsk)                    \
{                                                                   \
	const char * str = GTT_GETTER (tsk);                             \
	return gh_str2scm (str, strlen (str));                           \
}                                                                   \
                                                                    \
static SCM                                                          \
RET_FUNC (SCM task_list)                                            \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_task (ghtml, task_list, GTT_GETTER##_scm);    \
}

RET_TASK_STR (ret_task_notes,   gtt_task_get_notes)
		  
/* ============================================================== */
/* Handle ret_task_memo in the almost-standard way,
 * i.e. as
 * RET_TASK_STR (ret_task_memo, gtt_task_get_memo) 
 * except that we need to handle url links as well. 
 */
static SCM
get_task_memo_scm (GttGhtml *ghtml, GttTask *tsk)
{
	if (ghtml->show_links)
	{
		GString *str;
		str = g_string_new (NULL);
		g_string_append_printf (str, "<a href=\"gtt:task:0x%x\">", tsk);
		g_string_append (str, gtt_task_get_memo (tsk)); 
		g_string_append (str, "</a>");
		return gh_str2scm (str->str, str->len);
	}
	else 
	{
		const char * str = gtt_task_get_memo (tsk); 
		return gh_str2scm (str, strlen (str));
	}
}

static SCM
ret_task_memo (SCM task_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_apply_on_task (ghtml, task_list, get_task_memo_scm);
}

/* ============================================================== */

static const char *
task_get_billstatus (GttTask *tsk)
{
	switch (gtt_task_get_billstatus (tsk))
	{
		case GTT_HOLD: return _("Hold");
		case GTT_BILL: return _("Bill");
		case GTT_PAID: return _("Paid");
		default: return "";
	}
	return "";
};

static const char *
task_get_billable (GttTask *tsk)
{
	switch (gtt_task_get_billable (tsk))
	{
		case GTT_BILLABLE: return _("Billable");
		case GTT_NOT_BILLABLE: return _("Not Billable");
		case GTT_NO_CHARGE: return _("No Charge");
		default: return "";
	}
	return "";
};

static const char *
task_get_billrate (GttTask *tsk)
{
	switch (gtt_task_get_billrate (tsk))
	{
		case GTT_REGULAR: return _("Regular");
		case GTT_OVERTIME: return _("Overtime");
		case GTT_OVEROVER: return _("Double Overtime");
		case GTT_FLAT_FEE: return _("Flat Fee");
		default: return "";
	}
	return "";
};

static SCM
task_get_time_str_scm (GttGhtml *ghtml, GttTask *tsk)
{
	time_t task_secs;
	char buff[100];

	task_secs = gtt_task_get_secs_ever(tsk);
	print_hours_elapsed (buff, 100, task_secs, TRUE);
	return gh_str2scm (buff, strlen (buff));
}

static SCM
task_get_value_str_scm (GttGhtml *ghtml, GttTask *tsk)
{
	time_t task_secs;
	char buff[100];
	double value;
	GttProject *prj;

	task_secs = gtt_task_get_secs_ever(tsk);
	value = ((double) task_secs) / 3600.0;
	
	prj = gtt_task_get_parent (tsk);
	switch (gtt_task_get_billrate (tsk))
	{
		case GTT_REGULAR: value *= gtt_project_get_billrate (prj); break;
		case GTT_OVERTIME: value *= gtt_project_get_overtime_rate (prj); break;
		case GTT_OVEROVER: value *= gtt_project_get_overover_rate (prj); break;
		case GTT_FLAT_FEE: value = gtt_project_get_flat_fee (prj); break;
		default: value = 0.0;
	}
	/* hack alert should use i18n currency/monetary printing */
	snprintf (buff, 100, "$%.2f", value+0.0049);
						
	return gh_str2scm (buff, strlen (buff));
}

RET_TASK_STR (ret_task_billstatus, task_get_billstatus)
RET_TASK_STR (ret_task_billable,   task_get_billable)
RET_TASK_STR (ret_task_billrate,   task_get_billrate)
RET_TASK_SIMPLE (ret_task_time_str,  task_get_time_str)
RET_TASK_SIMPLE (ret_task_value_str, task_get_value_str)

/* ============================================================== */

#define RET_IVL_SIMPLE(RET_FUNC,GTT_GETTER)                         \
static SCM                                                          \
RET_FUNC (SCM ivl_list)                                             \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_interval (ghtml, ivl_list, GTT_GETTER##_scm); \
}


#define RET_IVL_STR(RET_FUNC,GTT_GETTER)                            \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttInterval *ivl)                \
{                                                                   \
	const char * str = GTT_GETTER (ivl);                             \
	return gh_str2scm (str, strlen (str));                           \
}                                                                   \
RET_IVL_SIMPLE(RET_FUNC,GTT_GETTER)


#define RET_IVL_ULONG(RET_FUNC,GTT_GETTER)                          \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttInterval *ivl)                \
{                                                                   \
	unsigned long i = GTT_GETTER (ivl);                              \
	return gh_ulong2scm (i);                                         \
}                                                                   \
RET_IVL_SIMPLE(RET_FUNC,GTT_GETTER)


RET_IVL_ULONG (ret_ivl_start, gtt_interval_get_start)
RET_IVL_ULONG (ret_ivl_stop,  gtt_interval_get_stop)
RET_IVL_ULONG (ret_ivl_fuzz,  gtt_interval_get_fuzz)

static SCM
get_ivl_elapsed_str_scm (GttGhtml *ghtml, GttInterval *ivl)
{
	char buff[100];
	time_t elapsed;
	elapsed = gtt_interval_get_stop (ivl);
	elapsed -= gtt_interval_get_start (ivl);
	print_hours_elapsed (buff, 100, elapsed, TRUE);
	return gh_str2scm (buff, strlen (buff));
}

RET_IVL_SIMPLE (ret_ivl_elapsed_str, get_ivl_elapsed_str);

static SCM
get_ivl_start_stop_common_str_scm (GttGhtml *ghtml, GttInterval *ivl, 
					 time_t starp, gboolean prt_date)
{
	char buff[100];

	if (prt_date) {
		print_date_time (buff, 100, starp);
	} else {
		print_time (buff, 100, starp);
	}

	if (ghtml->show_links)
	{
		GString *str;
		str = g_string_new (NULL);
		g_string_append_printf (str, "<a href=\"gtt:interval:0x%x\">", ivl);
		g_string_append (str, buff); 
		g_string_append (str, "</a>");
		return gh_str2scm (str->str, str->len);
	}
	else
	{
		return gh_str2scm (buff, strlen (buff));
	}
	
	return SCM_EOL; /* Not reached */
}

static SCM
get_ivl_start_str_scm (GttGhtml *ghtml, GttInterval *ivl)
{
	gboolean prt_date = TRUE;
	
	char buff[100];
	time_t start, prev_stop;
	
	/* Caution! use of the last_ivl_time thing makes this 
	 * stateful in a way that may be surprising ! 
	 */
	start = gtt_interval_get_start (ivl);
	prev_stop = ghtml->last_ivl_time;
	ghtml->last_ivl_time = start;

	if (0 != prev_stop)
	{
		prt_date = is_same_day(start, prev_stop);
	}
	return get_ivl_start_stop_common_str_scm (ghtml, ivl, start, prt_date);
}
RET_IVL_SIMPLE (ret_ivl_start_str, get_ivl_start_str);

static SCM
get_ivl_stop_str_scm (GttGhtml *ghtml, GttInterval *ivl)
{
	gboolean prt_date = TRUE;
	
	char buff[100];
	time_t stop, prev_start;
	
	/* Caution! use of the last_ivl_time thing makes this 
	 * stateful in a way that may be surprising ! 
	 */
	stop = gtt_interval_get_stop (ivl);
	prev_start = ghtml->last_ivl_time;
	ghtml->last_ivl_time = stop;
	if (0 != prev_start)
	{
		prt_date = is_same_day(prev_start, stop);
	}
	return get_ivl_start_stop_common_str_scm (ghtml, ivl, stop, prt_date);
}
RET_IVL_SIMPLE (ret_ivl_stop_str, get_ivl_stop_str);


static SCM
get_ivl_fuzz_str_scm (GttGhtml *ghtml, GttInterval *ivl)
{
	char buff[100];

	print_hours_elapsed (buff, 100, gtt_interval_get_fuzz (ivl), TRUE);
	return gh_str2scm (buff, strlen (buff));
}

RET_IVL_SIMPLE (ret_ivl_fuzz_str, get_ivl_fuzz_str);

/* ============================================================== */

void
gtt_ghtml_display (GttGhtml *ghtml, const char *filepath,
                   GttProject *prj)
{
	FILE *ph;
	GString *template;
	char *start, *end, *scmstart, *comstart, *comend;
	size_t nr;

	if (!ghtml) return;
	ghtml->prj = prj;

	if (!filepath)
	{
		if (ghtml->error)
		{
			(ghtml->error) (ghtml, 404, NULL, ghtml->user_data);
		}
		return;
	}

	/* Try to get the ghtml file ... */
	ph = fopen (filepath, "r");
	if (!ph)
	{
		if (ghtml->error)
		{
			(ghtml->error) (ghtml, 404, filepath, ghtml->user_data);
		}
		return;
	}

	/* Read in the whole file.  Hopefully its not huge */
	template = g_string_new (NULL);
	while (!feof (ph))
	{
#define BUFF_SIZE 4000
		char buff[BUFF_SIZE+1];
		nr = fread (buff, 1, BUFF_SIZE, ph);
		if (0 >= nr) break;  /* EOF I presume */
		buff[nr] = 0x0;
		g_string_append (template, buff);
	}
	fclose (ph);
		
	
	/* ugh. gag. choke. puke. */
	ghtml_guile_global_hack = ghtml;

	/* Load predefined scheme forms */
	gh_eval_file (gtt_ghtml_resolve_path("gtt.scm"));
	
	/* Now open the output stream for writing */
	if (ghtml->open_stream)
	{
		(ghtml->open_stream) (ghtml, ghtml->user_data);
	}

	/* Loop over input text, looking for scheme markup and 
	 * sgml comments. */
	start = template->str;
	while (start)
	{
		scmstart = NULL;
		
		/* look for scheme markup */
		end = strstr (start, "<?scm");

		/* look for comments, and blow past them. */
		comstart = strstr (start, "<!--");
		if (comstart && comstart < end)
		{
			comend = strstr (comstart, "-->");
			if (comend)
			{
				nr = comend - start;
				end = comend;
			}
			else
			{
				nr = strlen (start);
				end = NULL;
			}
			
			/* write everything that we got before the markup */
			if (ghtml->write_stream)
			{
				(ghtml->write_stream) (ghtml, start, nr, ghtml->user_data);
			}
			start = end;
			continue;
		}

		/* look for  termination of scm markup */
		if (end)
		{
			nr = end - start;
			*end = 0x0;
			scmstart = end+5;
			end = strstr (scmstart, "?>");
			if (end)
			{
				*end = 0;
				end += 2;
			}
		}
		else
		{
			nr = strlen (start);
		}
		
		/* write everything that we got before the markup */
		if (ghtml->write_stream)
		{
			(ghtml->write_stream) (ghtml, start, nr, ghtml->user_data);
		}

		/* if there is markup, then dispatch */
		if (scmstart)
		{
			gh_eval_str_with_standard_handler (scmstart);
			scmstart = NULL;
		}
		start = end;
	}

	if (ghtml->close_stream)
	{
		(ghtml->close_stream) (ghtml, ghtml->user_data);
	}
}

/* ============================================================== */
/* Register callback handlers for various internally defined 
 * scheme forms. 
 */

static int is_inited = 0;

static void
register_procs (void)
{
	gh_new_procedure("gtt-show",               show_scm,               1, 0, 0);
	gh_new_procedure("gtt-linked-project",     ret_linked_project,     0, 0, 0);
	gh_new_procedure("gtt-selected-project",   ret_selected_project,   0, 0, 0);
	gh_new_procedure("gtt-projects",           ret_projects,           0, 0, 0);

	gh_new_procedure("gtt-tasks",              ret_tasks,              1, 0, 0);
	gh_new_procedure("gtt-intervals",          ret_intervals,          1, 0, 0);
	gh_new_procedure("gtt-daily-totals",       ret_daily_totals,       1, 0, 0);
	
	gh_new_procedure("gtt-links-on",           set_links_on,           0, 0, 0);
	gh_new_procedure("gtt-links-off",          set_links_off,          0, 0, 0);
	
	gh_new_procedure("gtt-project-title",      ret_project_title,      1, 0, 0);
	gh_new_procedure("gtt-project-title-link", ret_project_title_link, 1, 0, 0);
	gh_new_procedure("gtt-project-desc",       ret_project_desc,       1, 0, 0);
	gh_new_procedure("gtt-project-notes",      ret_project_notes,      1, 0, 0);
	gh_new_procedure("gtt-project-urgency",    ret_project_urgency,    1, 0, 0);
	gh_new_procedure("gtt-project-importance", ret_project_importance, 1, 0, 0);
	gh_new_procedure("gtt-project-status",     ret_project_status,     1, 0, 0);
	gh_new_procedure("gtt-project-estimated-start", ret_project_est_start, 1, 0, 0);
	gh_new_procedure("gtt-project-estimated-end", ret_project_est_end, 1, 0, 0);
	gh_new_procedure("gtt-project-due-date",   ret_project_due_date, 1, 0, 0);
	gh_new_procedure("gtt-project-sizing",     ret_project_sizing, 1, 0, 0);
	gh_new_procedure("gtt-project-percent-complete", ret_project_percent, 1, 0, 0);
	
	gh_new_procedure("gtt-task-memo",          ret_task_memo,          1, 0, 0);
	gh_new_procedure("gtt-task-notes",         ret_task_notes,         1, 0, 0);
	gh_new_procedure("gtt-task-billstatus",    ret_task_billstatus,    1, 0, 0);
	gh_new_procedure("gtt-task-billable",      ret_task_billable,      1, 0, 0);
	gh_new_procedure("gtt-task-billrate",      ret_task_billrate,      1, 0, 0);
	gh_new_procedure("gtt-task-time-str",      ret_task_time_str,      1, 0, 0);
	gh_new_procedure("gtt-task-value-str",     ret_task_value_str,     1, 0, 0);
	
	gh_new_procedure("gtt-interval-start",     ret_ivl_start,          1, 0, 0);
	gh_new_procedure("gtt-interval-stop",      ret_ivl_stop,           1, 0, 0);
	gh_new_procedure("gtt-interval-fuzz",      ret_ivl_fuzz,           1, 0, 0);
	gh_new_procedure("gtt-interval-elapsed-str", ret_ivl_elapsed_str,  1, 0, 0);
	gh_new_procedure("gtt-interval-start-str", ret_ivl_start_str,      1, 0, 0);
	gh_new_procedure("gtt-interval-stop-str",  ret_ivl_stop_str,       1, 0, 0);
	gh_new_procedure("gtt-interval-fuzz-str",  ret_ivl_fuzz_str,       1, 0, 0);
	
}


/* ============================================================== */

GttGhtml *
gtt_ghtml_new (void)
{
	GttGhtml *p;
	int i;

	if (!is_inited)
	{
		is_inited = 1;
		register_procs();
	}

	p = g_new0 (GttGhtml, 1);

	p->prj = NULL;
	p->show_links = TRUE;
	p->really_hide_links = FALSE;
	p->last_ivl_time = 0;

	gtt_ghtml_deprecated_init (p);

	return p;
}

void 
gtt_ghtml_destroy (GttGhtml *p)
{
	if (!p) return;
	// XXX memory leak, but otherwise mystery coredump due to this g_free
	// g_free (p);
}

void 
gtt_ghtml_set_stream (GttGhtml *p, gpointer ud,
                                   GttGhtmlOpenStream op, 
                                   GttGhtmlWriteStream wr,
                                   GttGhtmlCloseStream cl, 
                                   GttGhtmlError er)
{
	if (!p) return;
	p->user_data = ud;
	p->open_stream = op;
	p->write_stream = wr;
	p->close_stream = cl;
	p->error = er;
}

/* This sets the over-ride flag, so that no internal links are shown, 
 * really really for real, when printing out to file. 
 */
void 
gtt_ghtml_show_links (GttGhtml *p, gboolean sl)
{
	if (!p) return;
	p->really_hide_links = (FALSE == sl);
	p->show_links = sl;
}

/* ===================== END OF FILE ==============================  */
