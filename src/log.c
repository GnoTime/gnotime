/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
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

#include <config.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>

#include "cur-proj.h"
#include "log.h"
#include "prefs.h"
#include "proj.h"

#define CAN_LOG ((config_logfile_name!=NULL)&&(config_logfile_use))


static gboolean 
log_write(time_t t, const char *s)
{
	FILE *f;
	char date[256];
	char *filename;

	g_return_val_if_fail (s != NULL, FALSE);

	if ( ! CAN_LOG)
		return TRUE;

	if ((config_logfile_name[0] == '~') &&
	    (config_logfile_name[1] == '/') &&
	    (config_logfile_name[2] != 0)) {
		filename = gnome_util_prepend_user_home(&config_logfile_name[2]);
		f = fopen(filename, "at");
		g_free (filename);
	} else {
		f = fopen (config_logfile_name, "at");
	}

	if (f == NULL) {
		g_warning (_("Cannot open logfile %s for append"),
			   config_logfile_name);
		return FALSE;
	}

	if (t < 0)
		t = time(NULL);

	/* Translators: Format to use in the gnotime logfile */
	if (strftime (date, sizeof (date), _("%b %d %H:%M:%S"),
		      localtime(&t)) <= 0)
		strcpy (date, "???");
	fprintf (f, "%s %s\n", date, s);
	fclose (f);
	return TRUE;
}


char *
printf_project(const char *format, GttProject *proj)
{
	GString *str;
	const char *p;
	char *ret;
	int sss;

	if (!format) return NULL;

	str = g_string_new (NULL);
	for (p = format; *p; p++) {
		if (*p != '%') {
			g_string_append_c(str, *p);
		} else {
			p++;
			
			switch (*p) {
			case 't':
			{
				const char * title = gtt_project_get_title(proj);
				if (title && title[0])
					g_string_append(str, title);
				else
					g_string_append(str, _("no title"));
				break;
			}
			case 'd': 
			{
				const char * desc = gtt_project_get_desc(proj);
				if (desc && desc[0])
					g_string_append(str, desc);
				else
					g_string_append(str, _("no description"));
				break;
			}
			case 'D': 
			   sss = gtt_project_get_id (proj);
				g_string_append_printf (str, "%d", sss);
				break;
				
			case 'h':
				sss = gtt_project_get_secs_ever (proj);
				g_string_append_printf (str, "%d", sss / 3600);
				break;

			case 'H':
				sss = gtt_project_get_secs_day (proj);
				g_string_append_printf (str, "%02d", sss / 3600);
				break;

			case 'm':
				sss = gtt_project_get_secs_ever (proj);
				g_string_append_printf (str, "%d", sss / 60);
				break;
				
			case 'M':
				sss = gtt_project_get_secs_day (proj);
				g_string_append_printf (str, "%02d", (sss / 60) % 60);
				break;
				
			case 's':
				sss = gtt_project_get_secs_ever (proj);
				g_string_append_printf (str,"%d", sss);
				break;

			case 'S':
				sss = gtt_project_get_secs_day (proj);
				g_string_append_printf (str, "%02d", sss % 60);
				break;

			case 'T': 
				sss = gtt_project_get_secs_ever(proj);
				g_string_append_printf(str, "%d:%02d:%02d", sss / 3600,
					   (sss / 60) % 60,
					   sss % 60);
				break;
				
			default:
				g_string_append_c(str, *p);
				break;
			}
		}
	}
	ret = str->str;
	/* Uhh, I'm not sure, but I think 'FALSE' means don't free 
	 * the char * array  */
	g_string_free (str, FALSE);
	return ret;
}


static char *
build_log_entry(const char *format, GttProject *proj)
{
	if (!format || !format[0])
		format = config_logfile_start;
	if (!proj)
		return g_strdup(_("program started"));

	return printf_project (format, proj);
}

static void
do_log_proj (time_t t, GttProject *proj, gboolean start)
{
	char *s;

	if (start) {
		s = build_log_entry (config_logfile_start, proj);
	} else /*stop*/ {
		s = build_log_entry (config_logfile_stop, proj);
	}

	log_write (t, s);
	g_free (s);
}

static void
log_proj_intern (GttProject *proj, gboolean log_if_equal)
{
	static GttProject *last_proj = NULL;
	static gboolean logged_last = FALSE;
	static time_t logged_last_time = 0;
	time_t t;

	if ( ! CAN_LOG)
		return;

	/* used for flushing, forcing a start entry, used at end of day */
	if (log_if_equal &&
	    last_proj == proj &&
	    logged_last) 
	{
		do_log_proj (-1, proj, TRUE /*start*/);
		return;
	}

	if (proj == NULL) {
		if (last_proj == NULL)
			return;
		if (logged_last)
			do_log_proj (-1, last_proj, FALSE /*start*/);
		last_proj = NULL;
		return;
	}

	if (last_proj == NULL) {
		last_proj = proj;
		logged_last_time = time (NULL);
		logged_last = FALSE;
	} else if (last_proj != proj) {
		if (logged_last)
			do_log_proj (-1, last_proj, FALSE /*start*/);
		last_proj = proj;
		logged_last_time = time (NULL);
		logged_last = FALSE;
	}

	t = time (NULL);

	if ( ! logged_last &&
	    (long)(t - logged_last_time) >= config_logfile_min_secs) {
		do_log_proj (logged_last_time, proj, TRUE /*start*/);
		logged_last = TRUE;
	}
}

void
log_proj (GttProject *proj)
{
	if ( ! CAN_LOG)
		return;

	log_proj_intern (proj, FALSE /*log_if_equal*/);
}

void
log_exit (void)
{
	if ( ! CAN_LOG)
		return;
	log_proj_intern (NULL, FALSE /*log_if_equal*/);
	log_write (-1, _("program exited"));
}

void
log_start (void)
{
	if ( ! CAN_LOG)
		return;
	log_write (-1, _("program started"));
}

void
log_endofday (void)
{
	if ( ! CAN_LOG)
		return;
	/* force a flush of the logfile */
	if (cur_proj != NULL)
		log_proj_intern (cur_proj, TRUE /*log_if_equal*/);
}
