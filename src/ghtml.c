/*   Generate gtt-parsed html output for GnoTime - a time tracker
 *   Copyright (C) 2001,2002 Linas Vepstas <linas@linas.org>
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
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "gtt.h"
#include "ghtml.h"
#include "proj.h"
#include "util.h"

typedef enum {
	NUL=0,

	/* task things */
	MEMO = 1,
	NOTES,
	TASK_TIME,
	BILLSTATUS,
	BILLABLE,
	BILLRATE,
	VALUE,
	BILLABLE_VALUE,

	/* interval things */
	START_DATIME =100,
	STOP_DATIME,
	ELAPSED,
	FUZZ,

	/* project things */
	TITLE =200,
	DESC,
	PNOTES,
	EST_START,
	EST_END,
	DUE_DATE,
	SIZING,
	PERCENT,
	URGENCY,
	IMPORTANCE,
	STATUS

} TableCol;

#define NCOL 30


struct gtt_ghtml_s
{
	/* stream interface for writing */
	void (*open_stream) (GttGhtml *, gpointer);
	void (*write_stream) (GttGhtml *, const char *, size_t len, gpointer);
	void (*close_stream) (GttGhtml *, gpointer);
	void (*error) (GttGhtml *, int errcode, const char * msg, gpointer);
	gpointer user_data;

	gboolean show_html;  /* Flag -- add html markup, or not */
	gboolean show_links; /* Flag -- show internal <a href> links */

	/* field delimiter, for tab/comma delim */
	char * delim;

	/* table layout info */
	int nproj_cols;
	TableCol proj_cols[NCOL];
	char * proj_titles[NCOL];

	int ntask_cols;
	TableCol task_cols[NCOL];
	char * task_titles[NCOL];

	int ninvl_cols;
	TableCol invl_cols[NCOL];
	char * invl_titles[NCOL];

	char **tp;
};

/* ============================================================== */
/* Seems to me like guile screwed the pooch; we need a global! */

static GttGhtml *ghtml_guile_global_hack = NULL;   

/* ============================================================== */
/* This routine outputs a simple, hard-coded table showing the 
 * project journal.  Its not terribly useful for use inside of
 * reports, but its a great place to understand how the other
 * code in this file works. */

