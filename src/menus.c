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
#define MENU_EDIT_CDC_POS 6
	GNOMEUIINFO_ITEM_STOCK(N_("Clear _Daily Counter"), 
		N_("Zero out todays timer by deleting todays time logs"),
			       menu_clear_daily_counter,
			       GNOME_STOCK_BLANK),
#define MENU_EDIT_JNL_POS 7
	GNOMEUIINFO_ITEM_STOCK(N_("_Journal..."), 
		N_("Show the timesheet journal for this project"),
			       edit_journal,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("New _Diary Entry"),
		N_("Create new task header for this project"),
			       new_task_ui,
			       GNOME_STOCK_BLANK),
#define MENU_EDIT_PROP_POS 9
	GNOMEUIINFO_MENU_PROPERTIES_ITEM(menu_properties,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_main_settings[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(menu_options,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_main_reports[] = {
	GNOMEUIINFO_ITEM_STOCK(N_("_Journal..."), 
		N_("Show the timesheet journal for this project"),
			       edit_journal,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Big Journal..."), 
		N_("Show the verbose timesheet journal for this project"),
			       edit_alldata,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Invoice..."), 
		N_("Show a sample invoice for this project"),
			       edit_invoice,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Daily..."), 
		N_("Show the total time spent on a project, day by day"),
			       edit_daily,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Status..."), 
		N_("Show the project descriptions and notes."),
			       edit_status,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_To Do..."), 
		N_("Show a sample to-do list"),
			       edit_todolist,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Primer..."), 
		N_("Show a sample introductory primer for designing custom reports"),
			       edit_primer,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_New Report..."), 
		N_("Define a path to a new GnoTime ghtml report file"),
			       new_report,
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
	GNOMEUIINFO_HELP("gtt"),
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
	GNOMEUIINFO_ITEM_STOCK(N_("_Journal..."), 
		N_("Show the timesheet journal for this project"),
			       edit_journal,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("New Diary _Entry"),
		N_("Change the current task for this project"),
			       new_task_ui,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_ITEM_STOCK(N_("_Edit Diary Entry"),
		N_("Edit task header for this project"),
			       edit_task_ui,
			       GNOME_STOCK_BLANK),
	GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_CUT_POS 4
	GNOMEUIINFO_MENU_CUT_ITEM(cut_project,NULL),
#define MENU_POPUP_COPY_POS 5
	GNOMEUIINFO_MENU_COPY_ITEM(copy_project,NULL),
#define MENU_POPUP_PASTE_POS 6
	GNOMEUIINFO_MENU_PASTE_ITEM(paste_project,NULL),
	GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_CDC_POS 8
	GNOMEUIINFO_ITEM_STOCK(N_("Clear _Daily Counter"),
		N_("Zero out todays timer by deleting todays time logs"),
			       menu_clear_daily_counter,
			       GNOME_STOCK_BLANK),
#define MENU_POPUP_PROP_POS 9
	GNOMEUIINFO_MENU_PROPERTIES_ITEM(menu_properties,NULL),
	GNOMEUIINFO_END
};




GtkWidget *
menus_get_popup(void)
{
	static GtkWidget *menu = NULL;

	if (menu) return menu;

	menu = gnome_popup_menu_new(menu_popup);
	return menu;
}



void
menus_create(GnomeApp *app)
{
	menus_get_popup(); /* initialize it */
	gnome_app_create_menus(app, menu_main);

}

void
menus_add_plugins (GnomeApp *app)
{
	GnomeUIInfo *plugins;
	char * path;
	GList *node;
	int len, i;

	node = gtt_plugin_get_list ();

	len = g_list_length (node);
	if (0 >= len) return;

	len ++;
	plugins = g_new0 (GnomeUIInfo, len);

	i = 0;
	for (node = gtt_plugin_get_list(); node; node=node->next)
	{
		GttPlugin *plg = node->data;

		plugins[i].type = GNOME_APP_UI_ITEM;
		plugins[i].label = plg->name;
		plugins[i].hint = plg->tooltip;
		plugins[i].moreinfo = invoke_report;
		plugins[i].user_data = plg->path;
		plugins[i].unused_data = NULL;
		plugins[i].pixmap_type = GNOME_APP_PIXMAP_STOCK;
		plugins[i].pixmap_info = GNOME_STOCK_BLANK;
		plugins[i].accelerator_key = 0;
		plugins[i].ac_mods = (GdkModifierType) 0;

		i++;
	}
	plugins[i].type = GNOME_APP_UI_ENDOFINFO;

	/* deal with the i18n menu path ...*/
	/* (is this right ??? or is this pre-i18n ???) */
	path = g_strdup_printf ("%s/<Separator>", _("Reports"));

	gnome_app_insert_menus (app, path, plugins);
	g_free (plugins);
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
				 (cutted_project) ? 1 : 0);

	if (menu_popup[MENU_POPUP_CUT_POS].widget)
	{
		gtk_widget_set_sensitive(menu_popup[MENU_POPUP_PASTE_POS].widget,
				 (cutted_project) ? 1 : 0);
	}
}

/* ======================= END OF FILE ===================== */

