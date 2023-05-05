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
#include "gtt-entry.h"

#include <glade/glade.h>
#include <gnome.h>
#include <string.h>

#include "dialog.h"
#include "gtt-select-list.h"
#include "proj.h"
#include "props-proj.h"
#include "util.h"

typedef struct _PropDlg
{
    GladeXML *gtxml;
    GnomePropertyBox *dlg;
    GtkEntry *title;
    GtkEntry *desc;
    GtkTextView *notes;

    GtkEntry *regular;
    GtkEntry *overtime;
    GtkEntry *overover;
    GtkEntry *flatfee;

    GtkEntry *minimum;
    GtkEntry *interval;
    GtkEntry *gap;

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

static void prop_set(GnomePropertyBox *pb, gint page, PropDlg *dlg)
{
    long ivl;
    const gchar *cstr;
    gchar *str;
    double rate;
    time_t tval;

    if (!dlg->proj)
        return;

    if (0 == page)
    {
        gtt_project_freeze(dlg->proj);
        cstr = gtk_entry_get_text(dlg->title);
        if (cstr && cstr[0])
        {
            gtt_project_set_title(dlg->proj, cstr);
        }
        else
        {
            gtt_project_set_title(dlg->proj, _("empty"));
            gtk_entry_set_text(dlg->title, _("empty"));
        }

        gtt_project_set_desc(dlg->proj, gtk_entry_get_text(dlg->desc));
        str = xxxgtk_textview_get_text(dlg->notes);
        gtt_project_set_notes(dlg->proj, str);
        g_free(str);
        gtt_project_thaw(dlg->proj);
    }

    if (1 == page)
    {
        gtt_project_freeze(dlg->proj);
        rate = atof(gtk_entry_get_text(dlg->regular));
        gtt_project_set_billrate(dlg->proj, rate);
        rate = atof(gtk_entry_get_text(dlg->overtime));
        gtt_project_set_overtime_rate(dlg->proj, rate);
        rate = atof(gtk_entry_get_text(dlg->overover));
        gtt_project_set_overover_rate(dlg->proj, rate);
        rate = atof(gtk_entry_get_text(dlg->flatfee));
        gtt_project_set_flat_fee(dlg->proj, rate);
        gtt_project_thaw(dlg->proj);
    }

    if (2 == page)
    {
        gtt_project_freeze(dlg->proj);
        ivl = atoi(gtk_entry_get_text(dlg->minimum));
        gtt_project_set_min_interval(dlg->proj, ivl);
        ivl = atoi(gtk_entry_get_text(dlg->interval));
        gtt_project_set_auto_merge_interval(dlg->proj, ivl);
        ivl = atoi(gtk_entry_get_text(dlg->gap));
        gtt_project_set_auto_merge_gap(dlg->proj, ivl);
        gtt_project_thaw(dlg->proj);
    }
    if (3 == page)
    {
        gtt_project_freeze(dlg->proj);

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

        gtt_project_thaw(dlg->proj);
    }
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

    if (!proj)
    {
        /* We null these out, because old values may be left
         * over from an earlier project */
        dlg->proj = NULL;
        gtk_entry_set_text(dlg->title, "");
        gtk_entry_set_text(dlg->desc, "");
        xxxgtk_textview_set_text(dlg->notes, "");
        gtk_entry_set_text(dlg->regular, "0.0");
        gtk_entry_set_text(dlg->overtime, "0.0");
        gtk_entry_set_text(dlg->overover, "0.0");
        gtk_entry_set_text(dlg->flatfee, "0.0");
        gtk_entry_set_text(dlg->minimum, "0");
        gtk_entry_set_text(dlg->interval, "0");
        gtk_entry_set_text(dlg->gap, "0");

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

    gtk_entry_set_text(dlg->title, gtt_project_get_title(proj));
    gtk_entry_set_text(dlg->desc, gtt_project_get_desc(proj));
    xxxgtk_textview_set_text(dlg->notes, gtt_project_get_notes(proj));

    /* hack alert should use local currencies for this */
    g_snprintf(buff, 132, "%.2f", gtt_project_get_billrate(proj));
    gtk_entry_set_text(dlg->regular, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_overtime_rate(proj));
    gtk_entry_set_text(dlg->overtime, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_overover_rate(proj));
    gtk_entry_set_text(dlg->overover, buff);
    g_snprintf(buff, 132, "%.2f", gtt_project_get_flat_fee(proj));
    gtk_entry_set_text(dlg->flatfee, buff);

    g_snprintf(buff, 132, "%d", gtt_project_get_min_interval(proj));
    gtk_entry_set_text(dlg->minimum, buff);
    g_snprintf(buff, 132, "%d", gtt_project_get_auto_merge_interval(proj));
    gtk_entry_set_text(dlg->interval, buff);
    g_snprintf(buff, 132, "%d", gtt_project_get_auto_merge_gap(proj));
    gtk_entry_set_text(dlg->gap, buff);

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
    gnome_property_box_set_modified(GNOME_PROPERTY_BOX(dlg->dlg), FALSE);
}

/* ============================================================== */

#define TAGGED(WDGT)                                                                  \
    ({                                                                                \
        gtk_signal_connect_object(                                                    \
            GTK_OBJECT(WDGT), "changed", GTK_SIGNAL_FUNC(gnome_property_box_changed), \
            GTK_OBJECT(dlg->dlg)                                                      \
        );                                                                            \
        WDGT;                                                                         \
    })

#define DATED(WDGT)                                                                        \
    ({                                                                                     \
        gtk_signal_connect_object(                                                         \
            GTK_OBJECT(WDGT), "date_changed", GTK_SIGNAL_FUNC(gnome_property_box_changed), \
            GTK_OBJECT(dlg->dlg)                                                           \
        );                                                                                 \
        gtk_signal_connect_object(                                                         \
            GTK_OBJECT(WDGT), "time_changed", GTK_SIGNAL_FUNC(gnome_property_box_changed), \
            GTK_OBJECT(dlg->dlg)                                                           \
        );                                                                                 \
        GTT_DATE_EDIT(WDGT);                                                               \
    })

static void wrapper(void *gobj, void *data)
{
    gnome_property_box_changed(GNOME_PROPERTY_BOX(data));
}

#define TEXTED(NAME)                                                              \
    ({                                                                            \
        GtkWidget *widget;                                                        \
        GtkTextBuffer *buff;                                                      \
        widget = glade_xml_get_widget(gtxml, NAME);                               \
        buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));                   \
        g_signal_connect_object(                                                  \
            G_OBJECT(buff), "changed", G_CALLBACK(wrapper), G_OBJECT(dlg->dlg), 0 \
        );                                                                        \
        widget;                                                                   \
    })

