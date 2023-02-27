/*   GnoTime Help Browser helper function
 *   Copyright (C) 2004 Linas Vepstas <linas@linas.org>
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

#include "dialog.h"

#include <string.h>

/* ================================================================= */

void gtt_help_popup(GtkWidget *widget, gpointer data)
{
    const gchar *const section = data;

    gchar *uri = NULL;
    if ((NULL != section) && (0 != strcmp("", section)))
    {
        uri = g_strdup_printf("ghelp:gnotime#%s", section);
    }
    else
    {
        uri = g_strdup("ghelp:gnotime");
    }

    GError *err = NULL;
    const gboolean ret = gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &err);

    g_free(uri);
    uri = NULL;

    if (FALSE == ret)
    {
        GtkWidget *mb;
        mb = gtk_message_dialog_new(
            GTK_IS_WINDOW(widget) ? GTK_WINDOW(widget) : NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", err->message
        );
        g_signal_connect(G_OBJECT(mb), "response", G_CALLBACK(gtk_widget_destroy), mb);
        gtk_widget_show(mb);

        g_warning("Failed to open documentation: %s", err->message);

        g_error_free(err);
        err = NULL;
    }
}

/* ==================== END OF FILE ================================ */
