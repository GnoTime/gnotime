/*   GUI dialog for global application preferences for GTimeTracker
 *   Copyright (C) 1997,98 Eckehard Berns
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

#include "config.h"

#include <qof.h>
#include <string.h>
#include <stdlib.h>

#include "app.h"
#include "cur-proj.h"
#include "dialog.h"
#include "gtt.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"
#include "util.h"

/* globals */
int config_show_secs = 0;
int config_show_statusbar = 1;
int config_show_clist_titles = 1;
int config_show_subprojects = 1;
int config_show_title_ever = 1;
int config_show_title_year = 0;
int config_show_title_month = 0;
int config_show_title_week = 0;
int config_show_title_lastweek = 0;
int config_show_title_day = 1;
int config_show_title_yesterday = 0;
int config_show_title_current = 0;
int config_show_title_desc = 1;
int config_show_title_task = 1;
int config_show_title_estimated_start = 0;
int config_show_title_estimated_end = 0;
int config_show_title_due_date = 0;
int config_show_title_sizing = 0;
int config_show_title_percent_complete = 0;
int config_show_title_urgency = 1;
int config_show_title_importance = 1;
int config_show_title_status = 0;

int config_show_toolbar = 1;
int config_show_tb_tips = 1;
int config_show_tb_new = 1;
int config_show_tb_ccp = 0;
int config_show_tb_journal = 1;
int config_show_tb_prop = 1;
int config_show_tb_timer = 1;
int config_show_tb_pref = 0;
int config_show_tb_help = 1;
int config_show_tb_exit = 1;

char *config_logfile_name = NULL;
char *config_logfile_start = NULL;
char *config_logfile_stop = NULL;
int config_logfile_use = 0;
int config_logfile_min_secs = 0;

int config_daystart_offset = 0;
int config_weekstart_offset = 0;

int config_time_format = TIME_FORMAT_LOCALE;

char *config_currency_symbol = NULL;
int config_currency_use_locale = 1;

char *config_data_url = NULL;

typedef struct _PrefsDialog
{
    GtkBuilder *gtkbuilder;
    GtkWidget *dlg;
    GtkCheckButton *show_secs;
    GtkCheckButton *show_statusbar;
    GtkCheckButton *show_clist_titles;
    GtkCheckButton *show_subprojects;

    GtkCheckButton *show_title_importance;
    GtkCheckButton *show_title_urgency;
    GtkCheckButton *show_title_status;
    GtkCheckButton *show_title_ever;
    GtkCheckButton *show_title_year;
    GtkCheckButton *show_title_month;
    GtkCheckButton *show_title_week;
    GtkCheckButton *show_title_lastweek;
    GtkCheckButton *show_title_day;
    GtkCheckButton *show_title_yesterday;
    GtkCheckButton *show_title_current;
    GtkCheckButton *show_title_desc;
    GtkCheckButton *show_title_task;
    GtkCheckButton *show_title_estimated_start;
    GtkCheckButton *show_title_estimated_end;
    GtkCheckButton *show_title_due_date;
    GtkCheckButton *show_title_sizing;
    GtkCheckButton *show_title_percent_complete;

    GtkCheckButton *logfileuse;
    GtkWidget *logfilename_l;
    GtkFileChooser *logfilename;
    GtkWidget *logfilestart_l;
    GtkEntry *logfilestart;
    GtkWidget *logfilestop_l;
    GtkEntry *logfilestop;
    GtkWidget *logfileminsecs_l;
    GtkEntry *logfileminsecs;

    GtkEntry *shell_start;
    GtkEntry *shell_stop;

    GtkCheckButton *show_toolbar;
    GtkCheckButton *show_tb_tips;
    GtkCheckButton *show_tb_new;
    GtkCheckButton *show_tb_ccp;
    GtkCheckButton *show_tb_journal;
    GtkCheckButton *show_tb_pref;
    GtkCheckButton *show_tb_timer;
    GtkCheckButton *show_tb_prop;
    GtkCheckButton *show_tb_help;
    GtkCheckButton *show_tb_exit;

    GtkEntry *idle_secs;
    GtkEntry *no_project_secs;
    GtkEntry *daystart_secs;
    GtkComboBox *daystart_menu;
    GtkComboBox *weekstart_menu;

    GtkRadioButton *time_format_am_pm;
    GtkRadioButton *time_format_24_hs;
    GtkRadioButton *time_format_locale;

    GtkEntry *currency_symbol;
    GtkWidget *currency_symbol_label;
    GtkCheckButton *currency_use_locale;

} PrefsDialog;