static GtkComboBox *init_combo(
    GladeXML *gtxml, const char *name, GtkSignalFunc changed_callback,
    GnomePropertyBox *property_box
)
{
    /* Note: Some uglyness, in taking GnomePropertyBox as data parameter,
     * callback is expected to be gnome_property_box_changed. Except that
     * to be cleaner and match props-task.c structure later. */

    GtkComboBox *combo_box;

    combo_box = GTK_COMBO_BOX(glade_xml_get_widget(gtxml, name));
    gtt_combo_select_list_init(combo_box);

    if (changed_callback != NULL)
    {
        // Note: gtk_signal_connect_object is deprecated, should later be replaced with
        // g_signal_connect_object or something.
        gtk_signal_connect_object(
            GTK_OBJECT(combo_box), "changed", changed_callback, GTK_OBJECT(property_box)
        );
    }

    return combo_box;
}

/* ================================================================= */

static void help_cb(GnomePropertyBox *propertybox, gint page_num, gpointer data)
{
    gtt_help_popup(GTK_WIDGET(propertybox), data);
}

static PropDlg *prop_dialog_new(void)
{
    PropDlg *dlg;
    GladeXML *gtxml;

    dlg = g_new0(PropDlg, 1);

    gtxml = gtt_glade_xml_new("glade/project_properties.glade", "Project Properties");
    dlg->gtxml = gtxml;

    dlg->dlg = GNOME_PROPERTY_BOX(glade_xml_get_widget(gtxml, "Project Properties"));

    gtk_signal_connect(
        GTK_OBJECT(dlg->dlg), "help", GTK_SIGNAL_FUNC(help_cb), "projects-editing"
    );

    gtk_signal_connect(GTK_OBJECT(dlg->dlg), "apply", GTK_SIGNAL_FUNC(prop_set), dlg);

    /* ------------------------------------------------------ */
    /* grab the various entry boxes and hook them up */

    GtkWidget *const title_table = glade_xml_get_widget(gtxml, "title table");

    GtkWidget *const entry7 = gtt_entry_new("project-title");
    gtt_entry_set_max_saved(GTT_ENTRY(entry7), 10);
    gtk_widget_set_name(entry7, "entry7");

    GtkWidget *const title_box = gtt_entry_gtk_entry(GTT_ENTRY(entry7));
    gtk_entry_set_activates_default(GTK_ENTRY(title_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(title_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(title_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(title_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(title_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(title_box), TRUE);
    gtk_widget_set_can_focus(title_box, TRUE);
    gtk_widget_set_name(title_box, "title box");
    gtk_widget_set_tooltip_text(title_box, _("A title to assign to this project"));
    dlg->title = GTK_ENTRY(TAGGED(title_box));
    gtk_widget_show(title_box);

    gtk_widget_show(entry7);

    gtk_table_attach(
        GTK_TABLE(title_table), entry7, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const entry9 = gtt_entry_new("project-description");
    gtt_entry_set_max_saved(GTT_ENTRY(entry9), 10);
    gtk_widget_set_name(entry9, "entry9");

    GtkWidget *const desc_box = gtt_entry_gtk_entry(GTT_ENTRY(entry9));
    gtk_entry_set_activates_default(GTK_ENTRY(desc_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(desc_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(desc_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(desc_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(desc_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(desc_box), TRUE);
    gtk_widget_set_can_focus(desc_box, TRUE);
    gtk_widget_set_name(desc_box, "desc box");
    gtk_widget_set_tooltip_text(
        desc_box, _("a short description that will be printed on the invoice.")
    );
    dlg->desc = GTK_ENTRY(TAGGED(desc_box));
    gtk_widget_show(desc_box);

    gtk_widget_show(entry9);

    gtk_table_attach(
        GTK_TABLE(title_table), entry9, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    dlg->notes = GTK_TEXT_VIEW(TEXTED("notes box"));

    GtkWidget *const rate_table = glade_xml_get_widget(gtxml, "rate table");

    GtkWidget *const combo_regular = gtt_entry_new("regular-rate");
    gtt_entry_set_max_saved(GTT_ENTRY(combo_regular), 10);
    gtk_widget_set_name(combo_regular, "combo regular");

    GtkWidget *const regular_box = gtt_entry_gtk_entry(GTT_ENTRY(combo_regular));
    gtk_entry_set_activates_default(GTK_ENTRY(regular_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(regular_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(regular_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(regular_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(regular_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(regular_box), TRUE);
    gtk_widget_set_can_focus(regular_box, TRUE);
    gtk_widget_set_name(regular_box, "regular box");
    gtk_widget_set_tooltip_text(
        regular_box, _("The dollars per hour normally charged for this project.")
    );
    dlg->regular = GTK_ENTRY(TAGGED(regular_box));
    gtk_widget_show(regular_box);

    gtk_widget_show(combo_regular);

    gtk_table_attach(
        GTK_TABLE(rate_table), combo_regular, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const overtime_combo = gtt_entry_new("overtime-rate");
    gtt_entry_set_max_saved(GTT_ENTRY(overtime_combo), 10);
    gtk_widget_set_name(overtime_combo, "overtime combo");

    GtkWidget *const overtime_box = gtt_entry_gtk_entry(GTT_ENTRY(overtime_combo));
    gtk_entry_set_activates_default(GTK_ENTRY(overtime_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(overtime_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(overtime_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(overtime_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(overtime_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(overtime_box), TRUE);
    gtk_widget_set_can_focus(overtime_box, TRUE);
    gtk_widget_set_name(overtime_box, "overtime box");
    gtk_widget_set_tooltip_text(
        overtime_box, _("The dollars per hour charged for overtime work on this project.")
    );
    dlg->overtime = GTK_ENTRY(TAGGED(overtime_box));
    gtk_widget_show(overtime_box);

    gtk_widget_show(overtime_combo);

    gtk_table_attach(
        GTK_TABLE(rate_table), overtime_combo, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const overover_combo = gtt_entry_new("overover-rate");
    gtt_entry_set_max_saved(GTT_ENTRY(overover_combo), 10);
    gtk_widget_set_name(overover_combo, "overover combo");

    GtkWidget *const overover_box = gtt_entry_gtk_entry(GTT_ENTRY(overover_combo));
    gtk_entry_set_activates_default(GTK_ENTRY(overover_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(overover_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(overover_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(overover_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(overover_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(overover_box), TRUE);
    gtk_widget_set_can_focus(overover_box, TRUE);
    gtk_widget_set_name(overover_box, "overover box");
    gtk_widget_set_tooltip_text(
        overover_box, _("The over-overtime rate (overtime on Sundays, etc.)")
    );
    dlg->overover = GTK_ENTRY(TAGGED(overover_box));
    gtk_widget_show(overover_box);

    gtk_widget_show(overover_combo);

    gtk_table_attach(
        GTK_TABLE(rate_table), overover_combo, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const flatfee_combo = gtt_entry_new("flat-fee");
    gtt_entry_set_max_saved(GTT_ENTRY(flatfee_combo), 10);
    gtk_widget_set_name(flatfee_combo, "flatfee combo");

    GtkWidget *const flatfee_box = gtt_entry_gtk_entry(GTT_ENTRY(overover_combo));
    gtk_entry_set_activates_default(GTK_ENTRY(flatfee_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(flatfee_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(flatfee_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(flatfee_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(flatfee_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(flatfee_box), TRUE);
    gtk_widget_set_can_focus(flatfee_box, TRUE);
    gtk_widget_set_name(flatfee_box, "flatfee box");
    gtk_widget_set_tooltip_text(
        flatfee_box, _("If this project is billed for one price no matter how long it takes, "
                       "enter the fee here.")
    );
    dlg->flatfee = GTK_ENTRY(TAGGED(flatfee_box));
    gtk_widget_show(flatfee_box);

    gtk_widget_show(flatfee_combo);

    gtk_table_attach(
        GTK_TABLE(rate_table), flatfee_combo, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const interval_table = glade_xml_get_widget(gtxml, "interval table");

    GtkWidget *const entry22 = gtt_entry_new("min-interval");
    gtt_entry_set_max_saved(GTT_ENTRY(entry22), 10);
    gtk_widget_set_name(entry22, "entry22");

    GtkWidget *const minimum_box = gtt_entry_gtk_entry(GTT_ENTRY(entry22));
    gtk_entry_set_activates_default(GTK_ENTRY(minimum_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(minimum_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(minimum_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(minimum_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(minimum_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(minimum_box), TRUE);
    gtk_widget_set_can_focus(minimum_box, TRUE);
    gtk_widget_set_name(minimum_box, "minimum box");
    gtk_widget_set_tooltip_text(
        minimum_box, _("Intervals smaller than this will be discarded")
    );
    dlg->minimum = GTK_ENTRY(TAGGED(minimum_box));
    gtk_widget_show(minimum_box);

    gtk_widget_show(entry22);

    gtk_table_attach(
        GTK_TABLE(interval_table), entry22, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const entry23 = gtt_entry_new("merge-interval");
    gtt_entry_set_max_saved(GTT_ENTRY(entry23), 10);
    gtk_widget_set_name(entry23, "entry23");

    GtkWidget *const interval_box = gtt_entry_gtk_entry(GTT_ENTRY(entry23));
    gtk_entry_set_activates_default(GTK_ENTRY(interval_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(interval_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(interval_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(interval_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(interval_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(interval_box), TRUE);
    gtk_widget_set_can_focus(interval_box, TRUE);
    gtk_widget_set_name(interval_box, "interval box");
    gtk_widget_set_tooltip_text(
        interval_box, _("Time below which an interval is merged with its neighbors")
    );
    dlg->interval = GTK_ENTRY(TAGGED(interval_box));
    gtk_widget_show(interval_box);

    gtk_widget_show(entry23);

    gtk_table_attach(
        GTK_TABLE(interval_table), entry23, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const entry24 = gtt_entry_new("merge-gap");
    gtt_entry_set_max_saved(GTT_ENTRY(entry24), 10);
    gtk_widget_set_name(entry24, "entry24");

    GtkWidget *const gap_box = gtt_entry_gtk_entry(GTT_ENTRY(entry24));
    gtk_entry_set_activates_default(GTK_ENTRY(gap_box), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(gap_box), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(gap_box), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(gap_box), '*');
    gtk_entry_set_max_length(GTK_ENTRY(gap_box), 0);
    gtk_entry_set_visibility(GTK_ENTRY(gap_box), TRUE);
    gtk_widget_set_can_focus(gap_box, TRUE);
    gtk_widget_set_name(gap_box, "gap box");
    gtk_widget_set_tooltip_text(
        gap_box, _("If the gap between intervals is smaller than this, the intervals will be "
                   "merged together.")
    );
    dlg->gap = GTK_ENTRY(TAGGED(gap_box));
    gtk_widget_show(gap_box);

    gtk_widget_show(entry24);

    gtk_table_attach(
        GTK_TABLE(interval_table), entry24, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    dlg->urgency
        = init_combo(gtxml, "urgency menu", G_CALLBACK(gnome_property_box_changed), dlg->dlg);
    dlg->importance = init_combo(
        gtxml, "importance menu", G_CALLBACK(gnome_property_box_changed), dlg->dlg
    );
    dlg->status
        = init_combo(gtxml, "status menu", G_CALLBACK(gnome_property_box_changed), dlg->dlg);

    GtkWidget *const sizing_table = glade_xml_get_widget(gtxml, "sizing table");

    GtkWidget *const start_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->start = DATED(start_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(start_date), 7, 19);
    gtk_widget_set_name(start_date, "start date");
    gtk_widget_show(start_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), start_date, 1, 4, 3, 4, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const end_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->end = DATED(end_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(end_date), 7, 19);
    gtk_widget_set_name(end_date, "end date");
    gtk_widget_show(end_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), end_date, 1, 4, 4, 5, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    GtkWidget *const due_date
        = gtt_date_edit_new_flags(0, GTT_DATE_EDIT_24_HR | GTT_DATE_EDIT_SHOW_TIME);
    dlg->due = DATED(due_date);
    gtt_date_edit_set_popup_range(GTT_DATE_EDIT(due_date), 7, 19);
    gtk_widget_set_name(due_date, "due date");
    gtk_widget_show(due_date);

    gtk_table_attach(
        GTK_TABLE(sizing_table), due_date, 1, 4, 5, 6, GTK_EXPAND | GTK_FILL, 0, 0, 0
    );

    dlg->sizing = GTK_ENTRY(TAGGED(glade_xml_get_widget(gtxml, "sizing box")));
    dlg->percent = GTK_ENTRY(TAGGED(glade_xml_get_widget(gtxml, "percent box")));

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

    gnome_dialog_close_hides(GNOME_DIALOG(dlg->dlg), TRUE);

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
