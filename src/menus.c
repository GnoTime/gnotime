/*   Application menubar menu layout for GTimeTracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2002, 2003 Linas Vepstas <linas@linas.org>
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

#include <config.h>
#include <gnome.h>
#include <string.h>

#include "app.h"
#include "export.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "menus.h"
#include "plug-in.h"
#include "timer.h"


static GnomeUIInfo menu_main_file[] = {
	{GNOME_APP_UI_ITEM, N_("_Export Tasks"), NULL,
		export_file_picker, "tab-delim.ghtml", NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_SAVE,
		'E', GDK_CONTROL_MASK, NULL},
	{GNOME_APP_UI_ITEM, N_("Export _Projects"), NULL,
		export_file_picker, "todo-export.ghtml", NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_SAVE,
		'P', GDK_CONTROL_MASK, NULL},
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_EXIT_ITEM(app_quit,NULL),
	GNOMEUIINFO_END
};

/* Insert an item with a stock icon and a user data pointer */
#define GNOMEUIINFO_ITEM_STOCK_DATA(label, tooltip, callback, user_data, stock_id) \
   { GNOME_APP_UI_ITEM, label, tooltip, (gpointer)callback, user_data, NULL, \
     GNOME_APP_PIXMAP_STOCK, stock_id, 0, (GdkModifierType) 0, NULL }

static GnomeUIInfo menu_main_edit[] = {
	GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Project..."), NULL,
				  new_project, NULL),
	GNOMEUIINFO_SEPARATOR,
#define MENU_EDIT_CUT_POS 2
	GNOMEUIINFO_MENU_CUT_ITEM(cut_project,NULL),
#define MENU_EDIT_COPY_POS 3
	GNOMEUIINFO_MENU_COPY_ITEM(copy_project,NULL),
#define MENU_EDIT_PASTE_POS 4
	GNOMEUIINFO_MENU_PASTE_ITEM(paste_project,NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_STOCK(N_("Edit _Times"),
		N_("Edit the time interval associated with this project"),
			       menu_howto_edit_times,
			       GNOME_STOCK_BLANK),
#define MENU_EDIT_PROP_POS 7
	GNOMEUIINFO_MENU_PROPERTIES_ITEM(menu_properties,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_main_settings[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(menu_options,NULL),
	GNOMEUIINFO_END
};


static GnomeUIInfo menu_main_reports[] = {
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Journal..."), 
		N_("Show the journal for this project"),
			       show_report, "journal.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Activity..."), 
		N_("Show the journal together with the timestamps for this project"),
			       show_report, "time-interval.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Daily..."), 
		N_("Show the total time spent on a project, day by day"),
			       show_report, "daily.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Status..."), 
		N_("Show the project descriptions and notes."),
			       show_report, "status.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_To Do..."), 
		N_("Show a sample to-do list"),
			       show_report, "todo.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Invoice..."), 
		N_("Show a sample invoice for this project"),
			       show_report, "invoice.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Query..."), 
		N_("Run a sample Query Generator"),
			       show_report, "query.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Primer..."), 
		N_("Show a sample introductory primer for designing custom reports"),
			       show_report, "primer.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_New Report..."), 
		N_("Define a path to a new GnoTime ghtml report file"),
			       new_report,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Edit Reports..."), 
		N_("Edit the entries in the Reports pulldown menu (this menu)"),
			       report_menu_edit,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
};



static GnomeUIInfo menu_main_timer[] = {
#define MENU_TIMER_START_POS 0
	{GNOME_APP_UI_ITEM, N_("St_art"), 
		N_("Start the timer running"),
		menu_start_timer, NULL,
		NULL, GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_TIMER,
		'A', GDK_CONTROL_MASK, NULL},
#define MENU_TIMER_STOP_POS 1
	{GNOME_APP_UI_ITEM, N_("Sto_p"), 
		N_("Stop the timer"),
		menu_stop_timer, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_TIMER_STOP,
		'Z', GDK_CONTROL_MASK, NULL},
#define MENU_TIMER_TOGGLE_POS 2
	{GNOME_APP_UI_TOGGLEITEM, N_("_Timer Running"), NULL,
		menu_toggle_timer, NULL, NULL,
		GNOME_APP_PIXMAP_NONE, NULL,
		'T', GDK_CONTROL_MASK, NULL},
	GNOMEUIINFO_END
};


static GnomeUIInfo menu_main_help[] = {
	GNOMEUIINFO_HELP("gnotime"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_box,NULL),
	GNOMEUIINFO_END
};


static GnomeUIInfo menu_main[] = {
	GNOMEUIINFO_MENU_FILE_TREE(menu_main_file),
	GNOMEUIINFO_MENU_EDIT_TREE(menu_main_edit),
	GNOMEUIINFO_MENU_SETTINGS_TREE(menu_main_settings),
	GNOMEUIINFO_SUBTREE(N_("_Reports"), menu_main_reports),
	GNOMEUIINFO_SUBTREE(N_("_Timer"), menu_main_timer),
	GNOMEUIINFO_MENU_HELP_TREE(menu_main_help),
	GNOMEUIINFO_END
};



