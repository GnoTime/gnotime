/*   Global Options for GTimeTracker - a time tracker
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


#ifndef __GLOBAL_PREFS_H__
#define __GLOBAL_PREFS_H__


extern int config_show_secs;
extern int config_show_statusbar;
extern int config_show_clist_titles;
extern int config_show_subprojects;
extern int config_show_title_ever;
extern int config_show_title_day;
extern int config_show_title_week;
extern int config_show_title_month;
extern int config_show_title_year;
extern int config_show_title_current;
extern int config_show_title_desc;
extern int config_show_title_task;
extern int config_show_title_estimated_start;
extern int config_show_title_estimated_end;
extern int config_show_title_due_date;
extern int config_show_title_sizing;
extern int config_show_title_percent_complete;
extern int config_show_title_urgency;
extern int config_show_title_importance;
extern int config_show_title_status;
extern int config_show_tb_icons;
extern int config_show_tb_texts;
extern int config_show_tb_tips;
extern int config_show_tb_new;
extern int config_show_tb_file;
extern int config_show_tb_ccp;
extern int config_show_tb_journal;
extern int config_show_tb_calendar;
extern int config_show_tb_prop;
extern int config_show_tb_timer;
extern int config_show_tb_pref;
extern int config_show_tb_help;
extern int config_show_tb_exit;

extern char *config_command;
extern char *config_command_null;
extern char *config_logfile_name;
extern char *config_logfile_start;
extern char *config_logfile_stop;
extern int config_logfile_use;
extern int config_logfile_min_secs;

extern char *config_data_url;

/* pop up a dialog box for setting user preferences */
void prefs_dialog_show (void);

#endif /* __GLOBAL_PREFS_H__ */
