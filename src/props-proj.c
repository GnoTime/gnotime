/*   Project Properties for GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
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

#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>

#include "dialog.h"
#include "gtt-select-list.h"
#include "gtt-history-list.h"
#include "proj.h"
#include "props-proj.h"
#include "util.h"

typedef struct _PropDlg
{
    GtkBuilder *gtkbuilder;
    GtkWidget *dlg;
    GtkComboBox *title;
    GtkComboBox *desc;
    GtkTextView *notes;

    GtkComboBox *regular;
    GtkComboBox *overtime;
    GtkComboBox *overover;
    GtkComboBox *flatfee;

    GtkComboBox *minimum;
    GtkComboBox *interval;
    GtkComboBox *gap;

    GtkComboBox *urgency;
    GtkComboBox *importance;
    GtkComboBox *status;

    GttDateEdit *start;
    GttDateEdit *end;
    GttDateEdit *due;

    GtkEntry *sizing;
    GtkEntry *percent;

    GttProject *proj;
} PropDlg;

static void set_modified(PropDlg *dlg, gboolean modified)
{
    gtk_dialog_set_response_sensitive(GTK_DIALOG(dlg->dlg), GTK_RESPONSE_OK, modified);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(dlg->dlg), GTK_RESPONSE_APPLY, modified);
}

static void set_changed_cb(void *gobj, PropDlg *dlg)
{
    set_modified(dlg, TRUE);
}

static void prop_set(PropDlg *dlg)
{
    long ivl;
    const gchar *cstr;
    gchar *str;
    double rate;
    time_t tval;

    if (!dlg->proj)
        return;

    gtt_project_freeze(dlg->proj);

    {
        cstr = gtt_combo_entry_get_text(dlg->title);
        if (cstr && cstr[0])
        {
            gtt_project_set_title(dlg->proj, cstr);
        }
        else
        {
            gtt_project_set_title(dlg->proj, _("empty"));
            gtt_combo_entry_set_text(dlg->title, _("empty"));
        }

        gtt_project_set_desc(dlg->proj, gtt_combo_entry_get_text(dlg->desc));
        str = xxxgtk_textview_get_text(dlg->notes);
        gtt_project_set_notes(dlg->proj, str);
        g_free(str);

        gtt_combo_history_list_save(dlg->title, NULL, -1);
        gtt_combo_history_list_save(dlg->desc, NULL, -1);
    }

    {
        rate = atof(gtt_combo_entry_get_text(dlg->regular));
        gtt_project_set_billrate(dlg->proj, rate);
        rate = atof(gtt_combo_entry_get_text(dlg->overtime));
        gtt_project_set_overtime_rate(dlg->proj, rate);
        rate = atof(gtt_combo_entry_get_text(dlg->overover));
        gtt_project_set_overover_rate(dlg->proj, rate);
        rate = atof(gtt_combo_entry_get_text(dlg->flatfee));
        gtt_project_set_flat_fee(dlg->proj, rate);

        gtt_combo_history_list_save(dlg->regular, NULL, -1);
        gtt_combo_history_list_save(dlg->overtime, NULL, -1);
        gtt_combo_history_list_save(dlg->overover, NULL, -1);
        gtt_combo_history_list_save(dlg->flatfee, NULL, -1);
    }

    {
        ivl = atoi(gtt_combo_entry_get_text(dlg->minimum));
        gtt_project_set_min_interval(dlg->proj, ivl);
        ivl = atoi(gtt_combo_entry_get_text(dlg->interval));
        gtt_project_set_auto_merge_interval(dlg->proj, ivl);
        ivl = atoi(gtt_combo_entry_get_text(dlg->gap));
        gtt_project_set_auto_merge_gap(dlg->proj, ivl);

        gtt_combo_history_list_save(dlg->minimum, NULL, -1);
        gtt_combo_history_list_save(dlg->interval, NULL, -1);
        gtt_combo_history_list_save(dlg->gap, NULL, -1);
    }

    {
        ivl = (long) gtt_combo_select_list_get_value(dlg->urgency);
        gtt_project_set_urgency(dlg->proj, (GttRank) ivl);
        ivl = (long) gtt_combo_select_list_get_value(dlg->importance);
        gtt_project_set_importance(dlg->proj, (GttRank) ivl);

        ivl = (long) gtt_combo_select_list_get_value(dlg->status);
        gtt_project_set_status(dlg->proj, (GttProjectStatus) ivl);

        tval = gtt_date_edit_get_time(dlg->start);
        gtt_project_set_estimated_start(dlg->proj, tval);
        tval = gtt_date_edit_get_time(dlg->end);
        gtt_project_set_estimated_end(dlg->proj, tval);
        tval = gtt_date_edit_get_time(dlg->due);
        gtt_project_set_due_date(dlg->proj, tval);

        rate = atof(gtk_entry_get_text(dlg->sizing));
        ivl = rate * 3600.0;
        gtt_project_set_sizing(dlg->proj, ivl);

        ivl = atoi(gtk_entry_get_text(dlg->percent));
        gtt_project_set_percent_complete(dlg->proj, ivl);
    }

    gtt_project_thaw(dlg->proj);
}

/* ============================================================== */

