/*   XML I/O routines for GTimeTracker
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <glib.h>
#include <gnome-xml/parser.h>
#include <stdio.h>
#include <stdlib.h>

#include "err-throw.h"
#include "gtt.h"
#include "proj.h"
#include "xml-gtt.h"


/* Note: most of this code is a tediously boring cut-n-paste
 * of the same thing over & over again, and could//should be
 * auto-generated.  Diatribe: If the creators and true
 * beleivers of XML knew some scheme/lisp, or of IDL's, of
 * Corba or RPC fame, or even had an inkling of what 'object
 * introspection' was, then DTD's wouldn't be the abortion that
 * they are, and XML wouldn't be so sucky to begin with ...
 * Alas ...
 */

/* =========================================================== */


#define GET_TEXT(node)  ({					\
	char * sstr = NULL;					\
	xmlNodePtr text;					\
	text = node->childs;					\
	if (!text) { 						\
		gtt_err_set_code (GTT_FILE_CORRUPT);		\
	}							\
	else if (strcmp ("text", text->name)) {			\
                gtt_err_set_code (GTT_FILE_CORRUPT);		\
	} 							\
	else sstr = text->content;				\
	sstr;							\
})

#define GET_STR(SELF,FN,TOK)				\
	if (0 == strcmp (TOK, node->name))		\
	{						\
		const char *str = GET_TEXT (node);	\
		FN (SELF, str);				\
	} 						\
	else


#define GET_DBL(SELF,FN,TOK)				\
	if (0 == strcmp (TOK, node->name))		\
	{						\
		const char *str = GET_TEXT (node);	\
		double rate = atof (str);		\
		FN (SELF, rate);			\
	} 						\
	else

#define GET_INT(SELF,FN,TOK)				\
	if (0 == strcmp (TOK, node->name))		\
	{						\
		const char *str = GET_TEXT (node);	\
		int ival = atoi (str);			\
		FN (SELF, ival);			\
	} 						\
	else

#define GET_TIM(SELF,FN,TOK)				\
	if (0 == strcmp (TOK, node->name))		\
	{						\
		const char *str = GET_TEXT (node);	\
		time_t tval = atol (str);		\
		FN (SELF, tval);			\
	} 						\
	else

#define GET_BOL(SELF,FN,TOK)				\
	if (0 == strcmp (TOK, node->name))		\
	{						\
		const char *str = GET_TEXT (node);	\
		gboolean bval = atol (str);		\
		FN (SELF, bval);			\
	} 						\
	else

