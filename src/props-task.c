/*   Task Properties for GTimeTracker - a time tracker
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
#include "props-task.h"
#include "util.h"


typedef struct _PropTaskDlg 
{
	GladeXML *gtxml;
	GnomePropertyBox *dlg;
	GtkEntry *memo;
	GtkTextView *notes;
	GtkOptionMenu *billstatus;
	GtkOptionMenu *billable;
	GtkOptionMenu *billrate;
	GtkEntry *unit;
	GttTask *task;
} PropTaskDlg;


/* ============================================================== */

#define GET_MENU(WIDGET,NAME) ({				\
	GtkWidget *menu, *menu_item;				\
	menu = gtk_option_menu_get_menu (WIDGET);		\
       	menu_item = gtk_menu_get_active(GTK_MENU(menu));	\
       	(gtk_object_get_data(GTK_OBJECT(menu_item), NAME));	\
})

static void 
task_prop_set(GnomePropertyBox * pb, gint page, PropTaskDlg *dlg)
{
	GttBillStatus status;
	GttBillable able;
	GttBillRate rate;
	int ivl;
	const gchar *cstr;
	gchar *str;

	if (!dlg->task) return;

	if (0 == page)
	{
		gtt_task_freeze (dlg->task);
		cstr = gtk_entry_get_text(dlg->memo);
		if (cstr && cstr[0]) 
		{
			gtt_task_set_memo(dlg->task, cstr);
		} 
		else 
		{
			gtt_task_set_memo(dlg->task, _("empty"));
			gtk_entry_set_text(dlg->memo, _("empty"));
		}
	
		str = xxxgtk_textview_get_text(dlg->notes);
		gtt_task_set_notes(dlg->task, str);
		g_free (str);

		ivl = (int) (60.0 * atof (gtk_entry_get_text(dlg->unit)));
		gtt_task_set_bill_unit (dlg->task, ivl);

		status = (GttBillStatus) GET_MENU (dlg->billstatus, "billstatus");
		gtt_task_set_billstatus (dlg->task, status);

		able = (GttBillable) GET_MENU (dlg->billable, "billable");
		gtt_task_set_billable (dlg->task, able);

		rate = (GttBillRate) GET_MENU (dlg->billrate, "billrate");
		gtt_task_set_billrate (dlg->task, rate);
		gtt_task_thaw (dlg->task);
	}
}


/* ============================================================== */


static void 
do_set_task(GttTask *tsk, PropTaskDlg *dlg)
{
	GttBillStatus status;
	GttBillable able;
	GttBillRate rate;
	char buff[132];

	if (!tsk) 
	{

		dlg->task = NULL;
		gtk_entry_set_text(dlg->memo, "");
		xxxgtk_textview_set_text(dlg->notes, "");
		gtk_entry_set_text(dlg->unit, "0.0");
		return;
	}

	/* Set the task, even if its same as the old task.  Do this because
	 * the widget may contain rejected edit values.  */
	dlg->task = tsk;

	gtk_entry_set_text(dlg->memo, gtt_task_get_memo(tsk));
	xxxgtk_textview_set_text(dlg->notes, gtt_task_get_notes (tsk));

	g_snprintf (buff, 132, "%g", ((double) gtt_task_get_bill_unit(tsk))/60.0);
	gtk_entry_set_text(dlg->unit, buff);

	status = gtt_task_get_billstatus (tsk);
	if (GTT_HOLD == status) gtk_option_menu_set_history (dlg->billstatus, 0);
	else if (GTT_BILL == status) gtk_option_menu_set_history (dlg->billstatus, 1);
	else if (GTT_PAID == status) gtk_option_menu_set_history (dlg->billstatus, 2);

	able = gtt_task_get_billable (tsk);
	if (GTT_BILLABLE == able) gtk_option_menu_set_history (dlg->billable, 0);
	else if (GTT_NOT_BILLABLE == able) gtk_option_menu_set_history (dlg->billable, 1);
	else if (GTT_NO_CHARGE == able) gtk_option_menu_set_history (dlg->billable, 2);

	rate = gtt_task_get_billrate (tsk);
	if (GTT_REGULAR == rate) gtk_option_menu_set_history (dlg->billrate, 0);
	else if (GTT_OVERTIME == rate) gtk_option_menu_set_history (dlg->billrate, 1);
	else if (GTT_OVEROVER == rate) gtk_option_menu_set_history (dlg->billrate, 2);
	else if (GTT_FLAT_FEE == rate) gtk_option_menu_set_history (dlg->billrate, 3);

	/* Set to unmodified as it reflects the current state of the project. */
	gnome_property_box_set_modified(GNOME_PROPERTY_BOX(dlg->dlg),
					FALSE);
}

/* ============================================================== */

#define TAGGED(NAME) ({						\
	GtkWidget *widget;					\
	widget = glade_xml_get_widget (gtxml, NAME);		\
	gtk_signal_connect_object(GTK_OBJECT(widget), "changed",\
		GTK_SIGNAL_FUNC(gnome_property_box_changed), 	\
		GTK_OBJECT(dlg->dlg));				\
	widget; })

