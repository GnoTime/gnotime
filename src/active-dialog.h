/*   Keyboard activity dialog for GTimeTracker - a time tracker
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

#ifndef GTT_ACTIVITY_DIALOG_H_
#define GTT_ACTIVITY_DIALOG_H_

/* The Activity Dialog is poped up when no project is 
 * running, but keyboard/mouse activity has been detected
 * (after a long period of idleness).  It nags the user
 * to pick a project, and start a timer.
 *
 * Do not confuse the Activity Dialog with the Idle Dialog,
 * which is poped up when there is a running project, but the
 * keyboard/mouse has gone idle.
 *
 * Terminology:  A project which had been running, but was stopped
 * by the Idle Dialog, is called an 'expired' project.  If there
 * is an expired project, then the Idle Dialog is used to restart it.
 * If there is no expired project (and no currently-running project),
 * then the Activity Dialog is used to nag the user to start a timer.
 */

typedef struct GttActiveDialog_s GttActiveDialog;

GttActiveDialog * active_dialog_new (void);

/** This routine will display the active dialog, but only
 *  if the keyboard/mouse has been idle for some amount of time.
 */
void show_active_dialog (GttActiveDialog *id);

/** This routine will raise the active dialog to the top of the
 *  current screen. But it will do this only if the active dialog
 *  is already being diplayed, and if some mouse/keyboard events have
 *  been detected recently.   The problem that this routine is trying
 *  to solve is that the active dialog often ends up obscured by
 *  another window, or it ends up on a different workspace than the
 *  current workspace, and so the user can't see it, can't find it.
 */
void raise_active_dialog (GttActiveDialog *id);

void active_dialog_activate_timer (GttActiveDialog *id);
void active_dialog_deactivate_timer (GttActiveDialog *id);

#endif /* GTT_ACTIVITY_DIALOG_H_ */
