/*   Edit Interval Properties for GTimeTracker - a time tracker
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

#include "gtt-date-edit.h"
#include "gtt-select-list.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "props-invl.h"
#include "util.h"

struct EditIntervalDialog_s
{
    GttInterval *interval;
    GtkBuilder *gtkbuilder;
    GtkWidget *interval_edit;
    GtkWidget *start_widget;
    GtkWidget *stop_widget;
    GtkComboBox *fuzz_widget;
};

/* ============================================================== */
/* interval dialog edits */

static void interval_edit_apply_cb(GtkWidget *w, gpointer data)
{
    EditIntervalDialog *dlg = (EditIntervalDialog *) data;
    GttTask *task;
    GttProject *prj;
    time_t start, stop, tmp;
    int fuzz, min_invl;

    start = gtt_date_edit_get_time(GTT_DATE_EDIT(dlg->start_widget));
    stop = gtt_date_edit_get_time(GTT_DATE_EDIT(dlg->stop_widget));

    /* If user reversed start and stop, flip them back */
    if (start > stop)
    {
        tmp = start;
        start = stop;
        stop = tmp;
    }

    /* Caution: we must avoid setting very short time intervals
     * through this interface; otherwise the interval will get
     * scrubbed away on us, and we'll be holding an invalid pointer.
     * In fact, we should probably assume the pointer is invalid
     * if prj is null ...
     */

    task = gtt_interval_get_parent(dlg->interval);
    prj = gtt_task_get_parent(task);
    min_invl = gtt_project_get_min_interval(prj);
    if (min_invl >= stop - start)
        stop = start + min_invl + 1;

    gtt_interval_freeze(dlg->interval);
    gtt_interval_set_start(dlg->interval, start);
    gtt_interval_set_stop(dlg->interval, stop);

    fuzz = gtt_combo_select_list_get_value(dlg->fuzz_widget);

    gtt_interval_set_fuzz(dlg->interval, fuzz);

    /* The thaw may cause  the interval to change.  If so, redo the GUI. */
    dlg->interval = gtt_interval_thaw(dlg->interval);
    edit_interval_set_interval(dlg, dlg->interval);
}

static void interval_edit_ok_cb(GtkWidget *w, gpointer data)
{
    EditIntervalDialog *dlg = (EditIntervalDialog *) data;
    interval_edit_apply_cb(w, data);
    gtk_widget_hide(dlg->interval_edit);
    dlg->interval = NULL;
}

static void interval_edit_cancel_cb(GtkWidget *w, gpointer data)
{
    EditIntervalDialog *dlg = (EditIntervalDialog *) data;
    gtk_widget_hide(dlg->interval_edit);
    dlg->interval = NULL;
}

/* ============================================================== */
/* Set values into interval editor widgets */

void edit_interval_set_interval(EditIntervalDialog *dlg, GttInterval *ivl)
{
    GtkWidget *w;
    time_t start, stop;
    int fuzz;

    if (!dlg)
        return;
    dlg->interval = ivl;

    if (!ivl)
    {
        w = dlg->start_widget;
        gtt_date_edit_set_time(GTT_DATE_EDIT(w), 0);
        w = dlg->stop_widget;
        gtt_date_edit_set_time(GTT_DATE_EDIT(w), 0);

        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 0);
        return;
    }

    w = dlg->start_widget;
    start = gtt_interval_get_start(ivl);
    gtt_date_edit_set_time(GTT_DATE_EDIT(w), start);

    w = dlg->stop_widget;
    stop = gtt_interval_get_stop(ivl);
    gtt_date_edit_set_time(GTT_DATE_EDIT(w), stop);

    fuzz = gtt_interval_get_fuzz(dlg->interval);

    /* OK, now set the initial value */
    gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 0);
    if (90 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 300);
    if (450 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 600);
    if (750 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 900);
    if (1050 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 1200);
    if (1500 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 1800);
    if (2700 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 3600);
    if (5400 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 7200);
    if (9000 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 3 * 3600);
    if (6 * 3600 < fuzz)
        gtt_combo_select_list_set_active_by_value(dlg->fuzz_widget, 12 * 3600);
}

