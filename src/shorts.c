/*   GTimeTracker - a time tracker
 *   Copyright (C) 2000 Sven M. Hallberg on 2000-03-22.
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

/*
 * `shorts.c' defines the hotkey (shortcut) subsystem.
 * It was added by Sven M. Hallberg on 2000-03-22.
 */

#include <config.h>
#include <gnome.h>

#include "app.h"
#include "gtt.h"
#include "menucmd.h"
#include "shorts.h"


/* hotkey handler functions */
static void hk_move_focus(guint keyval);
static void hk_quit_app(guint keyval);

/* hotkey handler table */
struct hotkey_ent {
    guint keyval;
    void (*handler)(guint);
};
struct hotkey_ent hotkey_tab[] = {
    { 'j', hk_move_focus },
    { 'k', hk_move_focus },
    { 'q', hk_quit_app },
    { 0, NULL }		/* This line terminates the table */
};


/* keypress event handler */
static gint hotkey_press(GtkWidget *widget, GdkEventKey *event, gpointer data);

void
connect_short_cuts(void)
{
	GtkWidget *glist = ctree_get_widget(global_ptw);
	g_signal_connect(G_OBJECT(window), "key_press_event",
			   G_CALLBACK(hotkey_press), NULL);
	/* Focus the list when focusing the app. */
	/* FIXME: Which event is really the right one??? */
	g_signal_connect_object(G_OBJECT(window), "realize",
				  G_CALLBACK(gtk_widget_grab_focus),
				  GTK_OBJECT(glist), 0);
}

static gint
hotkey_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	unsigned int i;		/* iterator for handler table */
	gint rv = FALSE;		/* return value indicating whether event
	                        has been handled or not */

#ifdef HOTKEY_DEBUG
	printf("Some key was pressed. Event data:\n");
	printf("keyval: %d ('%c')\n", event->keyval, event->keyval);
	printf("length: %d\n", event->length);
	printf("string: %s\n", event->string);
#endif

	for(i=0; hotkey_tab[i].keyval; i++) 
	{
		if(hotkey_tab[i].keyval == event->keyval) {
#ifdef HOTKEY_DEBUG
			printf("Handling key '%c'...\n", event->keyval);
#endif
			rv = TRUE;
			hotkey_tab[i].handler(event->keyval);
		}
	}
	return rv;
}

static void
hk_quit_app(guint keyval)
{
	app_quit(NULL, NULL);
}

static void
hk_move_focus(guint keyval)
{
	GtkWidget *glist = ctree_get_widget(global_ptw);
	if (! GTK_IS_CLIST (glist)) return;

	gtk_widget_grab_focus(glist); /* in case we lost it somewhere */
	gtk_clist_freeze(GTK_CLIST(glist));
	switch(keyval) 
	{
		case 'j':
			if(GTK_CLIST(glist)->focus_row < GTK_CLIST(glist)->rows - 1)
			GTK_CLIST(glist)->focus_row += 1;
			break;
		case 'k':
			if(GTK_CLIST(glist)->focus_row > 0)
			GTK_CLIST(glist)->focus_row -= 1;
			break;
		default:
			g_warning("Unimplemented focus move key: '%c'\n", keyval);
		break;
	}
	gtk_clist_thaw(GTK_CLIST(glist));
}

/* =====================  END OF FILE ===================== */
