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

#include "config.h"
#include <errno.h>
#include <glade/glade.h>
#include <glib.h>
#include <gnome.h>

#include "app.h"
#include "journal.h"
#include "plug-in.h"
#include "util.h"

struct NewPluginDialog_s
{
	GladeXML  *gtxml;
	GtkDialog *dialog;
	GtkEntry  *plugin_name;
	GtkEntry  *plugin_path;
	GtkEntry  *plugin_tooltip;
	GnomeApp  *app;
};

/* ============================================================ */

static GList *plugin_list = NULL;

GList * 
gtt_plugin_get_list (void)
{
	return plugin_list;
}

GttPlugin *
gtt_plugin_new (const char * nam, const char * pth)
{
	GttPlugin *plg;

	if (!nam || !pth) return NULL;

	plg = g_new (GttPlugin, 1);
	plg->name = g_strdup(nam);
	plg->path = g_strdup(pth);
	plg->tooltip = NULL;

	plugin_list = g_list_append (plugin_list, plg);

	return plg;
}

/* ============================================================ */

static void 
new_plugin_create_cb (GtkWidget * w, gpointer data)
{
	FILE *fh;
	GnomeUIInfo entry[2];
	const char *title, *path, *tip;
	NewPluginDialog *dlg = data;
	GttPlugin *plg;

	/* Get the dialog contents */
	title = gtk_entry_get_text (dlg->plugin_name);
	path = gtk_entry_get_text (dlg->plugin_path);
	tip = gtk_entry_get_text (dlg->plugin_tooltip);

	/* do a basic sanity check */
	fh = fopen (path, "r");
	if (!fh) 
	{
		gchar *msg;
		GtkWidget *mb;
		int nerr = errno;
		msg = g_strdup_printf (_("Unable to open the file %s\n%s"),
			path, strerror (nerr)); 
		mb = gnome_message_box_new (msg,
			GNOME_MESSAGE_BOX_ERROR, 
			GTK_STOCK_CLOSE,
			NULL);
		gtk_widget_show (mb);
		// g_free (msg);
	}
	else
	{
		fclose (fh);

		/* create the plugin */
		plg = gtt_plugin_new (title,path);
		plg->tooltip = g_strdup (tip);
	
		/* add the thing to the Reports menu */
		entry[0].type = GNOME_APP_UI_ITEM;
		entry[0].label = plg->name;
		entry[0].hint = plg->tooltip;
		entry[0].moreinfo = invoke_report;
		entry[0].user_data = plg->path;
		entry[0].unused_data = NULL;
		entry[0].pixmap_type = GNOME_APP_PIXMAP_STOCK;
		entry[0].pixmap_info = GNOME_STOCK_MENU_BLANK;
		entry[0].accelerator_key = 0;
		entry[0].ac_mods = (GdkModifierType) 0;
	
		entry[1].type = GNOME_APP_UI_ENDOFINFO;
	
		gnome_app_insert_menus (dlg->app,  N_("Reports/<Separator>"), entry);
	
		/* zero-out entries, so next time user doesn't see them again */
		gtk_entry_set_text (dlg->plugin_name, "");
		gtk_entry_set_text (dlg->plugin_path, "");
		gtk_entry_set_text (dlg->plugin_tooltip, "");
	}
	gtk_widget_hide (GTK_WIDGET(dlg->dialog));
}

static void 
new_plugin_cancel_cb (GtkWidget * w, gpointer data)
{
	NewPluginDialog *dlg = data;
	gtk_widget_hide (GTK_WIDGET(dlg->dialog));
}

/* ============================================================ */


NewPluginDialog *
new_plugin_dialog_new (void)
{
	NewPluginDialog *dlg;
	GladeXML *gtxml;
	GtkWidget *e;

	dlg = g_malloc(sizeof(NewPluginDialog));
	dlg->app = GNOME_APP (app_window);

	gtxml = gtt_glade_xml_new ("glade/plugin.glade", "Plugin New");
	dlg->gtxml = gtxml;

	dlg->dialog = GTK_DIALOG (glade_xml_get_widget (gtxml,  "Plugin New"));

	glade_xml_signal_connect_data (gtxml, "on_ok_button_clicked",
		GTK_SIGNAL_FUNC (new_plugin_create_cb), dlg);
	  
	glade_xml_signal_connect_data (gtxml, "on_cancel_button_clicked",
		GTK_SIGNAL_FUNC (new_plugin_cancel_cb), dlg);

	/* ------------------------------------------------------ */
	/* grab the various entry boxes and hook them up */
	e = glade_xml_get_widget (gtxml, "plugin name");
	dlg->plugin_name = GTK_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin path");
	dlg->plugin_path = GTK_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin tooltip");
	dlg->plugin_tooltip = GTK_ENTRY(e);

	gtk_widget_hide_on_delete (GTK_WIDGET(dlg->dialog));

	return dlg;
}

/* ============================================================ */

void 
new_plugin_dialog_show(NewPluginDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_show(GTK_WIDGET(dlg->dialog));
}

void 
new_plugin_dialog_destroy(NewPluginDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_destroy (GTK_WIDGET(dlg->dialog));
	g_free (dlg);
}

/* ============================================================ */

static NewPluginDialog *pdlg = NULL;

void
new_report(GtkWidget *widget, gpointer data)
{
	if (!pdlg) pdlg = new_plugin_dialog_new ();
	gtk_widget_show(GTK_WIDGET(pdlg->dialog));
}

/* ====================== END OF FILE ========================= */
