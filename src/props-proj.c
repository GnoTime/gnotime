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

#include <glade/glade.h>
#include <gnome.h>
#include <libgnome/gnome-help.h>
#include <string.h>

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

	GtkOptionMenu *urgency;
	GtkOptionMenu *importance;
	GtkOptionMenu *status;

	GnomeDateEdit *start;
	GnomeDateEdit *end;
	GnomeDateEdit *due;

	GtkEntry *sizing;
	GtkEntry *percent;

	GttProject *proj;
} PropDlg;


/* ============================================================== */

#define GET_MENU(WIDGET,NAME) ({                                \
        GtkWidget *menu, *menu_item;                            \
        menu = gtk_option_menu_get_menu (WIDGET);               \
        menu_item = gtk_menu_get_active(GTK_MENU(menu));        \
        (gtk_object_get_data(GTK_OBJECT(menu_item), NAME));     \
})


static void 
prop_set(GnomePropertyBox * pb, gint page, PropDlg *dlg)
{
	int ivl;
	const gchar *cstr;
	gchar *str;
	double rate;
	time_t tval;

	if (!dlg->proj) return;

	if (0 == page)
	{
		gtt_project_freeze (dlg->proj);
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
		gtt_project_thaw (dlg->proj);
	}

	if (1 == page)
	{
		gtt_project_freeze (dlg->proj);
		rate = atof (gtk_entry_get_text(dlg->regular));
		gtt_project_set_billrate (dlg->proj, rate);
		rate = atof (gtk_entry_get_text(dlg->overtime));
		gtt_project_set_overtime_rate (dlg->proj, rate);
		rate = atof (gtk_entry_get_text(dlg->overover));
		gtt_project_set_overover_rate (dlg->proj, rate);
		rate = atof (gtk_entry_get_text(dlg->flatfee));
		gtt_project_set_flat_fee (dlg->proj, rate);
		gtt_project_thaw (dlg->proj);
	}
	
	if (2 == page)
	{
		gtt_project_freeze (dlg->proj);
		ivl = atoi (gtk_entry_get_text(dlg->minimum));
		gtt_project_set_min_interval (dlg->proj, ivl);
		ivl = atoi (gtk_entry_get_text(dlg->interval));
		gtt_project_set_auto_merge_interval (dlg->proj, ivl);
		ivl = atoi (gtk_entry_get_text(dlg->gap));
		gtt_project_set_auto_merge_gap (dlg->proj, ivl);
		gtt_project_thaw (dlg->proj);
	}
	if (3 == page)
	{
		gtt_project_freeze (dlg->proj);
		
		ivl = (int) GET_MENU (dlg->urgency, "urgency");
		gtt_project_set_urgency (dlg->proj, (GttRank) ivl);
		ivl = (int) GET_MENU (dlg->importance, "importance");
		gtt_project_set_importance (dlg->proj, (GttRank) ivl);

		ivl = (int) GET_MENU (dlg->status, "status");
		gtt_project_set_status (dlg->proj, (GttProjectStatus) ivl);

		tval = gnome_date_edit_get_time (dlg->start);
		gtt_project_set_estimated_start (dlg->proj, tval);
		tval = gnome_date_edit_get_time (dlg->end);
		gtt_project_set_estimated_end (dlg->proj, tval);
		tval = gnome_date_edit_get_time (dlg->due);
		gtt_project_set_due_date (dlg->proj, tval);

		rate = atof (gtk_entry_get_text(dlg->sizing));
		ivl = rate * 3600.0;
		gtt_project_set_sizing (dlg->proj, ivl);

		ivl = atoi (gtk_entry_get_text(dlg->percent));
		gtt_project_set_percent_complete (dlg->proj, ivl);

		gtt_project_thaw (dlg->proj);
	}
}


/* ============================================================== */


