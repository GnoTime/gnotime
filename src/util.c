/*   Utilities for GTimeTracker - a time tracker
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


#define _GNU_SOURCE
#define __EXTENSIONS__

#include "config.h"

#include <ctype.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glade/glade.h>
#include <gnome.h>

/* hack alert --xxx fixme -- we need to configure.in for have_langinfo */
#define HAVE_LANGINFO_D_FMT
#ifdef HAVE_LANGINFO_D_FMT
#include <langinfo.h>
#endif

#include "util.h"


#ifdef HAVE_LANGINFO_D_FMT
#  define GNC_D_FMT (nl_langinfo (D_FMT))
#  define GNC_D_T_FMT (nl_langinfo (D_T_FMT))
#  define GNC_T_FMT (nl_langinfo (T_FMT))
#else
#  define GNC_D_FMT "%Y-%m-%d"
#  define GNC_D_T_FMT "%Y-%m-%d %r"
#  define GNC_T_FMT "%r"
#endif

/* ============================================================== */

char *
print_hours_elapsed (char * buff, int len, int secs, gboolean show_secs)
{
	size_t flen;
	if (0 <= secs)
	{
		if (show_secs)
		{
			flen = g_snprintf(buff, len,
			   "%02d:%02d:%02d", (int)(secs / 3600),
			   (int)((secs % 3600) / 60), (int)(secs % 60));
		}
		else
		{
			flen = g_snprintf(buff, len, 
			   "%02d:%02d", (int)(secs / 3600),
			   (int)((secs % 3600) / 60));
		}
	} 
	else 
	{
		if (show_secs)
		{
			flen = g_snprintf(buff, len,
			   "-%02d:%02d:%02d", (int)(-secs / 3600),
			   (int)((-secs % 3600) / 60), (int)(-secs % 60));
		}
		else
		{
			flen = g_snprintf(buff, len,
			   "-%02d:%02d", (int)(-secs / 3600),
			   (int)((-secs % 3600) / 60));
		}
	}
	return buff+flen;
}

/* ============================================================== */
/* The following code baldly stolen fron GnuCash */

static DateFormat dateFormat = DATE_FORMAT_LOCALE;

void 
set_date_format(DateFormat df)
{
  dateFormat = df;
}

char * 
print_date (char * buff, size_t len, time_t secs)
{
  int flen;
  int day, month, year;
  struct tm ltm;
  
  if (!buff) return buff;

  /* Note that when printing year, we use %-4d in format string;
   * this causes a one, two or three-digit year to be left-adjusted
   * when printed (i.e. padded with blanks on the right).  This is 
   * important while the user is editing the year, since erasing a 
   * digit can temporarily cause a three-digit year, and having the 
   * blank on the left is a real pain for the user.  So pad on the 
   * right.
   */
  ltm = *localtime (&secs);
  day = ltm.tm_mday;
  month = ltm.tm_mon +1;
  year = ltm.tm_year +1900;
  
  switch(dateFormat)
  {
    case DATE_FORMAT_UK:
      flen = g_snprintf (buff, len, "%2d/%2d/%-4d", day, month, year);
      break;
    case DATE_FORMAT_CE:
      flen = g_snprintf (buff, len, "%2d.%2d.%-4d", day, month, year);
      break;
    case DATE_FORMAT_ISO:
      flen = g_snprintf (buff, len, "%04d-%02d-%02d", year, month, day);
      break;
    case DATE_FORMAT_LOCALE:
      {
        flen = strftime (buff, len, GNC_D_FMT, &ltm);
      }
      break;

    case DATE_FORMAT_US:
    default:
      flen = g_snprintf (buff, len, "%2d/%2d/%-4d", month, day, year);
      break;
  }
  return buff + flen;
}

char * 
print_date_time (char * buff, size_t len, time_t secs)
{
  int flen;
  int day, month, year, hour, min, sec;
  struct tm ltm;
  
  if (!buff) return buff;

  /* Note that when printing year, we use %-4d in format string;
   * this causes a one, two or three-digit year to be left-adjusted
   * when printed (i.e. padded with blanks on the right).  This is 
   * important while the user is editing the year, since erasing a 
   * digit can temporarily cause a three-digit year, and having the 
   * blank on the left is a real pain for the user.  So pad on the 
   * right.
   */
  ltm = *localtime (&secs);
  day = ltm.tm_mday;
  month = ltm.tm_mon +1;
  year = ltm.tm_year +1900;
  hour = ltm.tm_hour;
  min = ltm.tm_min;
  sec = ltm.tm_sec;
  
  switch(dateFormat)
  {
    case DATE_FORMAT_UK:
      flen = g_snprintf (buff, len, "%2d/%2d/%-4d %2d:%02d", day, month, year, hour, min);
      break;
    case DATE_FORMAT_CE:
      flen = g_snprintf (buff, len, "%2d.%2d.%-4d %2d:%02d", day, month, year, hour, min);
      break;
    case DATE_FORMAT_ISO:
      flen = g_snprintf (buff, len, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, min);
      break;
    case DATE_FORMAT_LOCALE:
      {
        flen = strftime (buff, len, GNC_D_T_FMT, &ltm);
      }
      break;

    case DATE_FORMAT_US:
    default:
      flen = g_snprintf (buff, len, "%2d/%2d/%-4d %2d:%02d", month, day, year, hour, min);
      break;
  }
  return buff + flen;
}

char * 
print_time (char * buff, size_t len, time_t secs)
{
  int flen;
  struct tm ltm;
  
  if (!buff) return buff;
  ltm = *localtime (&secs);
  flen = strftime (buff, len, GNC_T_FMT, &ltm);

  return buff + flen;
}

/* ============================================================== */

int
is_same_day (time_t ta, time_t tb)
{
  struct tm lta, ltb;
  lta = *localtime (&ta);
  ltb = *localtime (&tb);
  if (lta.tm_year == ltb.tm_year)
  {
    return (ltb.tm_yday - lta.tm_yday);
  }
  return (ltb.tm_year - lta.tm_year)*365;  /* very approximate */
}

/* ============================================================== */

void
xxxgtk_text_set_text (GtkText *text, const char *str)
{
	gint pos=0;
	if (!str) str = "";
	gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
	gtk_editable_insert_text (GTK_EDITABLE (text), str,
                            strlen(str), &pos);

}

const char *
xxxgtk_text_get_text (GtkText *text)
{
 	return gtk_editable_get_chars (GTK_EDITABLE(text), 0, -1);
}

/* ============================================================== */

/* Glade loader, it will look in the right directories */
GladeXML *
gtt_glade_xml_new (const char *filename, const char *widget)
{
	GladeXML *xml = NULL;

	g_return_val_if_fail (filename != NULL, NULL);

	if (g_file_test (filename, G_FILE_TEST_EXISTS))
		xml = glade_xml_new (filename, widget, NULL);

	if (xml == NULL) {
		char *file = g_concat_dir_and_file (GTTGLADEDIR, filename);
		xml = glade_xml_new (file, widget, NULL);
		g_free (file);
	}
	return xml;
}

/* ===================== END OF FILE ============================ */

