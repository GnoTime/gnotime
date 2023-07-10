/*   Report Plugins for GTimeTracker - a time tracker
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
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

#include "gtt-gsettings-io.h"

#include "app.h"
#include "journal.h"
#include "menus.h"
#include "plug-in.h"
#include "util.h"

#include <gio/gio.h>

struct NewPluginDialog_s
{
    GtkBuilder *gtkbuilder;
    GtkDialog *dialog;
    GtkEntry *plugin_name;
    GtkFileChooser *plugin_path;
    GtkEntry *plugin_tooltip;
};

/* ============================================================ */

GttPlugin *gtt_plugin_new(const char *nam, const char *pth)
{
    GttPlugin *plg;

    if (!nam || !pth)
        return NULL;

    plg = g_new0(GttPlugin, 1);
    plg->name = g_strdup(nam);
    plg->path = g_strdup(pth);
    plg->tooltip = NULL;

    return plg;
}

GttPlugin *gtt_plugin_copy(GttPlugin *orig)
{
    GttPlugin *plg;

    if (!orig)
        return NULL;

    plg = g_new0(GttPlugin, 1);
    gtt_plugin_copy_to(orig, plg);

    return plg;
}

void gtt_plugin_copy_to(GttPlugin *orig, GttPlugin *target)
{
    if (!orig)
        return;

    // This will only free the contained strings, not the struct itself.
    gtt_plugin_free(target);

    target->name = NULL;
    if (orig->name)
        target->name = g_strdup(orig->name);

    target->path = NULL;
    if (orig->path)
        target->path = g_strdup(orig->path);

    target->tooltip = NULL;
    if (orig->tooltip)
        target->tooltip = g_strdup(orig->tooltip);

    target->last_url = NULL;
    if (orig->last_url)
        target->last_url = g_strdup(orig->last_url);
}

void gtt_plugin_free(GttPlugin *plg)
{
    if (!plg)
        return;
    if (plg->name)
        g_free(plg->name);
    if (plg->path)
        g_free(plg->path);
    if (plg->tooltip)
        g_free(plg->tooltip);
    if (plg->last_url)
        g_free(plg->last_url);
}

/* ============================================================ */

static void new_plugin_create_cb(GtkWidget *w, gpointer data)
{
    const char *title, *tip;
    NewPluginDialog *dlg = data;

    /* Get the dialog contents */
    title = gtk_entry_get_text(dlg->plugin_name);
    char *path = gtk_file_chooser_get_uri(dlg->plugin_path);
    tip = gtk_entry_get_text(dlg->plugin_tooltip);

    /* Do a basic sanity check */
    GFile *plugin_file = g_file_new_for_uri(path);
    const gboolean exists = g_file_query_exists(plugin_file, NULL);
    g_object_unref(plugin_file);
    plugin_file = NULL;
    if (!exists)
    {
        gchar *msg;
        GtkWidget *mb;
        msg = g_strdup_printf(_("Unable to open the report file %s\n"), path);
        mb = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", msg);
        gtk_widget_show(mb);
        /* g_free (msg);   XXX memory leak needs fixing. */
    }
    else
    {
        GttPlugin *plg;

        /* Create the plugin */
        plg = gtt_plugin_new(title, path);
        plg->tooltip = g_strdup(tip);

        /* Add the thing to the Reports menu */
        gtt_reports_menu_prepend_entry(plg);

        /* Save to file, too.  That way, if system core dumps later,
         * at least we managed to get this set of changes saved. */
        gtt_gsettings_save_reports_menu();

        /* zero-out entries, so next time user doesn't see them again */
        /* Uh, no, don't
        gtk_entry_set_text (dlg->plugin_name, "");
        gtk_entry_set_text (dlg->plugin_path, "");
        gtk_entry_set_text (dlg->plugin_tooltip, "");
        */
    }
    g_free(path);
    gtk_widget_hide(GTK_WIDGET(dlg->dialog));
}

static void new_plugin_cancel_cb(GtkWidget *w, gpointer data)
{
    NewPluginDialog *dlg = data;
    gtk_widget_hide(GTK_WIDGET(dlg->dialog));
}

static void connect_signals_cb(
    GtkBuilder *builder, GObject *object, const gchar *signal_name, const gchar *handler_name,
    GObject *connect_object, GConnectFlags flags, gpointer user_data
)
{
    if (g_strcmp0(handler_name, "on_ok_button_clicked") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(new_plugin_create_cb), user_data);

    if (g_strcmp0(handler_name, "on_cancel_button_clicked") == 0)
        g_signal_connect(object, signal_name, G_CALLBACK(new_plugin_cancel_cb), user_data);
}

/* ============================================================ */

NewPluginDialog *new_plugin_dialog_new(void)
{
    NewPluginDialog *dlg;
    GtkBuilder *builder;
    GObject *e;

    dlg = g_malloc(sizeof(NewPluginDialog));

    builder = gtt_builder_new_from_file("ui/plugin.ui");
    dlg->gtkbuilder = builder;

    dlg->dialog = GTK_DIALOG(gtk_builder_get_object(builder, "Plugin New"));

    gtk_builder_connect_signals_full(builder, connect_signals_cb, dlg);

    /* ------------------------------------------------------ */
    /* grab the various entry boxes and hook them up */
    e = gtk_builder_get_object(builder, "plugin name");
    dlg->plugin_name = GTK_ENTRY(e);

    e = gtk_builder_get_object(builder, "plugin path");
    dlg->plugin_path = GTK_FILE_CHOOSER(e);

    e = gtk_builder_get_object(builder, "plugin tooltip");
    dlg->plugin_tooltip = GTK_ENTRY(e);

    gtk_widget_hide_on_delete(GTK_WIDGET(dlg->dialog));

    return dlg;
}

/* ============================================================ */

void new_plugin_dialog_show(NewPluginDialog *dlg)
{
    if (!dlg)
        return;
    gtk_widget_show(GTK_WIDGET(dlg->dialog));
}

void new_plugin_dialog_destroy(NewPluginDialog *dlg)
{
    if (!dlg)
        return;
    gtk_widget_destroy(GTK_WIDGET(dlg->dialog));
    g_free(dlg);
}

/* ============================================================ */

static NewPluginDialog *pdlg = NULL;

void new_report(GtkWidget *widget, gpointer data)
{
    if (!pdlg)
        pdlg = new_plugin_dialog_new();
    gtk_widget_show(GTK_WIDGET(pdlg->dialog));
}

/* ====================== END OF FILE ========================= */
