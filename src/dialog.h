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


void new_dialog_ok(const char *title, GtkWidget **dlg, GtkBox **vbox,
		   const char *s, GtkSignalFunc sigfunc, gpointer data);
void new_dialog_ok_cancel(const char *title, GtkWidget **dlg, GtkBox **vbox,
  		  const char *s_ok, GtkSignalFunc sigfunc, gpointer data,
		  const char *s_cancel, GtkSignalFunc c_sigfunc, gpointer c_data);

void msgbox_ok(const char *title, const char *text, const char *ok_text,
	       GtkSignalFunc func);
void msgbox_ok_cancel(const char *title, const char *text,
		      const char *ok_text, const char *cancel_text,
		      GtkSignalFunc func);

void qbox_ok_cancel(const char *title, const char *text,
                   const char *ok_text, GtkSignalFunc sigfunc, gpointer data,
                   const char *cancel_text, GtkSignalFunc c_sigfunc, gpointer c_data);

#endif
