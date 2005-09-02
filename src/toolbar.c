/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2002,2003 Linas Vepstas <linas@lionas.org>
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
#include "dialog.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "menus.h"
#include "myoaf.h"
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
	GtkImage  *timer_button_image;
	GtkWidget *calendar_w;
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

void
toolbar_set_states(void)
{
	g_return_if_fail(mytbar != NULL);
	g_return_if_fail(mytbar->tbar != NULL);
	g_return_if_fail(GTK_IS_TOOLBAR(mytbar->tbar));

	if (config_show_toolbar)
	{
		gtk_widget_show_all(GTK_WIDGET(mytbar->tbar));
		// gnome_app_set_toolbar(GNOME_APP(app_window), mytbar->tbar);
	}
	else
	{
		//  XXX this should hide the toolbar completely,
		//  but it doesn't, sinse some bonobo fudge is needed.
		gtk_widget_hide_all(GTK_WIDGET(mytbar->tbar));
		// gnome_app_set_toolbar(GNOME_APP(app_window), mytbar->null_tbar);
		return;
	}

	if (mytbar->tbar && mytbar->tbar->tooltips)
	{
		if (config_show_tb_tips)
			gtk_tooltips_enable(mytbar->tbar->tooltips);
		else
			gtk_tooltips_disable(mytbar->tbar->tooltips);
	}

	if (mytbar->paste)
	{
		gtk_widget_set_sensitive(mytbar->paste, have_cutted_project());
	}

	if (mytbar->timer_button_image)
	{
		gtk_image_set_from_stock (mytbar->timer_button_image,
				 ((timer_is_running()) ?
				     GNOME_STOCK_TIMER_STOP :
				     GNOME_STOCK_TIMER),
				 GTK_ICON_SIZE_LARGE_TOOLBAR);
	}
}

/* ================================================================= */
/* A small utility routine to use a stock image with custom text,
 * and put the whole thing into the toolbar
 */
static GtkWidget *
toolbar_append_stock_button (GtkToolbar *toolbar,
                             const gchar *text,
                             const gchar *tooltip_text,
                             const gchar *stock_icon_id,
                             GtkSignalFunc callback,
                             gpointer user_data)
{
	GtkWidget *w, *image;

	image = gtk_image_new_from_stock (stock_icon_id,
	                GTK_ICON_SIZE_LARGE_TOOLBAR);
	w = gtk_toolbar_append_item(toolbar, text, tooltip_text,
	                NULL, image,
	                callback, user_data);
	return w;
}

/* ================================================================= */
/* Assemble the buttons in the toolbar.  Which ones
 * are visible depends on the config settings.
 * Returns a pointer to the (still hidden) GtkToolbar
 */

