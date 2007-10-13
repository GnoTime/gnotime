/*********************************************************************
 *                
 * Copyright (C) 2007,  Goedson Teixeira Paixao
 *                
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *                
 * Filename:      status-icon.c
 * Author:        Goedson Teixeira Paixao <goedson@debian.org>
 * Description:   GnoTime's status icon implementation
 *                
 * Created at:    Fri Oct 12 11:45:39 2007
 * Modified at:   Fri Oct 12 22:43:57 2007
 * Modified by:   Goedson Teixeira Paixao <goedson@debian.org>
 ********************************************************************/

#include <gtk/gtk.h>
#include <gnome.h>
#include "status-icon.h"

static GtkStatusIcon *status_icon;

void
gtt_status_icon_create()
{
	status_icon = gtk_status_icon_new_from_stock (GNOME_STOCK_TIMER_STOP);
	gtk_status_icon_set_tooltip (status_icon, _("Timer is not running"));
}

void
gtt_status_icon_destroy()
{
	g_object_unref (G_OBJECT (status_icon));
}

void
gtt_status_icon_start_timer(GttProject *prj)
{
	gtk_status_icon_set_from_stock (status_icon, GNOME_STOCK_TIMER);
	gchar *text = g_strdup_printf(_("Timer running for %s"),
								  gtt_project_get_title(prj));
	gtk_status_icon_set_tooltip(status_icon, text);
	g_free (text);
}


void
gtt_status_icon_stop_timer(GttProject *prj)
{
	gtk_status_icon_set_tooltip (status_icon, _("Timer is not running"));
	gtk_status_icon_set_from_stock (status_icon, GNOME_STOCK_TIMER_STOP);
}