static SCM
do_show_journal (GttGhtml *ghtml, GttProject *prj)
{
	char buff[100];
	GList *node;
	GString *p;
	char * ps;
	gboolean show_links = ghtml->show_links;

	if (NULL == ghtml->write_stream) return;

	p = g_string_new(NULL);
	g_string_append_printf (p, "<table border=1>\n"
		"<tr><th colspan=4>%s</th></tr>\n"
		"<tr><th>&nbsp;</th><th>%s</th><th>%s</th><th>%s</th></tr>\n",
		_("Diary Entry"), _("Start"), _("Stop"), _("Elapsed"));

	(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);

	for (node = gtt_project_get_tasks(prj); node; node=node->next)
	{
		const char *pp;
		int prt_date = 1;
		time_t prev_stop = 0;
		GList *in;
		GttTask *tsk = node->data;
		
		p = g_string_truncate (p, 0);
		p = g_string_append (p, "<tr><td colspan=4>");
		if (show_links) 
		{
			g_string_append_printf (p, "<a href=\"gtt:task:0x%x\">", tsk);
		}

		pp = gtt_task_get_memo(tsk);
		if (!pp || !pp[0]) pp = _("(empty)");
		p = g_string_append (p, pp);
		if (show_links) p = g_string_append (p, "</a>");
		p = g_string_append (p, "</td></tr>\n");

		(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);

		
		for (in=gtt_task_get_intervals(tsk); in; in=in->next)
		{
			GttInterval *ivl = in->data;
			time_t start, stop, elapsed;
			start = gtt_interval_get_start (ivl);
			stop = gtt_interval_get_stop (ivl);
			elapsed = stop - start;
			
			p = g_string_truncate (p, 0);
			p = g_string_append (p, 
				"<tr><td>&nbsp;&nbsp;&nbsp;</td>"
				"<td align=right>&nbsp;&nbsp;");
			if (show_links) 
			{
				g_string_append_printf (p, "<a href=\"gtt:interval:0x%x\">", ivl);
			}

			/* print hour only or date too? */
			if (0 != prev_stop) {
				prt_date = is_same_day(start, prev_stop);
			}
			if (prt_date) {
				print_date_time (buff, 100, start);
				p = g_string_append (p, buff);
			} else {
				print_time (buff, 100, start);
				p = g_string_append (p, buff);
			}

			/* print hour only or date too? */
			prt_date = is_same_day(start, stop);
			if (show_links) p = g_string_append (p, "</a>");
			p = g_string_append (p, 
				"&nbsp;&nbsp;</td>"
				"<td>&nbsp;&nbsp;");
			if (show_links)
			{
				g_string_append_printf (p, "<a href=\"gtt:interval:0x%x\">", ivl);
			}
			if (prt_date) {
				print_date_time (buff, 100, stop);
				p = g_string_append (p, buff);
			} else {
				print_time (buff, 100, stop);
				p = g_string_append (p, buff);
			}

			prev_stop = stop;

			if (show_links) p = g_string_append (p, "</a>");
			p = g_string_append (p, "&nbsp;&nbsp;</td><td>&nbsp;&nbsp;");
			print_hours_elapsed (buff, 100, elapsed, TRUE);
			p = g_string_append (p, buff);
			p = g_string_append (p, "&nbsp;&nbsp;</td></tr>\n");
			(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);
		}

	}
	
	ps = "</table>\n";
	(ghtml->write_stream) (ghtml, ps, strlen(ps), ghtml->user_data);

	/* should the free-segment be false or true ??? */
	g_string_free (p, FALSE);

	return SCM_UNSPECIFIED;  
}

/* ============================================================== */
/* This routine will reverse the order of a scheme list */

static SCM
reverse_list (SCM node_list)
{
	SCM rc, node;
	rc = gh_eval_str ("()");

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

	/* if its a list, then process the list */
	else if (SCM_CONSP(project))
	{
		SCM proj_list = project;
		
		/* Get a pointer to null */
		rc = gh_eval_str ("()");
	
		while (FALSE == SCM_NULLP(proj_list))
		{
			SCM evl;
			project = gh_car (proj_list);
			evl = do_apply_on_project (ghtml, project, func);
			rc = gh_cons (evl, rc);
			proj_list = gh_cdr (proj_list);
		}

		/* reverse the list. Ughh */
		/* gh_reverse (rc);  this doesn't work, to it manually */
		rc = reverse_list (rc);
		
		return rc;
	}
	
	g_warning ("expecting gtt project as argument, got something else\n");
	rc = gh_eval_str ("()");
	return rc;
}

/* ============================================================== */

#define PROJ_COL_TITLE(DEFAULT_STR)                        \
{                                                          \
   if (ghtml->proj_titles[i]) {                            \
      p = g_string_append (p, ghtml->proj_titles[i]);      \
   } else {                                                \
      p = g_string_append (p, DEFAULT_STR);                \
   }                                                       \
}

#define TASK_COL_TITLE(DEFAULT_STR)                        \
{                                                          \
   if (ghtml->task_titles[i]) {                            \
      p = g_string_append (p, ghtml->task_titles[i]);      \
   } else {                                                \
      p = g_string_append (p, DEFAULT_STR);                \
   }                                                       \
}

#define INVL_COL_TITLE(DEFAULT_STR)                        \
{                                                          \
   if (ghtml->invl_titles[i]) {                            \
      p = g_string_append (p, ghtml->invl_titles[i]);      \
   } else {                                                \
      p = g_string_append (p, DEFAULT_STR);                \
   }                                                       \
}

