/*   Project data manipulation for GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
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

#ifndef __GTT_PROJ_H__
#define __GTT_PROJ_H__

#include <glib.h>


#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#endif /* TM_IN_SYS_TIME */
#include <time.h>
#endif /* TIME_WITH_SYS_TIME */


/* hack alert -- these are hard-coded enums; they should 
 * probably be replaced by a system of user-defined 
 * enumerated values, especially for GttProjectStatus
 */


typedef enum 
{
	GTT_BILLABLE = 1,   /* billable time */
	GTT_NOT_BILLABLE,   /* not billable to customer, internal only */
	GTT_NO_CHARGE       /* shows on invoice as 'no charge/free' */
} GttBillable;

typedef enum 
{
	GTT_REGULAR = 0,
	GTT_OVERTIME,
	GTT_OVEROVER,
	GTT_FLAT_FEE
} GttBillRate;
		
typedef enum 
{
	GTT_HOLD = 0,	 /* needs review, will not appear on invoice */
	GTT_BILL = 1,    /* print this on invoice, its done, ready */
	GTT_PAID         /* its been paid; do not print on invoice any more */
} GttBillStatus;

typedef enum
{
	GTT_UNDEFINED = 0,
	GTT_LOW = 1,
	GTT_MEDIUM,
	GTT_HIGH
} GttRank;

typedef enum
{
	GTT_NOT_STARTED = 0,  /* hack alert-- we should allow */
	GTT_IN_PROGRESS,      /* user-defined status states */
	GTT_ON_HOLD,          /* wating for something */
	GTT_CANCELLED,
	GTT_COMPLETED
} GttProjectStatus;
		

/* The three basic structures */
typedef struct gtt_project_s GttProject;
typedef struct gtt_task_s GttTask;
typedef struct gtt_interval_s GttInterval;

typedef void (*GttProjectChanged) (GttProject *, gpointer);
typedef int (*GttProjectCB) (GttProject *, gpointer);

/* -------------------------------------------------------- */
/* project data */

/* create, destroy a new project */
GttProject *	gtt_project_new(void);
GttProject *	gtt_project_new_title_desc(const char *, const char *);
void 		gtt_project_destroy(GttProject *);

/* The gtt_project_dup() routine will make a copy of the indicated
 *    project. Note, it will copy the sub-projects, but it will *not*
 *    copy the tasks (I dunno, this is probably a bug, I think it should
 *    copy the tasks as well ....)
 */
GttProject *	gtt_project_dup(GttProject *);

/* The gtt_project_remove() routine will take the project out of 
 *    either the master list, or out of any parents' list of 
 *    sub-projects that it might below to.  As a result, this 
 *    project will dangle there -- don't loose it 
 */
void 		gtt_project_remove(GttProject *p);

void 		gtt_project_set_title(GttProject *, const char *);
void 		gtt_project_set_desc(GttProject *, const char *);
void 		gtt_project_set_notes(GttProject *, const char *);
void 		gtt_project_set_custid(GttProject *, const char *);

/* These two routines return the title & desc strings.
 * Do *not* free these strings when done.  Note that 
 * are freed when project is deleted. */
const char * 	gtt_project_get_title (GttProject *);
const char * 	gtt_project_get_desc (GttProject *);
const char * 	gtt_project_get_notes (GttProject *);
const char * 	gtt_project_get_custid (GttProject *);

/* The gtt_project_compat_set_secs() routine provides a
 *    backwards-compatible routine for setting the total amount of
 *    time spent on a project.  It does this by creating a new task,
 *    labelling it as 'old gtt data', and putting the time info
 *    into that task.  Depricated. Don't use in new code.
 */
void		gtt_project_compat_set_secs (GttProject *proj, 
			int secs_ever, int secs_day, time_t last_update);


/* The billrate is the currency amount to charge for an hour's work.
 *     overtime_rate is the over-time rate (usually 1.5x billrate)
 *     overover_rate is the double over-time rate (usually 2x billrate)
 *     flat_fee is charged, independent of the length of time.
 */
