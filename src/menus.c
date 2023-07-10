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
#include "util.h"
#include "dialog.h"


static GtkBuilder * menu_builder;


/* Insert an item with a stock icon and a user data pointer */
#define GNOMEUIINFO_ITEM_STOCK_DATA(label, tooltip, callback, user_data, stock_id) \
    {                                                                              \
        GNOME_APP_UI_ITEM, label, tooltip, (gpointer) callback, user_data, NULL,   \
            GNOME_APP_PIXMAP_STOCK, stock_id, 0, (GdkModifierType) 0, NULL         \
    }

static GnomeUIInfo menu_popup[] = {
#define MENU_POPUP_JNL_POS 0
    GNOMEUIINFO_ITEM_STOCK_DATA(
        N_("_Activity..."), N_("Show the timesheet journal for this project"), show_report,
        ACTIVITY_REPORT, GNOME_STOCK_BLANK
    ),
    GNOMEUIINFO_ITEM_STOCK(
        N_("Edit _Times"), N_("Edit the time interval associated with this project"),
        menu_howto_edit_times, GNOME_STOCK_BLANK
    ),
    GNOMEUIINFO_ITEM_STOCK(
        N_("_New Diary Entry"), N_("Change the current task for this project"), new_task_ui,
        GNOME_STOCK_BLANK
    ),
    GNOMEUIINFO_ITEM_STOCK(
        N_("_Edit Diary Entry"), N_("Edit task header for this project"), edit_task_ui,
        GNOME_STOCK_BLANK
    ),
    GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_CUT_POS 5
    GNOMEUIINFO_MENU_CUT_ITEM(cut_project, NULL),
#define MENU_POPUP_COPY_POS 6
    GNOMEUIINFO_MENU_COPY_ITEM(copy_project, NULL),
#define MENU_POPUP_PASTE_POS 7
    GNOMEUIINFO_MENU_PASTE_ITEM(paste_project, NULL),
    GNOMEUIINFO_SEPARATOR,
#define MENU_POPUP_PROP_POS 9
    GNOMEUIINFO_MENU_PROPERTIES_ITEM(menu_properties, NULL),
    GNOMEUIINFO_END
};

GtkMenuShell *menus_get_popup(void)
{
    static GtkMenuShell *menu = NULL;

    if (menu)
        return menu;

    menu = (GtkMenuShell *) gtk_menu_new();
    gnome_app_fill_menu(menu, menu_popup, NULL, TRUE, 0);
    return menu;
}

static void attach_menu_action(GtkBuilder * builder, const char * item_name,
                               GCallback callback, void * user_data)
{
    GtkWidget *item = GTK_WIDGET(gtk_builder_get_object(builder, item_name));

    g_signal_connect(item, "activate", callback, user_data);
}

void menus_create(GnomeApp *app)
{
    menus_get_popup(); /* initialize it */

    GtkBuilder *builder;
    builder = gtt_builder_new_from_file("ui/mainmenu.ui");
    GtkMenuBar * menubar = GTK_MENU_BAR(gtk_builder_get_object(builder, "mainmenu"));

    gnome_app_set_menus(app, menubar);

    menu_builder = builder;

    // Cannot get accelerators to work. Ignore for now, as I expect this way to
    // build the menus will not last very long. /OB 2023-07-10
    // gtk_window_add_accel_group(GTK_WINDOW(app), gtk_builder_get_object(builder, "accelgroup1"));
    // gtk_menu_set_accel_group(gtk_builder_get_object(builder, "menu_file"),
    //                          app->accel_group);

    // File menu actions.
    attach_menu_action(builder, "mi_export_tasks", G_CALLBACK(export_file_picker), TAB_DELIM_EXPORT);
    attach_menu_action(builder, "mi_export_projects", G_CALLBACK(export_file_picker), TODO_EXPORT);
    attach_menu_action(builder, "mi_quit", G_CALLBACK(app_quit), NULL);

    // Project menu actions.
    attach_menu_action(builder, "mi_new_project", G_CALLBACK(new_project), NULL);
    attach_menu_action(builder, "mi_cut_project", G_CALLBACK(cut_project), NULL);
    attach_menu_action(builder, "mi_copy_project", G_CALLBACK(copy_project), NULL);
    attach_menu_action(builder, "mi_paste_project", G_CALLBACK(paste_project), NULL);
    attach_menu_action(builder, "mi_edit_times", G_CALLBACK(menu_howto_edit_times), NULL);
    attach_menu_action(builder, "mi_edit_project", G_CALLBACK(menu_properties), NULL);

    // Settings menu actions.
    attach_menu_action(builder, "mi_preferences", G_CALLBACK(menu_options), NULL);

    // Reports menu actions.
    attach_menu_action(builder, "mi_report_journal", G_CALLBACK(show_report), JOURNAL_REPORT);
    attach_menu_action(builder, "mi_report_activty", G_CALLBACK(show_report), ACTIVITY_REPORT);
    attach_menu_action(builder, "mi_report_daily", G_CALLBACK(show_report), DAILY_REPORT);
    attach_menu_action(builder, "mi_report_status", G_CALLBACK(show_report), STATUS_REPORT);
    attach_menu_action(builder, "mi_report_todo", G_CALLBACK(show_report), TODO_REPORT);
    attach_menu_action(builder, "mi_report_invoice", G_CALLBACK(show_report), INVOICE_REPORT);
    attach_menu_action(builder, "mi_report_query", G_CALLBACK(show_report), QUERY_REPORT);
    attach_menu_action(builder, "mi_report_primer", G_CALLBACK(show_report), PRIMER_REPORT);
    attach_menu_action(builder, "mi_report_new", G_CALLBACK(new_report), NULL);
    attach_menu_action(builder, "mi_report_edit", G_CALLBACK(report_menu_edit), NULL);

    // Timer menu actions.
    attach_menu_action(builder, "mi_timer_start", G_CALLBACK(menu_start_timer), NULL);
    attach_menu_action(builder, "mi_timer_stop", G_CALLBACK(menu_stop_timer), NULL);
    attach_menu_action(builder, "mi_timer_toggle", G_CALLBACK(menu_toggle_timer), NULL);

    // Help menu actions.
    attach_menu_action(builder, "mi_help_contents", G_CALLBACK(gtt_help_popup), NULL);
    attach_menu_action(builder, "mi_help_about", G_CALLBACK(about_box), NULL);
}

