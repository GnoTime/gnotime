/*   file io routines for GTimeTracker - a time tracker
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

#ifndef __GTT_FILEIO_H__
#define __GTT_FILEIO_H__

#include <glib.h>



/* The routine gtt_save_config() will save configuration/user-preference data
 *    data to indicated file.  If none specified, then ~/.gnome/gtt is used.
 *    If an error occurs, a GttErrCode is set.
 *
 * The routine gtt_load_config() will load GTT configuration data from 
 *    the indicated file.  If none specified, then ~/.gnome/gtt is used.
 *    This routine is 'backwards compatible', in that it will load old
 *    config files formats.
 *    If an error occurs, a GttErrCode is set.
 */
void gtt_save_config (const char *fname);
void gtt_load_config (const char *fname);

/* The gtt_post_data_config() routine should be called *after* the 
 *    project data has been loaded. It performs some final configuration
 *    and setup, such as setting the last (current) active project,
 *    starting timers, etc.
 */

void gtt_post_data_config (void);

/* returns the 'real path' to the config file that was/would be used */
const char * gtt_get_config_filepath (void);

/* ??? */
gboolean project_list_export (const char *fname);


#endif /* __GTT_FILEIO_H__ */