static void
do_show_table (GttGhtml *ghtml, GttProject *prj, int invoice)
{
	int i;
	GList *node;
	char buff [100];
	GString *p;
	gboolean output_html = ghtml->show_html;
	gboolean show_links = ghtml->show_links;

	if (NULL == ghtml->write_stream) return;

	p = g_string_new (NULL);
	if (output_html) p = g_string_append (p, "<table border=1>");

	/* write out the table header */
	if (output_html && (0 < ghtml->ntask_cols))
	{
		p = g_string_append (p, "<tr>");
	}
	for (i=0; i<ghtml->ntask_cols; i++)
	{
		switch (ghtml->task_cols[i]) 
		{
			case MEMO:
			{
				int mcols;
				mcols = ghtml->ninvl_cols - ghtml->ntask_cols;
				if (0 >= mcols) mcols = 1; 
				if (output_html) g_string_append_printf (p, "<th colspan=%d>", mcols);
				TASK_COL_TITLE (_("Diary Entry"));
				break;
			}
			case NOTES:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Notes"));
				break;
			case TASK_TIME:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Task Time"));
				break;
			case BILLSTATUS:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Bill Status"));
				break;
			case BILLABLE:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Billable"));
				break;
			case BILLRATE:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Bill Rate"));
				break;
			case VALUE:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Value"));
				break;
			case BILLABLE_VALUE:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("Billable Value"));
				break;
			default:
				if (output_html) p = g_string_append (p, "<th>");
				TASK_COL_TITLE (_("No Default Value"));
		}
		p = g_string_append (p, ghtml->delim);	
		if (output_html) p = g_string_append (p, "</th>");
	}
	p = g_string_append (p, "\r\n");

	if (output_html && (0 < ghtml->ninvl_cols))
	{
		p = g_string_append (p, "</tr><tr>");
	}
	for (i=0; i<ghtml->ninvl_cols; i++)
	{
		if (output_html) p = g_string_append (p, "<th>");
		switch (ghtml->invl_cols[i]) 
		{
			case START_DATIME:
				INVL_COL_TITLE (_("Start"));
				break;
			case STOP_DATIME:
				INVL_COL_TITLE (_("Stop"));
				break;
			case ELAPSED:
				INVL_COL_TITLE (_("Elapsed"));
				break;
			case FUZZ:
				INVL_COL_TITLE (_("Start Time Fuzziness"));
				break;
			default:
				TASK_COL_TITLE (_("No Default Value"));
		}
		if (output_html) p = g_string_append (p, "</th>");
	}
	if (output_html && (0 < ghtml->ninvl_cols))
	{
		p = g_string_append (p, "</tr>");
	}
	p = g_string_append (p, "\r\n");

	(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);

	for (node = gtt_project_get_tasks(prj); node; node=node->next)
	{
		GttBillStatus billstatus;
		GttBillable billable;
		GttBillRate billrate;
		const char *pp;
		time_t prev_stop = 0;
		GList *in;
		GttTask *tsk = node->data;
		int task_secs;
		double hours, value=0.0, billable_value=0.0;
		
		/* set up data */
		billstatus = gtt_task_get_billstatus (tsk);
		billable = gtt_task_get_billable (tsk);
		billrate = gtt_task_get_billrate (tsk);

		/* if we are in invoice mode, then skip anything not billable */
		if (invoice)
		{
			if ((GTT_BILL != billstatus) || 
			    (GTT_NOT_BILLABLE == billable)) continue;
		}

		task_secs = gtt_task_get_secs_ever(tsk);
		hours = task_secs;
		hours /= 3600;
		switch (billrate)
		{
			case GTT_REGULAR:
				value = hours * gtt_project_get_billrate (prj);
				break;
			case GTT_OVERTIME:
				value = hours * gtt_project_get_overtime_rate (prj);
				break;
			case GTT_OVEROVER:
				value = hours * gtt_project_get_overover_rate (prj);
				break;
			case GTT_FLAT_FEE:
				value = gtt_project_get_flat_fee (prj);
				break;
		}

		switch (billable)
		{
			case GTT_BILLABLE:
				billable_value = value;
				break;
			case GTT_NOT_BILLABLE:
				billable_value = 0.0;
				break;
			case GTT_NO_CHARGE:
				billable_value = 0.0;
				break;
		}

		/* start with an empty string */
		p = g_string_truncate (p,0);

		/* write the task data */
		if (output_html && (0 < ghtml->ntask_cols))
		{
			p = g_string_append (p, "<tr>");
		}
		for (i=0; i<ghtml->ntask_cols; i++)
		{
			switch (ghtml->task_cols[i]) 
			{
				case MEMO:
				{
					int mcols;
					mcols = ghtml->ninvl_cols - ghtml->ntask_cols;
					if (0 >= mcols) mcols = 1; 
					if (output_html) g_string_append_printf (p, "<td colspan=%d>", mcols);
					if (show_links) 
					{
						g_string_append_printf (p, "<a href=\"gtt:task:0x%x\">", tsk);
					}
					pp = gtt_task_get_memo(tsk);
					if (!pp || !pp[0]) pp = _("(empty)");
					p = g_string_append (p, pp);
					if (show_links) p = g_string_append (p, "</a>");
					break;
				}

				case NOTES:
					if (output_html) p = g_string_append (p, "<td align=left>");
					pp = gtt_task_get_notes(tsk);
					if (!pp || !pp[0]) pp = _("(empty)");
					p = g_string_append (p, pp); 
					break;

				case TASK_TIME:
					if (output_html) p = g_string_append (p, "<td align=right>");
					print_hours_elapsed (buff, 100, task_secs, TRUE);
					p = g_string_append (p, buff); 
					break;

				case BILLSTATUS:
					if (output_html) p = g_string_append (p, "<td>");
					switch (billstatus)
					{
						case GTT_HOLD:
							p = g_string_append (p, _("Hold"));
							break;
						case GTT_BILL:
							p = g_string_append (p, _("Bill"));
							break;
						case GTT_PAID:
							p = g_string_append (p, _("Paid"));
							break;
					}
					break;

				case BILLABLE:
					if (output_html) p = g_string_append (p, "<td>");
					switch (billable)
					{
						case GTT_BILLABLE:
							p = g_string_append (p, _("Billable"));
							break;
						case GTT_NOT_BILLABLE:
							p = g_string_append (p, _("Not Billable"));
							break;
						case GTT_NO_CHARGE:
							p = g_string_append (p, _("No Charge"));
							break;
					}
					break;

				case BILLRATE:
					if (output_html) p = g_string_append (p, "<td>");
					switch (billrate)
					{
						case GTT_REGULAR:
							p = g_string_append (p, _("Regular"));
							break;
						case GTT_OVERTIME:
							p = g_string_append (p, _("Overtime"));
							break;
						case GTT_OVEROVER:
							p = g_string_append (p, _("Double Overtime"));
							break;
						case GTT_FLAT_FEE:
							p = g_string_append (p, _("Flat Fee"));
							break;
					}
					break;

				case VALUE:
					if (output_html) p = g_string_append (p, "<td align=right>");
					
					/* hack alert should use i18n currency/monetary printing */
					g_string_append_printf (p, "$%.2f", value+0.0049);
					break;

				case BILLABLE_VALUE:
					if (output_html) p = g_string_append (p, "<td align=right>");
					/* hack alert should use i18n currency/monetary printing */
					g_string_append_printf (p, "$%.2f", billable_value+0.0049);
					break;

				default:
					if (output_html) p = g_string_append (p, "<td>");
					p = g_string_append (p, _("Error - Unknown"));
			}
			p = g_string_append (p, ghtml->delim);
			if (output_html) p = g_string_append (p, "</td>");
		}

		if (0 < ghtml->ntask_cols)
		{
			if (output_html) p = g_string_append (p, "</tr>");
			p = g_string_append (p, "\r\n");
			(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);
		}
		
		/* write out intervals */
		for (in=gtt_task_get_intervals(tsk); in; in=in->next)
		{
			GttInterval *ivl = in->data;
			time_t start, stop, elapsed;
			int prt_start_date = 1;
			int prt_stop_date = 1;

			/* data setup */
			start = gtt_interval_get_start (ivl);
			stop = gtt_interval_get_stop (ivl);
			elapsed = stop - start;

			/* print hour only or date too? */
			prt_stop_date = is_same_day(start, stop);
			if (0 != prev_stop) {
				prt_start_date = is_same_day(start, prev_stop);
			}
			prev_stop = stop;


			/* -------------------------- */
			p = g_string_truncate (p,0);
			if (output_html) p = g_string_append (p, "<tr>");
			for (i=0; i<ghtml->ninvl_cols; i++)
			{

				switch (ghtml->invl_cols[i]) 
				{
	case START_DATIME:
	{
		if (output_html) p = g_string_append (p, "<td align=right>&nbsp;&nbsp;");
		if (show_links)
		{
			g_string_append_printf (p, "<a href=\"gtt:interval:0x%x\">", ivl);
		}
		if (prt_start_date) {
			print_date_time (buff, 100, start);
			p = g_string_append (p, buff);
		} else {
			print_time (buff, 100, start);
			p = g_string_append (p, buff);
		}
		if (show_links) p = g_string_append (p, "</a>");
		p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	case STOP_DATIME:
	{
		if (output_html) p = g_string_append (p, "<td align=right>&nbsp;&nbsp;");
		if (show_links)
		{
			g_string_append_printf (p, "<a href=\"gtt:interval:0x%x\">", ivl);
		}
		if (prt_stop_date) {
			print_date_time (buff, 100, stop);
			p = g_string_append (p, buff);
		} else {
			print_time (buff, 100, stop);
			p = g_string_append (p, buff);
		}
		if (show_links) p = g_string_append (p, "</a>");
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	case ELAPSED:
	{
		if (output_html) p = g_string_append (p, "<td>&nbsp;&nbsp;");
		print_hours_elapsed (buff, 100, elapsed, TRUE);
		p = g_string_append (p, buff);
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	case FUZZ:
	{
		if (output_html) p = g_string_append (p, "<td>&nbsp;&nbsp;");
		print_hours_elapsed (buff, 100, gtt_interval_get_fuzz(ivl), TRUE);
		p = g_string_append (p, buff);
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	default:
		if (output_html) p = g_string_append (p, "<td>");
				}
				if (output_html) p = g_string_append (p, "</td>");
			}

			if (output_html) p = g_string_append (p, "</tr>");
			p = g_string_append (p, ghtml->delim);
			(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);
		}

		p = g_string_append (p, "\n");
	}

	g_string_free (p, FALSE);
	
	if (output_html)
	{
		char * ps = "</table>\n";
		(ghtml->write_stream) (ghtml, ps, strlen(ps), ghtml->user_data);
	}
}

/* ============================================================== */

static SCM 
gtt_hello (void)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	char *p;
	if (NULL == ghtml->write_stream) return SCM_UNSPECIFIED;

	p = "Hello World!";

	(ghtml->write_stream) (ghtml, p, strlen(p), ghtml->user_data);

	/* maybe we should return something meaningful, like the string? */
	return SCM_UNSPECIFIED;  
}

/* ============================================================== */
/* This routine scans the character string for column names,
 * such as '$fuzz', and sets them up in the ghtml C structure
 * for later use.
 */

#define PROJ_COL(TYPE) {                                        \
        ghtml->proj_cols[ghtml->nproj_cols] = TYPE;             \
        ghtml->tp = &(ghtml->proj_titles[ghtml->nproj_cols]);   \
        *(ghtml->tp) = NULL;                                    \
        if (NCOL-1 > ghtml->nproj_cols) ghtml->nproj_cols ++;   \
}

#define TASK_COL(TYPE) {                                        \
        ghtml->task_cols[ghtml->ntask_cols] = TYPE;             \
        ghtml->tp = &(ghtml->task_titles[ghtml->ntask_cols]);   \
        *(ghtml->tp) = NULL;                                    \
        if (NCOL-1 > ghtml->ntask_cols) ghtml->ntask_cols ++;   \
}