/* Global: the user-defined reports pull-down menu */
static GnomeUIInfo *reports_menu = NULL;

GnomeUIInfo *gtt_get_reports_menu(void)
{
    return (reports_menu);
}

void gtt_set_reports_menu(GnomeApp *app, GnomeUIInfo *new_menus)
{
    int i;
    char *path;

    /* Build the i18n menu path ... */
    /* (is this right ??? or is this pre-i18n ???) */
    path = g_strdup_printf("%s/<Separator>", _("Reports"));

    /* If there are old menu items, remove them and free them. */
    if (reports_menu)
    {
        int nreports;
        for (i = 0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++)
        {
        }
        nreports = i;
        gnome_app_remove_menu_range(app, path, 1, nreports);

        if (new_menus != reports_menu)
        {
            for (i = 0; i < nreports; i++)
            {
                // XXX can't free this, since 'append' recycles old pointers !!
                // there's probably a minor memory leak here ...
                // gtt_plugin_free(reports_menu[i].user_data);
            }
            g_free(reports_menu);
        }
    }

    /* Now install the new menu items. */
    reports_menu = new_menus;
    if (!reports_menu)
    {
        reports_menu = g_new0(GnomeUIInfo, 1);
        reports_menu[0].type = GNOME_APP_UI_ENDOFINFO;
    }

    /* fixup */
    for (i = 0; GNOME_APP_UI_ENDOFINFO != reports_menu[i].type; i++)
    {
        reports_menu[i].moreinfo = invoke_report;
    }
    gnome_app_insert_menus(app, path, reports_menu);
}

/* ============================================================ */
/* Slide a new menu entry into first place */

void gtt_reports_menu_prepend_entry(GnomeApp *app, GnomeUIInfo *new_entry)
{
    int i, nitems;
    GnomeUIInfo *current_sysmenu, *new_sysmenu;

    current_sysmenu = gtt_get_reports_menu();
    for (i = 0; GNOME_APP_UI_ENDOFINFO != current_sysmenu[i].type; i++)
    {
    }
    nitems = i + 1;

    new_sysmenu = g_new0(GnomeUIInfo, nitems + 1);
    new_sysmenu[0] = *new_entry;

    memcpy(&new_sysmenu[1], current_sysmenu, nitems * sizeof(GnomeUIInfo));
    gtt_set_reports_menu(app, new_sysmenu);
}

/* ============================================================ */

void menus_add_plugins(GnomeApp *app)
{
    gtt_set_reports_menu(app, reports_menu);
}

void menu_set_states(void)
{
    if (menu_builder == NULL)
        return;

    GtkWidget * mi_paste_project;
    GtkWidget * mi_timer_start, * mi_timer_stop, * mi_timer_toggle;

    mi_timer_start = GTK_WIDGET(gtk_builder_get_object(menu_builder, "mi_timer_start"));
    mi_timer_stop = GTK_WIDGET(gtk_builder_get_object(menu_builder, "mi_timer_stop"));
    mi_timer_toggle = GTK_WIDGET(gtk_builder_get_object(menu_builder, "mi_timer_toggle"));

    gtk_widget_set_sensitive(mi_timer_toggle, 1);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi_timer_toggle), timer_is_running());

    /* XXX would be nice to change this menu entry to say
     * 'timer stopped' when the timer is stopped.  But don't
     * know how to change the menu label in gtk */
    /* Note (Oskar): The proposed "timer stopped" is not ideal.
     * The common pattern for a check menu item is that the label describes the state
     * that would apply if the item is active. "timer running" is not good either,
     * as the label makes it sound like the timer is running even if it isn't. Especially
     * if the theme designer didn't feel the need to visualize a non-active check mark.
     * The label "Run timer" would work better and match common patterns such
     * as e.g. "Show toolbar".
     *
     * But also - why do we have Start, Stop, and a toogle as three separate menu items?
     * Probably at least one too many. */

    gtk_widget_set_sensitive(mi_timer_start, (FALSE == timer_is_running()));
    gtk_widget_set_sensitive(mi_timer_stop, (timer_is_running()));

    mi_paste_project = GTK_WIDGET(gtk_builder_get_object(menu_builder, "mi_paste_project"));
    gtk_widget_set_sensitive(mi_paste_project, (have_cutted_project()));

    if (menu_popup[MENU_POPUP_CUT_POS].widget)
    {
        gtk_widget_set_sensitive(
            menu_popup[MENU_POPUP_PASTE_POS].widget, (have_cutted_project())
        );
    }
}

/* ======================= END OF FILE ===================== */
