/*   Config file input/output handling for GnoTime
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
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

#include "gtt-gsettings-io.h"

#include <errno.h>
#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "cur-proj.h"
#include "file-io.h"
#include "gconf-io.h"
#include "gtt.h"
#include "prefs.h"
#include "proj.h"
#include "timer.h"

static const char *gtt_config_filepath = NULL;

int cur_proj_id = -1;
int run_timer = FALSE;
time_t last_timer = -1;
extern char *first_proj_title; /* command line flag */
int save_count = 0;

/* ============================================================= */
/* Configuration file I/O routines:
 * Note that this file supports reading from several old, 'obsolete' config file formats that
 * GTT has used over the years. It supports these reads so that users are not left out in the
 * cold when upgrading from old versions of GTT.  All 'saves' are in the new file format
 * (currently GSettings).
 *
 * 1) First try reading settings from GSettings
 * 2) Last try GConf
 */

/* ======================================================= */

void gtt_load_config(void)
{
    // Check if GSettings has been written to at least once and use it if so (this check is
    // conducted to allow the loading of attributes from previous configuration systems)
    if (FALSE == gtt_gsettings_initial_access())
    {
        gtt_gsettings_load();
        gtt_config_filepath = NULL;
        return;
    }

    /* Check for gconf2, and use that if it exists */
    if (gtt_gconf_exists())
    {
        gtt_gconf_load();
        gtt_config_filepath = NULL;
        return;
    }

    config_data_url = XML_DATA_FILENAME;
}

/* ======================================================= */

void gtt_post_data_config(void)
{
    /* Assume we've already read the XML data, and just
     * set the current project */
    cur_proj_set(gtt_project_locate_from_id(cur_proj_id));

    /* Over-ride the current project based on the
     * command-line setting */
    if (first_proj_title)
    {
        GList *node;
        node = gtt_project_list_get_list(master_list);
        for (; node; node = node->next)
        {
            GttProject *prj = node->data;
            if (!gtt_project_get_title(prj))
                continue;

            /* set project based on command line */
            if (0 == strcmp(gtt_project_get_title(prj), first_proj_title))
            {
                cur_proj_set(prj);
                break;
            }
        }
    }

    /* FIXME: this is a mem leak, depending on usage in main.c */
    first_proj_title = NULL;

    /* reset the clocks, if needed */
    if (0 < last_timer)
    {
        set_last_reset(last_timer);
        zero_daily_counters(NULL);
    }

    /* if a project is running, then set it running again,
     * otherwise be sure to stop the clock. */
    if (FALSE == run_timer)
    {
        cur_proj_set(NULL);
    }
}

void gtt_post_ctree_config(void)
{
    gchar *xpn = NULL;

    /* Assume the ctree has been set up.  Now punch in the final
     * bit of ctree state.
     */

    /* Restore the expander state */
    if (FALSE == gtt_gsettings_initial_access())
    {
        xpn = gtt_gsettings_get_expander();
    }
    else if (gtt_gconf_exists())
    {
        xpn = gtt_gconf_get_expander();
    }

    if (xpn)
    {
        gtt_projects_tree_set_expander_state(projects_tree, xpn);
    }
}

/* ======================================================= */
/* Save only the GUI configuration info, not the actual data */

void gtt_save_config(void)
{
    gtt_gsettings_save();
}

/* ======================================================= */

const char *gtt_get_config_filepath(void)
{
    return gtt_config_filepath;
}

/* =========================== END OF FILE ========================= */
