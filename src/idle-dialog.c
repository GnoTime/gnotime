/*   Keyboard inactivity timout dialog for GTimeTracker - a time tracker
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
 *   Copyright (C) 2007 Goedson Paixao <goedson@debian.org>
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

#include <glib.h>
#include <glade/glade.h>
#include <gdk/gdkx.h>
#include <gnome.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include <qof.h>

#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "idle-dialog.h"
#include "dialog.h"
#include "proj.h"
#include "util.h"
#include "app.h"


int config_idle_timeout = -1;

struct GttIdleDialog_s 
{
	GladeXML    *gtxml;
	GtkDialog   *dlg;
	GtkButton   *yes_btn;
	GtkButton   *no_btn;
	GtkButton   *help_btn;
	GtkLabel    *idle_label;
	GtkLabel    *credit_label;
	GtkLabel    *time_label;
	GtkRange    *scale;

	Display     *display;
	gboolean    xss_extension_supported;
	XScreenSaverInfo *xss_info;
	guint       timeout_event_source;

	gboolean    visible;

	GttProject  *prj;
	time_t      last_activity;
	time_t      previous_credit;
};

static gboolean idle_timeout_func (gpointer data);

static void
schedule_idle_timeout (gint timeout, GttIdleDialog *idle_dialog)
{
	if (idle_dialog->timeout_event_source != 0)
	{
		g_source_remove (idle_dialog->timeout_event_source);
	}
	if (timeout > 0 && idle_dialog->xss_extension_supported)
	{
		/* If we already have an idle timeout
		 * sceduled, cancel it.
		 */
		idle_dialog->timeout_event_source = g_timeout_add_seconds (timeout, idle_timeout_func, idle_dialog);
	}
}

static gboolean
idle_timeout_func (gpointer data)
{
	GttIdleDialog *idle_dialog = (GttIdleDialog *) data;
	GdkWindow *gdk_window = gtk_widget_get_root_window (app_window);
	XID drawable = gdk_x11_drawable_get_xid (GDK_DRAWABLE(gdk_window));
	Status xss_query_ok = XScreenSaverQueryInfo (idle_dialog->display,
												 drawable,
												 idle_dialog->xss_info);
	if (xss_query_ok)
	{
		int idle_seconds = idle_dialog->xss_info->idle / 1000;
		if (cur_proj != NULL &&
			config_idle_timeout > 0 &&
			idle_seconds >= config_idle_timeout)
		{
			time_t now = time(0);
			idle_dialog->last_activity = now - idle_seconds;
			show_idle_dialog (idle_dialog);
			/* schedule a new timeout for one minute ahead to
			   update the dialog. */
			schedule_idle_timeout (60,  idle_dialog);
		}
		else
		{
			if (cur_proj != NULL)
			{
				schedule_idle_timeout (config_idle_timeout - idle_seconds, idle_dialog);
			}
			else if (idle_dialog->visible)
			{
				raise_idle_dialog (idle_dialog);
				schedule_idle_timeout (60, idle_dialog);
			}
		}
	}
	return FALSE;
}


/* =========================================================== */

static void
help_cb (GObject *obj, GttIdleDialog *dlg)
{
	gtt_help_popup (GTK_WIDGET(dlg->dlg), "idletimer");
}

/* =========================================================== */

static void
dialog_close (GObject *obj, GttIdleDialog *dlg)
{
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
	dlg->visible = FALSE;
}

/* =========================================================== */

static void
dialog_kill (GObject *obj, GttIdleDialog *dlg)
{
	gtk_widget_destroy (GTK_WIDGET(dlg->dlg));
	dlg->dlg = NULL;
	dlg->gtxml = NULL;
	dlg->visible = FALSE;
}

/* =========================================================== */

static void
restart_proj (GObject *obj, GttIdleDialog *dlg)
{
	dlg->last_activity = time(0);  /* bug fix, sometimes events are lost */
	ctree_start_timer (dlg->prj);
	dialog_kill (obj, dlg);
}

/* =========================================================== */

static void
adjust_timer (GttIdleDialog *dlg, time_t adjustment)
{
	GttInterval *ivl;
	time_t stop;
	
	ivl = gtt_project_get_first_interval (dlg->prj);
	stop = gtt_interval_get_stop (ivl);
	stop -= dlg->previous_credit;
	stop += adjustment;
	gtt_interval_set_stop (ivl, stop);

	dlg->previous_credit = adjustment;
}

/* =========================================================== */
/* Gnome Pango is picky about having bare ampersands in text.
 * Escape the ampersands into proper html.
 * Basically, replace & by &amp; unless its already &amp; 
 * free the returned string when done.
 */