#define INVL_COL(TYPE) {                                        \
        ghtml->invl_cols[ghtml->ninvl_cols] = TYPE;             \
        ghtml->tp = &(ghtml->invl_titles[ghtml->ninvl_cols]);   \
        *(ghtml->tp) = NULL;                                    \
        if (NCOL-1 > ghtml->ninvl_cols) ghtml->ninvl_cols ++;   \
}

static void
decode_column (GttGhtml *ghtml, const char * tok)
{
	if ('$' != tok[0])
	{
		if (ghtml->tp)
		{
			if (*ghtml->tp) g_free (*ghtml->tp);
			*ghtml->tp = g_strdup (tok);
		}
	}
	else
	if (0 == strncmp (tok, "$title", 6))
	{
		PROJ_COL (TITLE);
	}
	else
	if (0 == strncmp (tok, "$desc", 5))
	{
		PROJ_COL (DESC);
	}
	else
	if (0 == strncmp (tok, "$pnotes", 7))
	{
		PROJ_COL (PNOTES);
	}
	else
	if (0 == strncmp (tok, "$est_start", 10))
	{
		PROJ_COL (EST_START);
	}
	else
	if (0 == strncmp (tok, "$est_end", 8))
	{
		PROJ_COL (EST_END);
	}
	else
	if (0 == strncmp (tok, "$due_date", 9))
	{
		PROJ_COL (DUE_DATE);
	}
	else
	if (0 == strncmp (tok, "$sizing", 7))
	{
		PROJ_COL (SIZING);
	}
	else
	if (0 == strncmp (tok, "$percent", 8))
	{
		PROJ_COL (PERCENT);
	}
	else
	if (0 == strncmp (tok, "$urgency", 8))
	{
		PROJ_COL (URGENCY);
	}
	else
	if (0 == strncmp (tok, "$importance", 11))
	{
		PROJ_COL (IMPORTANCE);
	}
	else
	if (0 == strncmp (tok, "$status", 7))
	{
		PROJ_COL (STATUS);
	}
	else
	if (0 == strncmp (tok, "$start_datime", 13))
	{
		INVL_COL (START_DATIME);
	}
	else
	if (0 == strncmp (tok, "$stop_datime", 12))
	{
		INVL_COL (STOP_DATIME);
	}
	else
	if (0 == strncmp (tok, "$fuzz", 5))
	{
		INVL_COL (FUZZ);
	}
	else
	if (0 == strncmp (tok, "$elapsed", 8))
	{
		INVL_COL (ELAPSED);
	}
	else
	if (0 == strncmp (tok, "$memo", 5))
	{
		TASK_COL(MEMO);
	}
	else
	if (0 == strncmp (tok, "$notes", 6))
	{
		TASK_COL(NOTES);
	}
	else
	if (0 == strncmp (tok, "$task_time", 10))
	{
		TASK_COL(TASK_TIME);
	}
	else
	if (0 == strncmp (tok, "$billstatus", 9))
	{
		TASK_COL(BILLSTATUS);
	}
	else
	if (0 == strncmp (tok, "$billable", 9))
	{
		TASK_COL(BILLABLE);
	}
	else
	if (0 == strncmp (tok, "$billrate", 9))
	{
		TASK_COL(BILLRATE);
	}
	else
	if (0 == strncmp (tok, "$value", 6))
	{
		TASK_COL(VALUE);
	}
	else
	if (0 == strncmp (tok, "$bill_value", 11))
	{
		TASK_COL(BILLABLE_VALUE);
	}
	else if (ghtml->write_stream)
	{
		const char * str;
		str = _("unknown token: >>>>");
		(ghtml->write_stream) (ghtml, str, strlen(str), ghtml->user_data);
		str = tok;
		(ghtml->write_stream) (ghtml, str, strlen(str), ghtml->user_data);
		str = "<<<<";
		(ghtml->write_stream) (ghtml, str, strlen(str), ghtml->user_data);
	}
}

