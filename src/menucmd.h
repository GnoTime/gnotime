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

#ifndef __MENUCMD_H__
#define __MENUCMD_H__

#include "proj.h"

extern GttProject *cutted_project;

void quit_app(GtkWidget *, gpointer);
void about_box(GtkWidget *, gpointer);

void new_project(GtkWidget *, gpointer);
void init_project_list(GtkWidget *, gpointer);
void save_project_list(GtkWidget *, gpointer);
void export_current_state(GtkWidget *, gpointer);
void cut_project(GtkWidget *w, gpointer data);
void paste_project(GtkWidget *w, gpointer data);
void copy_project(GtkWidget *w, gpointer data);

void menu_start_timer(GtkWidget *w, gpointer data);
void menu_stop_timer(GtkWidget *w, gpointer data);
void menu_toggle_timer(GtkWidget *w, gpointer data);

void menu_set_states(void);

void menu_options(GtkWidget *w, gpointer data);

void menu_properties(GtkWidget *w, gpointer data);

void menu_clear_daily_counter(GtkWidget *w, gpointer data);


#ifdef DEBUG
void menu_test(GtkWidget *w, gpointer data);
#endif

#endif
