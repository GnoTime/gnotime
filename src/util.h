/*   utilities for GTimeTracker - a time tracker
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

#ifndef __GTT_UTIL_H__
#define __GTT_UTIL_H__

#include <glib.h>
#include <gtk/gtktext.h>
#include <glade/glade.h>

#include "gnc-date.h"


/* ------------------------------------------------------------------ */
/* date and time utilities */

/* The print_hours_elapsed() routine will print the 'secs' argument
 *    as HH:MM, and will print the seconds if show_secs is true.  
 *    Thus, for example, secs=3599 will print as 0:59
 *    In analogy to the gnu stpcpy, returns a pointer to the trailing 
 *    null character.
 */
char * print_hours_elapsed (char * buff, int len, int secs, gboolean show_secs);
char * print_minutes_elapsed (char * buff, int len, int secs, gboolean show_secs);

/* The set_date_format() routine sets date format to one of 
 *    US, UK, CE, OR ISO.  Checks to make sure it's a legal value.
 *    Args: DateFormat: enumeration indicating preferred format
 *
 * The print_date() routine converts a date into a localized string
 *    representation.  Returns a pointer to the terminating null byte.
 *
 * The print_time() routine prints only the hour-part of the date.
 *    Thus, if secs is  ...
 *    Returns a pointer to the terminating null byte.
 */

void set_date_format (DateFormat df);
char * print_date (char * buff, size_t len, time_t secs);
char * print_time (char * buff, size_t len, time_t secs);
char * print_date_time (char * buff, size_t len, time_t secs);


/* The is_same_day() routine returns 0 if both times are in the 
 * same day.
 */

int is_same_day (time_t, time_t);

/* ------------------------------------------------------------------ */
/* some gtk-like utilities */
void xxxgtk_textview_set_text (GtkTextView *text, const char *str);
char * xxxgtk_textview_get_text (GtkTextView *text);

/* Glade loader, it will look in the right directories */
GladeXML *gtt_glade_xml_new (const char *filename, const char *widget);

#endif /* __GTT_UTIL_H__ */
