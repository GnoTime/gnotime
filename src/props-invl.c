/*   Edit Interval Properties for GTimeTracker - a time tracker
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

#include "config.h"

#include <glade/glade.h>
#include <glib.h>
#include <gnome.h>
#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "props-invl.h"
#include "util.h"

struct EditIntervalDialog_s
{
	GttInterval *interval;
	GladeXML *gtxml;
	GtkWidget *interval_edit;
	GtkWidget *start_widget;
	GtkWidget *stop_widget;
	GtkWidget *fuzz_widget;
};

/* ============================================================== */
/* interval dialog edits */

static void
interval_edit_apply_cb(GtkWidget * w, gpointer data) 
{
	EditIntervalDialog *dlg = (EditIntervalDialog *) data;
	GtkWidget *menu, *menu_item;
	GttTask * task;
	GttProject * prj;
	time_t start, stop;
	int fuzz, min_invl;

	start = gnome_date_edit_get_date(GNOME_DATE_EDIT(dlg->start_widget));
	stop = gnome_date_edit_get_date(GNOME_DATE_EDIT(dlg->stop_widget));

	/* Caution: we must avoid setting very short time intervals
	 * through this interface; otherwise the interval will get
	 * scrubbed away on us, and we'll be holding an invalid pointer.
	 * In fact, we should probably assume the pointer is invalid
	 * if prj is null ...
	 */

	task = gtt_interval_get_parent (dlg->interval);
	prj = gtt_task_get_parent (task);
	min_invl = gtt_project_get_min_interval (prj);
	if (min_invl >= stop-start) stop = start + min_invl+1;

	gtt_interval_freeze (dlg->interval);
	gtt_interval_set_start (dlg->interval, start);
	gtt_interval_set_stop (dlg->interval, stop);

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU(dlg->fuzz_widget));
	menu_item = gtk_menu_get_active(GTK_MENU(menu));
	fuzz = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(menu_item),
                                             "fuzz_factor"));

	gtt_interval_set_fuzz (dlg->interval, fuzz);
	gtt_interval_thaw (dlg->interval);
}

static void
interval_edit_ok_cb(GtkWidget * w, gpointer data) 
{
	EditIntervalDialog *dlg = (EditIntervalDialog *) data;
	interval_edit_apply_cb(w, data);
	gtk_widget_hide (dlg->interval_edit);
	dlg->interval = NULL;
}

static void
interval_edit_cancel_cb(GtkWidget * w, gpointer data) 
{
	EditIntervalDialog *dlg = (EditIntervalDialog *) data;
	gtk_widget_hide (dlg->interval_edit);
	dlg->interval = NULL;
}

/* ============================================================== */
/* set values into interval editor widgets */

void 
edit_interval_set_interval (EditIntervalDialog *dlg, GttInterval *ivl)
{
	GtkWidget *w;
	GtkOptionMenu *fw;
	time_t start, stop;
	int fuzz;

	if (!dlg) return;
	dlg->interval = ivl;

	if (!ivl) 
	{
		w = dlg->start_widget;
		gnome_date_edit_set_time (GNOME_DATE_EDIT(w), 0);
		w = dlg->stop_widget;
		gnome_date_edit_set_time (GNOME_DATE_EDIT(w), 0);

		fw = GTK_OPTION_MENU(dlg->fuzz_widget);
		gtk_option_menu_set_history(fw, 0);
		return;
	}

	w = dlg->start_widget;
	start = gtt_interval_get_start (ivl);
	gnome_date_edit_set_time (GNOME_DATE_EDIT(w), start);

	w = dlg->stop_widget;
	stop = gtt_interval_get_stop (ivl);
	gnome_date_edit_set_time (GNOME_DATE_EDIT(w), stop);

	fuzz = gtt_interval_get_fuzz (dlg->interval);
	fw = GTK_OPTION_MENU(dlg->fuzz_widget);

	/* OK, now set the initial value */
	gtk_option_menu_set_history(fw, 0);
	if (90 < fuzz) gtk_option_menu_set_history(fw, 1);
	if (450 < fuzz) gtk_option_menu_set_history(fw, 2);
	if (750 < fuzz) gtk_option_menu_set_history(fw, 3);
	if (1050 < fuzz) gtk_option_menu_set_history(fw, 4);
	if (1500 < fuzz) gtk_option_menu_set_history(fw, 5);
	if (2700 < fuzz) gtk_option_menu_set_history(fw, 6);
	if (5400 < fuzz) gtk_option_menu_set_history(fw, 7);
	if (9000 < fuzz) gtk_option_menu_set_history(fw, 8);
	if (6*3600 < fuzz) gtk_option_menu_set_history(fw, 9);

}

/* ============================================================== */
/* interval popup actions */

EditIntervalDialog *
edit_interval_dialog_new (void)
{
	EditIntervalDialog *dlg;
	GladeXML  *glxml;
	GtkWidget *w, *menu, *menu_item;

	dlg = g_malloc (sizeof(EditIntervalDialog));
	dlg->interval = NULL;

	glxml = gtt_glade_xml_new ("glade/interval_edit.glade", "Interval Edit");
	dlg->gtxml = glxml;

	dlg->interval_edit = glade_xml_get_widget (glxml, "Interval Edit");

	glade_xml_signal_connect_data (glxml, "on_ok_button_clicked",
	        GTK_SIGNAL_FUNC (interval_edit_ok_cb), dlg);
	  
	glade_xml_signal_connect_data (glxml, "on_apply_button_clicked",
	        GTK_SIGNAL_FUNC (interval_edit_apply_cb), dlg);

	glade_xml_signal_connect_data (glxml, "on_cancel_button_clicked",
	        GTK_SIGNAL_FUNC (interval_edit_cancel_cb), dlg);
	  
	dlg->start_widget = glade_xml_get_widget (glxml, "start_date");
	dlg->stop_widget = glade_xml_get_widget (glxml, "stop_date");
	dlg->fuzz_widget = glade_xml_get_widget (glxml, "fuzz_menu");

	/* ----------------------------------------------- */
	/* install option data by hand ... ugh 
	 * wish glade did this for us .. */
	w = dlg->fuzz_widget;
	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU(w));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 0);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(0));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 1);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(300));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 2);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(600));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 3);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(900));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 4);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(1200));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 5);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(1800));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 6);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(3600));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 7);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(7200));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 8);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(3*3600));

	gtk_option_menu_set_history(GTK_OPTION_MENU(w), 9);
	menu_item =  gtk_menu_get_active(GTK_MENU(menu));
	gtk_object_set_data(GTK_OBJECT(menu_item), 
		"fuzz_factor", GINT_TO_POINTER(12*3600));

	gnome_dialog_close_hides(GNOME_DIALOG(dlg->interval_edit), TRUE);
/*
	gnome_dialog_set_parent(GNOME_DIALOG(dlg->dlg), GTK_WINDOW(window));

*/
	return dlg;
}

/* ============================================================== */

void 
edit_interval_dialog_show(EditIntervalDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_show(GTK_WIDGET(dlg->interval_edit));
}

void 
edit_interval_dialog_destroy(EditIntervalDialog *dlg)
{
	if (!dlg) return;
	gtk_widget_destroy (GTK_WIDGET(dlg->interval_edit));
	g_free (dlg);
}

/* ===================== END OF FILE ==============================  */
