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


#include <gnome-xml/tree.h>
#include <stdio.h>

#include "err-throw.h"
#include "gtt.h"
#include "proj.h"
#include "xml-gtt.h"

static xmlNodePtr gtt_project_list_to_dom_tree (GList *list);

/* Note: most of this code is a tediously boring cut-n-paste
 * of the same thing over & over again, and could//should be 
 * auto-generated.  Diatribe: If the creators and true 
 * beleivers of XML knew some scheme/lisp, or of IDL's, of
 * Corba or RPC fame, or even had an inkling of what 'object 
 * introspection' was, then DTD's wouldn't be the abortion that
 * they are, and XML wouldn't be so sucky to begin with ...
 * Alas ...
 */

/* ======================================================= */

#define PUT_STR(TOK,VAL)	{			\
	const char * str = (VAL);			\
	if (str && 0 != str[0])				\
	{						\
		node = xmlNewNode (NULL, TOK);		\
		xmlNodeAddContent(node, str);		\
		xmlAddChild (topnode, node);		\
	}						\
}

#define PUT_INT(TOK,VAL)	{			\
	char buff[80];					\
	g_snprintf (buff, sizeof(buff), "%d", (VAL));	\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, buff);			\
	xmlAddChild (topnode, node);			\
}

#define PUT_LONG(TOK,VAL)	{			\
	char buff[80];					\
	g_snprintf (buff, sizeof(buff), "%ld", (VAL));	\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, buff);			\
	xmlAddChild (topnode, node);			\
}


#define PUT_DBL(TOK,VAL)	{			\
	char buff[80];					\
	g_snprintf (buff, sizeof(buff), "%.18g", (VAL));\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, buff);			\
	xmlAddChild (topnode, node);			\
}

#define PUT_BOOL(TOK,VAL)	{			\
	gboolean boll = (VAL);				\
	node = xmlNewNode (NULL, TOK);			\
        if (boll) {					\
		xmlNodeAddContent(node, "T");		\
	} else {					\
		xmlNodeAddContent(node, "F");		\
	}						\
	xmlAddChild (topnode, node);			\
}

#define PUT_ENUM_3(TOK,VAL,A,B,C) {			\
	const char * str = #A;				\
	switch (VAL)					\
	{						\
		case GTT_##A: str = #A; break;		\
		case GTT_##B: str = #B; break;		\
		case GTT_##C: str = #C; break; 		\
	}						\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, str);			\
	xmlAddChild (topnode, node);			\
}

#define PUT_ENUM_4(TOK,VAL,A,B,C, D) {			\
	const char * str = #A;				\
	switch (VAL)					\
	{						\
		case GTT_##A: str = #A; break;		\
		case GTT_##B: str = #B; break;		\
		case GTT_##C: str = #C; break; 		\
		case GTT_##D: str = #D; break; 		\
	}						\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, str);			\
	xmlAddChild (topnode, node);			\
}

#define PUT_ENUM_5(TOK,VAL,A,B,C,D,E) {			\
	const char * str = #A;				\
	switch (VAL)					\
	{						\
		case GTT_##A: str = #A; break;		\
		case GTT_##B: str = #B; break;		\
		case GTT_##C: str = #C; break; 		\
		case GTT_##D: str = #D; break; 		\
		case GTT_##E: str = #E; break; 		\
	}						\
	node = xmlNewNode (NULL, TOK);			\
	xmlNodeAddContent(node, str);			\
	xmlAddChild (topnode, node);			\
}

/* ======================================================= */

/* convert one interval to a dom tree */

static xmlNodePtr
gtt_xml_interval_to_dom_tree (GttInterval *ivl)
{
	xmlNodePtr node, topnode;

	if (!ivl) return NULL;

	topnode = xmlNewNode (NULL, "gtt:interval");

	PUT_LONG("start", gtt_interval_get_start(ivl));
	PUT_LONG("stop", gtt_interval_get_stop(ivl));
	PUT_INT("fuzz", gtt_interval_get_fuzz(ivl));
	PUT_BOOL("running", gtt_interval_get_running(ivl));

	return topnode;
}

