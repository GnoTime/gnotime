/*   Report Menu Editor for GTimeTracker - a time tracker
 *   Copyright (C) 2001,2003 Linas Vepstas <linas@linas.org>
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

struct PluginEditorDialog_s
{
	GladeXML  *gtxml;
	GtkDialog *dialog;

	GtkTreeView *treeview;
	GtkTreeStore *treestore;
	GArray *menus;   /* array of GnomeUIInfo */

	GtkEntry  *plugin_name;    /* AKA 'Label' */
	GnomeFileEntry  *plugin_path;
	GtkEntry  *plugin_tooltip;
	GnomeApp  *app;
};

/* ============================================================ */

static void
edit_plugin_redraw_tree (struct PluginEditorDialog_s *ped)
{
	int i;

printf ("duude redraw len=%d\n", ped->menus->len);
	/* Walk the current menu list */
	for (i=0; i<ped->menus->len; i++)
	{
	}
}

/* ============================================================ */

static void 
edit_plugin_create_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;

#if 0
	FILE *fh;
	GnomeUIInfo entry[2];
	const char *title, *path, *tip;
	GttPlugin *plg;

	/* Get the dialog contents */
	title = gtk_entry_get_text (dlg->plugin_name);
	path = gnome_file_entry_get_full_path (dlg->plugin_path, TRUE);
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
		/* g_free (msg);   XXX memory leak needs fixing. */
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
		/*
		gtk_entry_set_text (dlg->plugin_name, "");
		gtk_entry_set_text (dlg->plugin_path, "");
		gtk_entry_set_text (dlg->plugin_tooltip, "");
		*/
	}
#endif
	gtk_widget_hide (GTK_WIDGET(dlg->dialog));
printf ("OK cleicked\n");
}

static void 
edit_plugin_apply_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;
printf ("apply cleicked\n");
}

static void 
edit_plugin_cancel_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;
	gtk_widget_hide (GTK_WIDGET(dlg->dialog));
printf ("cancle cleicked\n");
}

/* ============================================================ */

static void 
edit_plugin_add_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;
	GnomeUIInfo item;
	const char *title, *path, *tip;
	int index;
	GttPlugin *plg;
printf ("add clicked\n");
	
	gtk_entry_set_text (dlg->plugin_name, "New Item");
	/* gtk_entry_set_text (dlg->plugin_path, "");
	gtk_entry_set_text (dlg->plugin_tooltip, ""); */

	/* Get the dialog contents */
	title = gtk_entry_get_text (dlg->plugin_name);
	path = gnome_file_entry_get_full_path (dlg->plugin_path, TRUE);
	tip = gtk_entry_get_text (dlg->plugin_tooltip);

   index = 0; /* XXX should be current in ctree */

	/* Create a plugin */
	plg = gtt_plugin_new (title, path);
	plg->tooltip = g_strdup (tip);

	/* Add item to array */
	item.type = GNOME_APP_UI_ITEM;
	item.label = title;
	item.hint = tip;
	item.moreinfo = NULL; /* XXX should be modified invoke_report */
	item.user_data = plg;
	item.unused_data = NULL;
	item.pixmap_type = GNOME_APP_PIXMAP_STOCK;
	item.pixmap_info = GNOME_STOCK_BLANK;
	item.accelerator_key = 0;
	item.ac_mods = (GdkModifierType) 0;
	g_array_insert_val (dlg->menus, index, item);
	
	/* Redraw the tree */
	edit_plugin_redraw_tree (dlg);
}

static void 
edit_plugin_delete_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;
printf ("delete cleicked\n");
}

/* ============================================================ */

#define NCOL 3

PluginEditorDialog *
edit_plugin_dialog_new (void)
{
	PluginEditorDialog *dlg;
	GladeXML *gtxml;
	GtkWidget *e;
	int i;
	const char *col_titles[NCOL];

	dlg = g_malloc(sizeof(PluginEditorDialog));
	dlg->app = GNOME_APP (app_window);

	gtxml = gtt_glade_xml_new ("glade/plugin_editor.glade", "Plugin Editor");
	dlg->gtxml = gtxml;

	dlg->dialog = GTK_DIALOG (glade_xml_get_widget (gtxml,  "Plugin Editor"));

	/* ------------------------------------------------------ */
	/* Dialog dismissal buttons */
	
	glade_xml_signal_connect_data (gtxml, "on_ok_button_clicked",
		GTK_SIGNAL_FUNC (edit_plugin_create_cb), dlg);
	  
	glade_xml_signal_connect_data (gtxml, "on_apply_button_clicked",
		GTK_SIGNAL_FUNC (edit_plugin_apply_cb), dlg);

	glade_xml_signal_connect_data (gtxml, "on_cancel_button_clicked",
		GTK_SIGNAL_FUNC (edit_plugin_cancel_cb), dlg);

	/* ------------------------------------------------------ */
	/* Menu item add/delete buttons */
	glade_xml_signal_connect_data (gtxml, "on_add_button_clicked",
		GTK_SIGNAL_FUNC (edit_plugin_add_cb), dlg);
	  
	glade_xml_signal_connect_data (gtxml, "on_delete_button_clicked",
		GTK_SIGNAL_FUNC (edit_plugin_delete_cb), dlg);
	  
	/* ------------------------------------------------------ */
	/* Set up the Treeview Widget */
	e = glade_xml_get_widget (gtxml, "editor treeview");
	dlg->treeview = GTK_TREE_VIEW (e);

	{
		GType col_type[NCOL];
		for (i=0;i<NCOL;i++) { col_type[i] = G_TYPE_STRING; }
      dlg->treestore = gtk_tree_store_newv (NCOL, col_type);
	}
	gtk_tree_view_set_model (dlg->treeview, GTK_TREE_MODEL(dlg->treestore));

	/* Set up the columns in the treeview widget */
	col_titles[0] = "Name";
	col_titles[1] = "Path";
	col_titles[2] = "Tooltip";
	for (i=0; i<NCOL; i++)
	{
		GtkTreeViewColumn *col;
		GtkCellRenderer *renderer;

		renderer = gtk_cell_renderer_text_new ();

		col = gtk_tree_view_column_new_with_attributes (
		                 col_titles[i],
		                 renderer, "text", i, NULL);
		
		gtk_tree_view_insert_column (dlg->treeview, col, i);
				
	}

	/* XXX should copy-in from system */
	dlg->menus = g_array_new (TRUE, FALSE, sizeof (GnomeUIInfo));

	/* ------------------------------------------------------ */
	/* grab the various entry boxes and hook them up */
	e = glade_xml_get_widget (gtxml, "plugin name");
	dlg->plugin_name = GTK_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin path");
	dlg->plugin_path = GNOME_FILE_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin tooltip");
	dlg->plugin_tooltip = GTK_ENTRY(e);

	gtk_widget_hide_on_delete (GTK_WIDGET(dlg->dialog));

	return dlg;
}

/* ============================================================ */

void 
edit_plugin_dialog_show(PluginEditorDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_show(GTK_WIDGET(dlg->dialog));
}

void 
edit_plugin_dialog_destroy(PluginEditorDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_destroy (GTK_WIDGET(dlg->dialog));
	g_free (dlg);
}

/* ============================================================ */

static PluginEditorDialog *epdlg = NULL;

void
report_menu_edit(GtkWidget *widget, gpointer data)
{
	if (!epdlg) epdlg = edit_plugin_dialog_new ();
	gtk_widget_show(GTK_WIDGET(epdlg->dialog));
}

/* ====================== END OF FILE ========================= */
