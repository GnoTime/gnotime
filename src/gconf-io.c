

#ifndef GTT_GCONF_IO_H_
#define GTT_GCONF_IO_H_

#include <glib.h>

void gtt_gconf_save (void);
void gtt_gconf_load (void);
gboolean gtt_gconf_exists (void);

#endif /* GTT_GCONF_IO_H_ */

#include <gconf/gconf-client.h>
#include <glib.h>

#include "prefs.h"


#define GTT_GCONF "/apps/GnoTimeDebug"

void
gtt_gconf_save (void)
{
	GConfClient *client;
	GError *err_ret= NULL;

	client = gconf_client_get_default ();
	gconf_client_set_bool (client, GTT_GCONF "/Display/ShowSecs", config_show_secs, &err_ret);

	if (err_ret) printf ("duude its %s\n", err_ret->message);
	
	gtt_gconf_exists ();
}

gboolean
gtt_gconf_exists (void)
{
	gboolean rc;
	GConfClient *client;
	GError *err_ret= NULL;

	client = gconf_client_get_default ();

	rc = gconf_client_dir_exists (client, GTT_GCONF, &err_ret);
	if (err_ret) printf ("duude err %s\n", err_ret->message);

	return rc;
}
	
void
gtt_gconf_load (void)
{
	gboolean rc;
	GConfClient *client;
	GError *err_ret= NULL;

	client = gconf_client_get_default ();

	rc = gconf_client_dir_exists (client, GTT_GCONF, &err_ret);
	if (err_ret) printf ("duude err %s\n", err_ret->message);

}