static void set_modified(PrefsDialog *dlg, gboolean modified)
{
    gtk_dialog_set_response_sensitive(GTK_DIALOG(dlg->dlg), GTK_RESPONSE_OK, modified);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(dlg->dlg), GTK_RESPONSE_APPLY, modified);
}

static void set_changed_cb(void *gobj, PrefsDialog *dlg)
{
    set_modified(dlg, TRUE);
}

/* Update the properties of the project view according to current settings */

void prefs_update_projects_view(void)
{
    GList *columns = NULL;

    if (config_show_title_importance)
    {
        columns = g_list_insert(columns, "importance", -1);
    }

    if (config_show_title_urgency)
    {
        columns = g_list_insert(columns, "urgency", -1);
    }

    if (config_show_title_status)
    {
        columns = g_list_insert(columns, "status", -1);
    }

    if (config_show_title_ever)
    {
        columns = g_list_insert(columns, "time_ever", -1);
    }

    if (config_show_title_year)
    {
        columns = g_list_insert(columns, "time_year", -1);
    }

    if (config_show_title_month)
    {
        columns = g_list_insert(columns, "time_month", -1);
    }

    if (config_show_title_week)
    {
        columns = g_list_insert(columns, "time_week", -1);
    }

    if (config_show_title_lastweek)
    {
        columns = g_list_insert(columns, "time_lastweek", -1);
    }

    if (config_show_title_yesterday)
    {
        columns = g_list_insert(columns, "time_yesterday", -1);
    }

    if (config_show_title_day)
    {
        columns = g_list_insert(columns, "time_today", -1);
    }

    if (config_show_title_current)
    {
        columns = g_list_insert(columns, "time_task", -1);
    }

    /* The title column is mandatory */
    columns = g_list_insert(columns, "title", -1);

    if (config_show_title_desc)
    {
        columns = g_list_insert(columns, "description", -1);
    }

    if (config_show_title_task)
    {
        columns = g_list_insert(columns, "task", -1);
    }

    if (config_show_title_estimated_start)
    {
        columns = g_list_insert(columns, "estimated_start", -1);
    }

    if (config_show_title_estimated_end)
    {
        columns = g_list_insert(columns, "estimated_end", -1);
    }

    if (config_show_title_due_date)
    {
        columns = g_list_insert(columns, "due_date", -1);
    }

    if (config_show_title_sizing)
    {
        columns = g_list_insert(columns, "sizing", -1);
    }

    if (config_show_title_percent_complete)
    {
        columns = g_list_insert(columns, "percent_done", -1);
    }

    gtt_projects_tree_set_visible_columns(projects_tree, columns);
    g_list_free(columns);

    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(projects_tree), config_show_subprojects);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(projects_tree), config_show_clist_titles);
}

void prefs_set_show_secs()
{
    gtt_projects_tree_set_show_seconds(projects_tree, config_show_secs);
}

/* ============================================================== */
/** parse an HH:MM:SS string for the time returning seconds
 * XXX should probably use getdate or fdate or something like that
 */

static int scan_time_string(const char *str)
{
    int hours = 0, minutes = 0, seconds = 0;
    char buff[24];
    strncpy(buff, str, 24);
    buff[23] = 0;
    char *p = strchr(buff, ':');
    if (p)
        *p = 0;
    hours = atoi(buff);
    if (p)
    {
        char *m = ++p;
        p = strchr(m, ':');
        if (p)
            *p = 0;
        minutes = atoi(m);
        if (p)
        {
            seconds = atoi(++p);
        }
    }
    seconds %= 60;
    minutes %= 60;
    hours %= 24;

    int totalsecs = hours * 3600 + minutes * 60 + seconds;
    if (12 * 3600 < totalsecs)
        totalsecs -= 24 * 3600;
    return totalsecs;
}

