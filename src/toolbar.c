/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
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


typedef struct _MyToggle MyToggle;
typedef struct _MyToolbar MyToolbar;

struct _MyToolbar {
	GtkToolbar *tbar;
	GtkWidget *cut, *copy, *paste; /* to make them sensible
					  as needed */
	GtkWidget *journal_w;
	GtkWidget *prop_w;
	GtkWidget *timer_w;
	// GnomeStock *timer;  broken by gnome-2.0 ....
	GtkWidget *calendar_w;
};

MyToolbar *mytbar = NULL;



static GtkWidget *
add_stock_button(GtkToolbar *tbar, const char *text, const char *tt_text,
		 const char *icon, GtkSignalFunc sigfunc)
{
	GtkWidget *w, *pixmap;

	pixmap = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	w = gtk_toolbar_append_item(tbar, text, tt_text, NULL, pixmap,
				    sigfunc, NULL);
	return w;
}


#ifdef GNOME_20_BROKEN_NEEDS_FIXING
static GnomeStock *
add_toggle_button(GtkToolbar *tbar, char *text, char *tt_text,
		 char *icon, GtkSignalFunc sigfunc, GtkWidget **wptr)
{
	GtkWidget *w;

/* i think I'm supposed to use 
 * w = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
 *
 * and later use gtk_image_set_from_stock()
 * to toggle the thing ...
 */
	w = gnome_stock_pixmap_widget((GtkWidget *)window, icon);
	(*wptr) = gtk_toolbar_append_item(tbar, text, tt_text, NULL, w,
					  sigfunc, NULL);
	return GNOME_STOCK(w);
}
#endif



void
toolbar_set_states(void)
{
	extern GttProject *cutted_project;
	GtkToolbarStyle tb_style;

	g_return_if_fail(mytbar != NULL);
	g_return_if_fail(mytbar->tbar != NULL);
	g_return_if_fail(GTK_IS_TOOLBAR(mytbar->tbar));

	if (mytbar->tbar && mytbar->tbar->tooltips) {
		if (config_show_tb_tips)
			gtk_tooltips_enable(mytbar->tbar->tooltips);
		else
			gtk_tooltips_disable(mytbar->tbar->tooltips);
	}
#if 0
/* not done any more, use the focus project instead */
	if (mytbar->cut)
		gtk_widget_set_sensitive(mytbar->cut, (cur_proj != NULL));
	if (mytbar->copy)
		gtk_widget_set_sensitive(mytbar->copy, (cur_proj != NULL));
#endif
	if (mytbar->paste)
		gtk_widget_set_sensitive(mytbar->paste,
					 (cutted_project != NULL));
#if 0
/* not done any more, use the focus project instead */
	if (mytbar->prop_w)
		gtk_widget_set_sensitive(mytbar->prop_w, (cur_proj != NULL));
	if (mytbar->journal_w)
		gtk_widget_set_sensitive(mytbar->journal_w, (cur_proj != NULL));
#endif
#ifdef GNOME_20_BROKEN_NEEDS_FIXING
	if (mytbar->timer)
		gnome_stock_set_icon(mytbar->timer,
				     (timer_is_running()) ?
				     GNOME_STOCK_TIMER_STOP :
				     GNOME_STOCK_TIMER);
#endif

#if 0
/* not done any more, use the focus project */
	if (mytbar->timer_w)
		gtk_widget_set_sensitive(GTK_WIDGET(mytbar->timer_w),
			(NULL != prev_proj) || (NULL != cur_proj));
#endif

	if ((config_show_tb_icons) && (config_show_tb_texts)) {
		tb_style = GTK_TOOLBAR_BOTH;
	} else if ((!config_show_tb_icons) && (config_show_tb_texts)) {
		tb_style = GTK_TOOLBAR_TEXT;
	} else {
		tb_style = GTK_TOOLBAR_ICONS;
	}
	gtk_toolbar_set_style(mytbar->tbar, tb_style);
}



static void
toolbar_help(GtkWidget *widget, gpointer data)
{
	GError *err;
	gnome_help_display ("index.html", "gtt", &err);
}



/* returns a pointer to the (still hidden) GtkToolbar */
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
	if (config_show_tb_file) 
	{
		gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_OPEN,
				_("Reload Configuration File"), NULL,
				(GtkSignalFunc)init_project_list, NULL,
				position ++);
		gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_SAVE,
				_("Save Configuration File"), NULL,
				(GtkSignalFunc)save_project_list, NULL,
				position ++);
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
		mytbar->journal_w = add_stock_button(mytbar->tbar, 
				 _("Journal"),
				 _("View and Edit Timestamp Logs"),
				 GNOME_STOCK_BOOK_OPEN,
				 (GtkSignalFunc)edit_journal);
		position ++;
	}
	if (config_show_tb_prop) 
	{
		mytbar->prop_w = gtk_toolbar_insert_stock (mytbar->tbar, 
				GTK_STOCK_PROPERTIES,
				_("Edit Project Properties..."), NULL,
				(GtkSignalFunc)menu_properties, NULL,
				position ++);
	}
	if (config_show_tb_timer) 
	{
#ifdef GNOME_20_BROKEN_NEEDS_FIXING
		mytbar->timer = add_toggle_button(mytbar->tbar, _("Timer"),
				_("Start/Stop Timer"),
				GNOME_STOCK_TIMER,
				(GtkSignalFunc)menu_toggle_timer,
				&(mytbar->timer_w));
		position ++;
#endif
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
				 (GtkSignalFunc)quit_app, NULL,
				position ++);
	}

	return GTK_WIDGET(mytbar->tbar);
}



/* TODO: I have to completely rebuild the toolbar, when I want to add or
   remove items. There should be a better way now */
void
update_toolbar_sections(void)
{
	GtkWidget *tb;
	GtkWidget *w;

	if (!window) return;
	if (!mytbar) return;

	w = GTK_WIDGET(mytbar->tbar)->parent;
	if (w) {
		gtk_container_remove(GTK_CONTAINER(w),
				     GTK_WIDGET(mytbar->tbar));
	}

	g_free(mytbar);
	mytbar = NULL;
	tb = build_toolbar();
	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(mytbar->tbar));
	gtk_widget_show(GTK_WIDGET(tb));
}