static void 
do_set_project(GttProject *proj, PropDlg *dlg)
{
	GttProjectStatus status;
	GttRank rank;
	time_t tval;
	char buff[132];
	time_t now = time(NULL);

	if (!dlg) return;

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

		gnome_date_edit_set_time(dlg->start, now);
		gnome_date_edit_set_time(dlg->end, now);
		gnome_date_edit_set_time(dlg->due, now+86400);
		gtk_entry_set_text(dlg->sizing, "0.0");
		gtk_entry_set_text(dlg->percent, "0");
		return;
	}


	/* set all the values. Do this even is new project is same as old
	 * project, since widget may be holding rejected changes. */
	dlg->proj = proj;

	gtk_entry_set_text(dlg->title, gtt_project_get_title(proj));
	gtk_entry_set_text(dlg->desc, gtt_project_get_desc(proj));
	xxxgtk_textview_set_text(dlg->notes, gtt_project_get_notes (proj));

	/* hack alert should use local currencies for this */
	g_snprintf (buff, 132, "%.2f", gtt_project_get_billrate(proj));
	gtk_entry_set_text(dlg->regular, buff);
	g_snprintf (buff, 132, "%.2f", gtt_project_get_overtime_rate(proj));
	gtk_entry_set_text(dlg->overtime, buff);
	g_snprintf (buff, 132, "%.2f", gtt_project_get_overover_rate(proj));
	gtk_entry_set_text(dlg->overover, buff);
	g_snprintf (buff, 132, "%.2f", gtt_project_get_flat_fee(proj));
	gtk_entry_set_text(dlg->flatfee, buff);

	g_snprintf (buff, 132, "%d", gtt_project_get_min_interval(proj));
	gtk_entry_set_text(dlg->minimum, buff);
	g_snprintf (buff, 132, "%d", gtt_project_get_auto_merge_interval(proj));
	gtk_entry_set_text(dlg->interval, buff);
	g_snprintf (buff, 132, "%d", gtt_project_get_auto_merge_gap(proj));
	gtk_entry_set_text(dlg->gap, buff);

	rank = gtt_project_get_urgency (proj);
	if (GTT_UNDEFINED   == rank) gtk_option_menu_set_history (dlg->urgency, 0);
	else if (GTT_LOW    == rank) gtk_option_menu_set_history (dlg->urgency, 1);
	else if (GTT_MEDIUM == rank) gtk_option_menu_set_history (dlg->urgency, 2);
	else if (GTT_HIGH   == rank) gtk_option_menu_set_history (dlg->urgency, 3);

	rank = gtt_project_get_importance (proj);
	if (GTT_UNDEFINED   == rank) gtk_option_menu_set_history (dlg->importance, 0);
	else if (GTT_LOW    == rank) gtk_option_menu_set_history (dlg->importance, 1);
	else if (GTT_MEDIUM == rank) gtk_option_menu_set_history (dlg->importance, 2);
	else if (GTT_HIGH   == rank) gtk_option_menu_set_history (dlg->importance, 3);

	status = gtt_project_get_status (proj);
	if (GTT_NOT_STARTED      == status) gtk_option_menu_set_history (dlg->status, 0);
	else if (GTT_IN_PROGRESS == status) gtk_option_menu_set_history (dlg->status, 1);
	else if (GTT_ON_HOLD     == status) gtk_option_menu_set_history (dlg->status, 2);
	else if (GTT_CANCELLED   == status) gtk_option_menu_set_history (dlg->status, 3);
	else if (GTT_COMPLETED   == status) gtk_option_menu_set_history (dlg->status, 4);

	tval = gtt_project_get_estimated_start (proj);
	if (-1 == tval) tval = now;
	gnome_date_edit_set_time (dlg->start, tval);
	tval = gtt_project_get_estimated_end (proj);
	if (-1 == tval) tval = now+3600;
	gnome_date_edit_set_time (dlg->end, tval);
	tval = gtt_project_get_due_date (proj);
	if (-1 == tval) tval = now+86400;
	gnome_date_edit_set_time (dlg->due, tval);

	g_snprintf (buff, 132, "%.2f", ((double) gtt_project_get_sizing(proj))/3600.0);
	gtk_entry_set_text(dlg->sizing, buff);
	g_snprintf (buff, 132, "%d", gtt_project_get_percent_complete(proj));
	gtk_entry_set_text(dlg->percent, buff);

	/* set to unmodified as it reflects the current state of the project */
	gnome_property_box_set_modified(GNOME_PROPERTY_BOX(dlg->dlg),
					FALSE);
}