/* ============================================================== */

static void toolbar_sensitive_cb(GtkWidget *w, PrefsDialog *odlg)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_toolbar));
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_new), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_ccp), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_journal), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_pref), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_timer), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_prop), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_help), state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->show_tb_exit), state);

    // gtk_widget_set_sensitive(odlg->logfilename_l, state);
}

/* ============================================================== */

static gboolean save_on_idle_cb()
{
    save_properties();

    // Return FALSE for call-once semantics.
    return FALSE;
}


/* ============================================================== */

#define ENTRY_TO_CHAR(a, b)                    \
    {                                          \
        const char *s = gtk_entry_get_text(a); \
        if (s[0])                              \
        {                                      \
            if (b)                             \
                g_free(b);                     \
            b = g_strdup(s);                   \
        }                                      \
        else                                   \
        {                                      \
            if (b)                             \
                g_free(b);                     \
            b = NULL;                          \
        }                                      \
    }

#define SHOW_CHECK(TOK)                                                                \
    {                                                                                  \
        int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_##TOK)); \
        if (config_show_##TOK != state)                                                \
        {                                                                              \
            change = 1;                                                                \
            config_show_##TOK = state;                                                 \
        }                                                                              \
    }

#define SET_VAL(to, from) \
    {                     \
        if (to != from)   \
        {                 \
            change = 1;   \
            to = from;    \
        }                 \
    }

static void prefs_set(PrefsDialog *odlg)
{
    int state;

    {
        int change = 0;

        SHOW_CHECK(title_importance);
        SHOW_CHECK(title_urgency);
        SHOW_CHECK(title_status);
        SHOW_CHECK(title_ever);
        SHOW_CHECK(title_year);
        SHOW_CHECK(title_month);
        SHOW_CHECK(title_week);
        SHOW_CHECK(title_lastweek);
        SHOW_CHECK(title_day);
        SHOW_CHECK(title_yesterday);
        SHOW_CHECK(title_current);
        SHOW_CHECK(title_desc);
        SHOW_CHECK(title_task);
        SHOW_CHECK(title_estimated_start);
        SHOW_CHECK(title_estimated_end);
        SHOW_CHECK(title_due_date);
        SHOW_CHECK(title_sizing);
        SHOW_CHECK(title_percent_complete);

        if (change)
        {
            prefs_update_projects_view();
        }
    }

    {
        /* display options */
        state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_secs));
        if (state != config_show_secs)
        {
            config_show_secs = state;
            prefs_set_show_secs();
            update_status_bar();
            if (status_bar)
                gtk_widget_queue_resize(status_bar);
            start_main_timer();
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_statusbar)))
        {
            gtk_widget_show(GTK_WIDGET(status_bar));
            config_show_statusbar = 1;
        }
        else
        {
            gtk_widget_hide(GTK_WIDGET(status_bar));
            config_show_statusbar = 0;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_clist_titles)))
        {
            config_show_clist_titles = 1;
        }
        else
        {
            config_show_clist_titles = 0;
        }

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_subprojects)))
        {
            config_show_subprojects = 1;
        }
        else
        {
            config_show_subprojects = 0;
        }
        prefs_update_projects_view();
    }

    {
        /* shell command options */
        ENTRY_TO_CHAR(odlg->shell_start, config_shell_start);
        ENTRY_TO_CHAR(odlg->shell_stop, config_shell_stop);
    }

    {
        /* log file options */
        config_logfile_use = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->logfileuse));
        config_logfile_name = gtk_file_chooser_get_filename(odlg->logfilename);
        ENTRY_TO_CHAR(odlg->logfilestart, config_logfile_start);
        ENTRY_TO_CHAR(odlg->logfilestop, config_logfile_stop);
        config_logfile_min_secs = atoi(gtk_entry_get_text(odlg->logfileminsecs));
    }

    {
        int change = 0;

        /* toolbar */
        config_show_toolbar
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_toolbar));
        config_show_tb_tips
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->show_tb_tips));

        /* toolbar sections */
        SHOW_CHECK(tb_new);
        SHOW_CHECK(tb_ccp);
        SHOW_CHECK(tb_journal);
        SHOW_CHECK(tb_prop);
        SHOW_CHECK(tb_timer);
        SHOW_CHECK(tb_pref);
        SHOW_CHECK(tb_help);
        SHOW_CHECK(tb_exit);

        if (change)
        {
            update_toolbar_sections();
        }

        toolbar_set_states();
    }

    {
        int change = 0;
        config_idle_timeout = atoi(gtk_entry_get_text(GTK_ENTRY(odlg->idle_secs)));
        config_no_project_timeout = atoi(gtk_entry_get_text(GTK_ENTRY(odlg->no_project_secs)));

        if (timer_is_running())
        {
            start_idle_timer();
        }
        else
        {
            start_no_project_timer();
        }

        /* Hunt for the hour-of night on which to start */
        const char *buff = gtk_entry_get_text(odlg->daystart_secs);
        int off = scan_time_string(buff);
        SET_VAL(config_daystart_offset, off);

        int day = gtk_combo_box_get_active(odlg->weekstart_menu);
        SET_VAL(config_weekstart_offset, day);

        if (change)
        {
            /* Need to recompute everything, including the bining */
            gtt_project_list_compute_secs();
            gtt_projects_tree_update_all_rows(projects_tree);
        }
    }

    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->time_format_am_pm)))
        {
            config_time_format = TIME_FORMAT_AM_PM;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->time_format_24_hs)))
        {
            config_time_format = TIME_FORMAT_24_HS;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->time_format_locale)))
        {
            config_time_format = TIME_FORMAT_LOCALE;
        }

        ENTRY_TO_CHAR(odlg->currency_symbol, config_currency_symbol);
        int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->currency_use_locale));
        if (config_currency_use_locale != state)
        {
            config_currency_use_locale = state;
        }
    }

    // Schedule a save-to-file to be called from the main loop promptly.
    // Doing it this way instead of a direct call gives Gtk a chance to
    // first react to any changes, e.g. recalculate columns widths.
    g_idle_add(save_on_idle_cb, NULL);
}


