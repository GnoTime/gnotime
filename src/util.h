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

#define print_minutes_elapsed qof_print_minutes_elapsed_buff
#define print_hours_elapsed qof_print_hours_elapsed_buff
#define print_date_time qof_print_date_time_buff
#define print_time qof_print_time_buff
#define print_date qof_print_date_buff
#define is_same_day qof_is_same_day

/* ------------------------------------------------------------------ */
/* some gtk-like utilities */
void xxxgtk_textview_set_text (GtkTextView *text, const char *str);
char * xxxgtk_textview_get_text (GtkTextView *text);

/* Glade loader, it will look in the right directories */
GladeXML *gtt_glade_xml_new (const char *filename, const char *widget);

#endif /* __GTT_UTIL_H__ */