void 		gtt_project_set_billrate (GttProject *, double);
double 		gtt_project_get_billrate (GttProject *);
void 		gtt_project_set_overtime_rate (GttProject *, double);
double 		gtt_project_get_overtime_rate (GttProject *);
void 		gtt_project_set_overover_rate (GttProject *, double);
double 		gtt_project_get_overover_rate (GttProject *);
void 		gtt_project_set_flat_fee (GttProject *, double);
double 		gtt_project_get_flat_fee (GttProject *);

/* The gtt_project_set_min_interval() routine sets the smallest
 *    time unit, below which work intervals will not be recorded
 *    (and will instead be discarded).   Default is 3 seconds, 
 *    but it should be 60 seconds.
 *
 * The gtt_project_auto_merge_interval() routine sets the smallest
 *    time unit, below which work intervals will be merged into 
 *    prior work intervals rather than being counted as seperate.
 *    Default is 1 minute, but should be 5 minutes.
 */
void		gtt_project_set_min_interval (GttProject *, int);
int		gtt_project_get_min_interval (GttProject *);
void		gtt_project_set_auto_merge_interval (GttProject *, int);
int		gtt_project_get_auto_merge_interval (GttProject *);
void		gtt_project_set_auto_merge_gap (GttProject *, int);
int		gtt_project_get_auto_merge_gap (GttProject *);

/* Todo-list management stuff.
 * The estimated start date is when we expect to first start working 
 * on this project.  The estimated end date is when we expect to be 
 * done working on this project.  The due date is the date that the
 * project is supposed to be finished.
 *
 * The sizing is the estimate (in seconds) of how long it will take to 
 * complete this task.  The precent-complete is the estimate of how
 * much of this project has been completed.
 *
 * The urgency is how time-critical this task is:  does it need an
 * immediate response?  The importance is a ranking of how important
 * this task is.  Note that things may be urgent, but not important
 * (Bob wants me to answer his email today, but I don't really care).
 * Things can be important but not urgent.  (Its important for me
 * to pass  this exam, but its not urgent -- I have all school-year 
 * to study for it.)
 * 
 * The project status indicates whether this is an ongoing project or
 * not.  We really should allow configurable values, so that users
 * could define multiple stages: e.g. 'testing', 'designing', 
 * 'on-hold', 'waiting for response', etc.
 */
void		gtt_project_set_estimated_start (GttProject *, time_t);
time_t		gtt_project_get_estimated_start (GttProject *);
void		gtt_project_set_estimated_end (GttProject *, time_t);
time_t		gtt_project_get_estimated_end (GttProject *);
void		gtt_project_set_due_date (GttProject *, time_t);
time_t		gtt_project_get_due_date (GttProject *);
void		gtt_project_set_sizing (GttProject *, int);
int		gtt_project_get_sizing (GttProject *);
void		gtt_project_set_percent_complete (GttProject *, int);
int		gtt_project_get_percent_complete (GttProject *);
void		gtt_project_set_urgency (GttProject *, GttRank);
GttRank		gtt_project_get_urgency (GttProject *);
void		gtt_project_set_importance (GttProject *, GttRank);
GttRank		gtt_project_get_importance (GttProject *);
void		gtt_project_set_status (GttProject *, GttProjectStatus);
GttProjectStatus		gtt_project_get_status (GttProject *);


/* The id is a simple id, handy for .. stuff */
void 		gtt_project_set_id (GttProject *, int id);
int  		gtt_project_get_id (GttProject *);

/* return a project, given only its id; NULL if not found */
GttProject * 	gtt_project_locate_from_id (int prj_id);

/* The gtt_project_add_notifier() routine allows anoter component
 *    (e.g. a GUI) to add a signal that will be called whenever the 
 *    time associated with a project changes. (except timers ???)
 *
 * The gtt_project_freeze() routine prevents notifiers from being
 *    invoked.
 *
 * The gtt_project_thaw() routine causes notifiers to be sent.
 */ 