#define GET_ENUM_3(SELF,FN,TOK,A,B,C)				\
	if (0 == strcmp (TOK, node->name))			\
	{							\
		const char *str = GET_TEXT (node);		\
		int ival = GTT_##A;				\
		if (!strcmp (#A, str)) ival = GTT_##A;		\
		else if (!strcmp (#B, str)) ival = GTT_##B;	\
		else if (!strcmp (#C, str)) ival = GTT_##C;	\
                else gtt_err_set_code (GTT_UNKNOWN_VALUE);	\
		FN (SELF, ival);				\
	} 							\
	else

#define GET_ENUM_4(SELF,FN,TOK,A,B,C,D)				\
	if (0 == strcmp (TOK, node->name))			\
	{							\
		const char *str = GET_TEXT (node);		\
		int ival = GTT_##A;				\
		if (!strcmp (#A, str)) ival = GTT_##A;		\
		else if (!strcmp (#B, str)) ival = GTT_##B;	\
		else if (!strcmp (#C, str)) ival = GTT_##C;	\
		else if (!strcmp (#D, str)) ival = GTT_##D;	\
                else gtt_err_set_code (GTT_UNKNOWN_VALUE);	\
		FN (SELF, ival);				\
	} 							\
	else

#define GET_ENUM_5(SELF,FN,TOK,A,B,C,D,E)			\
	if (0 == strcmp (TOK, node->name))			\
	{							\
		const char *str = GET_TEXT (node);		\
		int ival = GTT_##A;				\
		if (!strcmp (#A, str)) ival = GTT_##A;		\
		else if (!strcmp (#B, str)) ival = GTT_##B;	\
		else if (!strcmp (#C, str)) ival = GTT_##C;	\
		else if (!strcmp (#D, str)) ival = GTT_##D;	\
		else if (!strcmp (#E, str)) ival = GTT_##E;	\
                else gtt_err_set_code (GTT_UNKNOWN_VALUE);	\
		FN (SELF, ival);				\
	} 							\
	else

/* =========================================================== */

static GttInterval *
parse_interval (xmlNodePtr interval)
{
	xmlNodePtr node;
	GttInterval *ivl = NULL;

	if (!interval) { gtt_err_set_code (GTT_FILE_CORRUPT); return ivl; }

	if (strcmp ("interval", interval->name)) {
		gtt_err_set_code (GTT_FILE_CORRUPT); return ivl; }

	ivl = gtt_interval_new ();
	for (node=interval->childs; node; node=node->next)
	{
		GET_TIM (ivl, gtt_interval_set_start, "start")
		GET_TIM (ivl, gtt_interval_set_stop, "stop")
		GET_TIM (ivl, gtt_interval_set_fuzz, "fuzz")
		GET_BOL (ivl, gtt_interval_set_running, "running")
		{ 
			gtt_err_set_code (GTT_UNKNOWN_TOKEN);
		}
	}
	return ivl;
}

/* =========================================================== */

static GttTask *
parse_task (xmlNodePtr task)
{
	xmlNodePtr node;
	GttTask *tsk = NULL;

	if (!task) { gtt_err_set_code (GTT_FILE_CORRUPT); return tsk; }

	if (strcmp ("task", task->name)) {
		gtt_err_set_code (GTT_FILE_CORRUPT); return tsk; }

	tsk = gtt_task_new ();
	for (node=task->childs; node; node=node->next)
	{
		GET_STR (tsk, gtt_task_set_memo, "memo")
		GET_STR (tsk, gtt_task_set_notes, "notes")
		GET_INT (tsk, gtt_task_set_bill_unit, "bill_unit")

		GET_ENUM_3 (tsk, gtt_task_set_billable, "billable", 
			NOT_BILLABLE, BILLABLE, NO_CHARGE)
		GET_ENUM_3 (tsk, gtt_task_set_billstatus, "billstatus", 
			HOLD, BILL, PAID)
		GET_ENUM_4 (tsk, gtt_task_set_billrate, "billrate", 
			REGULAR, OVERTIME, OVEROVER, FLAT_FEE)
		if (0 == strcmp ("interval-list", node->name))
		{
			xmlNodePtr tn;
			for (tn=node->childs; tn; tn=tn->next)
			{
				GttInterval *ival;
				ival = parse_interval (tn);
				gtt_task_append_interval (tsk, ival);
			}
		} 
		else
		{ 
			gtt_err_set_code (GTT_UNKNOWN_TOKEN);
		}
	}
	return tsk;
}

/* =========================================================== */


static GttProject *
parse_project (xmlNodePtr project)
{
	xmlNodePtr node;
	GttProject *prj = NULL;

	if (!project) { gtt_err_set_code (GTT_FILE_CORRUPT); return prj; }

	if (strcmp ("project", project->name)) {
		gtt_err_set_code (GTT_FILE_CORRUPT); return prj; }

	prj = gtt_project_new ();
	gtt_project_freeze (prj);
	for (node=project->childs; node; node=node->next)
	{
		GET_STR (prj, gtt_project_set_title, "title")
		GET_STR (prj, gtt_project_set_desc, "desc")
		GET_STR (prj, gtt_project_set_notes, "notes")
		GET_STR (prj, gtt_project_set_custid, "custid")

		GET_DBL (prj, gtt_project_set_billrate, "billrate")
		GET_DBL (prj, gtt_project_set_overtime_rate, "overtime_rate")
		GET_DBL (prj, gtt_project_set_overover_rate, "overover_rate")
		GET_DBL (prj, gtt_project_set_flat_fee, "flat_fee")

		GET_INT (prj, gtt_project_set_min_interval, "min_interval")
		GET_INT (prj, gtt_project_set_auto_merge_interval, "auto_merge_interval")
		GET_INT (prj, gtt_project_set_auto_merge_gap, "auto_merge_gap")

		GET_INT (prj, gtt_project_set_id, "id")

		GET_TIM (prj, gtt_project_set_estimated_start, "estimated_start")
		GET_TIM (prj, gtt_project_set_estimated_end, "estimated_end")
		GET_TIM (prj, gtt_project_set_due_date, "due_date")
		GET_INT (prj, gtt_project_set_sizing, "sizing")
		GET_INT (prj, gtt_project_set_percent_complete, "percent_complete")

		GET_ENUM_4 (prj, gtt_project_set_urgency, "urgency",
			UNDEFINED, LOW, MEDIUM, HIGH)
		GET_ENUM_4 (prj, gtt_project_set_importance, "importance",
			UNDEFINED, LOW, MEDIUM, HIGH)
		GET_ENUM_5 (prj, gtt_project_set_status, "status",
			NOT_STARTED, IN_PROGRESS, ON_HOLD, CANCELLED, COMPLETED)

		if (0 == strcmp ("task-list", node->name))
		{
			xmlNodePtr tn;
			for (tn=node->childs; tn; tn=tn->next)
			{
				GttTask *tsk;
				tsk = parse_task (tn);
				gtt_project_append_task (prj, tsk);
			}
		} 
		else
		if (0 == strcmp ("project-list", node->name))
		{
			xmlNodePtr tn;
			for (tn=node->childs; tn; tn=tn->next)
			{
				GttProject *child;
				child = parse_project (tn);
				gtt_project_append_project (prj, child);
			}
		} 
		else
		{ 
			g_warning ("unexpected node %s", node->name);
			gtt_err_set_code (GTT_UNKNOWN_TOKEN);
		}
	}
	gtt_project_thaw (prj);
	return prj;
}

/* =========================================================== */

GList *
gtt_xml_read_projects (const char * filename)
{
	GList *prjs = NULL;
	xmlDocPtr doc;
	xmlNodePtr root, project_list, project;

	doc = xmlParseFile (filename);

	if (!doc) { gtt_err_set_code (GTT_CANT_OPEN_FILE); return NULL; }
	root = doc->root;
	if (strcmp ("gtt", root->name)) {
		gtt_err_set_code (GTT_NOT_A_GTT_FILE); return NULL; }

	project_list = root->childs;

	/* If no children, then no projects -- a clean slate */
	if (!project_list) return NULL;

	if (strcmp ("project-list", project_list->name)) {
		gtt_err_set_code (GTT_FILE_CORRUPT); return NULL; }

	for (project=project_list->childs; project; project=project->next)
	{
		GttProject *prj;
		prj = parse_project (project);
		prjs = g_list_append (prjs, prj);
	}
	return prjs;
}

/* =========================================================== */

void
gtt_xml_read_file (const char * filename)
{
	GList *node, *prjs = NULL;

	prjs = gtt_xml_read_projects (filename);

	for (node=prjs; node; node=node->next)
	{
		GttProject *prj = node->data;
		gtt_project_list_append (prj);
	}

	/* recompute the cached counters */
	gtt_project_list_compute_secs ();
}

/* ====================== END OF FILE =============== */