static char *
util_escape_html_markup (const char *str)
{
	char * p;
	char * ret;

    if (str == NULL) return g_strdup("");

	p = strchr (str, '&');
	if (!p) return g_strdup (str);

	/* count number of ampersands */
	int ampcnt = 0;
	while (p)
	{
		ampcnt ++;
		p = strchr (p+1, '&');
	}
	
	/* make room for the escapes */
	int len = strlen(str);
	ret = g_new0 (char, len+4*ampcnt+1);
	
	/* replace & by &amp; unless its already &amp; */
	p = strchr (str, '&');
	const char *start = str;
	while (p)
	{
		strncat (ret, start, p-start);
		if (strncmp (p, "&amp;", 5))
		{
			strcat (ret, "&amp;");
		}
		else
		{
			strcat (ret, "&");
		}
		start = p+1;
		p = strchr (start, '&');
	}
	strcat (ret, start);
	return ret;
}

/* =========================================================== */

static void
display_value (GttIdleDialog *dlg, time_t credit)
{
	char tbuff [30];
	char mbuff [130];
	char * msg;
	time_t now = time(0);
	time_t idle_time;
	
	/* Set a value for the thingy under the slider */
	if (3600 > credit)
	{
		qof_print_minutes_elapsed_buff (tbuff, 30, credit, TRUE);
		g_snprintf (mbuff, 130, _("%s minutes"), tbuff);
	}
	else 
	{
		qof_print_hours_elapsed_buff (tbuff, 30, credit, FALSE);
		g_snprintf (mbuff, 130, _("%s hours"), tbuff);
	}
	gtk_label_set_text (dlg->time_label, mbuff);
	
	/* Set a value in the main message; show hours,
	 * or minutes, as is more appropriate.
	 */
	if (3600 > credit)
	{
		msg = g_strdup_printf (
		         _("The timer will be credited "
		           "with %ld minutes since the last keyboard/mouse "
		           "activity.  If you want to change the amount "
		           "of time credited, use the slider below to "
		           "adjust the value."),
		           (credit+30)/60);
	}
	else
	{
		msg = g_strdup_printf (
		         _("The timer will be credited "
		           "with %s hours since the last keyboard/mouse "
		           "activity.  If you want to change the amount "
		           "of time credited, use the slider below to "
		           "adjust the value."),
		           tbuff);
	}
	gtk_label_set_text (dlg->credit_label, msg);
	g_free (msg);

	/* Update the total elapsed time part of the message */
	idle_time = now - dlg->last_activity;
	
	char *ptitle = util_escape_html_markup (
	                            gtt_project_get_title(dlg->prj));
	char *pdesc = util_escape_html_markup (
	                            gtt_project_get_desc(dlg->prj));
	if (3600 > idle_time)
	{
		msg = g_strdup_printf (
			_("The keyboard and mouse have been idle "
			  "for %ld minutes.  The currently running "
			  "project, <b><i>%s - %s</i></b>, "
			  "has been stopped. "
			  "Do you want to restart it?"),
			(idle_time+30)/60, ptitle, pdesc);
	}
	else
	{
		msg = g_strdup_printf (
			_("The keyboard and mouse have been idle "
			  "for %ld:%02ld hours.  The currently running "
			  "project (%s - %s) "
			  "has been stopped. "
			  "Do you want to restart it?"),
			(idle_time+30)/3600, ((idle_time+30)/60)%60,
		   ptitle, pdesc);
	}
	
	gtk_label_set_markup (dlg->idle_label, msg);
	g_free (msg);
	g_free (ptitle);
	g_free (pdesc);
}

/* =========================================================== */

static void
value_changed (GObject *obj, GttIdleDialog *dlg)
{
	double slider_value;
	time_t credit;
	time_t now = time(0);

	slider_value = gtk_range_get_value (dlg->scale);
	slider_value /= 90.0;
	slider_value *= (now - dlg->last_activity);

	credit = (time_t) slider_value;
	
	display_value (dlg, credit);  /* display value in GUI */
	adjust_timer (dlg, credit);   /* change value in data store */
}

/* =========================================================== */
/* XXX the new GtkDialog is broken; it can't hide-on-close,
 * unlike to old, deprecated GnomeDialog.  Thus, we have to
 * do a heavyweight re-initialization each time.  Urgh.
 */

