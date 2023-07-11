/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2002,2003 Linas Vepstas <linas@linas.org>
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

#include <string.h>
#include <glib/gi18n.h>

#include "app.h"
#include "dialog.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"

typedef struct _MyToolbar MyToolbar;

struct _MyToolbar
{
    GtkToolbar *tbar;
    GtkToolbar *null_tbar;
    GtkWidget *new_w;
    GtkWidget *cut, *copy, *paste; /* to make them sensible as needed */
    GtkWidget *journal_button;
    GtkWidget *prop_w;
    GtkWidget *timer_button;
    GtkImage *timer_button_image;
    GtkWidget *pref;
    GtkWidget *help;
    GtkWidget *exit;

    int spa;
    int spb;
    int spc;
};

MyToolbar *mytbar = NULL;

/* ================================================================= */
/* This routine updates the appearence/behaviour of the toolbar.
 * In particular, the 'paste' button becomes active when there
 * is something to paste, and the timer button toggles it's
 * image when a project timer is started/stopped.
 */

void toolbar_set_states(void)
{
    g_return_if_fail(mytbar != NULL);
    g_return_if_fail(mytbar->tbar != NULL);
    g_return_if_fail(GTK_IS_TOOLBAR(mytbar->tbar));

    if (config_show_toolbar)
    {
        update_toolbar_sections();
    }
    else
    {
        /* Rebuild the toolbar so that it is really hidden. There should be
           a better way of doing this.
        */
        update_toolbar_sections();
        return;
    }

    /* TODO: Investigate if GTK>=3 offers a way to selectively en-/disable tooltips
    if (mytbar->tbar && mytbar->tbar->tooltips)
    {
        if (config_show_tb_tips)
            gtk_tooltips_enable(mytbar->tbar->tooltips);
        else
            gtk_tooltips_disable(mytbar->tbar->tooltips);
    }
    */

    if (mytbar->paste)
    {
        gtk_widget_set_sensitive(mytbar->paste, have_cutted_project());
    }

    if (mytbar->timer_button_image)
    {
        gtk_image_set_from_stock(
            mytbar->timer_button_image,
            ((timer_is_running()) ? GTK_STOCK_MEDIA_STOP : GTK_STOCK_MEDIA_RECORD),
            GTK_ICON_SIZE_LARGE_TOOLBAR
        );
    }
}

static GtkWidget *toolbar_insert_item(
    GtkToolbar *toolbar, const gchar *text, const gchar *tooltip_text,
    GtkWidget *icon, GCallback callback, gpointer user_data, gint pos)
{
    GtkToolItem *item;

    item = gtk_tool_button_new(icon, text);
    gtk_tool_item_set_tooltip_text(item, tooltip_text);

    // Show both text and label to match existing behaviour.
    gtk_tool_item_set_is_important(item, TRUE);

    g_signal_connect(G_OBJECT(item), "clicked", callback, user_data);

    gtk_widget_show(GTK_WIDGET(item));
    gtk_widget_show(GTK_WIDGET(icon));

    gtk_toolbar_insert(toolbar, item, pos);

    return GTK_WIDGET(item);
}

static GtkWidget *toolbar_insert_item_stock(
    GtkToolbar *toolbar, const gchar *stock_id, const gchar *tooltip_text,
    GCallback callback, gpointer user_data, gint pos)
{
    GtkToolItem *item;

    item = gtk_tool_button_new_from_stock(stock_id);
    gtk_tool_item_set_tooltip_text(item, tooltip_text);

    // Show both text and label to match existing behaviour.
    gtk_tool_item_set_is_important(item, TRUE);

    g_signal_connect(G_OBJECT(item), "clicked", callback, user_data);

    gtk_widget_show(GTK_WIDGET(item));
    gtk_toolbar_insert(toolbar, item, pos);

    return GTK_WIDGET(item);
}

static GtkWidget *toolbar_append_separator(GtkToolbar *toolbar)
{
    GtkToolItem *item;

    item = gtk_separator_tool_item_new();

    gtk_widget_show(GTK_WIDGET(item));
    gtk_toolbar_insert(toolbar, item, -1);

    return GTK_WIDGET(item);
}

/* ================================================================= */
/* A small utility routine to use a stock image with custom text,
 * and put the whole thing into the toolbar
 */
static GtkWidget *toolbar_append_stock_button(
    GtkToolbar *toolbar, const gchar *text, const gchar *tooltip_text,
    const gchar *stock_icon_id, GCallback callback, gpointer user_data, gint pos
)
{
    GtkWidget *w, *image;

    image = gtk_image_new_from_stock(stock_icon_id, GTK_ICON_SIZE_LARGE_TOOLBAR);

    w = toolbar_insert_item(toolbar, text, tooltip_text, image, callback, user_data, pos);
    return w;
}

/* ================================================================= */
/* Assemble the buttons in the toolbar.  Which ones
 * are visible depends on the config settings.
 * Returns a pointer to the (still hidden) GtkToolbar
 */