/* ============================================================== */

static void logfile_sensitive_cb(GtkWidget *w, PrefsDialog *odlg)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(odlg->logfileuse));
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilename), state);
    gtk_widget_set_sensitive(odlg->logfilename_l, state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilestart), state);
    gtk_widget_set_sensitive(odlg->logfilestart_l, state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfilestop), state);
    gtk_widget_set_sensitive(odlg->logfilestop_l, state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->logfileminsecs), state);
    gtk_widget_set_sensitive(odlg->logfileminsecs_l, state);
}

static void currency_sensitive_cb(GtkWidget *w, PrefsDialog *odlg)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->currency_symbol), !state);
    gtk_widget_set_sensitive(GTK_WIDGET(odlg->currency_symbol_label), !state);
}

#define SET_ACTIVE(TOK) \
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->show_##TOK), config_show_##TOK);

static void options_dialog_set(PrefsDialog *odlg)
{
    char s[30];

    SET_ACTIVE(secs);
    SET_ACTIVE(statusbar);
    SET_ACTIVE(clist_titles);
    SET_ACTIVE(subprojects);

    SET_ACTIVE(title_importance);
    SET_ACTIVE(title_urgency);
    SET_ACTIVE(title_status);
    SET_ACTIVE(title_ever);
    SET_ACTIVE(title_year);
    SET_ACTIVE(title_month);
    SET_ACTIVE(title_week);
    SET_ACTIVE(title_lastweek);
    SET_ACTIVE(title_day);
    SET_ACTIVE(title_yesterday);
    SET_ACTIVE(title_current);
    SET_ACTIVE(title_desc);
    SET_ACTIVE(title_task);
    SET_ACTIVE(title_estimated_start);
    SET_ACTIVE(title_estimated_end);
    SET_ACTIVE(title_due_date);
    SET_ACTIVE(title_sizing);
    SET_ACTIVE(title_percent_complete);

    if (config_shell_start)
        gtk_entry_set_text(odlg->shell_start, config_shell_start);
    else
        gtk_entry_set_text(odlg->shell_start, "");

    if (config_shell_stop)
        gtk_entry_set_text(odlg->shell_stop, config_shell_stop);
    else
        gtk_entry_set_text(odlg->shell_stop, "");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->logfileuse), config_logfile_use);
    if (config_logfile_name)
        gtk_file_chooser_set_filename(odlg->logfilename, config_logfile_name);
    else
        gtk_file_chooser_unselect_all(odlg->logfilename);

    if (config_logfile_start)
        gtk_entry_set_text(odlg->logfilestart, config_logfile_start);
    else
        gtk_entry_set_text(odlg->logfilestart, "");

    if (config_logfile_stop)
        gtk_entry_set_text(odlg->logfilestop, config_logfile_stop);
    else
        gtk_entry_set_text(odlg->logfilestop, "");

    g_snprintf(s, sizeof(s), "%d", config_logfile_min_secs);
    gtk_entry_set_text(GTK_ENTRY(odlg->logfileminsecs), s);

    logfile_sensitive_cb(NULL, odlg);

    /* toolbar sections */
    SET_ACTIVE(toolbar);
    SET_ACTIVE(tb_tips);
    SET_ACTIVE(tb_new);
    SET_ACTIVE(tb_ccp);
    SET_ACTIVE(tb_journal);
    SET_ACTIVE(tb_prop);
    SET_ACTIVE(tb_timer);
    SET_ACTIVE(tb_pref);
    SET_ACTIVE(tb_help);
    SET_ACTIVE(tb_exit);

    toolbar_sensitive_cb(NULL, odlg);

    /* misc section */
    g_snprintf(s, sizeof(s), "%d", config_idle_timeout);
    gtk_entry_set_text(GTK_ENTRY(odlg->idle_secs), s);

    g_snprintf(s, sizeof(s), "%d", config_no_project_timeout);
    gtk_entry_set_text(GTK_ENTRY(odlg->no_project_secs), s);

    /* Set the correct menu item based on current values */
    int hour;
    if (0 < config_daystart_offset)
    {
        hour = (config_daystart_offset + 1800) / 3600;
    }
    else
    {
        hour = (config_daystart_offset - 1800) / 3600;
    }
    if (-3 > hour)
        hour = -3; /* menu runs from 9pm */
    if (6 < hour)
        hour = 6; /* menu runs till 6am */
    hour += 3;    /* menu starts at 9PM */
    gtk_combo_box_set_active(odlg->daystart_menu, hour);

    /* Print the daystart offset as a string in 24 hour time */
    int secs = config_daystart_offset;
    if (0 > secs)
        secs += 24 * 3600;
    char buff[24];
    xxxqof_print_hours_elapsed_buff(buff, 24, secs, config_show_secs);
    gtk_entry_set_text(odlg->daystart_secs, buff);

    /* Set the correct menu item based on current values */
    int day = config_weekstart_offset;
    gtk_combo_box_set_active(odlg->weekstart_menu, day);

    switch (config_time_format)
    {
    case TIME_FORMAT_AM_PM:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_am_pm), TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_24_hs), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_locale), FALSE);
        break;
    case TIME_FORMAT_24_HS:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_am_pm), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_24_hs), TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_locale), FALSE);
        break;

    case TIME_FORMAT_LOCALE:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_am_pm), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_24_hs), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(odlg->time_format_locale), TRUE);
        break;
    }

    g_snprintf(s, sizeof(s), "%s", config_currency_symbol);
    gtk_entry_set_text(GTK_ENTRY(odlg->currency_symbol), s);
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(odlg->currency_use_locale), config_currency_use_locale
    );

    /* set to unmodified as it reflects the current state of the app */
    set_modified(odlg, FALSE);
}

