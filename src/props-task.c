/*   Task Properties for GTimeTracker - a time tracker
 *   Copyright (C) 2001,2002,2003,2004 Linas Vepstas <linas@linas.org>
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

#include <string.h>

#include "dialog.h"
#include "gtt-select-list.h"
#include "gtt-history-list.h"
#include "proj.h"
#include "props-task.h"
#include "util.h"

#include <glib/gi18n.h>

#include <stdlib.h>

#define MEMO_HISTORY_ID "task_memo"
#define UNIT_HISTORY_ID "bill_unit"

typedef struct PropTaskDlg_s
{
    GladeXML *gtxml;
    GtkDialog *dlg;
    GtkComboBox *memo;
    GtkTextView *notes;
    GtkComboBox *billstatus;
    GtkComboBox *billable;
    GtkComboBox *billrate;
    GtkComboBox *unit;

    GttTask *task;

    /* The goal of 'ignore events' is to prevent an infinite
     * loop of cascading events as we modify the project and the GUI.
     */
    gboolean ignore_events;

    /* The goal of the freezes is to prevent more than one update
     * of windows per second.  The problem is that without this,
     * there would be one event per keystroke, which could cause
     * a redraw of e.g. the journal window.  In such a case, even
     * moderate typists on a slow CPU could saturate the CPU entirely.
     */
    gboolean task_freeze;

} PropTaskDlg;

/* ============================================================== */

#define TSK_SETUP(dlg)         \
    if (NULL == dlg->task)     \
        return;                \
    if (dlg->ignore_events)    \
        return;                \
                               \
    dlg->ignore_events = TRUE; \
    dlg->task_freeze = TRUE;   \
    gtt_task_freeze(dlg->task);


static void save_task_notes(GtkWidget *w, PropTaskDlg *dlg)
{
    const gchar *cstr;
    gchar *str;

    TSK_SETUP(dlg);

    cstr = gtt_combo_entry_get_text(dlg->memo);
    if (cstr && cstr[0])
    {
        gtt_task_set_memo(dlg->task, cstr);
    }
    else
    {
        gtt_task_set_memo(dlg->task, "");
        gtt_combo_entry_set_text(dlg->memo, "");
    }

    gtt_combo_history_list_save(dlg->memo, MEMO_HISTORY_ID, -1);

    str = xxxgtk_textview_get_text(dlg->notes);
    gtt_task_set_notes(dlg->task, str);
    g_free(str);

    dlg->ignore_events = FALSE;
}

static void save_task_billinfo(GtkWidget *w, PropTaskDlg *dlg)
{
    GttBillStatus status;
    GttBillable able;
    GttBillRate rate;
    int ivl;

    TSK_SETUP(dlg);

    ivl = (int) (60.0 * atof(gtt_combo_entry_get_text(dlg->unit)));
    gtt_task_set_bill_unit(dlg->task, ivl);

    gtt_combo_history_list_save(dlg->unit, UNIT_HISTORY_ID, -1);

    status = (GttBillStatus) gtt_combo_select_list_get_value(dlg->billstatus);
    gtt_task_set_billstatus(dlg->task, status);

    able = (GttBillable) gtt_combo_select_list_get_value(dlg->billable);
    gtt_task_set_billable(dlg->task, able);

    rate = (GttBillRate) gtt_combo_select_list_get_value(dlg->billrate);
    gtt_task_set_billrate(dlg->task, rate);

    dlg->ignore_events = FALSE;
}

/* ============================================================== */
/* Copy values from gnotime object to widget */

static void do_set_task(GttTask *tsk, PropTaskDlg *dlg)
{
    GttBillStatus status;
    GttBillable able;
    GttBillRate rate;
    char buff[132];

    gtt_combo_history_list_init(dlg->memo, MEMO_HISTORY_ID);
    gtt_combo_history_list_init(dlg->unit, UNIT_HISTORY_ID);

    if (!tsk)
    {
        dlg->task = NULL;
        gtt_combo_entry_set_text(dlg->memo, "");
        xxxgtk_textview_set_text(dlg->notes, "");
        gtt_combo_entry_set_text(dlg->unit, "0.0");
        return;
    }

    /* Set the task, even if its same as the old task.  Do this because
     * the widget may contain rejected edit values.  */
    dlg->task = tsk;
    TSK_SETUP(dlg);

    gtt_combo_entry_set_text(dlg->memo, gtt_task_get_memo(tsk));
    xxxgtk_textview_set_text(dlg->notes, gtt_task_get_notes(tsk));

    g_snprintf(buff, 132, "%g", ((double) gtt_task_get_bill_unit(tsk)) / 60.0);
    gtt_combo_entry_set_text(dlg->unit, buff);

    status = gtt_task_get_billstatus(tsk);
    gtt_combo_select_list_set_active_by_value(dlg->billstatus, status);

    able = gtt_task_get_billable(tsk);
    gtt_combo_select_list_set_active_by_value(dlg->billable, able);

    rate = gtt_task_get_billrate(tsk);
    gtt_combo_select_list_set_active_by_value(dlg->billrate, rate);

    dlg->ignore_events = FALSE;
}

/* ============================================================== */

static void redraw(GttProject *prj, gpointer data)
{
    PropTaskDlg *dlg = data;
    do_set_task(dlg->task, dlg);
}

static void close_cb(GtkWidget *w, PropTaskDlg *dlg)
{
    GttProject *prj;
    prj = gtt_task_get_parent(dlg->task);
    gtt_project_remove_notifier(prj, redraw, dlg);

    dlg->ignore_events = FALSE;
    gtt_task_thaw(dlg->task);
    dlg->task_freeze = FALSE;
    save_task_notes(w, dlg);
    save_task_billinfo(w, dlg);
    dlg->task = NULL;
    gtk_widget_hide(GTK_WIDGET(dlg->dlg));
}