/* ============================================================== */

static SCM
decode_scm_col_list (GttGhtml *ghtml, SCM col_list)
{
	SCM col_name;
	int len;
	char * tok = NULL;

	/* reset the parser */
	ghtml->ninvl_cols = 0;
	ghtml->ntask_cols = 0;
	ghtml->nproj_cols = 0;
		
	while (FALSE == SCM_NULLP(col_list))
	{
		col_name = gh_car (col_list);

		/* either a 'symbol or a "quoted string" */
		if (!SCM_SYMBOLP(col_name) && !SCM_STRINGP (col_name))
		{
			col_list = gh_cdr (col_list);
			continue;
		}
		tok = gh_scm2newstr (col_name, &len);
		decode_column (ghtml, tok);

		free (tok);
		col_list = gh_cdr (col_list);
	}

	return SCM_UNSPECIFIED;
}

static SCM
show_table (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	SCM rc;
	GttProject *prj = ctree_get_focus_project (global_ptw);
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-table");
	rc = decode_scm_col_list (ghtml, col_list);
	do_show_table (ghtml, prj, FALSE);
	return rc;
}

static SCM
show_invoice (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	SCM rc;
	GttProject *prj = ctree_get_focus_project (global_ptw);
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-invoice");
	rc = decode_scm_col_list (ghtml, col_list);
	do_show_table (ghtml, prj, TRUE);
	return rc;
}