/* ============================================================== */

static void daystart_menu_changed(gpointer data, GtkComboBox *w)
{
    PrefsDialog *dlg = data;

    int hour = gtk_combo_box_get_active(dlg->daystart_menu);

    g_return_if_fail(hour >= 0);

    hour += -3; /* menu starts at 9PM */

    int secs = hour * 3600;
    if (0 > secs)
        secs += 24 * 3600;
    char buff[24];
    xxxqof_print_hours_elapsed_buff(buff, 24, secs, config_show_secs);
    gtk_entry_set_text(dlg->daystart_secs, buff);
}

/* ============================================================== */

#define GETWID(strname)                                                            \
    ({                                                                             \
        GtkWidget *e;                                                              \
        e = GTK_WIDGET(gtk_builder_get_object(builder, strname));                  \
        g_signal_connect(                                                          \
            G_OBJECT(e), "changed", G_CALLBACK(set_changed_cb), dlg              \
        );                                                                         \
        e;                                                                         \
    })

#define GETCHWID(strname)                                                          \
    ({                                                                             \
        GtkWidget *e;                                                              \
        e = GTK_WIDGET(gtk_builder_get_object(builder, strname));                  \
        g_signal_connect(                                                          \
            G_OBJECT(e), "toggled", G_CALLBACK(set_changed_cb), dlg              \
        );                                                                         \
        e;                                                                         \
    })

