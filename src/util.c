/*   Utilities for GTimeTracker - a time tracker
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

#include "config.h"

#include <glade/glade.h>
#include <glib.h>
#include <gnome.h>
#include <qof.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LANGINFO_H
#define HAVE_LANGINFO_D_FMT 1
#include <langinfo.h>
#endif
#ifdef HAVE_LANGINFO_D_FMT
#define QOF_D_FMT (nl_langinfo(D_FMT))
#define QOF_D_T_FMT (nl_langinfo(D_T_FMT))
#define QOF_T_FMT (nl_langinfo(T_FMT))
#else
#define QOF_D_FMT "%F"
#define QOF_D_T_FMT "%F %r"
#define QOF_T_FMT "%r"
#endif

#include "util.h"

/* ============================================================== */

void xxxgtk_textview_set_text(GtkTextView *text, const char *str)
{
    GtkTextBuffer *buff = gtk_text_view_get_buffer(text);
    if (!str)
        str = "";
    gtk_text_buffer_set_text(buff, str, strlen(str));
}

char *xxxgtk_textview_get_text(GtkTextView *text)
{
    GtkTextIter start, end;
    GtkTextBuffer *buff = gtk_text_view_get_buffer(text);
    gtk_text_buffer_get_start_iter(buff, &start);
    gtk_text_buffer_get_end_iter(buff, &end);
    return gtk_text_buffer_get_text(buff, &start, &end, TRUE);
}

/**
 * @brief Create a new GnomeEntry instance
 * @param[out] outer_gnome_entry The new enclosing GnomeEntry instance
 * @param[out] inner_gtk_entry The new enclosed GtkEntry instance
 * @param[in] history_id The history id for the GnomeEntry
 * @param[in] widget_name The name the enclosing GnomeEntry widget shall have
 * @param[in] entry_name The name the enclosed GtkEntry widget shall have
 * @param[in] tooltip An optional (can be `NULL`) tooltip text for the GtkEntry
 */
void gtt_create_gnome_entry(
    GtkWidget **const outer_gnome_entry, GtkWidget **const inner_gtk_entry,
    const gchar *const history_id, const gchar *const widget_name,
    const gchar *const entry_name, const gchar *const tooltip
)
{
    *outer_gnome_entry = gnome_entry_new(history_id);
    gnome_entry_set_max_saved(GNOME_ENTRY(*outer_gnome_entry), 10);
    gtk_widget_set_name(*outer_gnome_entry, widget_name);

    *inner_gtk_entry = gnome_entry_gtk_entry(GNOME_ENTRY(*outer_gnome_entry));
    gtk_entry_set_activates_default(GTK_ENTRY(*inner_gtk_entry), FALSE);
    gtk_entry_set_editable(GTK_ENTRY(*inner_gtk_entry), TRUE);
    gtk_entry_set_has_frame(GTK_ENTRY(*inner_gtk_entry), TRUE);
    gtk_entry_set_invisible_char(GTK_ENTRY(*inner_gtk_entry), '*');
    gtk_entry_set_max_length(GTK_ENTRY(*inner_gtk_entry), 0);
    gtk_entry_set_visibility(GTK_ENTRY(*inner_gtk_entry), TRUE);
    gtk_widget_set_can_focus(*inner_gtk_entry, TRUE);
    gtk_widget_set_name(*inner_gtk_entry, entry_name);
    if (NULL != tooltip)
    {
        gtk_widget_set_tooltip_text(*inner_gtk_entry, tooltip);
    }
    gtk_widget_show(*inner_gtk_entry);

    gtk_widget_show(*outer_gnome_entry);
}

/* ============================================================== */

/* Glade loader, it will look in the right directories */
GladeXML *gtt_glade_xml_new(const char *filename, const char *widget)
{
    GladeXML *xml = NULL;

    g_return_val_if_fail(filename != NULL, NULL);

    if (g_file_test(filename, G_FILE_TEST_EXISTS))
        xml = glade_xml_new(filename, widget, NULL);

    if (xml == NULL)
    {
        char *file = g_concat_dir_and_file(GTTGLADEDIR, filename);
        xml = glade_xml_new(file, widget, NULL);
        g_free(file);
    }
    return xml;
}

static GtkBuilder *gtt_builder_new_from_exact_file(const char *filename)
{
    GtkBuilder *builder = NULL;
    GError *error = NULL;

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, filename, &error))
    {
        g_error("failed to add UI from file %s: %s", filename, error->message);
    }

    return builder;
}

/* GtkBuilder loader, it will look in the right directories */
GtkBuilder *gtt_builder_new_from_file(const char *filename)
{
    GtkBuilder *builder = NULL;

    g_return_val_if_fail(filename != NULL, NULL);

    if (g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        builder = gtt_builder_new_from_exact_file(filename);
    }
    else
    {
        char *file = g_concat_dir_and_file(GTTGLADEDIR, filename);
        builder = gtt_builder_new_from_exact_file(file);
        g_free(file);
    }

    return builder;
}

/* ============================================================== */
/* Used to be in qof, but is now deprecated there. */

size_t xxxqof_print_hours_elapsed_buff(char *buff, size_t len, int secs, gboolean show_secs)
{
    size_t flen;
    if (0 <= secs)
    {
        if (show_secs)
        {
            flen = g_snprintf(
                buff, len, "%02d:%02d:%02d", (int) (secs / 3600), (int) ((secs % 3600) / 60),
                (int) (secs % 60)
            );
        }
        else
        {
            flen = g_snprintf(
                buff, len, "%02d:%02d", (int) (secs / 3600), (int) ((secs % 3600) / 60)
            );
        }
    }
    else
    {
        if (show_secs)
        {
            flen = g_snprintf(
                buff, len, "-%02d:%02d:%02d", (int) (-secs / 3600), (int) ((-secs % 3600) / 60),
                (int) (-secs % 60)
            );
        }
        else
        {
            flen = g_snprintf(
                buff, len, "-%02d:%02d", (int) (-secs / 3600), (int) ((-secs % 3600) / 60)
            );
        }
    }
    return flen;
}