static GnomeUIInfo menu_popup[] = {
#define MENU_POPUP_JNL_POS 0
	GNOMEUIINFO_ITEM_STOCK_DATA(N_("_Activity..."), 
		N_("Show the timesheet journal for this project"),
			       show_report, "time-interval.ghtml",
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("Edit _Times"),
		N_("Edit the time interval associated with this project"),
			       menu_howto_edit_times,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_New Diary Entry"),
		N_("Change the current task for this project"),
			       new_task_ui,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Edit Diary Entry"),
		N_("Edit task header for this project"),
			       edit_task_ui,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_CUT_POS 5
	GNOMEUIINFO_MENU_CUT_ITEM(cut_project,NULL),
#define MENU_POPUP_COPY_POS 6
	GNOMEUIINFO_MENU_COPY_ITEM(copy_project,NULL),
#define MENU_POPUP_PASTE_POS 7
	GNOMEUIINFO_MENU_PASTE_ITEM(paste_project,NULL),
	GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_PROP_POS 9
	GNOMEUIINFO_MENU_PROPERTIES_ITEM(menu_properties,NULL),
	GNOMEUIINFO_END
};




GtkMenuShell *
menus_get_popup(void)
{
	static GtkMenuShell *menu = NULL;

	if (menu) return menu;
    
	menu = (GtkMenuShell *)gtk_menu_new();
	gnome_app_fill_menu(menu, menu_popup, NULL, TRUE, 0);
	return menu;
}



void
menus_create(GnomeApp *app)
{
	menus_get_popup(); /* initialize it */
	gnome_app_create_menus(app, menu_main);

}

/* Global: the user-defined reports pull-down menu */
static GnomeUIInfo *reports_menu = NULL;
	
GnomeUIInfo *
gtt_get_reports_menu (void)
{
	return (reports_menu);
}

void
gtt_set_reports_menu (GnomeApp *app, GnomeUIInfo *new_menus)
{
	int i;
	char * path;
	
	/* Build the i18n menu path ... */
	/* (is this right ??? or is this pre-i18n ???) */
	path = g_strdup_printf ("%s/<Separator>", _("Reports"));
	
	/* If there are old menu items, remove them and free them. */
	if (reports_menu)
	{
		int nreports;
		for (i=0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++) {}
		nreports = i;
		gnome_app_remove_menu_range (app, path, 1, nreports);
		
		if (new_menus != reports_menu)
		{
			for (i=0; i<nreports; i++)
			{
				// XXX can't free this, since 'append' recycles old pointers !!
				// there's probably a minor memory leak here ... 
				// gtt_plugin_free(reports_menu[i].user_data);
			}
			g_free (reports_menu);
		}
	}

	/* Now install the new menu items. */
	reports_menu = new_menus;
	if (!reports_menu)
	{
		reports_menu = g_new0 (GnomeUIInfo, 1);
		reports_menu[0].type = GNOME_APP_UI_ENDOFINFO;
	}

	/* fixup */
	for (i=0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++)
	{
		reports_menu[i].moreinfo = invoke_report;
	}
	gnome_app_insert_menus (app, path, reports_menu);
}

/* ============================================================ */
/* Slide a new menu entry into first place */

void
gtt_reports_menu_prepend_entry (GnomeApp *app, GnomeUIInfo *new_entry)
{
	int i, nitems;
	GnomeUIInfo * current_sysmenu, *new_sysmenu;

	current_sysmenu = gtt_get_reports_menu ();
	for (i=0; GNOME_APP_UI_ENDOFINFO != current_sysmenu[i].type; i++) {}
	nitems = i+1;

	new_sysmenu = g_new0 (GnomeUIInfo, nitems+1);
	new_sysmenu[0] = *new_entry;

	memcpy (&new_sysmenu[1], current_sysmenu, nitems*sizeof(GnomeUIInfo));
	gtt_set_reports_menu (app, new_sysmenu);
}

/* ============================================================ */

void
menus_add_plugins (GnomeApp *app)
{
	gtt_set_reports_menu (app, reports_menu);
}


void
menu_set_states(void)
{
	GtkCheckMenuItem *mi;

	if (!menu_main_timer[MENU_TIMER_START_POS].widget) return;
	gtk_widget_set_sensitive(menu_main_timer[MENU_TIMER_TOGGLE_POS].widget,
				 1);
	mi = GTK_CHECK_MENU_ITEM(menu_main_timer[MENU_TIMER_TOGGLE_POS].widget);
	/* Can't call the 'set_active' directly, as that issues an 
	 * event which puts us in an infinite loop.  Instead,
	 * just set the value.
	 * gtk_check_menu_item_set_active (mi, timer_is_running());
	 */
	mi->active = timer_is_running();

	/* XXX would be nice to change this menu entry to say 
	 * 'timer stopped' when the timer is stopped.  But don't
	 * know how to change the menu label in gtk */
			  
	gtk_widget_set_sensitive(menu_main_timer[MENU_TIMER_START_POS].widget,
				 (FALSE == timer_is_running()) );
	gtk_widget_set_sensitive(menu_main_timer[MENU_TIMER_STOP_POS].widget,
				 (timer_is_running()) );
	gtk_widget_set_sensitive(menu_main_edit[MENU_EDIT_PASTE_POS].widget,
				 (have_cutted_project()) );

	if (menu_popup[MENU_POPUP_CUT_POS].widget)
	{
		gtk_widget_set_sensitive(menu_popup[MENU_POPUP_PASTE_POS].widget,
				 (have_cutted_project()) );
	}
}

/* ======================= END OF FILE ===================== */

