/*   Keyboard inactivity timout dialog for GTimeTracker - a time tracker
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

#ifndef GTT_IDLE_DIALOG_H_
#define GTT_IDLE_DIALOG_H_

/* The Idle Dialog is popped up when a project is active (a project
 * timer is running), but the keyboard/mouse have been idle for some
 * set amount of time.
 *
 * Do not confuse this with the "Activity Dialog", which pops up
 * when NO project is running, but the keyboard/mouse IS active.
 * The 'activity' project nags you to run a timer.
 *
 * The idle dialog asks the user if they want to restart the
 * idled project, and allows user to credit a variable amount
 * of time to it.
 *
 * Terminology: a project that used to be running, but was auto-stopped
 * due to keyboard inactivity, is called an 'expired' project.  This
 * dialog allows the user to restart expired projects.
 */

typedef struct GttIdleDialog_s GttIdleDialog;

GttIdleDialog * idle_dialog_new (void);

/** This routine will display the idle dialog, but only
 *  if the keyboard/mouse has been idle for some amount of time.
 *  It will cause the timer for the currently active project to
 *  be stopped (and the ctree display to be updated to reflect the
 *  stopped project).
 */
void show_idle_dialog (GttIdleDialog *id);

/** This routine will raise the idle dialog to the top of the
 *  current screen. But it will do this only if the idle dialog
 *  is already being displayed, and if some mouse/keyboard events have
 *  been detected recently.   The problem that this routine is trying
 *  to solve is that the idle dialog often ends up obscured by
 *  another window, or it ends up on a different workspace than the
 *  current workspace, and so the user can't see it, can't find it.
 */
void raise_idle_dialog (GttIdleDialog *id);

/** This routine will activate the idle timer, so that the idle dialog
 *  is raised when needed.
 */
void idle_dialog_activate_timer (GttIdleDialog *id);


/** This routine will deactivate the idle timer.
 */
void idle_dialog_deactivate_timer (GttIdleDialog *id);

gboolean idle_dialog_is_visible(GttIdleDialog *id);

#endif /* GTT_IDLE_DIALOG_H_ */
