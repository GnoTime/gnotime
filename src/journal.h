/*   Display Journal Timestamp Log entries for GTimeTracker - a time tracker
 *   Copyright (C) 2001 Linas Vepstas
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

#ifndef __GTT_JOURNAL_H__
#define __GTT_JOURNAL_H__

void edit_journal(GtkWidget *, gpointer);
void edit_alldata(GtkWidget *, gpointer);
void edit_invoice(GtkWidget *, gpointer);
void edit_primer(GtkWidget *, gpointer);
void invoke_report(GtkWidget *, gpointer);

void change_task(GtkWidget *, gpointer);

#endif /* __GTT_JOURNAL_H__ */
