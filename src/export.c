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
	GttGhtml         *ghtml;     /* output device */
	const char       *template;  /* output template */
};

static export_format_t *
export_format_new (void)
{
	export_format_t * rc;
	rc = g_new (export_format_t, 1);
	rc->picker = NULL;
	rc->uri = NULL;
	rc->ghtml = NULL;
	rc->template = NULL;
	return rc;
}

/* ======================================================= */
/* 
 * Print out the projects using the standard guile-based
 * printing infrastructure.
 */

static void 
export_write (GttGhtml *gxp, const char *str, size_t len, 
					 export_format_t *xp)
{
	fprintf (xp->fp, "%s", str);
}

static void 
export_err (GttGhtml *gxp, int errcode, const char *msg,
					 export_format_t *xp)
{
	GtkWidget *w;
	char *s = g_strdup_printf (_("Error exporting data: %s"), msg);
	w = gnome_error_dialog (s);
	gnome_dialog_set_parent (GNOME_DIALOG (w), GTK_WINDOW (xp->picker));
	g_free (s);
}

static gint
export_projects (export_format_t *xp)
{
	GttProject *prj;

	/* Get the currently selected project */
	prj = ctree_get_focus_project (global_ptw);
	if (!prj) return;

	xp->ghtml = gtt_ghtml_new();
	gtt_ghtml_set_stream (xp->ghtml, xp, 
						 NULL, 
						 (GttGhtmlWriteStream) export_write,
						 NULL, 
						 (GttGhtmlError) export_err);

	gtt_ghtml_display (xp->ghtml, xp->template, prj);
	
	gtt_ghtml_destroy (xp->ghtml);
	xp->ghtml = NULL;

	g_free((char *) xp->template);
	xp->template = NULL;

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

		if (0 == gnome_dialog_run (GNOME_DIALOG (w))) return;
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
	const char * template_filename = data;

	dialog = gtk_file_selection_new (_("Tab-Delimited Export"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (app_window));

	xp = export_format_new ();
	xp->picker = GTK_FILE_SELECTION (dialog);
	xp->template = gtt_ghtml_resolve_path (template_filename);

	g_signal_connect (G_OBJECT (dialog), "destroy",
			    G_CALLBACK (gtk_widget_destroyed),
			    &dialog);

	g_signal_connect_object (G_OBJECT (xp->picker->cancel_button), "clicked",
				   G_CALLBACK (gtk_widget_destroy),
				   G_OBJECT (xp->picker), 0);

	g_signal_connect (G_OBJECT (xp->picker->ok_button), "clicked",
			    G_CALLBACK (export_really),
			    xp);

	gtk_widget_show (dialog);
}

/* ======================= END OF FILE ======================= */