void		gtt_project_freeze (GttProject *prj);
void		gtt_project_thaw (GttProject *prj);
void		gtt_task_freeze (GttTask *tsk);
void		gtt_task_thaw (GttTask *tsk);
void		gtt_interval_freeze (GttInterval *ivl);
void		gtt_interval_thaw (GttInterval *ivl);

void		gtt_project_add_notifier (GttProject *,
			GttProjectChanged, gpointer);
void		gtt_project_remove_notifier (GttProject *,
			GttProjectChanged, gpointer);

/* generic place to hang arbitrary data on the project (used by GUI) */
gpointer	gtt_project_get_private_data (GttProject *);
void		gtt_project_set_private_data (GttProject *, gpointer);

/* The gtt_project_foreach() routine calls the indicated function
 *    on the project and each of the sub-projects.  The recursion is
 *    stopped if the callback returns zero, otherwise it continues
 *    until all sub-projects ahve been visited.
 *    This routine returns the value of the last callback.
 */ 
int		gtt_project_foreach (GttProject *,  GttProjectCB, gpointer);

/* -------------------------------------------------------- */
/* project manipulation */

/* The project_timer_start() routine logs the time when
 *    a new task interval starts.
 * The project_timer_update() routine updates the end-time
 *    for a task interval. 
 */
void 		gtt_project_timer_start (GttProject *);
void 		gtt_project_timer_update (GttProject *);
void 		gtt_project_timer_stop (GttProject *);


/* The gtt_project_get_secs_day() routine returns the
 *    number of seconds spent on this project today,
 *    *NOT* including its sub-projects.
 *
 * The gtt_project_get_secs_ever() routine returns the
 *    number of seconds spent on this project,
 *    *NOT* including its sub-projects.
 *
 * The gtt_project_total_secs_day() routine returns the
 *    total number of seconds spent on this project today,
 *    including a total of all its sub-projects.
 *
 * The gtt_project_total_secs_ever() routine returns the
 *    total number of seconds spent on this project,
 *    including a total of all its sub-projects.
 */

int 		gtt_project_get_secs_current (GttProject *proj);
int 		gtt_project_get_secs_day (GttProject *proj);
int 		gtt_project_get_secs_week (GttProject *proj);
int 		gtt_project_get_secs_month (GttProject *proj);
int 		gtt_project_get_secs_year (GttProject *proj);
int 		gtt_project_get_secs_ever (GttProject *proj);

int 		gtt_project_total_secs_current (GttProject *proj);
int 		gtt_project_total_secs_day (GttProject *proj);
int 		gtt_project_total_secs_week (GttProject *proj);
int 		gtt_project_total_secs_month (GttProject *proj);
int 		gtt_project_total_secs_year (GttProject *proj);
int 		gtt_project_total_secs_ever (GttProject *proj);


void gtt_project_list_compute_secs (void);


/* return a list of the children of this project */
GList * 	gtt_project_get_children (GttProject *);
GList * 	gtt_project_get_tasks (GttProject *);

GttProject * 	gtt_project_get_parent (GttProject *);

/* 
 * The following routines maintain a heirarchical tree of projects.
 * Note, however, that these routines do not sanity check the tree
 * globally, so it is possible to create loops or disconected
 * components if one is sloppy.
 *
 * The gtt_project_append_project() routine makes 'child' a subproject 
 *    of 'parent'.  In doing this, it removes the child from its
 *    present location (whether as child of a diffferent project,
 *    or from the master project list).  It appends the child
 *    to the parent's list: the child becomes the 'last one'.
 *    If 'parent' is NULL, then the child is appended to the 
 *    top-level project list.
 */
void	gtt_project_append_project (GttProject *parent, GttProject *child);

