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
#include "dialog.h"
#include "journal.h"
#include "menus.h"
#include "plug-in.h"
#include "util.h"

struct PluginEditorDialog_s
{
	GladeXML  *gtxml;
	GtkDialog *dialog;

	GtkTreeView *treeview;
	GtkTreeStore *treestore;
	GArray *menus;   /* array of GnomeUIInfo */

	GtkTreeSelection *selection;
	gboolean have_selection;
	GtkTreeIter curr_selection;
	gboolean do_redraw;
	
	GtkEntry  *plugin_name;    /* AKA 'Label' */
	GnomeFileEntry  *plugin_path;
	GtkEntry  *plugin_tooltip;

	GnomeApp  *app;
};

#define NCOLUMNS   4
#define PTRCOL    (NCOLUMNS-1)

/* ============================================================ */
/* Redraw one row of the tree widget */

static void
edit_plugin_redraw_row (struct PluginEditorDialog_s *ped, 
					 GtkTreeIter *iter, GnomeUIInfo *uientry)
{
	GttPlugin *plg;
	GValue val = {G_TYPE_INVALID};
	GValue pval = {G_TYPE_INVALID};

	if (!uientry || !uientry->user_data) return;
	plg = uientry->user_data;

	g_value_init(&val, G_TYPE_STRING);
	g_value_set_string (&val, plg->name);
	gtk_tree_store_set_value (ped->treestore, iter, 0, &val);
	
	g_value_set_string (&val, plg->path);
	gtk_tree_store_set_value (ped->treestore, iter, 1, &val);
	
	g_value_set_string (&val, plg->tooltip);
	gtk_tree_store_set_value (ped->treestore, iter, 2, &val);
	
	g_value_init(&pval, G_TYPE_POINTER);
	g_value_set_pointer (&pval, uientry);
	gtk_tree_store_set_value (ped->treestore, iter, PTRCOL, &pval);
}

static void
edit_plugin_redraw_tree (struct PluginEditorDialog_s *ped)
{
	int i,rc;
	GtkTreeIter iter;

printf ("duude redraw len=%d\n", ped->menus->len);

	/* Walk the current menu list */
	rc = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(ped->treestore), &iter);
	for (i=0; i<ped->menus->len; i++)
	{
		GnomeUIInfo *uientry = (GnomeUIInfo *)ped->menus->data;
   	if (0 == rc)
		{
			gtk_tree_store_append (ped->treestore, &iter, NULL);
		}
		edit_plugin_redraw_row (ped, &iter, &uientry[i]);
		rc = gtk_tree_model_iter_next (GTK_TREE_MODEL(ped->treestore), &iter);
	}
}

/* ============================================================ */

static void 
edit_plugin_widgets_to_item (PluginEditorDialog *dlg, GnomeUIInfo *gui)
{
	const char *title, *path, *tip;
	GttPlugin *plg;

	if (!gui) return;

	/* Get the dialog contents */
	title = gtk_entry_get_text (dlg->plugin_name);
	path = gnome_file_entry_get_full_path (dlg->plugin_path, TRUE);

	if (!path)
	{
		msgbox_ok(_("Warning"),
	   	          _("You must specify a complete filepath to the report, "
	   	            "including a leading slash.  The file that you specify "
	   	            "must exist."),
	      	       GTK_STOCK_OK,
	         	    NULL);
	}

	tip = gtk_entry_get_text (dlg->plugin_tooltip);
	if (!path) path="";
	if (!tip) path="";

	/* set the values into the item */
	gui->type = GNOME_APP_UI_ITEM;
	gui->label = title;
	gui->hint = tip;
	gui->moreinfo = invoke_report;
	gui->unused_data = NULL;
	gui->pixmap_type = GNOME_APP_PIXMAP_STOCK;
	gui->pixmap_info = GNOME_STOCK_BLANK;
	gui->accelerator_key = 0;
	gui->ac_mods = (GdkModifierType) 0;
	
	plg = gui->user_data;
	if (plg->name) g_free (plg->name);
	plg->name = g_strdup (title);
	if (plg->path) g_free (plg->path);
	plg->path = g_strdup (path);
	if (plg->tooltip) g_free (plg->tooltip);
	plg->tooltip = g_strdup (tip);
}

static void 
edit_plugin_item_to_widgets (PluginEditorDialog *dlg, GnomeUIInfo *gui)
{
	GttPlugin *plg;

	if (!gui) return;
	plg = gui->user_data;
	
	gtk_entry_set_text (dlg->plugin_name, plg->name);
	gnome_file_entry_set_filename (dlg->plugin_path, plg->path);
	gtk_entry_set_text (dlg->plugin_tooltip, plg->tooltip);
}

/* ============================================================ */

static void
edit_plugin_tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
	PluginEditorDialog *dlg = data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean have_selection;

	have_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
	
	dlg->do_redraw = FALSE;
	if (dlg->have_selection)
	{
		GnomeUIInfo *curr_item;
		GValue val = {G_TYPE_INVALID};
		gtk_tree_model_get_value (model, &dlg->curr_selection, PTRCOL, &val);
		curr_item = g_value_get_pointer(&val);

		/* Save current values of widgets to current item */
		edit_plugin_widgets_to_item (dlg, curr_item);
	}

	if (have_selection)
	{
		GnomeUIInfo *curr_item;
		GValue val = {G_TYPE_INVALID};
		gtk_tree_model_get_value (model, &iter, PTRCOL, &val);
		curr_item = g_value_get_pointer(&val);

		dlg->have_selection = TRUE;
		dlg->curr_selection = iter;
		edit_plugin_item_to_widgets (dlg, curr_item);
	}
	dlg->do_redraw = TRUE;
}