static void
idle_dialog_realize (GttIdleDialog * id)
{
	GladeXML *gtxml;

	id->prj = NULL;

	gtxml = gtt_glade_xml_new ("glade/idle.glade", "Idle Dialog");
	id->gtxml = gtxml;

	id->dlg = GTK_DIALOG (glade_xml_get_widget (gtxml, "Idle Dialog"));

	id->yes_btn = GTK_BUTTON(glade_xml_get_widget (gtxml, "yes button"));
	id->no_btn  = GTK_BUTTON(glade_xml_get_widget (gtxml, "no button"));
	id->help_btn = GTK_BUTTON(glade_xml_get_widget (gtxml, "helpbutton1"));
	id->idle_label = GTK_LABEL (glade_xml_get_widget (gtxml, "idle label"));
	id->credit_label = GTK_LABEL (glade_xml_get_widget (gtxml, "credit label"));
	id->time_label = GTK_LABEL (glade_xml_get_widget (gtxml, "time label"));
	id->scale = GTK_RANGE (glade_xml_get_widget (gtxml, "scale"));

	g_signal_connect(G_OBJECT(id->dlg), "destroy",
	          G_CALLBACK(dialog_close), id);

	g_signal_connect(G_OBJECT(id->yes_btn), "clicked",
	          G_CALLBACK(restart_proj), id);

	g_signal_connect(G_OBJECT(id->no_btn), "clicked",
	          G_CALLBACK(dialog_kill), id);

	g_signal_connect(G_OBJECT(id->help_btn), "clicked",
	          G_CALLBACK(help_cb), id);

	g_signal_connect(G_OBJECT(id->scale), "value_changed",
	          G_CALLBACK(value_changed), id);

}

/* =========================================================== */

GttIdleDialog *
idle_dialog_new (void)
{
	GttIdleDialog *id;

	id = g_new0 (GttIdleDialog, 1);
	id->prj = NULL;

	id->gtxml = NULL;

	gchar *display_name = gdk_get_display ();
	id->display = XOpenDisplay (display_name);
	if (id->display == NULL)
	{
		g_warning ("Could not open display %s", display_name);
	}
	else
	{
		int xss_events, xss_error;
		id->xss_extension_supported = XScreenSaverQueryExtension (id->display, &xss_events, &xss_error);
		if (id->xss_extension_supported)
		{
			id->xss_info = XScreenSaverAllocInfo ();
			if (config_idle_timeout > 0)
			{
				schedule_idle_timeout (config_idle_timeout, id);
			}
		}
		else
		{
			g_warning (_("The XScreenSaver is not supported on this display.\n"
						 "The idle timeout functionality will not be available."));
		}
	}
	g_free(display_name);
	
	return id;
}

/* =========================================================== */

void 
show_idle_dialog (GttIdleDialog *id)
{
	time_t now;
	time_t idle_time;
	GttProject *prj = cur_proj;

	if (!id) return;
	if (0 > config_idle_timeout) return;
	if (!prj) return;

	now = time(0);
	idle_time = now - id->last_activity;

	/* Due to GtkDialog broken-ness, re-realize the GUI */
	if (NULL == id->gtxml)
	{
		idle_dialog_realize (id);
	}

	/* Stop the timer on the current project */
	ctree_stop_timer (cur_proj);
	id->prj = prj;

	/* The idle timer can trip because gtt was left running
	 * on a laptop, which was them put in suspend mode (i.e.
	 * by closing the cover).  When the laptop is resumed,
	 * the poll_last_activity will return the many hours/days
	 * that the laptop has been shut down, and merely stoping
	 * the timer (as above) will credit hours/days to the 
	 * current active project.  We don't want this, we need
	 * to undo this damage.
	 */
	id->previous_credit = idle_time;
	adjust_timer (id, config_idle_timeout);


	raise_idle_dialog (id);
}

/* =========================================================== */

void 
raise_idle_dialog (GttIdleDialog *id)
{
	g_return_if_fail(id);
	g_return_if_fail(id->gtxml);

	/* Now, draw the messages in the GUI popup. */
	display_value (id, config_idle_timeout);


	/* The following will raise the window, and put it on the current
	 * workspace, at least if the metacity WM is used. Haven't tried
	 * other window managers.
	 */

	gtk_window_present (GTK_WINDOW (id->dlg));
	id->visible = TRUE;
}

void
idle_dialog_activate_timer (GttIdleDialog *idle_dialog)
{
	schedule_idle_timeout (config_idle_timeout, idle_dialog);
}

void
idle_dialog_deactivate_timer (GttIdleDialog *idle_dialog)
{
	if (idle_dialog->timeout_event_source != 0)
	{
		g_source_remove (idle_dialog->timeout_event_source);
		idle_dialog->timeout_event_source = 0;
	}
}

gboolean
idle_dialog_is_visible(GttIdleDialog *idle_dialog)
{
	return idle_dialog->visible;
}

/* =========================== END OF FILE ============================== */
