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
#ifndef __DIALOG_H__
#define __DIALOG_H__


/* Most of these dialog popups are probably obsolete, and should
 * be replaced by gtk-standard API's.  However, the gtk-standard
 * api's are still buggy, and so we stick to these for now ...
 * (lins -- april 2004)
 */
void new_dialog_ok_cancel(const char *title, GtkWidget **dlg, GtkBox **vbox,
  		  const char *s_ok, GCallback sigfunc, gpointer data,
		  const char *s_cancel, GCallback c_sigfunc, gpointer c_data);

void msgbox_ok(const char *title, const char *text, const char *ok_text,
	       GCallback func);
void msgbox_ok_cancel(const char *title, const char *text,
		      const char *ok_text, const char *cancel_text,
		      GCallback func);

void qbox_ok_cancel(const char *title, const char *text,
                   const char *ok_text, GCallback sigfunc, gpointer data,
                   const char *cancel_text, GCallback c_sigfunc, gpointer c_data);

/* Popup the appropriate help/documentaiton subsystem */
void gtt_help_popup(GtkWidget *widget, gpointer data);

#endif
