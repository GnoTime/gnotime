/*   GTimeTracker - a time tracker
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

#ifndef __GTT_APP_H__
#define __GTT_APP_H__

#include <gnome.h>
#include "ctree.h"

extern ProjTreeWindow *global_ptw;

extern GtkWidget *window;
extern GtkWidget *glist;
extern GtkWidget *status_bar;

/* true if command line over-rides geometry */
extern gboolean geom_size_override;
extern gboolean geom_place_override;

void update_status_bar(void);

void app_new(int argc, char *argv[], const char *geometry_string);

void app_show(void);


#endif /* __GTT_APP_H__ */
