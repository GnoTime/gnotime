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
/* Convert strings to gnome enums */

#define MATCH(str, x)         \
    if (0 == strcmp(str, #x)) \
        return x;

static GnomeUIInfoType string_to_gnome_ui_info_type(const char *str)
{
    MATCH(str, GNOME_APP_UI_ENDOFINFO);
    MATCH(str, GNOME_APP_UI_ITEM);
    MATCH(str, GNOME_APP_UI_TOGGLEITEM);
    MATCH(str, GNOME_APP_UI_RADIOITEMS);
    MATCH(str, GNOME_APP_UI_SUBTREE);
    MATCH(str, GNOME_APP_UI_SEPARATOR);
    MATCH(str, GNOME_APP_UI_HELP);
    MATCH(str, GNOME_APP_UI_BUILDER_DATA);
    MATCH(str, GNOME_APP_UI_ITEM_CONFIGURABLE);
    MATCH(str, GNOME_APP_UI_SUBTREE_STOCK);
    MATCH(str, GNOME_APP_UI_INCLUDE);
    return 0;
}

static GnomeUIPixmapType string_to_gnome_ui_pixmap_type(const char *str)
{
    MATCH(str, GNOME_APP_PIXMAP_NONE);
    MATCH(str, GNOME_APP_PIXMAP_STOCK);
    MATCH(str, GNOME_APP_PIXMAP_DATA);
    MATCH(str, GNOME_APP_PIXMAP_FILENAME);
    return 0;
}

/* ======================================================= */
/* Restore the contents of a GnomeUIInfo structure from GConf */

void gtt_restore_gnomeui_from_gconf(GConfClient *client, const char *path, GnomeUIInfo *gui)
{
    char *savepath, *tokptr;

    if (!client || !gui || !path)
        return;

    /* Reserve a big enough buffer for ourselves */
    savepath = g_strdup_printf("%sXXXXXXXXXXXXXXXXXXXX", path);
    tokptr = savepath + strlen(path);

    /* Restore the info */
    strcpy(tokptr, "Type");
    gui->type = string_to_gnome_ui_info_type(F_GETSTR(savepath, ""));
    strcpy(tokptr, "Label");
    gui->label = F_GETSTR(savepath, "");
    strcpy(tokptr, "Hint");
    gui->hint = F_GETSTR(savepath, "");
    strcpy(tokptr, "PixmapType");
    gui->pixmap_type = string_to_gnome_ui_pixmap_type(F_GETSTR(savepath, ""));
    strcpy(tokptr, "PixmapInfo");
    gui->pixmap_info = F_GETSTR(savepath, "");
    strcpy(tokptr, "AcceleratorKey");
    gui->accelerator_key = F_GETINT(savepath, 0);
    strcpy(tokptr, "AcMods");
    gui->ac_mods = F_GETINT(savepath, 0);

    g_free(savepath);
}

/* ======================= END OF FILE ======================== */
