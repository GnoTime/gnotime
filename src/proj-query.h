/*   Project data query for GTimeTracker
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

#ifndef __GTT_PROJ_QUERY_H__
#define __GTT_PROJ_QUERY_H__

#include <glib.h>
#include "proj.h"

/* This file contains routines that return various info about
 * the data in the system.  In some fancier world, these would
 * be replaced by a generic query mechanism; but for right now,
 * these are some hard-coded routines that return what we need.
 *
 * (The port to a fancier query system is in progress, but far
 * from complete.  Some of the routines here may go away in the
 * future.)
 */


/* The gtt_project_get_unfinished() routine returns a list
 *    of projects that are not marked as 'completed' or
 *    'cancelled'.  The returned list is a flat list, not
 *    a heirarchical list.
 */

GList * gtt_project_get_unfinished (void);

#endif /* __GTT_PROJ_QUERY_H__ */
