/*   Project Properties for GTimeTracker - a time tracker
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
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


#ifndef __PROPS_PROJ_H__
#define __PROPS_PROJ_H__

#include "proj.h"

/* pop up a dialog box for editing a project */
/* currently, this uses the same dailog over & over, we should probably
 * change this to use gnome MDI ?? 
 */
void prop_dialog_set_project(GttProject *proj);
void prop_dialog_show(GttProject *proj);


#endif /* __PROPS_PROJ_H__ */