GtkWidget *build_toolbar(void)
{
    int position = 0;
    if (!mytbar)
    {
        mytbar = g_malloc0(sizeof(MyToolbar));
        mytbar->tbar = GTK_TOOLBAR(gtk_toolbar_new());
        mytbar->null_tbar = GTK_TOOLBAR(gtk_toolbar_new());
        gtk_toolbar_set_style(mytbar->tbar, GTK_TOOLBAR_BOTH_HORIZ);
    }

    if (config_show_toolbar)
    {
        if (config_show_tb_new)
        {
            mytbar->new_w = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_NEW, _("Create a New Project..."),
                G_CALLBACK(new_project), NULL, position++
            );
            toolbar_append_separator(mytbar->tbar);
            mytbar->spa = position;
            position++;
        }
        if (config_show_tb_ccp)
        {
            mytbar->cut = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_CUT, _("Cut Selected Project"),
                G_CALLBACK(cut_project), NULL, position++
            );
            mytbar->copy = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_COPY, _("Copy Selected Project"),
                G_CALLBACK(copy_project), NULL, position++
            );
            mytbar->paste = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_PASTE, _("Paste Project"),
                G_CALLBACK(paste_project), NULL, position++
            );
            toolbar_append_separator(mytbar->tbar);
            mytbar->spb = position;
            if (mytbar->spa)
                mytbar->spb--;
            position++;
        }
        if (config_show_tb_journal)
        {
            /* There is no true 'stock' item for journal, so
             * instead we draw our own button, and use a stock
             * image. */
            mytbar->journal_button = toolbar_append_stock_button(
                mytbar->tbar, _("Activity Journal"), _("View and Edit Timestamp Logs"),
                GTK_STOCK_INDEX, G_CALLBACK(show_report), ACTIVITY_REPORT, position
            );
            position++;
        }
        if (config_show_tb_prop)
        {
            mytbar->prop_w = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_PROPERTIES, _("Edit Project Properties..."),
                G_CALLBACK(menu_properties), NULL, position++
            );
        }
        if (config_show_tb_timer)
        {
            /* There is no true 'stock' item for timer, so
             * instead we draw our own button, and use a
             * pair of stock images to toggle between.
             */
            mytbar->timer_button_image = GTK_IMAGE(gtk_image_new());
            gtk_image_set_from_stock(
                mytbar->timer_button_image, GTK_STOCK_MEDIA_RECORD, GTK_ICON_SIZE_LARGE_TOOLBAR
            );

            mytbar->timer_button = toolbar_insert_item(
                mytbar->tbar, _("Timer"), _("Start/Stop Timer"),
                GTK_WIDGET(mytbar->timer_button_image), G_CALLBACK(menu_toggle_timer), NULL, position
            );
            position++;
        }
        if (((config_show_tb_timer) || (config_show_tb_journal) || (config_show_tb_prop))
            && ((config_show_tb_pref) || (config_show_tb_help) || (config_show_tb_exit)))
        {
            toolbar_append_separator(mytbar->tbar);
            mytbar->spc = position;
            if (mytbar->spa)
                mytbar->spc--;
            if (mytbar->spb)
                mytbar->spc--;
            position++;
        }
        if (config_show_tb_pref)
        {
            mytbar->pref = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_PREFERENCES, _("Edit Preferences..."),
                G_CALLBACK(menu_options), NULL, position++
            );
        }
        if (config_show_tb_help)
        {
            mytbar->help = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_HELP, _("User's Guide and Manual"),
                G_CALLBACK(gtt_help_popup), NULL, position++
            );
        }
        if (config_show_tb_exit)
        {
            mytbar->exit = toolbar_insert_item_stock(
                mytbar->tbar, GTK_STOCK_QUIT, _("Quit GnoTime"), G_CALLBACK(app_quit),
                NULL, position++
            );
        }
    }
    return GTK_WIDGET(mytbar->tbar);
}

/* ================================================================= */
/* TODO: I have to completely rebuild the toolbar, when I want to add or
   remove items. There should be a better way now */

#define ZAP(w)                          \
    if (w)                              \
    {                                   \
        (w) = NULL;                     \
    }

#define ZING(pos)                                      \
    if (pos)                                           \
    {                                                  \
        (pos) = 0;                                     \
    }

void update_toolbar_sections(void)
{
    GtkContainer *tbc;
    GtkWidget *tb;

    if (!app_window)
        return;
    if (!mytbar)
        return;

    tbc = GTK_CONTAINER(mytbar->tbar);

    gtk_container_foreach(tbc, (void*) gtk_widget_destroy, NULL);

    ZING(mytbar->spa);
    ZING(mytbar->spb);
    ZING(mytbar->spc);

    ZAP(mytbar->new_w);
    ZAP(mytbar->cut);
    ZAP(mytbar->copy);
    ZAP(mytbar->paste);
    ZAP(mytbar->journal_button);
    ZAP(mytbar->prop_w);
    ZAP(mytbar->timer_button);
    ZAP(mytbar->pref);
    ZAP(mytbar->help);
    ZAP(mytbar->exit);

    tb = build_toolbar();
    gtk_widget_show(tb);
}

/* ======================= END OF FILE ======================= */