static void do_set_project(GttProject *proj, PropDlg *dlg)
{
    GttProjectStatus status;
    GttRank rank;
    time_t tval;
    char buff[132];
    time_t now = time(NULL);

    if (!dlg)
        return;

    gtt_combo_history_list_init(dlg->title, NULL);
    gtt_combo_history_list_init(dlg->desc, NULL);
    gtt_combo_history_list_init(dlg->regular, NULL);
    gtt_combo_history_list_init(dlg->overtime, NULL);
    gtt_combo_history_list_init(dlg->overover, NULL);
    gtt_combo_history_list_init(dlg->flatfee, NULL);
    gtt_combo_history_list_init(dlg->minimum, NULL);
    gtt_combo_history_list_init(dlg->interval, NULL);
    gtt_combo_history_list_init(dlg->gap, NULL);

    if (!proj)
    {
        /* We null these out, because old values may be left
         * over from an earlier project */
        dlg->proj = NULL;
        gtt_combo_entry_set_text(dlg->title, "");
        gtt_combo_entry_set_text(dlg->desc, "");
        xxxgtk_textview_set_text(dlg->notes, "");
        gtt_combo_entry_set_text(dlg->regular, "0.0");
        gtt_combo_entry_set_text(dlg->overtime, "0.0");
        gtt_combo_entry_set_text(dlg->overover, "0.0");
        gtt_combo_entry_set_text(dlg->flatfee, "0.0");
        gtt_combo_entry_set_text(dlg->minimum, "0");
        gtt_combo_entry_set_text(dlg->interval, "0");
        gtt_combo_entry_set_text(dlg->gap, "0");

        gtt_date_edit_set_time(dlg->start, now);
        gtt_date_edit_set_time(dlg->end, now);
        gtt_date_edit_set_time(dlg->due, now + 86400);
        gtk_entry_set_text(dlg->sizing, "0.0");
        gtk_entry_set_text(dlg->percent, "0");
        return;
    }

    /* set all the values. Do this even is new project is same as old
     * project, since widget may be holding rejected changes. */
    dlg->proj = proj;

    gtt_combo_entry_set_text(dlg->title, gtt_project_get_title(proj));
    gtt_combo_entry_set_text(dlg->desc, gtt_project_get_desc(proj));
    xxxgtk_textview_set_text(dlg->notes, gtt_project_get_notes(proj));

    /* hack alert should use local currencies for this */
    g_snprintf(buff, 132, "%.2f", gtt_project_get_billrate(proj));
    gtt_combo_entry_set_text(dlg->regular, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_overtime_rate(proj));
    gtt_combo_entry_set_text(dlg->overtime, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_overover_rate(proj));
    gtt_combo_entry_set_text(dlg->overover, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_flat_fee(proj));
    gtt_combo_entry_set_text(dlg->flatfee, buff);

    g_snprintf(buff, 132, "%d", gtt_project_get_min_interval(proj));
    gtt_combo_entry_set_text(dlg->minimum, buff);
    g_snprintf(buff, 132, "%d", gtt_project_get_auto_merge_interval(proj));
    gtt_combo_entry_set_text(dlg->interval, buff);
    g_snprintf(buff, 132, "%d", gtt_project_get_auto_merge_gap(proj));
    gtt_combo_entry_set_text(dlg->gap, buff);

    rank = gtt_project_get_urgency(proj);
    gtt_combo_select_list_set_active_by_value(dlg->urgency, rank);

    rank = gtt_project_get_importance(proj);
    gtt_combo_select_list_set_active_by_value(dlg->importance, rank);

    status = gtt_project_get_status(proj);
    gtt_combo_select_list_set_active_by_value(dlg->status, status);

    tval = gtt_project_get_estimated_start(proj);
    if (-1 == tval)
        tval = now;
    gtt_date_edit_set_time(dlg->start, tval);
    tval = gtt_project_get_estimated_end(proj);
    if (-1 == tval)
        tval = now + 3600;
    gtt_date_edit_set_time(dlg->end, tval);
    tval = gtt_project_get_due_date(proj);
    if (-1 == tval)
        tval = now + 86400;
    gtt_date_edit_set_time(dlg->due, tval);

    g_snprintf(buff, 132, "%.2f", ((double) gtt_project_get_sizing(proj)) / 3600.0);
    gtk_entry_set_text(dlg->sizing, buff);
    g_snprintf(buff, 132, "%d", gtt_project_get_percent_complete(proj));
    gtk_entry_set_text(dlg->percent, buff);

    /* set to unmodified as it reflects the current state of the project */
    set_modified(dlg, FALSE);
}

