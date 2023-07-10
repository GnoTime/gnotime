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

static const char * gtt_is_custom_report = "gtt_is_custom_report";


static GtkBuilder * menu_builder;


static void attach_menu_action(GtkBuilder * builder, const char * item_name,
                               GCallback callback, void * user_data)
{
    GtkWidget *item = GTK_WIDGET(gtk_builder_get_object(builder, item_name));

    g_signal_connect(item, "activate", callback, user_data);
}


GtkMenuShell *menus_get_popup(void)
{
    GtkBuilder *builder = menu_builder;

    static GtkMenuShell * menu = NULL;

    if (menu)
        return menu;

    menu = GTK_MENU_SHELL(gtk_builder_get_object(builder, "menu_popup_project_list"));

    attach_menu_action(builder, "mi_projpopup_activity", G_CALLBACK(show_report), ACTIVITY_REPORT);
    attach_menu_action(builder, "mi_projpopup_edittimes", G_CALLBACK(menu_howto_edit_times), NULL);
    attach_menu_action(builder, "mi_projpopup_newentry", G_CALLBACK(new_task_ui), NULL);
    attach_menu_action(builder, "mi_projpopup_editentry", G_CALLBACK(edit_task_ui), NULL);
    attach_menu_action(builder, "mi_projpopup_cut", G_CALLBACK(cut_project), NULL);
    attach_menu_action(builder, "mi_projpopup_copy", G_CALLBACK(copy_project), NULL);
    attach_menu_action(builder, "mi_projpopup_paste", G_CALLBACK(paste_project), NULL);
    attach_menu_action(builder, "mi_projpopup_props", G_CALLBACK(menu_properties), NULL);

    return menu;
}

void menus_create(GnomeApp *app)
{
    GtkBuilder *builder;
    builder = gtt_builder_new_from_file("ui/mainmenu.ui");
    menu_builder = builder;

    menus_get_popup(); /* initialize it */

    GtkMenuBar * menubar = GTK_MENU_BAR(gtk_builder_get_object(builder, "mainmenu"));
    gnome_app_set_menus(app, menubar);

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

/* Global: the user-defined reports pull-down menu (array) */
static GttPlugin *reports_menu = NULL;

GttPlugin *gtt_get_reports_menu(void)
{
    return (reports_menu);
}

static void gtt_append_custom_reports(GnomeApp *app, GttPlugin *report_menu_items)
{
    GtkMenuShell * reports_menu = GTK_MENU_SHELL(gtk_builder_get_object(menu_builder, "menu_report"));
    int i;

    for (i = 0; NULL != report_menu_items[i].name; i++)
    {
        GttPlugin * current = & (report_menu_items[i]);

        // It seems our UI to create these custom reports (menu items) does
        // not actually allow setting anything other than name and tooltip,
        // using the default item type. So it's enough to support that. (Even though
        // the plugin editor UI seems to contain widgetry that implies more, it doesn't
        // actually work.
        GtkWidget * item = gtk_menu_item_new_with_mnemonic(current->name);
        gtk_widget_set_tooltip_text(item, current->tooltip);

        g_signal_connect(item, "activate", G_CALLBACK(invoke_report), current);
            
        g_object_set_data(G_OBJECT(item), gtt_is_custom_report, (gpointer)gtt_is_custom_report);

        gtk_widget_show(item);
        gtk_menu_shell_append(reports_menu, item);
    }
}


void gtt_set_reports_menu(GnomeApp *app, GttPlugin *new_menus)
{
    int i;

    // If there are old menu items, remove them and free them.
    if (reports_menu)
    {
        // Iterate over items in the report menu, and destroy those that are
        // marked as custom entries.
        GtkMenuShell * reports_menu_widget;
        reports_menu_widget = GTK_MENU_SHELL(gtk_builder_get_object(menu_builder, "menu_report"));
        GList * reports_menu_items = gtk_container_get_children(GTK_CONTAINER(reports_menu_widget));
        GList * elem;
        for (elem = reports_menu_items; elem; elem = elem->next)
        {
            GtkWidget * item = elem->data;

            if (g_object_get_data(G_OBJECT(item), gtt_is_custom_report) == gtt_is_custom_report)
                gtk_widget_destroy(item);
        }
        g_list_free(reports_menu_items);

        // Free the old item meta data array.
        if (new_menus != reports_menu)
        {
            for (i = 0; NULL != reports_menu[i].name; i++)
            {
                // XXX can't free this, since 'append' recycles old pointers !!
                // there's probably a minor memory leak here ...
                // gtt_plugin_free(reports_menu[i].user_data);
            }
            g_free(reports_menu);
        }
    }

    // Replace NULL with a single end-of-array record if needed.
    reports_menu = new_menus;
    if (!reports_menu)
    {
        reports_menu = g_new0(GttPlugin, 1);
    }

    // Now install the new menu items.
    gtt_append_custom_reports(app, reports_menu);
}

/* ============================================================ */
/* Slide a new menu entry into first place */

void gtt_reports_menu_prepend_entry(GnomeApp *app, GttPlugin *new_entry)
{
    int i, nitems;
    GttPlugin *current_sysmenu, *new_sysmenu;

    current_sysmenu = gtt_get_reports_menu();
    for (i = 0; NULL != current_sysmenu[i].name; i++)
    {
    }
    nitems = i + 1;

    new_sysmenu = g_new0(GttPlugin, nitems + 1);
    new_sysmenu[0] = *new_entry;

    memcpy(&new_sysmenu[1], current_sysmenu, nitems * sizeof(GttPlugin));
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

    GtkWidget * mi_paste_project, * mi_projpopup_paste;
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

    mi_projpopup_paste = GTK_WIDGET(gtk_builder_get_object(menu_builder, "mi_projpopup_paste"));
    gtk_widget_set_sensitive(mi_projpopup_paste, (have_cutted_project()));
}

/* ======================= END OF FILE ===================== */