/* ============================================================== */


#define TAGGED(NAME) ({                                           \
	GtkWidget *widget;                                             \
	widget = glade_xml_get_widget (gtxml, NAME);                   \
	gtk_signal_connect_object(GTK_OBJECT(widget), "changed",       \
	        GTK_SIGNAL_FUNC(gnome_property_box_changed),           \
	        GTK_OBJECT(dlg->dlg));                                 \
	widget; })

#define DATED(NAME) ({                                            \
	GtkWidget *widget;                                             \
	widget = glade_xml_get_widget (gtxml, NAME);                   \
	gtk_signal_connect_object(GTK_OBJECT(widget), "date_changed",  \
	        GTK_SIGNAL_FUNC(gnome_property_box_changed),           \
	        GTK_OBJECT(dlg->dlg));                                 \
	gtk_signal_connect_object(GTK_OBJECT(widget), "time_changed",  \
	        GTK_SIGNAL_FUNC(gnome_property_box_changed),           \
	        GTK_OBJECT(dlg->dlg));                                 \
	GNOME_DATE_EDIT(widget); })

static void wrapper (void * gobj, void * data) {   
	gnome_property_box_changed (GNOME_PROPERTY_BOX(data)); 
} 

#define TEXTED(NAME) ({                                          \
	GtkWidget *widget;                                            \
	GtkTextBuffer *buff;                                          \
	widget = glade_xml_get_widget (gtxml, NAME);                  \
	buff = gtk_text_view_get_buffer (GTK_TEXT_VIEW(widget));      \
	g_signal_connect_object(G_OBJECT(buff), "changed",            \
	        G_CALLBACK(wrapper),                                  \
	        G_OBJECT(dlg->dlg), 0);                               \
	widget; })


#define MUGGED(NAME) ({                                          \
	GtkWidget *widget, *mw;                                       \
	widget = glade_xml_get_widget (gtxml, NAME);                  \
	mw = gtk_option_menu_get_menu (GTK_OPTION_MENU(widget));      \
	gtk_signal_connect_object(GTK_OBJECT(mw), "selection_done",   \
	         GTK_SIGNAL_FUNC(gnome_property_box_changed),         \
	         GTK_OBJECT(dlg->dlg));                               \
	GTK_OPTION_MENU(widget);                                      \
})


#define MENTRY(WIDGET,NAME,ORDER,VAL) {                          \
	GtkWidget *menu_item;                                         \
	GtkMenu *menu = GTK_MENU(gtk_option_menu_get_menu (WIDGET));  \
	gtk_option_menu_set_history (WIDGET, ORDER);                  \
	menu_item =  gtk_menu_get_active(menu);                       \
	gtk_object_set_data(GTK_OBJECT(menu_item), NAME,              \
	        (gpointer) VAL);                                      \
}


static PropDlg *
prop_dialog_new (void)
{
	PropDlg *dlg;
	GladeXML *gtxml;
	/* static GnomeHelpMenuEntry help_entry = { NULL, "dialogs.html#PROPERTIES" }; */

	dlg = g_new0(PropDlg, 1);

	gtxml = gtt_glade_xml_new ("glade/project_properties.glade", "Project Properties");
	dlg->gtxml = gtxml;

	dlg->dlg = GNOME_PROPERTY_BOX (glade_xml_get_widget (gtxml,  "Project Properties"));

#ifdef GNOME_20_HAS_INCOMPATIBLE_HELP_SYSTEM
	help_entry.name = gnome_app_id;
	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "help",
			   GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			   &help_entry);
