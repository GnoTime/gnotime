

#ifndef GTT_GCONF_GNOMEUI_H_
#define GTT_GCONF_GNOMEUI_H_

#include <gconf/gconf-client.h>
#include <gnome.h>

/* Save the contents of a GnomeUIInfo structure with GConf 
 * to the indicated path. */

void gtt_save_gnomeui_in_gconf (GConfClient *client, 
                const char * path, GnomeUIInfo *gui);

#endif