#define MUGGED(NAME) ({						\
	GtkWidget *widget, *mw;					\
	widget = glade_xml_get_widget (gtxml, NAME);		\
	mw = gtk_option_menu_get_menu (GTK_OPTION_MENU(widget));\
	gtk_signal_connect_object(GTK_OBJECT(mw), "selection_done", \
		 GTK_SIGNAL_FUNC(gnome_property_box_changed),	\
		 GTK_OBJECT(dlg->dlg));				\
	GTK_OPTION_MENU(widget);				\
})

static void wrapper (void * gobj, void * data) {   
	gnome_property_box_changed (GNOME_PROPERTY_BOX(data)); 
} 

#define TEXTED(NAME) ({						\
	GtkWidget *widget;					\
	GtkTextBuffer *buff;					\
	widget = glade_xml_get_widget (gtxml, NAME);		\
	buff = gtk_text_view_get_buffer (GTK_TEXT_VIEW(widget)); \
	g_signal_connect_object(G_OBJECT(buff), "changed",\
		G_CALLBACK(wrapper), 	\
		G_OBJECT(dlg->dlg), 0);				\
	widget; })

#define MENTRY(WIDGET,NAME,ORDER,VAL) {				\
	GtkWidget *menu_item;					\
	GtkMenu *menu = GTK_MENU(gtk_option_menu_get_menu (WIDGET));	\
	gtk_option_menu_set_history (WIDGET, ORDER);		\
	menu_item =  gtk_menu_get_active(menu);			\
	gtk_object_set_data(GTK_OBJECT(menu_item), NAME,	\
		(gpointer) VAL);				\
}


static  PropTaskDlg *
prop_task_dialog_new (void)
{
	PropTaskDlg *dlg = NULL;
	GladeXML *gtxml;
	// static GnomeHelpMenuEntry help_entry = { NULL, "index.html#TASK" };

	dlg = g_new0 (PropTaskDlg, 1);

	gtxml = gtt_glade_xml_new ("glade/task_properties.glade", "Task Properties");
	dlg->gtxml = gtxml;

	dlg->dlg = GNOME_PROPERTY_BOX (glade_xml_get_widget (gtxml,  "Task Properties"));

#ifdef GNOME_20_HELP_IS_DIFFERENT_AND_NEEDS_PORTING
	help_entry.name = gnome_app_id;
	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "help",
			   GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			   &help_entry);
#endif

	gtk_signal_connect(GTK_OBJECT(dlg->dlg), "apply",
			   GTK_SIGNAL_FUNC(task_prop_set), dlg);

	/* ------------------------------------------------------ */
	/* grab the various entry boxes and hook them up */

	dlg->memo       = GTK_ENTRY(TAGGED("memo box"));
	dlg->notes      = GTK_TEXT_VIEW(TEXTED("notes box"));
	dlg->billstatus = MUGGED("billstatus menu");
	dlg->billable   = MUGGED("billable menu");
	dlg->billrate   = MUGGED("billrate menu");
	dlg->unit       = GTK_ENTRY(TAGGED("unit box"));

	/* ------------------------------------------------------ */
	/* associate values with the three option menus */

	MENTRY (dlg->billstatus, "billstatus", 0, GTT_HOLD);
	MENTRY (dlg->billstatus, "billstatus", 1, GTT_BILL);
	MENTRY (dlg->billstatus, "billstatus", 2, GTT_PAID);

	MENTRY (dlg->billable, "billable", 0, GTT_BILLABLE);
	MENTRY (dlg->billable, "billable", 1, GTT_NOT_BILLABLE);
	MENTRY (dlg->billable, "billable", 2, GTT_NO_CHARGE);

	MENTRY (dlg->billrate, "billrate", 0, GTT_REGULAR);
	MENTRY (dlg->billrate, "billrate", 1, GTT_OVERTIME);
	MENTRY (dlg->billrate, "billrate", 2, GTT_OVEROVER);
	MENTRY (dlg->billrate, "billrate", 3, GTT_FLAT_FEE);

	gnome_dialog_close_hides(GNOME_DIALOG(dlg->dlg), TRUE);
	return dlg;
}

/* ============================================================== */

static void 
redraw (GttProject *prj, gpointer data)
{
	PropTaskDlg *dlg = data;
	do_set_task (dlg->task, dlg);
}

/* ============================================================== */

static PropTaskDlg *dlog = NULL;

void 
prop_task_dialog_show (GttTask *task)
{
	GttProject *prj;
	if (!task) return;
	if (!dlog) dlog = prop_task_dialog_new();

	
	prj = gtt_task_get_parent (dlog->task);
	gtt_project_remove_notifier (prj, redraw, dlog);

	do_set_task(task, dlog);
	
	prj = gtt_task_get_parent (task);
	gtt_project_add_notifier (prj, redraw, dlog);
	
	gtk_widget_show(GTK_WIDGET(dlog->dlg));
}

/* ===================== END OF FILE =========================== */
