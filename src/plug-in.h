/*   Report Plugins for GTimeTracker - a time tracker
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GTT_PLUG_IN_H__
#define __GTT_PLUG_IN_H__

#include <glib.h>
#include <gtk/gtk.h>

typedef struct GttPlugin_s
{
	char * name;
	char * tooltip;
	char * path;

} GttPlugin;

/* return the list of plugins */
GList * gtt_plugin_get_list (void);

GttPlugin * gtt_plugin_new (const char * name, const char * path);


/*-------------------------------------------- */
/* A really simple, stupid GUI that allows user to enter in the path
 * name to a phtml file, and then paste it into a menu.  The idea
 * here is to make it as easy as possible for new users to create
 * modified/custom reports.
 */

typedef struct NewPluginDialog_s NewPluginDialog;

NewPluginDialog * new_plugin_dialog_new (void);
void new_plugin_dialog_show(NewPluginDialog *dlg);
void new_plugin_dialog_destroy(NewPluginDialog *dlg);

void new_report(GtkWidget *widget, gpointer data);


#endif /* __GTT_PLUG_IN_H__ */