#endif

	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "apply",
			   GTK_SIGNAL_FUNC(prop_set), dlg);

	/* ------------------------------------------------------ */
	/* grab the various entry boxes and hook them up */

	dlg->title      = GTK_ENTRY(TAGGED("title box"));
	dlg->desc       = GTK_ENTRY(TAGGED("desc box"));
	dlg->notes      = GTK_TEXT_VIEW(TEXTED("notes box"));

	dlg->regular    = GTK_ENTRY(TAGGED("regular box"));
	dlg->overtime   = GTK_ENTRY(TAGGED("overtime box"));
	dlg->overover   = GTK_ENTRY(TAGGED("overover box"));
	dlg->flatfee    = GTK_ENTRY(TAGGED("flatfee box"));

	dlg->minimum    = GTK_ENTRY(TAGGED("minimum box"));
	dlg->interval   = GTK_ENTRY(TAGGED("interval box"));
	dlg->gap        = GTK_ENTRY(TAGGED("gap box"));

	dlg->urgency    = MUGGED("urgency menu");
	dlg->importance = MUGGED("importance menu");
	dlg->status     = MUGGED("status menu");

	dlg->start      = DATED("start date");
	dlg->end        = DATED("end date");
	dlg->due        = DATED("due date");

	dlg->sizing     = GTK_ENTRY(TAGGED("sizing box"));
	dlg->percent    = GTK_ENTRY(TAGGED("percent box"));

	/* ------------------------------------------------------ */
	/* initialize menu values */

	MENTRY (dlg->urgency, "urgency", 0, GTT_UNDEFINED);
	MENTRY (dlg->urgency, "urgency", 1, GTT_LOW);
	MENTRY (dlg->urgency, "urgency", 2, GTT_MEDIUM);
	MENTRY (dlg->urgency, "urgency", 3, GTT_HIGH);

	MENTRY (dlg->importance, "importance", 0, GTT_UNDEFINED);
	MENTRY (dlg->importance, "importance", 1, GTT_LOW);
	MENTRY (dlg->importance, "importance", 2, GTT_MEDIUM);
	MENTRY (dlg->importance, "importance", 3, GTT_HIGH);

	MENTRY (dlg->status, "status", 0, GTT_NOT_STARTED);
	MENTRY (dlg->status, "status", 1, GTT_IN_PROGRESS);
	MENTRY (dlg->status, "status", 2, GTT_ON_HOLD);
	MENTRY (dlg->status, "status", 3, GTT_CANCELLED);
	MENTRY (dlg->status, "status", 4, GTT_COMPLETED);

	gnome_dialog_close_hides(GNOME_DIALOG(dlg->dlg), TRUE);

	return dlg;
}


/* ============================================================== */

static void 
redraw (GttProject *prj, gpointer data)
{
	PropDlg *dlg = data;
	do_set_project(prj, dlg);
}

/* ============================================================== */

static PropDlg *dlog = NULL;

void 
prop_dialog_show(GttProject *proj)
{
	if (!dlog) dlog = prop_dialog_new();
 
	gtt_project_remove_notifier (dlog->proj, redraw, dlog);
	do_set_project(proj, dlog);
	gtt_project_add_notifier (proj, redraw, dlog);
	gtk_widget_show(GTK_WIDGET(dlog->dlg));
}

void 
prop_dialog_set_project(GttProject *proj)
{
	if (!dlog) return;
 
	gtt_project_remove_notifier (dlog->proj, redraw, dlog);
	do_set_project(proj, dlog);
	gtt_project_add_notifier (proj, redraw, dlog);
}

/* ==================== END OF FILE ============================= */
