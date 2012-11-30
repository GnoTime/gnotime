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

#ifndef __GTT_TOOLBAR_H__
#define __GTT_TOOLBAR_H__

/* The build_toolbar() routine assembles the buttons in the toolbar.
 * The toolbar appearence (i.e. which buttons are visible) is
 * dynamically determined by the configuration settings.
 * Returns a pointer to the GtkToolbar widget.
 */
GtkWidget *build_toolbar(void);

/* The toolbar_set_states() routine updates the appearence/behaviour
 * of the toolbar.  In particular, the 'paste' button becomes active
 * when there is something to paste, and the timer button toggles it's
 * image when a project timer is started/stopped.
 */
void toolbar_set_states(void);

/* The update_toolbar_sections() routine rebuilds the toolbar.
 * This routine needs to be called whenever the configuration settings
 * have changed, in order for them to take effect.
 */
void update_toolbar_sections(void);

#endif