/* 
 * The gtt_project_insert_before() routine will insert 'proj' on the 
 *     same list as 'before_me', be will insert it before it.  Note
 *     that 'before_me' may be in the top-level list, or it may be
 *     in a subproject list; either way, the routine works.  If
 *     'before_me' is null, then the project is appended to the master
 *     list.  The project is removed from its old location before
 *     being inserted into its new location.
 *
 * The gtt_project_insert_after() routine works similarly, except that
 *     if 'after_me' is null, then the proj is prepended to the 
 *     master list.
 */
void	gtt_project_insert_before (GttProject *proj, GttProject *before_me);
void	gtt_project_insert_after (GttProject *proj, GttProject *after_me);

void	gtt_project_append_task (GttProject *, GttTask *);
void	gtt_project_prepend_task (GttProject *, GttTask *);

/* The gtt_clear_daily_counter() will delete all intervals from 
 *    the project that started after midnight.  Typically, this 
 *    will result in the daily counters being zero'ed out, although 
 *    if a project started before midnight, some time may remain
 *    on deck.
 */

void gtt_clear_daily_counter (GttProject *proj);

/* -------------------------------------------------------- */
/* master project list */

/* Return a list of all projects */
GList * 	gtt_get_project_list (void);

/* append project to the master project list */
void 		gtt_project_list_append(GttProject *p);

void project_list_destroy(void);
void project_list_sort_current(void);
void project_list_sort_day(void);
void project_list_sort_week(void);
void project_list_sort_month(void);
void project_list_sort_year(void);
void project_list_sort_ever(void);
void project_list_sort_title(void);
void project_list_sort_desc(void);
void project_list_sort_start(void);
void project_list_sort_end(void);
void project_list_sort_due(void);
void project_list_sort_sizing(void);
void project_list_sort_percent(void);
void project_list_sort_urgency(void);
void project_list_sort_importance(void);
void project_list_sort_status(void);

/* The gtt_project_list_total_secs_day() routine returns the
 *    total number of seconds spent on all projects today,
 *    including a total of all sub-projects.
 *
 * The gtt_project_list_total_secs_ever() routine returns the
 *    total number of seconds spent all projects,
 *    including a total of all sub-projects.
 */

int 		gtt_project_list_total_secs_day (void);
int 		gtt_project_list_total_secs_ever (void);

/* -------------------------------------------------------- */
/* Tasks */
/* Taks may be a bit misnamed -- they should ave been called 
 * 'diary entries'.
 */

/* The gtt_task_dup() routine makes a copy of the indicated task.
 *    it copies the memo, notes, adn billing info, but not the 
 *    intervals, nor the parent.
 */

GttTask *	gtt_task_new (void);
GttTask *	gtt_task_dup (GttTask *);
void 		gtt_task_destroy (GttTask *);

void		gtt_task_set_memo (GttTask *, const char *);
const char *	gtt_task_get_memo (GttTask *);
void		gtt_task_set_notes (GttTask *, const char *);
const char *	gtt_task_get_notes (GttTask *);

void		gtt_task_set_billable (GttTask *, GttBillable);
GttBillable	gtt_task_get_billable (GttTask *);
void		gtt_task_set_billrate (GttTask *, GttBillRate);
GttBillRate	gtt_task_get_billrate (GttTask *);
void		gtt_task_set_billstatus (GttTask *, GttBillStatus);
GttBillStatus	gtt_task_get_billstatus (GttTask *);

/* The bill_unit is the minimum billable unit of time. 
 * Typically 15 minutes or an hour, it represents the smallest unit
 * of time that will appear on the invoice.
 */
void		gtt_task_set_bill_unit (GttTask *, int);
int		gtt_task_get_bill_unit (GttTask *);

/* The gtt_task_remove() routine will remove teh task from its parent
 *    project (presumably in preparation for reparenting).
 *
 * The gtt_task_new_insert() routine creates a new task with the same
 *    settings as the indicated task. It does *not* copy the intervals.
 *    It inserts the new task above the indicated task.
 *
 * The gtt_task_insert() routine pastes 'insertee' above 'where'.
 *
 * The gtt_task_merge_up() routine will take all of the intervals of 
 *    the indicated task, and move them into the task above, (thus
 *    gutting this task of its intervals).  It does not actually 
 *    destroy this task.
 *
 * The gtt_task_is_first_task() routine returns True if this task
 *    is the leading task of the project.
 */