/* convert a list of gtt tasks into a DOM tree */
static xmlNodePtr
gtt_interval_list_to_dom_tree (GList *list)
{
	GList *p;
	xmlNodePtr topnode;
	xmlNodePtr node;

	if (!list) return NULL;

	topnode = xmlNewNode (NULL, "gtt:interval-list");

	for (p=list; p; p=p->next)
	{
		GttInterval *ivl = p->data;
		node = gtt_xml_interval_to_dom_tree (ivl);
		xmlAddChild (topnode, node);
	}

   	return topnode;
}

/* ======================================================= */

/* convert one task to a dom tree */

static xmlNodePtr
gtt_xml_task_to_dom_tree (GttTask *task)
{
	GList *p;
	xmlNodePtr node, topnode;

	if (!task) return NULL;

	topnode = xmlNewNode (NULL, "gtt:task");

	PUT_STR ("memo", gtt_task_get_memo(task));
	PUT_STR ("notes", gtt_task_get_notes(task));
	PUT_INT ("bill_unit", gtt_task_get_bill_unit(task));

	PUT_ENUM_3 ("billable", gtt_task_get_billable(task),
		BILLABLE, NOT_BILLABLE, NO_CHARGE);
	PUT_ENUM_4 ("billrate", gtt_task_get_billrate(task),
		REGULAR, OVERTIME, OVEROVER, FLAT_FEE);
	PUT_ENUM_3 ("billstatus", gtt_task_get_billstatus(task),
		HOLD, BILL, PAID);


	/* add list of intervals */
	p = gtt_task_get_intervals (task);
	node = gtt_interval_list_to_dom_tree (p);
	xmlAddChild (topnode, node);

	return topnode;
}

/* convert a list of gtt tasks into a DOM tree */
static xmlNodePtr
gtt_task_list_to_dom_tree (GList *list)
{
	GList *p;
	xmlNodePtr topnode;
	xmlNodePtr node;

	if (!list) return NULL;

	topnode = xmlNewNode (NULL, "gtt:task-list");

	for (p=list; p; p=p->next)
	{
		GttTask *task = p->data;
		node = gtt_xml_task_to_dom_tree (task);
		xmlAddChild (topnode, node);
	}

   	return topnode;
}

/* ======================================================= */


/* convert one project into a dom tree */
static xmlNodePtr
gtt_xml_project_to_dom_tree (GttProject *prj)
{
	GList *children, *tasks;
	xmlNodePtr node, topnode;

	if (!prj) return NULL;
	topnode = xmlNewNode (NULL, "gtt:project");

	node = xmlNewNode (NULL, "title");
	xmlNodeAddContent(node, gtt_project_get_title(prj));
	xmlAddChild (topnode, node);

	PUT_STR ("desc", gtt_project_get_desc(prj));
	PUT_STR ("notes", gtt_project_get_notes(prj));
	PUT_STR ("custid", gtt_project_get_custid(prj));

	PUT_INT ("id", gtt_project_get_id(prj));

	PUT_DBL ("billrate", gtt_project_get_billrate(prj));
	PUT_DBL ("overtime_rate", gtt_project_get_overtime_rate(prj));
	PUT_DBL ("overover_rate", gtt_project_get_overover_rate(prj));
	PUT_DBL ("flat_fee", gtt_project_get_flat_fee(prj));

	PUT_INT ("min_interval", gtt_project_get_min_interval(prj));
	PUT_INT ("auto_merge_interval", gtt_project_get_auto_merge_interval(prj));
	PUT_INT ("auto_merge_gap", gtt_project_get_auto_merge_gap(prj));

	PUT_LONG ("estimated_start", gtt_project_get_estimated_start(prj));
	PUT_LONG ("estimated_end", gtt_project_get_estimated_end(prj));
	PUT_LONG ("due_date", gtt_project_get_due_date(prj));

	PUT_INT ("sizing", gtt_project_get_sizing(prj));
	PUT_INT ("percent_complete", gtt_project_get_percent_complete(prj));

	PUT_ENUM_4 ("urgency", gtt_project_get_urgency(prj),
		UNDEFINED, LOW, MEDIUM, HIGH);
	PUT_ENUM_4 ("importance", gtt_project_get_importance(prj),
		UNDEFINED, LOW, MEDIUM, HIGH);
	PUT_ENUM_5 ("status", gtt_project_get_status(prj),
		NOT_STARTED, IN_PROGRESS, ON_HOLD, CANCELLED, COMPLETED);

	/* handle tasks */
	tasks = gtt_project_get_tasks(prj);
	node = gtt_task_list_to_dom_tree (tasks);
	xmlAddChild (topnode, node);	

	/* handle sub-projects */
	children = gtt_project_get_children (prj);
	if (children)
	{
		node = gtt_project_list_to_dom_tree (children);
		xmlAddChild (topnode, node);
	}

	return topnode;
}