static SCM
show_export (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	gboolean save_show_html = ghtml->show_html;
	char *save_delim = ghtml->delim;
	
	GttProject *prj = ctree_get_focus_project (global_ptw);
	SCM rc;
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-export");
	rc = decode_scm_col_list (ghtml, col_list);
	
	ghtml->show_html = FALSE;
	ghtml->delim = "\t";
	
	do_show_table (ghtml, prj, FALSE);
	
	ghtml->show_html = save_show_html;
	ghtml->delim = save_delim;
	
	return rc;
}


/* ============================================================== */
/* This routine accepts an SCM node, and 'prints' it out.
 * (or tries to). It knows how to print numbers strings and lists.
 */

static SCM
do_show_scm (GttGhtml *ghtml, SCM node)
{
	int len;
	char * str = NULL;

	if (NULL == ghtml->write_stream) return SCM_UNSPECIFIED;

	/* Need to test for numbers first, since later tests 
	 * may core-dump guile-1.3.4 if the node was in fact a number. */
	if (SCM_NUMBERP(node))
	{
		char buf[132];
		double x = scm_num2dbl (node, "do_show_scm");
		sprintf (buf, "%g ", x);
		(ghtml->write_stream) (ghtml, buf, strlen(buf), ghtml->user_data);
	}
	else
	/* either a 'symbol or a "quoted string" */
	if (SCM_SYMBOLP(node) || SCM_STRINGP (node))
	{
		str = gh_scm2newstr (node, &len);
		(ghtml->write_stream) (ghtml, str, strlen(str), ghtml->user_data);
		(ghtml->write_stream) (ghtml, " ", 1, ghtml->user_data);
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

	/* We could return the printed string, but I'm not sure why.. */
	return SCM_UNSPECIFIED;
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
 * purpose for now.   Its baad because it exposes a C pointer to
 * the schemers, which could be used for evil purposes, such
 * as propagating viruses, worms, etc.  It sure would be nice to be
 * able to check that the pointer is a valid pointer to a gtt
 * project.  But for now, I don't know of a better way.  --linas
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

/* return a list of all of the projects */

static SCM
do_ret_projects (GttGhtml *ghtml, GList *proj_list)
{
	SCM rc;
	GList *n;

	/* Get a pointer to null */
	rc = gh_eval_str ("()");
	
	/* Get list of all top-level projects, then get tail */
	if (!proj_list) return rc;
	
	/* find the tail */
	for (n= proj_list; n->next; n=n->next) {}
	proj_list = n;
	
	/* walk backwards, creating a scheme list */
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
	GList *proj_list = gtt_get_project_list();
	return do_ret_projects (ghtml, proj_list);
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

#define RET_PROJECT_STR(RET_FUNC,GTT_GETTER)                        \
static SCM                                                          \
GTT_GETTER##_scm (GttGhtml *ghtml, GttProject *prj)                 \
{                                                                   \
	const char * str = GTT_GETTER (prj);                             \
	return gh_str2scm (str, strlen (str));                           \
}                                                                   \
                                                                    \
static SCM                                                          \
RET_FUNC (SCM proj_list)                                            \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_project (ghtml, proj_list, GTT_GETTER##_scm); \
}

RET_PROJECT_STR (ret_project_title, gtt_project_get_title)
RET_PROJECT_STR (ret_project_desc,  gtt_project_get_desc)
RET_PROJECT_STR (ret_project_notes, gtt_project_get_notes)


/* ============================================================== */
/* Handle re_project_title in the almost-standard way,
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

static SCM
ret_project_title_link (SCM proj_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	return do_apply_on_project (ghtml, proj_list, get_project_title_link_scm);
}

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
		default:
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
		default:
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
		default:
	}
	return _("Undefined");
}

RET_PROJECT_STR (ret_project_urgency,    get_urgency)
RET_PROJECT_STR (ret_project_importance, get_importance)
RET_PROJECT_STR (ret_project_status,     get_status)
		  
#define RET_PROJECT_SIMPLE(RET_FUNC,DO_SIMPLE)                      \
static SCM                                                          \
RET_FUNC (SCM proj_list)                                            \
{                                                                   \
	GttGhtml *ghtml = ghtml_guile_global_hack;                       \
	return do_apply_on_project (ghtml, proj_list, DO_SIMPLE);        \
}

RET_PROJECT_SIMPLE (show_journal, do_show_journal);

/* ============================================================== */

void
gtt_ghtml_display (GttGhtml *ghtml, const char *filepath,
                   GttProject *prj)
{
	FILE *ph;
	GString *template;
	char *start, *end, *scmstart, *scmend, *comstart, *comend;
	size_t nr;

	if (!ghtml) return;

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

	start = template->str;
	while (start)
	{
		scmstart = NULL;
		scmend = NULL;
		
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
	gh_new_procedure("gtt-hello",              gtt_hello,      0, 0, 0);
	gh_new_procedure("gtt-show-journal",       show_journal,   1, 0, 0);
	gh_new_procedure("gtt-show-table",         show_table,     1, 0, 0);
	gh_new_procedure("gtt-show-invoice",       show_invoice,   1, 0, 0);
	gh_new_procedure("gtt-show-export",        show_export,    1, 0, 0);
	gh_new_procedure("gtt-show",               show_scm,       1, 0, 0);
	gh_new_procedure("gtt-selected-project",   ret_selected_project,   0, 0, 0);
	gh_new_procedure("gtt-projects",           ret_projects,           0, 0, 0);
	gh_new_procedure("gtt-project-title",      ret_project_title,      1, 0, 0);
	gh_new_procedure("gtt-project-title-link", ret_project_title_link, 1, 0, 0);
	gh_new_procedure("gtt-project-desc",       ret_project_desc,       1, 0, 0);
	gh_new_procedure("gtt-project-notes",      ret_project_notes,      1, 0, 0);
	gh_new_procedure("gtt-project-urgency",    ret_project_urgency,    1, 0, 0);
	gh_new_procedure("gtt-project-importance", ret_project_importance, 1, 0, 0);
	gh_new_procedure("gtt-project-status",     ret_project_status,     1, 0, 0);
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

	p->show_html = TRUE;
	p->show_links = TRUE;
	p->delim = "";

	p->ninvl_cols = 0;
	p->ntask_cols = 0;
	p->nproj_cols = 0;
	p->tp = NULL;

	for (i=0; i<NCOL; i++)
	{
		p->proj_titles[i] = NULL;
		p->task_titles[i] = NULL;
		p->invl_titles[i] = NULL;
	}

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

void 
gtt_ghtml_show_links (GttGhtml *p, gboolean sl)
{
	if (!p) return;
	p->show_links = sl;
}

/* ===================== END OF FILE ==============================  */