void		gtt_task_remove (GttTask *);
GttTask *	gtt_task_new_insert (GttTask *);
void		gtt_task_insert (GttTask *where, GttTask *insertee);
void		gtt_task_merge_up (GttTask *);
gboolean	gtt_task_is_first_task (GttTask *);
GttProject *	gtt_task_get_parent (GttTask *);


GList *		gtt_task_get_intervals (GttTask *);
void		gtt_task_add_interval (GttTask *, GttInterval *);
void		gtt_task_append_interval (GttTask *, GttInterval *);

/* gtt_task_get_secs_ever() adds up and returns the total number of 
 * seconds in the intervals in this task. */
int 		gtt_task_get_secs_ever (GttTask *tsk);


/* intervals */
/* The gtt_interval_set_start() and gtt_interval_set_stop() set the
 *    start and stop times that define the interval.  These routines
 *    will not let you set a start time that is later than the stop 
 *    time: negative timer intervals are not allowed.  Note also that
 *    the system automatically removes ("scrubs away") short time 
 *    intervals when certain key operations are performed (e.g. 'thaw'). 
 *    This *    means that if you are holding a pointer to a very 
 *    short interval, it might disappear on you.  In other words, 
 *    don't use these routines to set a short time interval, and 
 *    still expect the interval to be around when you are done.
 *
 * The fuzz (measured in seconds) indicates how 'fuzzy' the 
 *    true start time was.  Typically, its 0, 300, 3600 or 12*3600
 *    Just because the start time is fuzy doesn't mean that the total 
 *    interval is inaccurate: the delta stop-start is still accurate 
 *    down to the second.  The fuzz is 'merely' used by the GUI
 *    to help the user report time spent post-facto, without having 
 *    to exactly nail down the start time.
 *
 * The is_running flag indicates whether the timer is running on this
 * interval.
 */
GttInterval *	gtt_interval_new (void);
void		gtt_interval_destroy (GttInterval *);

void		gtt_interval_set_start (GttInterval *, time_t);
void		gtt_interval_set_stop (GttInterval *, time_t);
void		gtt_interval_set_running (GttInterval *, gboolean);
void		gtt_interval_set_fuzz (GttInterval *, int);
time_t		gtt_interval_get_start (GttInterval *);
time_t		gtt_interval_get_stop (GttInterval *);
gboolean	gtt_interval_get_running (GttInterval *);
int		gtt_interval_get_fuzz (GttInterval *);

/* The gtt_interval_new_insert_after() routine creates a new interval 
 *    and inserts it after the interval "where".  It returns the new 
 *    interval.
 *
 * The gtt_interval_merge_up() routine merges the given interval with 
 *    the immediately more recent one above it.  It does this by 
 *    decrementing the start time.  The resulting interval has the
 *    max of the two fuzz factors, and is running if the first was.
 *    The merged interval is returned.
 *
 * The gtt_interval_merge_down() routine does the same, except that
 *    it merges with the next interval by incrementing its stop time.
 *
 * The gtt_interval_split() routine splits the list of intervals 
 *    into two pieces, with the indicated interval and everything
 *    following it going after the specified.  
 */
GttInterval *   gtt_interval_new_insert_after (GttInterval *where);
GttInterval *	gtt_interval_merge_up (GttInterval *);
GttInterval *	gtt_interval_merge_down (GttInterval *);
void		gtt_interval_split (GttInterval *, GttTask *);
GttTask *	gtt_interval_get_parent (GttInterval *);
gboolean	gtt_interval_is_first_interval (GttInterval *);
gboolean	gtt_interval_is_last_interval (GttInterval *);

#endif /* __GTT_PROJ_H__ */
