/*   gtt project private data structure file.
 *   Copyright (C) 2001 Linas Vepstas
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

#ifndef __GTT_PROJ_P_H__
#define __GTT_PROJ_P_H__

#include "config.h"

#include <glib.h>

#include "proj.h"
#include "timer.h"

struct gtt_project_s 
{
	/* 'protected' data, accessible trough setters & getters */
	/* This data defines the 'state' of the project. */
	char *title;     /* short title */
	char *desc;      /* breif description for invoice */
	char *notes;     /* long description */
	char *custid;    /* customer id (TBD -- index to addresbook) */

	int min_interval;  /* smallest recorded interval */
	int auto_merge_interval;  /* merge intervals smaller than this */
	int auto_merge_gap;       /* merge gaps smaller than this */

        double billrate;   /* billing rate, in units of currency per hour */
        double overtime_rate;  /*  in units of currency per hour */
        double overover_rate;  /*  the good money is here ... */
        double flat_fee;  /* flat price, in units of currency */

	time_t estimated_start;  /* projected/planned start date */
	time_t estimated_end;    /* projected/planned end date */
	time_t due_date;         /* drop-dead date */
	int sizing;  /* estimate amount of time to complete (seconds) */
	short percent_complete;  /* estimate how much work left */
	GttRank urgency;         /* does this have to get done quickly? */
	GttRank importance;      /* is this important to get done? */
	GttProjectStatus status; /* overall project status */

	GList *task_list;      /* annotated chunks of time */

	/* hack alert -- the project heriarachy should probably be 
	 * reimplemented as a GNode rather than a GList */
	GList *sub_projects;   /* sub-projects */
	GttProject *parent;    /* back pointer to parent project */

	/* hack alert - come gnome-2.0, listeners should be replaced
	 * by a gobj callback */
	GList *listeners;      /* listeners for change events */

        /* miscellaneous -- used by GUI to display */
        gpointer *private_data;

	int id;		/* simple id number */

	/* ------------------------------------------------ */
	/* 'private' internal data caches & etc. stores temp info */

	int being_destroyed : 1;  /* project is being destroyed */
	int frozen : 1 ;          /* defer recomputes of time totals */
	int dirty_time : 1 ;      /* the time totals are wrong */

	int secs_ever;   /* seconds spend on this project */
	int secs_year;  /* seconds spent on this project this year */
	int secs_month;  /* seconds spent on this project this month */
	int secs_week;   /* seconds spent on this project this week */
	int secs_day;    /* seconds spent on this project today */
};


/* A 'task' is a group of start-stops that have a common 'memo'
 * associated with them.   Note that by definition, the 'current',
 * active interval is the one at the head of the list.
 */
struct gtt_task_s 
{
	GttProject *parent;       /* parent project */
	char * memo;              /* invoiceable memo (customer sees this) */
	char * notes;             /* internal notes (office private) */
	GttBillable   billable;   /* if fees can be collected for this task */
	GttBillRate   billrate;   /* hourly rate at which to bill */
	GttBillStatus billstatus; /* disposition of this item */
	int	bill_unit;	  /* billable unit, in seconds */
	GList *interval_list;     /* collection of start-stop's */
};

/* one start-stop interval */
struct gtt_interval_s 
{
	GttTask *parent;	/* who I belong to */
	time_t	start;		/* when the timer started */
	time_t	stop;		/* if stopped, shows when timer stopped, 
				 * if running, then the most recent log point */
	int	fuzz;           /* how fuzzy the start time is.  In
	                         * seconds, typically 300, 3600 or 1/2 day */
	int	running : 1;	/* boolean: is the timer running? */
};

#endif /* __GTT_PROJ_P_H__ */
