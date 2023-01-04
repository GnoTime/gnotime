/*********************************************************************
 *
 * Copyright (C) 2007,  Michael Richardson <mcr@sandelman.ca>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston,
 * MA 02110-1301, USA
 *
 * Filename:      dbus.c
 * Author:        Goedson Teixeira Paixao <goedson@debian.org>
 * Description:   DBus interface to gnotime
 *
 * Created at:    Sun Aug 26 12:13:05 2007
 * Modified at:   Thu Aug 30 21:51:39 2007
 * Modified by:   Goedson Teixeira Paixao <goedson@debian.org>
 ********************************************************************/
/*
 *  D-Bus code derived from example-service.c in the dbus-glib bindings
 */
#if WITH_DBUS

#include <stdio.h>
#include <string.h>
#include "dbus.h"

#include <dbus/dbus-glib.h>

#include "timer.h"
#include "gtt.h"

typedef struct GnotimeDbus  GnotimeDbus;
typedef struct GnotimeDbusClass GnotimeDbusClass;

GType gnotime_dbus_get_type (void);

struct GnotimeDbus
{
  GObject parent;
};

struct GnotimeDbusClass
{
	GObjectClass parent;
};

#define GNOTIME_TYPE_DBUS              (gnotime_dbus_get_type ())
#define GNOTIME_DBUS(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), GNOTIME_TYPE_DBUS, GnotimeDbus))
#define GNOTIME_DBUS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GNOTIME_TYPE_DBUS, GnotimeDbusClass))
#define GNOTIME_IS_DBUS(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), GNOTIME_TYPE_DBUS))
#define GNOTIME_IS_DBUS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOTIME_TYPE_DBUS))
#define GNOTIME_DBUS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GNOTIME_TYPE_DBUS, GnotimeDbusClass))

G_DEFINE_TYPE(GnotimeDbus, gnotime_dbus, G_TYPE_OBJECT)

gboolean gnotime_dbus_timer(GnotimeDbus *obj, char *action,
			    guint32 *ret, GError **error);
gboolean gnotime_dbus_file(GnotimeDbus *obj, char *action,
			    guint32 *ret, GError **error);

#include "dbus-glue.h"

static void
gnotime_dbus_init (GnotimeDbus *obj)
{
}

static void
gnotime_dbus_class_init (GnotimeDbusClass *klass)
{
}

gboolean
gnotime_dbus_timer(GnotimeDbus *obj, char *action,
		   guint32 *ret, GError **error)
{
#if 1
	/* def DEBUGDBUS */
  printf( "gnotime_dbus_timer()\n  Got action = '%s'\n", action);
#endif

  if(strcasecmp(action, "start")==0) {
	  gen_start_timer();
  } else if(strcasecmp(action, "stop")==0) {
	  gen_stop_timer();
  } else {
	  (*ret)=1;
	  return TRUE;
  }

  (*ret) = 0;

  return TRUE;
}

gboolean
gnotime_dbus_file(GnotimeDbus *obj, char *action,
		   guint32 *ret, GError **error)
{
#if 1
	/* def DEBUGDBUS */
  printf( "gnotime_dbus_file()\n  Got action = '%s'\n", action);
#endif

  if(strcasecmp(action, "save")==0) {
	  save_projects();
  } else if(strcasecmp(action, "reload")==0) {
	  read_data(TRUE);
  } else {
	  (*ret)=1;
	  return TRUE;
  }

  (*ret) = 0;

  return TRUE;
}


void
gnotime_dbus_setup ( void )
{
  DBusGConnection *bus;
  GError *error = NULL;
  DBusGProxy *bus_proxy;
  GnotimeDbus *obj;
  guint request_name_result;

  dbus_g_object_type_install_info (GNOTIME_TYPE_DBUS,
				   &dbus_glib_gnotime_dbus_object_info);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus)
    {
      g_message ("Couldn't connect to session bus: %s", error->message );
      g_error_free (error);
      return;
    }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
					 "/org/freedesktop/DBus",
					 "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
			  G_TYPE_STRING, "net.sourceforge.gttr.gnotime",
			  G_TYPE_UINT, 0,
			  G_TYPE_INVALID,
			  G_TYPE_UINT, &request_name_result,
			  G_TYPE_INVALID))
    {
      g_message ("Failed to acquire net.sourceforge.gttr.gnotime: %s", error->message );
      g_error_free (error);
      g_object_unref( bus_proxy );
      dbus_g_connection_unref( bus );
      return;
    }
  g_object_unref( bus_proxy );

  obj = g_object_new (GNOTIME_TYPE_DBUS, NULL);

  dbus_g_connection_register_g_object(bus,
				      "/net/sourceforge/gttr/gnotime",
				      G_OBJECT (obj));

  // TODO: Find out if this is needed
  dbus_g_connection_unref( bus );

  return;
}

#endif //WITH_DBUS
