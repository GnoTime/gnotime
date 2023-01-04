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
extern int config_show_title_yesterday;
extern int config_show_title_week;
extern int config_show_title_lastweek;
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
extern int config_show_toolbar;
extern int config_show_tb_tips;
extern int config_show_tb_new;
extern int config_show_tb_ccp;
extern int config_show_tb_journal;
extern int config_show_tb_calendar;
extern int config_show_tb_prop;
extern int config_show_tb_timer;
extern int config_show_tb_pref;
extern int config_show_tb_help;
extern int config_show_tb_exit;

extern char *config_shell_start;
extern char *config_shell_stop;
extern char *config_logfile_name;
extern char *config_logfile_start;
extern char *config_logfile_stop;
extern int config_logfile_use;
extern int config_logfile_min_secs;

extern char *config_data_url;

extern int config_daystart_offset;
extern int config_weekstart_offset;

extern int config_time_format;

extern char *config_currency_symbol;
extern int  config_currency_use_locale;
#define TIME_FORMAT_AM_PM  1
#define TIME_FORMAT_24_HS  2
#define TIME_FORMAT_LOCALE 3

/* Pop up a dialog box for setting user preferences */
void prefs_dialog_show (void);

/* update the list of visible columns in the main projects tree based
   on current user preference */
void prefs_update_projects_view_columns (void);
void prefs_update_projects_view (void);
void prefs_set_show_secs (void);

#endif /* __GLOBAL_PREFS_H__ */