static void display_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETCHWID("show secs");
    dlg->show_secs = GTK_CHECK_BUTTON(w);

    w = GETCHWID("show statusbar");
    dlg->show_statusbar = GTK_CHECK_BUTTON(w);

    w = GETCHWID("show header");
    dlg->show_clist_titles = GTK_CHECK_BUTTON(w);

    w = GETCHWID("show sub");
    dlg->show_subprojects = GTK_CHECK_BUTTON(w);
}

#define DLGWID(strname)             \
    w = GETCHWID("show " #strname); \
    dlg->show_title_##strname = GTK_CHECK_BUTTON(w);

static void field_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    DLGWID(importance);
    DLGWID(urgency);
    DLGWID(status);
    DLGWID(ever);
    DLGWID(year);
    DLGWID(month);
    DLGWID(week);
    DLGWID(lastweek);
    DLGWID(day);
    DLGWID(yesterday);
    DLGWID(current);
    DLGWID(desc);
    DLGWID(task);
    DLGWID(estimated_start);
    DLGWID(estimated_end);
    DLGWID(due_date);
    DLGWID(sizing);
    DLGWID(percent_complete);
}

static void shell_command_options(PrefsDialog *dlg)
{
    GtkWidget *e;
    GtkBuilder *builder = dlg->gtkbuilder;

    e = GETWID("start project");
    dlg->shell_start = GTK_ENTRY(e);

    e = GETWID("stop project");
    dlg->shell_stop = GTK_ENTRY(e);
}

static void logfile_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETCHWID("use logfile");
    dlg->logfileuse = GTK_CHECK_BUTTON(w);
    g_signal_connect(
        G_OBJECT(w), "clicked", G_CALLBACK(logfile_sensitive_cb), (gpointer *) dlg
    );

    w = GTK_WIDGET(gtk_builder_get_object(builder, "filename label"));
    dlg->logfilename_l = w;

    w = GTK_WIDGET(gtk_builder_get_object(builder, "logfile path"));
    dlg->logfilename = GTK_FILE_CHOOSER(w);
    g_signal_connect(
        G_OBJECT(dlg->logfilename), "file-set", G_CALLBACK(set_changed_cb), dlg
    );

    w = GTK_WIDGET(gtk_builder_get_object(builder, "fstart label"));
    dlg->logfilestart_l = w;

    w = GETWID("fstart");
    dlg->logfilestart = GTK_ENTRY(w);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "fstop label"));
    dlg->logfilestop_l = w;

    w = GETWID("fstop");
    dlg->logfilestop = GTK_ENTRY(w);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "fmin label"));
    dlg->logfileminsecs_l = w;

    w = GETWID("fmin");
    dlg->logfileminsecs = GTK_ENTRY(w);
}

#define TBWID(strname)              \
    w = GETCHWID("show " #strname); \
    dlg->show_tb_##strname = GTK_CHECK_BUTTON(w);

static void toolbar_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETCHWID("show toolbar");
    dlg->show_toolbar = GTK_CHECK_BUTTON(w);

    g_signal_connect(
        G_OBJECT(w), "clicked", G_CALLBACK(toolbar_sensitive_cb), (gpointer *) dlg
    );

    TBWID(tips);
    TBWID(new);
    TBWID(ccp);
    TBWID(journal);
    TBWID(prop);
    TBWID(timer);
    TBWID(pref);
    TBWID(help);
    TBWID(exit);
}