/* convert a list of gtt projects into a DOM tree */
static xmlNodePtr
gtt_project_list_to_dom_tree (GList *list)
{
	GList *p;
	xmlNodePtr topnode;
	xmlNodePtr node;

	if (!list) return NULL;

	topnode = xmlNewNode (NULL, "gtt:project-list");

	for (p=list; p; p=p->next)
	{
		GttProject *prj = p->data;
		node = gtt_xml_project_to_dom_tree (prj);
		xmlAddChild (topnode, node);
	}

   	return topnode;
}

/* ======================================================= */

/* convert all gtt state into a DOM tree */
static xmlNodePtr
gtt_to_dom_tree (void)
{
	xmlNodePtr topnode;
	xmlNodePtr node;

	topnode = xmlNewNode(NULL, "gtt");
	xmlSetProp(topnode, "version", "1.0.0");

	node = gtt_project_list_to_dom_tree (gtt_get_project_list()); 
	if (node) xmlAddChild (topnode, node);

   	return topnode;

}

/* Write all gtt data to xml file */

void
gtt_xml_write_file (const char * filename)
{
	char * tmpfilename;
	xmlNodePtr topnode;
	FILE *fh;
	int rc;

	tmpfilename = g_strconcat (filename, ".tmp", NULL);
	fh = fopen (tmpfilename, "w");
	g_free (tmpfilename);
	if (!fh) { gtt_err_set_code (GTT_CANT_OPEN_FILE); return; }

	topnode = gtt_to_dom_tree();

	fprintf(fh, "<?xml version=\"1.0\"?>\n");
	xmlElemDump (fh, NULL, topnode);
	xmlFreeNode (topnode);

	fprintf(fh, "\n");

	/* The algorithm we use here is to write to a tmp file,
	 * make sure that the write succeeded, and only then 
	 * rename the temp file to the real file name.  Note that
	 * certain errors (e.g. no room on disk) are not reported
	 * until the fclose, which makes this an important code 
	 * to check.
	 * Sure wish there was a way of finding out if xmlElemDump
	 * suceeded ...
	 */
	rc = fflush (fh);
	if (rc) { gtt_err_set_code (GTT_CANT_WRITE_FILE); return; }

	rc = fclose (fh);
	if (rc) { gtt_err_set_code (GTT_CANT_WRITE_FILE); return; }

	/* If we were truly paranoid, we could, at this point, try
	 * to re-open and re-read the data file, and then match what 
	 * we read to the existing gtt data.   I'm not sure if 
	 * I should be that paranoid.  However, once upon a time,
	 * I did loose all my data during a gnome-desktop-shutdown,
	 * which freaked me out, but I cannot reproduce this loss.  
	 * What to do, what to do ...
	 */

	tmpfilename = g_strconcat (filename, ".tmp", NULL);
	rc = rename (tmpfilename, filename);
	g_free (tmpfilename);
	if (rc) { gtt_err_set_code (GTT_CANT_WRITE_FILE); return; }
}

/* ===================== END OF FILE ================== */