/* ============================================================== */

#define TAGGED(NAME)                                                                    \
    ({                                                                                  \
        GtkWidget *widget;                                                              \
        widget = GTK_WIDGET(gtk_builder_get_object(builder, NAME));                     \
        g_signal_connect(                                                               \
            G_OBJECT(widget), "changed", G_CALLBACK(set_changed_cb), dlg              \
        );                                                                              \
        widget;                                                                         \
    })

#define DATED(WDGT)                                                                        \
    ({                                                                                     \
        g_signal_connect(                                                                  \
            G_OBJECT(WDGT), "date_changed", G_CALLBACK(set_changed_cb), dlg              \
        );                                                                                 \
        g_signal_connect(                                                                  \
            G_OBJECT(WDGT), "time_changed", G_CALLBACK(set_changed_cb), dlg              \
        );                                                                                 \
        GTT_DATE_EDIT(WDGT);                                                               \
    })



#define TEXTED(NAME)                                                              \
    ({                                                                            \
        GtkWidget *widget;                                                        \
        GtkTextBuffer *buff;                                                      \
        widget = GTK_WIDGET(gtk_builder_get_object(builder, NAME));               \
        buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));                   \
        g_signal_connect(                                                         \
            G_OBJECT(buff), "changed", G_CALLBACK(set_changed_cb), dlg            \
        );                                                                        \
        widget;                                                                   \
    })

static GtkComboBox *init_combo(
    GtkBuilder *builder, const char *name, PropDlg *dlg
)
{
    GtkComboBox *combo_box;

    combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, name));
    gtt_combo_select_list_init(combo_box);

    g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(set_changed_cb), dlg);

    return combo_box;
}

/* ================================================================= */

static void response_cb(GtkDialog *gtk_dialog, gint response_id, PropDlg *dlg)
{
    switch (response_id)
    {    
        case GTK_RESPONSE_OK:
            prop_set(dlg);
            gtk_widget_hide(GTK_WIDGET(dlg->dlg));
            break;
        case GTK_RESPONSE_APPLY:
            prop_set(dlg);
            break;
        case GTK_RESPONSE_CLOSE:
            gtk_widget_hide(GTK_WIDGET(dlg->dlg));
            break;
        case GTK_RESPONSE_HELP:
            gtt_help_popup(GTK_WIDGET(dlg->dlg), "projects-editing");
            break;
    }
}