static void misc_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETWID("idle secs");
    dlg->idle_secs = GTK_ENTRY(w);

    w = GETWID("no project secs");
    dlg->no_project_secs = GTK_ENTRY(w);

    w = GETWID("daystart entry");
    dlg->daystart_secs = GTK_ENTRY(w);

    w = GETWID("daystart combobox");
    dlg->daystart_menu = GTK_COMBO_BOX(w);

    g_signal_connect_object(
        G_OBJECT(w), "changed", G_CALLBACK(daystart_menu_changed), dlg, 0
    );

    w = GETWID("weekstart combobox");
    dlg->weekstart_menu = GTK_COMBO_BOX(w);
}

static void time_format_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETCHWID("time_format_am_pm");
    dlg->time_format_am_pm = GTK_RADIO_BUTTON(w);

    w = GETCHWID("time_format_24_hs");
    dlg->time_format_24_hs = GTK_RADIO_BUTTON(w);

    w = GETCHWID("time_format_locale");
    dlg->time_format_locale = GTK_RADIO_BUTTON(w);
}

static void currency_options(PrefsDialog *dlg)
{
    GtkWidget *w;
    GtkBuilder *builder = dlg->gtkbuilder;

    w = GETWID("currency_symbol");
    dlg->currency_symbol = GTK_ENTRY(w);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "currency_symbol_label"));
    dlg->currency_symbol_label = w;

    w = GETCHWID("currency_use_locale");
    dlg->currency_use_locale = GTK_CHECK_BUTTON(w);

    g_signal_connect(
        G_OBJECT(w), "clicked", G_CALLBACK(currency_sensitive_cb), (gpointer *) dlg
    );
}

/* ============================================================== */

static void response_cb(GtkDialog *gtk_dialog, gint response_id, PrefsDialog *dlg)
{
    switch (response_id)
    {    
        case GTK_RESPONSE_OK:
            prefs_set(dlg);
            gtk_widget_hide(GTK_WIDGET(dlg->dlg));
            break;
        case GTK_RESPONSE_APPLY:
            prefs_set(dlg);
            break;
        case GTK_RESPONSE_CLOSE:
            gtk_widget_hide(GTK_WIDGET(dlg->dlg));
            break;
        case GTK_RESPONSE_HELP:
            gtt_help_popup(GTK_WIDGET(dlg->dlg), "preferences");
            break;
    }
}

static PrefsDialog *prefs_dialog_new(void)
{
    PrefsDialog *dlg;
    GtkBuilder *builder;

    dlg = g_malloc(sizeof(PrefsDialog));

    builder = gtt_builder_new_from_file("ui/prefs.ui");
    dlg->gtkbuilder = builder;

    GtkWidget *notebook = GTK_WIDGET(gtk_builder_get_object(builder, "Global Preferences"));

    dlg->dlg = gtk_dialog_new_with_buttons("Global Preferences",
                                           NULL,
                                           0,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                           GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
                                           GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                           NULL);

    GtkWidget *dlg_content = gtk_dialog_get_content_area(GTK_DIALOG(dlg->dlg));
    gtk_container_add(GTK_CONTAINER(dlg_content), notebook);

    g_signal_connect(
        G_OBJECT(dlg->dlg), "response", G_CALLBACK(response_cb), dlg
    );

    g_signal_connect(
        G_OBJECT(dlg->dlg), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL
    );

    /* ------------------------------------------------------ */
    /* grab the various entry boxes and hook them up */
    display_options(dlg);
    field_options(dlg);
    shell_command_options(dlg);
    logfile_options(dlg);
    toolbar_options(dlg);
    misc_options(dlg);
    time_format_options(dlg);
    currency_options(dlg);

    return dlg;
}

/* ============================================================== */

static PrefsDialog *dlog = NULL;

void prefs_dialog_show(void)
{
    if (!dlog)
        dlog = prefs_dialog_new();

    options_dialog_set(dlog);
    gtk_widget_show(GTK_WIDGET(dlog->dlg));
}

/* ==================== END OF FILE ============================= */