size_t xxxqof_print_date_time_buff(char *buff, size_t len, time_t secs)
{
    int flen;
    int day, month, year, hour, min;
    struct tm ltm, gtm;

    if (!buff)
        return 0;
    ltm = *localtime(&secs);
    day = ltm.tm_mday;
    month = ltm.tm_mon + 1;
    year = ltm.tm_year + 1900;
    hour = ltm.tm_hour;
    min = ltm.tm_min;
    // sec = ltm.tm_sec;
    switch (qof_date_format_get_current())
    {
    case QOF_DATE_FORMAT_UK:
        flen = g_snprintf(buff, len, "%2d/%2d/%-4d %2d:%02d", day, month, year, hour, min);
        break;
    case QOF_DATE_FORMAT_CE:
        flen = g_snprintf(buff, len, "%2d.%2d.%-4d %2d:%02d", day, month, year, hour, min);
        break;
    case QOF_DATE_FORMAT_ISO:
        flen = g_snprintf(buff, len, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, min);
        break;
    case QOF_DATE_FORMAT_UTC:
    {
        gtm = *gmtime(&secs);
        flen = strftime(buff, len, QOF_UTC_DATE_FORMAT, &gtm);
        break;
    }
    case QOF_DATE_FORMAT_LOCALE:
    {
        flen = strftime(buff, len, QOF_D_T_FMT, &ltm);
    }
    break;

    case QOF_DATE_FORMAT_US:
    default:
        flen = g_snprintf(buff, len, "%2d/%2d/%-4d %2d:%02d", month, day, year, hour, min);
        break;
    }
    return flen;
}

size_t xxxqof_print_time_buff(gchar *buff, size_t len, time_t secs)
{
    gint flen;
    struct tm ltm, gtm;

    if (!buff)
        return 0;
    if (qof_date_format_get_current() == QOF_DATE_FORMAT_UTC)
    {
        gtm = *gmtime(&secs);
        flen = strftime(buff, len, QOF_UTC_DATE_FORMAT, &gtm);
        return flen;
    }
    ltm = *localtime(&secs);
    flen = strftime(buff, len, QOF_T_FMT, &ltm);

    return flen;
}

static void xxxgnc_tm_set_day_start(struct tm *tm)
{
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    tm->tm_isdst = -1;
}

size_t xxxqof_print_date_dmy_buff(char *buff, size_t len, int day, int month, int year)
{
    int flen;
    if (!buff)
        return 0;
    switch (qof_date_format_get_current())
    {
    case QOF_DATE_FORMAT_UK:
        flen = g_snprintf(buff, len, "%2d/%2d/%-4d", day, month, year);
        break;
    case QOF_DATE_FORMAT_CE:
        flen = g_snprintf(buff, len, "%2d.%2d.%-4d", day, month, year);
        break;
    case QOF_DATE_FORMAT_LOCALE:
    {
        struct tm tm_str;
        time_t t;
        tm_str.tm_mday = day;
        tm_str.tm_mon = month - 1;
        tm_str.tm_year = year - 1900;
        xxxgnc_tm_set_day_start(&tm_str);
        t = mktime(&tm_str);
        localtime_r(&t, &tm_str);
        flen = strftime(buff, len, QOF_D_FMT, &tm_str);
        if (flen != 0)
            break;
    }
    case QOF_DATE_FORMAT_ISO:
    case QOF_DATE_FORMAT_UTC:
        flen = g_snprintf(buff, len, "%04d-%02d-%02d", year, month, day);
        break;
    case QOF_DATE_FORMAT_US:
    default:
        flen = g_snprintf(buff, len, "%2d/%2d/%-4d", month, day, year);
        break;
    }
    return flen;
}

size_t xxxqof_print_date_buff(char *buff, size_t len, time_t t)
{
    struct tm *theTime;
    if (!buff)
        return 0;
    theTime = localtime(&t);
    return xxxqof_print_date_dmy_buff(
        buff, len, theTime->tm_mday, theTime->tm_mon + 1, theTime->tm_year + 1900
    );
}

size_t xxxqof_print_minutes_elapsed_buff(char *buff, size_t len, int secs, gboolean show_secs)
{
    size_t flen;
    if (0 <= secs)
    {
        if (show_secs)
        {
            flen = g_snprintf(buff, len, "%02d:%02d", (int) (secs / 60), (int) (secs % 60));
        }
        else
        {
            flen = g_snprintf(buff, len, "%02d", (int) (secs / 60));
        }
    }
    else
    {
        if (show_secs)
        {
            flen = g_snprintf(buff, len, "-%02d:%02d", (int) (-secs / 60), (int) (-secs % 60));
        }
        else
        {
            flen = g_snprintf(buff, len, "-%02d", (int) (-secs / 60));
        }
    }
    return flen;
}

gboolean xxxqof_is_same_day(time_t ta, time_t tb)
{
    struct tm lta, ltb;
    lta = *localtime(&ta);
    ltb = *localtime(&tb);
    if (lta.tm_year == ltb.tm_year)
    {
        return (ltb.tm_yday - lta.tm_yday);
    }
    return (ltb.tm_year - lta.tm_year) * 365; /* very approximate */
}

/* ===================== END OF FILE ============================ */