static PropDlg *prop_dialog_new(void)
{
    PropDlg *dlg;
    GtkBuilder *builder;

    dlg = g_new0(PropDlg, 1);

    builder = gtt_builder_new_from_file("ui/project_properties.ui");
    dlg->gtkbuilder = builder;

    GtkWidget *notebook = GTK_WIDGET(gtk_builder_get_object(builder, "Project Properties"));

    dlg->dlg = gtk_dialog_new_with_buttons("Project Properties",
                                           NULL,
                                           0,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                           GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
                                           GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                           NULL);

    GtkWidget *dlg_content = gtk_dialog_get_content_area(GTK_DIALOG(dlg->dlg));
    gtk_container_add(GTK_CONTAINER(dlg_content), notebook);

    g_signal_connect(
        G_OBJECT(dlg->dlg), "response", G_CALLBACK(response_cb), dlg
    );

    g_signal_connect(
        G_OBJECT(dlg->dlg), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL
    );

    /* ------------------------------------------------------ */
    /* grab the various entry boxes and hook them up */

    dlg->title = GTK_COMBO_BOX(TAGGED("project_title"));
    dlg->desc = GTK_COMBO_BOX(TAGGED("project_description"));
    dlg->notes = GTK_TEXT_VIEW(TEXTED("notes box"));

    dlg->regular = GTK_COMBO_BOX(TAGGED("regular_rate"));
    dlg->overtime = GTK_COMBO_BOX(TAGGED("overtime_rate"));
    dlg->overover = GTK_COMBO_BOX(TAGGED("overover_rate"));
    dlg->flatfee = GTK_COMBO_BOX(TAGGED("flat_fee"));

    dlg->minimum = GTK_COMBO_BOX(TAGGED("min_interval"));
    dlg->interval = GTK_COMBO_BOX(TAGGED("merge_interval"));
    dlg->gap = GTK_COMBO_BOX(TAGGED("merge_gap"));

    dlg->urgency = init_combo(builder, "urgency menu", dlg);
    dlg->importance = init_combo(builder, "importance menu", dlg);
    dlg->status = init_combo(builder, "status menu", dlg);

    GtkWidget *const sizing_table = GTK_WIDGET(gtk_builder_get_object(builder, "sizing table"));

    GtkWidget *const start_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->start = DATED(start_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(start_date), 7, 19);
    gtk_widget_set_name(start_date, "start date");
    gtk_widget_show(start_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), start_date, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const end_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->end = DATED(end_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(end_date), 7, 19);
    gtk_widget_set_name(end_date, "end date");
    gtk_widget_show(end_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), end_date, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const due_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->due = DATED(due_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(due_date), 7, 19);
    gtk_widget_set_name(due_date, "due date");
    gtk_widget_show(due_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), due_date, 1, 2, 5, 6, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    dlg->sizing = GTK_ENTRY(TAGGED("sizing box"));
    dlg->percent = GTK_ENTRY(TAGGED("percent box"));

    /* ------------------------------------------------------ */
    /* initialize menu values */

    gtt_combo_select_list_append(dlg->urgency, _("Not Set"), GTT_UNDEFINED);
    gtt_combo_select_list_append(dlg->urgency, _("Low"), GTT_LOW);
    gtt_combo_select_list_append(dlg->urgency, _("Medium"), GTT_MEDIUM);
    gtt_combo_select_list_append(dlg->urgency, _("High"), GTT_HIGH);

    gtt_combo_select_list_append(dlg->importance, _("Not Set"), GTT_UNDEFINED);
    gtt_combo_select_list_append(dlg->importance, _("Low"), GTT_LOW);
    gtt_combo_select_list_append(dlg->importance, _("Medium"), GTT_MEDIUM);
    gtt_combo_select_list_append(dlg->importance, _("High"), GTT_HIGH);

    gtt_combo_select_list_append(dlg->status, _("No Status"), GTT_NO_STATUS);
    gtt_combo_select_list_append(dlg->status, _("Not Started"), GTT_NOT_STARTED);
    gtt_combo_select_list_append(dlg->status, _("In Progress"), GTT_IN_PROGRESS);
    gtt_combo_select_list_append(dlg->status, _("On Hold"), GTT_ON_HOLD);
    gtt_combo_select_list_append(dlg->status, _("Cancelled"), GTT_CANCELLED);
    gtt_combo_select_list_append(dlg->status, _("Completed"), GTT_COMPLETED);

    return dlg;
}

/* ============================================================== */

static void redraw(GttProject *prj, gpointer data)
{
    PropDlg *dlg = data;
    do_set_project(prj, dlg);
}

/* ============================================================== */

static PropDlg *dlog = NULL;

void prop_dialog_show(GttProject *proj)
{
    if (!dlog)
        dlog = prop_dialog_new();

    gtt_project_remove_notifier(dlog->proj, redraw, dlog);
    do_set_project(proj, dlog);
    gtt_project_add_notifier(proj, redraw, dlog);
    gtk_widget_show(GTK_WIDGET(dlog->dlg));
}

void prop_dialog_set_project(GttProject *proj)
{
    if (!dlog)
        return;

    gtt_project_remove_notifier(dlog->proj, redraw, dlog);
    do_set_project(proj, dlog);
    gtt_project_add_notifier(proj, redraw, dlog);
}

/* ==================== END OF FILE ============================= */
