/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2002, Linas Vepstas <linas@lionas.org>
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
#include <libgnome/gnome-help.h>
#include <string.h>

#include "app.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "myoaf.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"

typedef struct _MyToolbar MyToolbar;

struct _MyToolbar 
{
	GtkToolbar *tbar;
	GtkWidget *cut, *copy, *paste; /* to make them sensible as needed */
	GtkWidget *journal_button;
	GtkWidget *prop_w;
	GtkWidget *timer_button;
	GtkImage  *timer_button_image;
	GtkWidget *calendar_w;
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
	extern GttProject *cutted_project;
	GtkToolbarStyle tb_style;

	g_return_if_fail(mytbar != NULL);
	g_return_if_fail(mytbar->tbar != NULL);
	g_return_if_fail(GTK_IS_TOOLBAR(mytbar->tbar));

	if (mytbar->tbar && mytbar->tbar->tooltips) 
	{  
		if (config_show_tb_tips)
			gtk_tooltips_enable(mytbar->tbar->tooltips);
		else
			gtk_tooltips_disable(mytbar->tbar->tooltips);
	}
	
	if (mytbar->paste)
	{
		gtk_widget_set_sensitive(mytbar->paste,
					 (cutted_project != NULL));
	}

	if (mytbar->timer_button_image)
	{
		gtk_image_set_from_stock (mytbar->timer_button_image,
				 ((timer_is_running()) ?
				     GNOME_STOCK_TIMER_STOP :
				     GNOME_STOCK_TIMER),
				 GTK_ICON_SIZE_LARGE_TOOLBAR);
	}
		

	if ((config_show_tb_icons) && (config_show_tb_texts)) 
	{
		tb_style = GTK_TOOLBAR_BOTH;
	} 
	else if ((!config_show_tb_icons) && (config_show_tb_texts)) 
	{
		tb_style = GTK_TOOLBAR_TEXT;
	} 
	else 
	{
		tb_style = GTK_TOOLBAR_ICONS;
	}
	gtk_toolbar_set_style(mytbar->tbar, tb_style);
}


/* ================================================================= */

static void
toolbar_help(GtkWidget *widget, gpointer data)
{
	GError *err = NULL;
	gnome_help_display ("index.html", "gnotime", &err);
	if (err)
	{
		printf ("duude gnome help err msg: %s\n", err->message);
	}
}

/* ================================================================= */
/* A small utility routine to use a stock image with custom text,
 * and put the whole thing into the toolbar
 */
static GtkWidget *
add_stock_button(GtkToolbar *tbar, const char *text, const char *tt_text,
		 const char *icon, GtkSignalFunc sigfunc)
{
	GtkWidget *w, *image;

	image = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	w = gtk_toolbar_append_item(tbar, text, tt_text, NULL, image,
				    sigfunc, NULL);
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
	if (mytbar) return GTK_WIDGET(mytbar->tbar);
	mytbar = g_malloc0(sizeof(MyToolbar));
	mytbar->tbar = GTK_TOOLBAR(gtk_toolbar_new());

	if (config_show_tb_new) 
	{
		gtk_toolbar_insert_stock (mytbar->tbar, 
			GTK_STOCK_NEW,
			_("Create a New Project..."), NULL,
			(GtkSignalFunc)new_project, NULL,
			position++);
		gtk_toolbar_append_space(mytbar->tbar);
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
		position ++;
	}
	if (config_show_tb_journal) 
	{
		/* There is no true 'stock' item for journal, so
		 * instead we draw our own button, and use a stock 
		 * image. */
		mytbar->journal_button = add_stock_button(mytbar->tbar, 
				 _("Journal"),
				 _("View and Edit Timestamp Logs"),
				 GNOME_STOCK_BOOK_OPEN,
				 (GtkSignalFunc) edit_journal);
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
		mytbar->calendar_w = add_stock_button(mytbar->tbar, 
				_("Calendar"),
				_("View Calendar"),
				GNOME_STOCK_TEXT_BULLETED_LIST,
				(GtkSignalFunc)edit_calendar);
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
		position ++;
	}
	if (config_show_tb_pref)
	{
		gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_PREFERENCES,
				_("Edit Preferences..."), NULL,
				(GtkSignalFunc)menu_options, NULL,
				position ++);
	}
	if (config_show_tb_help) 
	{
		gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_HELP,
				_("User's Guide and Manual"), NULL,
				(GtkSignalFunc)toolbar_help, NULL,
				position ++);
	}
	if (config_show_tb_exit) 
	{
		gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_QUIT,
				_("Quit Gnome TimeTracker"), NULL,
				 (GtkSignalFunc)app_quit, NULL,
				position ++);
	}

	return GTK_WIDGET(mytbar->tbar);
}



/* ================================================================= */
/* TODO: I have to completely rebuild the toolbar, when I want to add or
   remove items. There should be a better way now */
void
update_toolbar_sections(void)
{
	GtkWidget *tb;
	GtkWidget *w;

	if (!app_window) return;
	if (!mytbar) return;

	w = GTK_WIDGET(mytbar->tbar)->parent;
	if (w) {
		gtk_container_remove(GTK_CONTAINER(w),
				     GTK_WIDGET(mytbar->tbar));
	}

	/* XXX probably a memory leak if we don't free/destroy
	 * all the toolbar widgets first ... */
	g_free(mytbar);
	mytbar = NULL;
	tb = build_toolbar();
	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(mytbar->tbar));
	gtk_widget_show(GTK_WIDGET(tb));
}


/* ======================= END OF FILE ======================= */
