/*   Deprecated guile/scheme html output for GnoTime
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

#include <gnc-date.h>

#include "app.h"
#include "ctree.h"
#include "gtt.h"
#include "ghtml.h"
#include "ghtml-deprecated.h"
#include "proj.h"
#include "util.h"

/* This file contains deprecated routines, which should go away 
 * sometime in 2004 or 2005, around gnotime version 3.0 or so
 */

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

} TableCol;

#define NCOL 30


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

	if (NULL == ghtml->write_stream) return SCM_UNSPECIFIED;  

	p = g_string_new(NULL);
	g_string_append_printf (p, 
		"<b>The use of this function is deprecated. "
		" Please see the examples for the recommended style.</b>"
		"<table border=1>\n"
		"<tr><th colspan=4>%s</th></tr>\n"
		"<tr><th> &nbsp; </th><th>%s</th><th>%s</th><th>%s</th></tr>\n",
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
			g_string_append_printf (p, "<a href=\"gtt:task:0x%lx\">", (long)tsk);
		}

		pp = gtt_task_get_memo(tsk);
		if (!pp || !pp[0]) pp = _("(empty)");
		p = g_string_append (p, pp);
		if (show_links) p = g_string_append (p, "</a>");
		p = g_string_append (p, "</td>\n</tr>\n");

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
				"<tr><td> &nbsp; &nbsp; &nbsp; </td>\n"
				"<td align=right> &nbsp; &nbsp; ");
			if (show_links) 
			{
				g_string_append_printf (p, "<a href=\"gtt:interval:0x%lx\">", (long)ivl);
			}

			/* print hour only or date too? */
			if (0 != prev_stop) {
				prt_date = qof_is_same_day(start, prev_stop);
			}
			if (prt_date) {
				qof_print_date_time_buff (buff, 100, start);
				p = g_string_append (p, buff);
			} else {
				qof_print_time_buff (buff, 100, start);
				p = g_string_append (p, buff);
			}

			/* print hour only or date too? */
			prt_date = qof_is_same_day(start, stop);
			if (show_links) p = g_string_append (p, "</a>");
			p = g_string_append (p, 
				" &nbsp; &nbsp; </td>\n"
				"<td> &nbsp; &nbsp; ");
			if (show_links)
			{
				g_string_append_printf (p, "<a href=\"gtt:interval:0x%lx\">", (long)ivl);
			}
			if (prt_date) {
				qof_print_date_time_buff (buff, 100, stop);
				p = g_string_append (p, buff);
			} else {
				qof_print_time_buff (buff, 100, stop);
				p = g_string_append (p, buff);
			}

			prev_stop = stop;

			if (show_links) p = g_string_append (p, "</a>");
			p = g_string_append (p, " &nbsp; &nbsp; </td>\n<td> &nbsp; &nbsp; ");
			qof_print_hours_elapsed_buff (buff, 100, elapsed, TRUE);
			p = g_string_append (p, buff);
			p = g_string_append (p, " &nbsp; &nbsp; </td></tr>\n");
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
	p = g_string_append (p,
		"<b>The use of this function is deprecated. "
		" Please see the examples for the recommended style.</b>");

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
		if (output_html) p = g_string_append (p, "</th>\n");
	}

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
		if (output_html) p = g_string_append (p, "</th>\n");
	}
	if (output_html && (0 < ghtml->ninvl_cols))
	{
		p = g_string_append (p, "</tr>");
	}
	p = g_string_append (p, "\n");

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
						g_string_append_printf (p, "<a href=\"gtt:task:0x%lx\">", (long)tsk);
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
					qof_print_hours_elapsed_buff (buff, 100, task_secs, TRUE);
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
			if (output_html) p = g_string_append (p, "</td>\n");
		}

		if (0 < ghtml->ntask_cols)
		{
			if (output_html) p = g_string_append (p, "</tr>");
			p = g_string_append (p, "\n");
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
			prt_stop_date = qof_is_same_day(start, stop);
			if (0 != prev_stop) {
				prt_start_date = qof_is_same_day(start, prev_stop);
			}
			prev_stop = stop;


			/* -------------------------- */
			p = g_string_truncate (p,0);
			if (output_html && (0<ghtml->ninvl_cols)) p = g_string_append (p, "<tr>");
			for (i=0; i<ghtml->ninvl_cols; i++)
			{

				switch (ghtml->invl_cols[i]) 
				{
	case START_DATIME:
	{
		if (output_html) p = g_string_append (p, "<td align=right>&nbsp;&nbsp;");
		if (show_links)
		{
			g_string_append_printf (p, "<a href=\"gtt:interval:0x%lx\">", (long)ivl);
		}
		if (prt_start_date) {
			qof_print_date_time_buff (buff, 100, start);
			p = g_string_append (p, buff);
		} else {
			qof_print_time_buff (buff, 100, start);
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
			g_string_append_printf (p, "<a href=\"gtt:interval:0x%lx\">", (long)ivl);
		}
		if (prt_stop_date) {
			qof_print_date_time_buff (buff, 100, stop);
			p = g_string_append (p, buff);
		} else {
			qof_print_time_buff (buff, 100, stop);
			p = g_string_append (p, buff);
		}
		if (show_links) p = g_string_append (p, "</a>");
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	case ELAPSED:
	{
		if (output_html) p = g_string_append (p, "<td>&nbsp;&nbsp;");
		qof_print_hours_elapsed_buff (buff, 100, elapsed, TRUE);
		p = g_string_append (p, buff);
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	case FUZZ:
	{
		if (output_html) p = g_string_append (p, "<td>&nbsp;&nbsp;");
		qof_print_hours_elapsed_buff (buff, 100, gtt_interval_get_fuzz(ivl), TRUE);
		p = g_string_append (p, buff);
		if (output_html) p = g_string_append (p, "&nbsp;&nbsp;");
		break;
	}
	default:
		if (output_html) p = g_string_append (p, "<td>");
				}
				if (output_html) p = g_string_append (p, "</td>\n");
			}

			if (output_html && (0<ghtml->ninvl_cols)) p = g_string_append (p, "</tr>\n");
			p = g_string_append (p, ghtml->delim);
			if (0 < p->len) 
			{
				(ghtml->write_stream) (ghtml, p->str, p->len, ghtml->user_data);
			}
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
show_journal (SCM junk)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	do_show_journal (ghtml, ghtml->prj);
	return SCM_UNSPECIFIED;
}

static SCM
show_table (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	SCM rc;
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-table");
	rc = decode_scm_col_list (ghtml, col_list);
	do_show_table (ghtml, ghtml->prj, FALSE);
	return rc;
}

static SCM
show_invoice (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	SCM rc;
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-invoice");
	rc = decode_scm_col_list (ghtml, col_list);
	do_show_table (ghtml, ghtml->prj, TRUE);
	return rc;
}

static SCM
show_export (SCM col_list)
{
	GttGhtml *ghtml = ghtml_guile_global_hack;
	
	SCM rc;
	SCM_ASSERT ( SCM_CONSP (col_list), col_list, SCM_ARG1, "gtt-show-export");
	rc = decode_scm_col_list (ghtml, col_list);
	
	ghtml->show_html = FALSE;
	ghtml->show_links = FALSE;
	ghtml->delim = "\t";
	
	do_show_table (ghtml, ghtml->prj, FALSE);
	
	
	return rc;
}

/* ============================================================== */
/* Register callback handlers for various internally defined 
 * scheme forms. 
 */

static int depr_is_inited = 0;

static void
depr_register_procs (void)
{
	gh_new_procedure("gtt-hello",              gtt_hello,      0, 0, 0);
	gh_new_procedure("gtt-show-journal",       show_journal,   1, 0, 0);
	gh_new_procedure("gtt-show-table",         show_table,     1, 0, 0);
	gh_new_procedure("gtt-show-invoice",       show_invoice,   1, 0, 0);
	gh_new_procedure("gtt-show-export",        show_export,    1, 0, 0);
	
}


/* ============================================================== */

void
gtt_ghtml_deprecated_init (GttGhtml *p)
{
	int i;

	if (!depr_is_inited)
	{
		depr_is_inited = 1;
		depr_register_procs();
	}

	p->show_html = TRUE;
	p->delim = "";

	p->ninvl_cols = 0;
	p->ntask_cols = 0;
	p->tp = NULL;

	for (i=0; i<NCOL; i++)
	{
		p->task_titles[i] = NULL;
		p->invl_titles[i] = NULL;
	}
}

/* ===================== END OF FILE ==============================  */
