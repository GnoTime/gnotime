

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
/* Save the contents of a GnomeUIInfo structure with GConf */

void
gtt_save_gnomeui_in_gconf (GConfClient *client, 
                const char * path, GnomeUIInfo *gui)
{
	char *savepath, *tokptr;

	if (!gui || !path) return;

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
}

/* ======================= END OF FILE ======================== */
