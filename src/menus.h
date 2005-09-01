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

#ifndef __GTT_MENUS_H__
#define __GTT_MENUS_H__

#include <gnome.h>

/* names of reports */
#define ACTIVITY_REPORT "activity.ghtml"
#define DAILY_REPORT    "daily.ghtml"
#define INVOICE_REPORT  "invoice.ghtml"
#define JOURNAL_REPORT  "journal.ghtml"
#define PRIMER_REPORT   "primer.ghtml"
#define QUERY_REPORT    "query.ghtml"
#define STATUS_REPORT   "status.ghtml"
#define TODO_REPORT     "todo.ghtml"

#define TAB_DELIM_EXPORT "tab-delim.ghtml"
#define TODO_EXPORT      "todo-export.ghtml"

GtkMenuShell *menus_get_popup(void);
void menus_create(GnomeApp *app);
void menus_set_states(void);

void menus_add_plugins(GnomeApp *app);

/** Return pointer to user-defined reports menu */
GnomeUIInfo * gtt_get_reports_menu (void);

/** Install the indicate user reports menu */
void gtt_set_reports_menu (GnomeApp *app, GnomeUIInfo *new_menus);

/** Prepend the indicated user-defined report entry into the 
 *   user-defined reports menu.
 */
void gtt_reports_menu_prepend_entry (GnomeApp *app, GnomeUIInfo *new_entry);
		  
#endif /* __GTT_MENUS_H__ */