GtkWidget *
build_toolbar(void)
{
	int position = 0;
	if (!mytbar)
	{
		mytbar = g_malloc0(sizeof(MyToolbar));
		mytbar->tbar = GTK_TOOLBAR(gtk_toolbar_new());
		mytbar->null_tbar = GTK_TOOLBAR(gtk_toolbar_new());
	}

	if (config_show_tb_new)
	{
		mytbar->new_w = gtk_toolbar_insert_stock (mytbar->tbar,
			GTK_STOCK_NEW,
			_("Create a New Project..."), NULL,
			(GtkSignalFunc)new_project, NULL,
			position++);
		gtk_toolbar_append_space(mytbar->tbar);
		mytbar->spa = position;
		position ++;
	}
	if (config_show_tb_ccp)
	{
		mytbar->cut = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_CUT,
				_("Cut Selected Project"), NULL,
				(GtkSignalFunc)cut_project, NULL,
				position ++);
		mytbar->copy = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_COPY,
				_("Copy Selected Project"), NULL,
				(GtkSignalFunc)copy_project, NULL,
				position ++);
		mytbar->paste = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_PASTE,
				_("Paste Project"), NULL,
				(GtkSignalFunc)paste_project, NULL,
				position ++);
		gtk_toolbar_append_space(mytbar->tbar);
		mytbar->spb = position;
		if (mytbar->spa) mytbar->spb--;
		position ++;
	}
	if (config_show_tb_journal)
	{
		/* There is no true 'stock' item for journal, so
		 * instead we draw our own button, and use a stock
		 * image. */
		mytbar->journal_button = toolbar_append_stock_button(mytbar->tbar,
				 _("Activity Journal"),
				 _("View and Edit Timestamp Logs"),
				 GNOME_STOCK_BOOK_OPEN,
				 (GtkSignalFunc) show_report, ACTIVITY_REPORT);
		position ++;
	}
	if (config_show_tb_prop)
	{
		mytbar->prop_w = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_PROPERTIES,
				_("Edit Project Properties..."), NULL,
				(GtkSignalFunc) menu_properties, NULL,
				position ++);
	}
	if (config_show_tb_timer)
	{
		/* There is no true 'stock' item for timer, so
		 * instead we draw our own button, and use a
		 * pair of stock images to toggle between.
		 */
		mytbar->timer_button_image = GTK_IMAGE(gtk_image_new());
		gtk_image_set_from_stock (mytbar->timer_button_image,
				 GNOME_STOCK_TIMER, GTK_ICON_SIZE_LARGE_TOOLBAR);

		mytbar->timer_button =
		       gtk_toolbar_append_item(mytbar->tbar,
				 _("Timer"),
				 _("Start/Stop Timer"),
				 NULL,
				 GTK_WIDGET(mytbar->timer_button_image),
				 (GtkSignalFunc) menu_toggle_timer, NULL);
		position ++;
	}
	if (config_show_tb_calendar)
	{
		mytbar->calendar_w = toolbar_append_stock_button(mytbar->tbar,
				_("Calendar"),
				_("View Calendar"),
				GNOME_STOCK_TEXT_BULLETED_LIST,
				(GtkSignalFunc)edit_calendar, NULL);
		position ++;
	}
	if (((config_show_tb_timer)    ||
	     (config_show_tb_journal)  ||
	     (config_show_tb_calendar) ||
	     (config_show_tb_prop)     ) &&
	    ((config_show_tb_pref) ||
	     (config_show_tb_help) ||
	     (config_show_tb_exit)))
	{
		gtk_toolbar_append_space(mytbar->tbar);
		mytbar->spc = position;
		if (mytbar->spa) mytbar->spc--;
		if (mytbar->spb) mytbar->spc--;
		position ++;
	}
	if (config_show_tb_pref)
	{
		mytbar->pref = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_PREFERENCES,
				_("Edit Preferences..."), NULL,
				(GtkSignalFunc)menu_options, NULL,
				position ++);
	}
	if (config_show_tb_help)
	{
		mytbar->help = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_HELP,
				_("User's Guide and Manual"), NULL,
				(GtkSignalFunc)gtt_help_popup, NULL,
				position ++);
	}
	if (config_show_tb_exit)
	{
		mytbar->exit = gtk_toolbar_insert_stock (mytbar->tbar,
				GTK_STOCK_QUIT,
				_("Quit GnoTime"), NULL,
				 (GtkSignalFunc)app_quit, NULL,
				position ++);
	}

	return GTK_WIDGET(mytbar->tbar);
}



/* ================================================================= */
/* TODO: I have to completely rebuild the toolbar, when I want to add or
   remove items. There should be a better way now */

#define ZAP(w)                                      \
   if (w) { gtk_container_remove(tbc, (w)); (w) = NULL; }

#define ZING(pos)                                   \
   if (pos) { gtk_toolbar_remove_space (mytbar->tbar, (pos)); (pos)=0; }

void
update_toolbar_sections(void)
{
	GtkContainer *tbc;
	GtkWidget *tb;

	if (!app_window) return;
	if (!mytbar) return;

	tbc = GTK_CONTAINER(mytbar->tbar);
	ZING (mytbar->spa);
	ZING (mytbar->spb);
	ZING (mytbar->spc);

	ZAP (mytbar->new_w);
	ZAP (mytbar->cut);
	ZAP (mytbar->copy);
	ZAP (mytbar->paste);
	ZAP (mytbar->journal_button);
	ZAP (mytbar->prop_w);
	ZAP (mytbar->timer_button);
	ZAP (mytbar->calendar_w);
	ZAP (mytbar->pref);
	ZAP (mytbar->help);
	ZAP (mytbar->exit);

	tb = build_toolbar();
	gtk_widget_show(tb);
}

/* ======================= END OF FILE ======================= */
