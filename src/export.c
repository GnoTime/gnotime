/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002 Linas Vepstas <linas@linas.org>
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
#include "ghtml.h"
#include "proj.h"

/* Project data export */

#define gtt_sure_string(x) ((x)?(x):"")

/* ======================================================= */

typedef struct export_format_s export_format_t;

struct export_format_s 
{
	GtkFileSelection *picker;    /* URI picker (file selection) */
	const char       *uri;       /* aka filename */
	FILE             *fp;        /* file handle */
};

static export_format_t *
export_format_new (void)
{
	export_format_t * rc;
	rc = g_new (export_format_t, 1);
	rc->picker = NULL;
	rc->uri = NULL;
	return rc;
}

/* ======================================================= */
/* XXXX
 * This project printer should be replaced by a guile-based 
 * thingy in parallel to the ghtml stuff.
 */

static void 
export_write (GttGhtml *gxp, const char *str, size_t len, 
					 export_format_t *xp)
{
	printf ("duuude %s\n", str);
	fprintf (xp->fp, "%s", str);
}

static void 
export_err (GttGhtml *gxp, int errcode, const char *msg,
					 export_format_t *xp)
{
	printf ("duuude uhh oh %s\n", msg);
}

static gint
export_projects (export_format_t *xp)
{
	GList *node;
	GttGhtml *gxp;

	gxp = gtt_ghtml_new();
	gtt_ghtml_set_stream (gxp, xp, NULL, (GttGhtmlWriteStream) export_write,
						 NULL, (GttGhtmlError) export_err);

	for (node = gtt_get_project_list(); node; node = node->next) 
	{
	
		GttProject *prj = node->data;
		gtt_ghtml_display (gxp, "/tmp/tab-delim.ghtml", prj);
	}
	gtt_ghtml_destroy (gxp);

	return 0;
}

/* ======================================================= */

static void
export_really (GtkWidget *widget, export_format_t *xp)
{
	gboolean rc;

	xp->uri = gtk_file_selection_get_filename (xp->picker);

	if (0 == access (xp->uri, F_OK)) 
	{
		GtkWidget *w;
		char *s;

		s = g_strdup_printf (_("File %s exists, overwrite?"),
				     xp->uri);
		w = gnome_question_dialog_parented (s, NULL, NULL,
						    GTK_WINDOW (xp->picker));
		g_free (s);

		if (gnome_dialog_run (GNOME_DIALOG (w)) != 0)
			return;
	}

	xp->fp = fopen (xp->uri, "w");
	if (NULL == xp->fp)
	{
		GtkWidget *w = gnome_error_dialog (_("File could not be opened"));
		gnome_dialog_set_parent (GNOME_DIALOG (w), GTK_WINDOW (xp->picker));
		return;
	}
	
	rc = export_projects (xp);
	if (rc)
	{
		GtkWidget *w = gnome_error_dialog (_("Error occured during export"));
		gnome_dialog_set_parent (GNOME_DIALOG (w), GTK_WINDOW (xp->picker));
		return;
	}

	fclose (xp->fp);
	gtk_widget_destroy (GTK_WIDGET (xp->picker));
}

/* ======================================================= */

void
export_file_picker (GtkWidget *widget, gpointer data)
{
	export_format_t *xp;
	GtkWidget *dialog;

	dialog = gtk_file_selection_new (_("Tab-Delimited Export"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

	xp = export_format_new ();
	xp->picker = GTK_FILE_SELECTION (dialog);

	gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &dialog);

	gtk_signal_connect_object (GTK_OBJECT (xp->picker->cancel_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (xp->picker));

	gtk_signal_connect (GTK_OBJECT (xp->picker->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (export_really),
			    xp);

	gtk_widget_show (dialog);
}

/* ======================= END OF FILE ======================= */
