/*   GnomeUI to GConf2 input/output handling for GTimeTracker - a time tracker
 *   Copyright (C) 2003 Linas Vepstas <linas@linas.org>
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

#include <gconf/gconf-client.h>
#include <gnome.h>

#include "gconf-gnomeui.h"
#include "gconf-io-p.h"

/* ======================================================= */
/* Convert gnome enums to strings */

#define CASE(x)  case x: return #x; 

static const char * 
gnome_ui_info_type_to_string (GnomeUIInfoType typ)
{
	switch (typ)
	{
		CASE (GNOME_APP_UI_ENDOFINFO);
		CASE (GNOME_APP_UI_ITEM);
		CASE (GNOME_APP_UI_TOGGLEITEM);
		CASE (GNOME_APP_UI_RADIOITEMS);
		CASE (GNOME_APP_UI_SUBTREE);
		CASE (GNOME_APP_UI_SEPARATOR);
		CASE (GNOME_APP_UI_HELP);
		CASE (GNOME_APP_UI_BUILDER_DATA);
		CASE (GNOME_APP_UI_ITEM_CONFIGURABLE);
		CASE (GNOME_APP_UI_SUBTREE_STOCK);
		CASE (GNOME_APP_UI_INCLUDE);
	}
	return "";
}

static const char * 
gnome_ui_pixmap_type_to_string (GnomeUIPixmapType typ)
{
	switch (typ)
	{
		CASE (GNOME_APP_PIXMAP_NONE);
		CASE (GNOME_APP_PIXMAP_STOCK);
		CASE (GNOME_APP_PIXMAP_DATA);
		CASE (GNOME_APP_PIXMAP_FILENAME);
	}
	return "";
}

/* ======================================================= */
/* Convert strings to gnome enums */

#define MATCH(str,x)  if (0==strcmp (str, #x)) return x;

static GnomeUIInfoType 
string_to_gnome_ui_info_type (const char *str)
{
	MATCH (str, GNOME_APP_UI_ENDOFINFO);
	MATCH (str, GNOME_APP_UI_ITEM);
	MATCH (str, GNOME_APP_UI_TOGGLEITEM);
	MATCH (str, GNOME_APP_UI_RADIOITEMS);
	MATCH (str, GNOME_APP_UI_SUBTREE);
	MATCH (str, GNOME_APP_UI_SEPARATOR);
	MATCH (str, GNOME_APP_UI_HELP);
	MATCH (str, GNOME_APP_UI_BUILDER_DATA);
	MATCH (str, GNOME_APP_UI_ITEM_CONFIGURABLE);
	MATCH (str, GNOME_APP_UI_SUBTREE_STOCK);
	MATCH (str, GNOME_APP_UI_INCLUDE);
	return 0;
}

static GnomeUIPixmapType
string_to_gnome_ui_pixmap_type (const char * str)
{
	MATCH (str, GNOME_APP_PIXMAP_NONE);
	MATCH (str, GNOME_APP_PIXMAP_STOCK);
	MATCH (str, GNOME_APP_PIXMAP_DATA);
	MATCH (str, GNOME_APP_PIXMAP_FILENAME);
	return 0;
}

/* ======================================================= */
/* Save the contents of a GnomeUIInfo structure with GConf */

void
gtt_save_gnomeui_in_gconf (GConfClient *client, 
                const char * path, GnomeUIInfo *gui)
{
	char *savepath, *tokptr;

	if (!client || !gui || !path) return;

	if (GNOME_APP_UI_ENDOFINFO == gui->type) return; 
	
	/* Reserve a big enough buffer for ourselves */
	savepath = g_strdup_printf ("%sXXXXXXXXXXXXXXXXXXXX",path);
	tokptr = savepath + strlen (path);
	
	/* Store the info */
	strcpy (tokptr, "Type");
	F_SETSTR (savepath, gnome_ui_info_type_to_string(gui->type));
	strcpy (tokptr, "Label");
	F_SETSTR (savepath, gui->label);
	strcpy (tokptr, "Hint");
	F_SETSTR (savepath, gui->hint);
	strcpy (tokptr, "PixmapType");
	F_SETSTR (savepath, gnome_ui_pixmap_type_to_string(gui->pixmap_type));
	strcpy (tokptr, "PixmapInfo");
	F_SETSTR (savepath, gui->pixmap_info);
	strcpy (tokptr, "AcceleratorKey");
	F_SETINT (savepath, gui->accelerator_key);
	strcpy (tokptr, "AcMods");
	F_SETINT (savepath, gui->ac_mods);

	g_free (savepath);
}

/* ======================================================= */
/* Restore the contents of a GnomeUIInfo structure from GConf */

void
gtt_restore_gnomeui_from_gconf (GConfClient *client, 
                const char * path, GnomeUIInfo *gui)
{
	char *savepath, *tokptr;

	if (!client || !gui || !path) return;

	/* Reserve a big enough buffer for ourselves */
	savepath = g_strdup_printf ("%sXXXXXXXXXXXXXXXXXXXX",path);
	tokptr = savepath + strlen (path);
	
	/* ReStore the info */
	strcpy (tokptr, "Type");
	gui->type = string_to_gnome_ui_info_type(F_GETSTR (savepath, ""));
	strcpy (tokptr, "Label");
	gui->label = F_GETSTR (savepath, "");
	strcpy (tokptr, "Hint");
	gui->hint = F_GETSTR (savepath, "");
	strcpy (tokptr, "PixmapType");
	gui->pixmap_type = string_to_gnome_ui_pixmap_type(F_GETSTR (savepath, ""));
	strcpy (tokptr, "PixmapInfo");
	gui->pixmap_info = F_GETSTR (savepath, "");
	strcpy (tokptr, "AcceleratorKey");
	gui->accelerator_key = F_GETINT (savepath, 0);
	strcpy (tokptr, "AcMods");
	gui->ac_mods = F_GETINT (savepath, 0);

	g_free (savepath);
}

/* ======================= END OF FILE ======================== */