static void connect_signals_popup_cb(
    GtkBuilder *builder, GObject *object, const gchar *signal_name, const gchar *handler_name,
    GObject *connect_object, GConnectFlags flags, gpointer user_data
)
{
    if (g_strcmp0(handler_name, "on_ok_button_clicked") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(interval_edit_ok_cb), user_data);

    if (g_strcmp0(handler_name, "on_apply_button_clicked") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(interval_edit_apply_cb), user_data);

    if (g_strcmp0(handler_name, "on_cancel_button_clicked") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(interval_edit_cancel_cb), user_data);

    if (g_strcmp0(handler_name, "gtk_widget_hide_on_delete") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(gtk_widget_hide_on_delete), user_data);
}

/* ============================================================== */
/* interval popup actions */

EditIntervalDialog *edit_interval_dialog_new(void)
{
    EditIntervalDialog *dlg;
    GtkBuilder *builder;

    dlg = g_malloc(sizeof(EditIntervalDialog));
    dlg->interval = NULL;

    builder = gtt_builder_new_from_file("ui/interval_edit.ui");
    dlg->gtkbuilder = builder;

    dlg->interval_edit = GTK_WIDGET(gtk_builder_get_object(builder, "Interval Edit"));

    gtk_builder_connect_signals_full(builder, connect_signals_popup_cb, dlg);

    GtkWidget *const table1 = GTK_WIDGET(gtk_builder_get_object(builder, "table1"));

    GtkWidget *const start_date = gtt_date_edit_new_flags(
        0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_DISPLAY_SECONDS | GTT_DATE_EDIT_SHOW_TIME
    );
    dlg->start_widget = start_date;
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(start_date), 7, 19);
    gtk_widget_set_name(start_date, "start_date");
    gtk_widget_show(start_date);

    gtk_table_attach(GTK_TABLE(table1), start_date, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 4, 0);

    GtkWidget *const stop_date = gtt_date_edit_new_flags(
        0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_DISPLAY_SECONDS | GTT_DATE_EDIT_SHOW_TIME
    );
    dlg->stop_widget = stop_date;
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(stop_date), 7, 19);
    gtk_widget_set_name(stop_date, "stop_date");
    gtk_widget_show(stop_date);

    gtk_table_attach(GTK_TABLE(table1), stop_date, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 4, 0);

    /* ----------------------------------------------- */
    /* initialize fuzz combo box                       */
    dlg->fuzz_widget = GTK_COMBO_BOX(gtk_builder_get_object(builder, "fuzz_menu"));
    gtt_combo_select_list_init(dlg->fuzz_widget);

    gtt_combo_select_list_append(dlg->fuzz_widget, _("Exact Time"), 0);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("5 Min"), 300);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("10 Min"), 600);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("15 Min"), 900);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("20 Min"), 1200);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("30 Min"), 1800);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("1 Hour"), 3600);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("2 Hours"), 2 * 3600);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("3 Hours"), 3 * 3600);
    gtt_combo_select_list_append(dlg->fuzz_widget, _("Today"), 12 * 3600);

    return dlg;
}

/* ============================================================== */

void edit_interval_dialog_show(EditIntervalDialog *dlg)
{
    if (!dlg)
        return;
    gtk_widget_show(GTK_WIDGET(dlg->interval_edit));
}

void edit_interval_dialog_destroy(EditIntervalDialog *dlg)
{
    if (!dlg)
        return;
    gtk_widget_destroy(GTK_WIDGET(dlg->interval_edit));
    g_free(dlg);
}

void edit_interval_set_close_callback(EditIntervalDialog *dlg, GCallback f, gpointer data)
{
    g_signal_connect(dlg->interval_edit, "close", f, data);
}

/* ===================== END OF FILE ==============================  */