static PropTaskDlg *global_dlog = NULL;

static void destroy_cb(GtkWidget *w, PropTaskDlg *dlg)
{
    close_cb(w, dlg);

    // FIXME: Poor design that we are asked to destroy parameter dlg and also clear some global...
    global_dlog = NULL;

    // FIXME: Are we supposed to destroy some widgets and list models here?
    g_free(dlg);
}

/* ============================================================== */

#define TEXTED(NAME)                                                                   \
    ({                                                                                 \
        GtkWidget *widget;                                                             \
        GtkTextBuffer *buff;                                                           \
        widget = glade_xml_get_widget(gtxml, NAME);                                    \
        buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));                        \
        g_signal_connect(G_OBJECT(buff), "changed", G_CALLBACK(save_task_notes), dlg); \
        widget;                                                                        \
    })

static GtkComboBox* init_combo(GladeXML *gtxml, const char *name,
    GCallback changed_callback, void *callback_data)
{
    GtkComboBox *combo_box;

    combo_box = GTK_COMBO_BOX(glade_xml_get_widget(gtxml, name));
    gtt_combo_select_list_init(combo_box);

    if (changed_callback != NULL)
    {
        g_signal_connect(G_OBJECT(combo_box), "changed", changed_callback, callback_data);
    }

    return combo_box;
}

static PropTaskDlg *prop_task_dialog_new(void)
{
    PropTaskDlg *dlg = NULL;
    GladeXML *gtxml;

    dlg = g_new0(PropTaskDlg, 1);

    gtxml = gtt_glade_xml_new("glade/task_properties.glade", "Task Properties");
    dlg->gtxml = gtxml;

    dlg->dlg = GTK_DIALOG(glade_xml_get_widget(gtxml, "Task Properties"));

    glade_xml_signal_connect_data(
        gtxml, "on_help_button_clicked", GTK_SIGNAL_FUNC(gtt_help_popup), "properties"
    );

    glade_xml_signal_connect_data(
        gtxml, "on_ok_button_clicked", GTK_SIGNAL_FUNC(close_cb), dlg
    );

    g_signal_connect(G_OBJECT(dlg->dlg), "close", G_CALLBACK(close_cb), dlg);
    g_signal_connect(G_OBJECT(dlg->dlg), "destroy", G_CALLBACK(destroy_cb), dlg);

    /* ------------------------------------------------------ */
    /* grab the various entry boxes and hook them up */

    dlg->memo = GTK_COMBO_BOX(glade_xml_get_widget(gtxml, "task_memo"));
    g_signal_connect(G_OBJECT(dlg->memo), "changed", G_CALLBACK(save_task_notes), dlg);

    dlg->notes = GTK_TEXT_VIEW(TEXTED("notes box"));

    dlg->billstatus =
        init_combo(gtxml, "billstatus menu", G_CALLBACK(save_task_billinfo), dlg);

    dlg->billable =
        init_combo(gtxml, "billable menu", G_CALLBACK(save_task_billinfo), dlg);

    dlg->billrate =
        init_combo(gtxml, "billrate menu", G_CALLBACK(save_task_billinfo), dlg);

    dlg->unit = GTK_COMBO_BOX(glade_xml_get_widget(gtxml, "bill_unit"));
    g_signal_connect(G_OBJECT(dlg->unit), "changed", G_CALLBACK(save_task_billinfo), dlg);

    /* ------------------------------------------------------ */
    /* associate values with the three option menus */

    gtt_combo_select_list_append(dlg->billstatus, _("Hold"), GTT_HOLD);
    gtt_combo_select_list_append(dlg->billstatus, _("Bill"), GTT_BILL);
    gtt_combo_select_list_append(dlg->billstatus, _("Paid"), GTT_PAID);

    gtt_combo_select_list_append(dlg->billable, _("Billable"), GTT_BILLABLE);
    gtt_combo_select_list_append(dlg->billable, _("Not Billable"), GTT_NOT_BILLABLE);
    gtt_combo_select_list_append(dlg->billable, _("No Charge"), GTT_NO_CHARGE);

    gtt_combo_select_list_append(dlg->billrate, _("Regular"), GTT_REGULAR);
    gtt_combo_select_list_append(dlg->billrate, _("Overtime"), GTT_OVERTIME);
    gtt_combo_select_list_append(dlg->billrate, _("OverOver"), GTT_OVEROVER);
    gtt_combo_select_list_append(dlg->billrate, _("Flat Fee"), GTT_FLAT_FEE);

    dlg->ignore_events = FALSE;
    dlg->task_freeze = FALSE;
    gtk_widget_hide_on_delete(GTK_WIDGET(dlg->dlg));

    return dlg;
}

/* ============================================================== */

void gtt_diary_timer_callback(gpointer nuts)
{
    /* If there was a more elegant timer add func,
     * we wouldn't need this global */
    PropTaskDlg *dlg = global_dlog;
    if (!dlg)
        return;
    dlg->ignore_events = TRUE;
    if (dlg->task_freeze)
    {
        gtt_task_thaw(dlg->task);
        dlg->task_freeze = FALSE;
    }
    dlg->ignore_events = FALSE;
}

void prop_task_dialog_show(GttTask *task)
{
    GttProject *prj;
    if (!task)
        return;
    if (!global_dlog)
        global_dlog = prop_task_dialog_new();

    do_set_task(task, global_dlog);

    prj = gtt_task_get_parent(task);
    gtt_project_add_notifier(prj, redraw, global_dlog);

    gtk_widget_show(GTK_WIDGET(global_dlog->dlg));
}

/* ===================== END OF FILE =========================== */