/* ============================================================ */

static void 
edit_plugin_create_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;

#if 0
	FILE *fh;
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
	int index;
	GttPlugin *plg;
printf ("add clicked dlg=%x\n",dlg);

   index = 0; /* XXX should be current in ctree */

	/* Create a plugin */
	plg = gtt_plugin_new ("x", "/x");
	if (!plg) return;

	item.user_data = plg;
	edit_plugin_widgets_to_item (dlg, &item);
	g_array_insert_val (dlg->menus, index, item);
	
	/* Redraw the tree */
	edit_plugin_redraw_tree (dlg);
	/* XXX set the new thing to selected row */
}

static void 
edit_plugin_delete_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;
printf ("delete clicked\n");
}

static void 
edit_plugin_changed_cb (GtkWidget * w, gpointer data)
{
	PluginEditorDialog *dlg = data;

	if (!dlg->do_redraw) return;
printf ("duude changed\n");
	if (dlg->have_selection)
	{
		GnomeUIInfo *curr_item;
		GValue val = {G_TYPE_INVALID};
		gtk_tree_model_get_value (GTK_TREE_MODEL(dlg->treestore), 
		                &dlg->curr_selection, PTRCOL, &val);
		curr_item = g_value_get_pointer(&val);

		/* Save current values of widgets to current item */
		edit_plugin_widgets_to_item (dlg, curr_item);
		edit_plugin_redraw_row (dlg, &dlg->curr_selection, curr_item);
	}
}

/* ============================================================ */


PluginEditorDialog *
edit_plugin_dialog_new (void)
{
	PluginEditorDialog *dlg;
	GladeXML *gtxml;
	GtkWidget *e;
	int i;
	const char *col_titles[NCOLUMNS];

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
	/* Inpout widget changed events */
	glade_xml_signal_connect_data (gtxml, "on_plugin_name_changed",
		GTK_SIGNAL_FUNC (edit_plugin_changed_cb), dlg);
	  
	glade_xml_signal_connect_data (gtxml, "on_plugin_path_changed",
		GTK_SIGNAL_FUNC (edit_plugin_changed_cb), dlg);
	  
	glade_xml_signal_connect_data (gtxml, "on_plugin_tooltip_changed",
		GTK_SIGNAL_FUNC (edit_plugin_changed_cb), dlg);
	  
	/* ------------------------------------------------------ */
	/* Set up the Treeview Widget */
	e = glade_xml_get_widget (gtxml, "editor treeview");
	dlg->treeview = GTK_TREE_VIEW (e);

	{
		GType col_type[NCOLUMNS];
		for (i=0;i<NCOLUMNS-1;i++) { col_type[i] = G_TYPE_STRING; }
		col_type[NCOLUMNS-1] = G_TYPE_POINTER;
      dlg->treestore = gtk_tree_store_newv (NCOLUMNS, col_type);
	}
	gtk_tree_view_set_model (dlg->treeview, GTK_TREE_MODEL(dlg->treestore));

	/* Set up the columns in the treeview widget */
	col_titles[0] = "Name";
	col_titles[1] = "Path";
	col_titles[2] = "Tooltip";
	for (i=0; i<NCOLUMNS-1; i++)
	{
		GtkTreeViewColumn *col;
		GtkCellRenderer *renderer;

		renderer = gtk_cell_renderer_text_new ();

		col = gtk_tree_view_column_new_with_attributes (
		                 col_titles[i],
		                 renderer, "text", i, NULL);
		
		gtk_tree_view_insert_column (dlg->treeview, col, i);
	}

	/* Copy-in existing menus from the system */
	{
		int nitems;
		GnomeUIInfo *sysmenus;
		
		sysmenus = gtt_get_reports_menu ();
		for (i=0; GNOME_APP_UI_ENDOFINFO != sysmenus[i].type; i++) {}
		nitems = i;

		dlg->menus = g_array_new (TRUE, FALSE, sizeof (GnomeUIInfo));
		dlg->menus = g_array_append_vals (dlg->menus, sysmenus, nitems);
	}

	/* Redraw the tree */
	edit_plugin_redraw_tree (dlg);

	/* Hook up the row-selection callback */
	dlg->have_selection = FALSE;
	dlg->selection = gtk_tree_view_get_selection (dlg->treeview);
	gtk_tree_selection_set_mode (dlg->selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (dlg->selection), "changed",
	            G_CALLBACK (edit_plugin_tree_selection_changed_cb), dlg);

	/* ------------------------------------------------------ */
	/* Grab the various entry boxes and hook them up */
	e = glade_xml_get_widget (gtxml, "plugin name");
	dlg->plugin_name = GTK_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin path");
	dlg->plugin_path = GNOME_FILE_ENTRY(e);

	e = glade_xml_get_widget (gtxml, "plugin tooltip");
	dlg->plugin_tooltip = GTK_ENTRY(e);

	gtk_entry_set_text (dlg->plugin_name, _("New Item"));
	gnome_file_entry_set_filename (dlg->plugin_path, "");
	gtk_entry_set_text (dlg->plugin_tooltip, "");

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
