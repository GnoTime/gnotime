/*   Interval Properties for GTimeTracker - a time tracker
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


#ifndef __PROPS_INVL_H__
#define __PROPS_INVL_H__

#include "proj.h"

typedef struct EditIntervalDialog_s EditIntervalDialog;

EditIntervalDialog * edit_interval_dialog_new (void);
void edit_interval_dialog_destroy(EditIntervalDialog *dlg);

void edit_interval_set_interval (EditIntervalDialog *dlg, GttInterval *ivl);

/* pop up a dialog box for editing an interval */
void edit_interval_dialog_show(EditIntervalDialog *dlg);


#endif /* __PROPS_INVL_H__ */
