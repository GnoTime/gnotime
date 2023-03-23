/*
 * GnoTime - a time tracker
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * Copyright (C) 2023      Markus Prasser
 * All rights reserved.
 *
 * This file is part of GnoTime (originally the Gnome Library).
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */

#ifndef GTT_DATE_EDIT_H
#define GTT_DATE_EDIT_H

#include <gtk/gtk.h>

#include <time.h>

typedef enum
{
    GTT_DATE_EDIT_SHOW_TIME = 1 << 0,
    GTT_DATE_EDIT_24_HR = 1 << 1,
    GTT_DATE_EDIT_WEEK_STARTS_ON_MONDAY = 1 << 2,
    GTT_DATE_EDIT_DISPLAY_SECONDS = 1 << 3
} GttDateEditFlags;

#define GTT_TYPE_DATE_EDIT (gtt_date_edit_get_type())
#define GTT_DATE_EDIT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTT_TYPE_DATE_EDIT, GttDateEdit))
#define GTT_DATE_EDIT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GTT_TYPE_DATE_EDIT, GttDateEditClass))
#define GTT_IS_DATE_EDIT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTT_TYPE_DATE_EDIT))
#define GTT_IS_DATE_EDIT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTT_TYPE_DATE_EDIT))
#define GTT_DATE_EDIT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GTT_TYPE_DATE_EDIT, GttDateEditClass))

typedef struct _GttDateEdit GttDateEdit;
typedef struct _GttDateEditPrivate GttDateEditPrivate;
typedef struct _GttDateEditClass GttDateEditClass;

struct _GttDateEdit
{
    GtkHBox hbox;

    // < private >
    GttDateEditPrivate *_priv;
};

struct _GttDateEditClass
{
    GtkHBoxClass parent_class;

    void (*date_changed)(GttDateEdit *gde);
    void (*time_changed)(GttDateEdit *gde);

    // Padding for possible expansion
    gpointer padding1;
    gpointer padding2;
};

GType gtt_date_edit_get_type(void) G_GNUC_CONST;
GtkWidget *gtt_date_edit_new(time_t the_time, gboolean show_time, gboolean use_24_format);
GtkWidget *gtt_date_edit_new_flags(time_t the_time, GttDateEditFlags flags);

// Note that everything that can be achieved with gtt_date_edit_new can be achieved with
// gtt_date_edit_new_flags, so that's why this call is like the _new_flags call
void gtt_date_edit_construct(GttDateEdit *gde, time_t the_time, GttDateEditFlags flags);

void gtt_date_edit_set_time(GttDateEdit *gde, time_t the_time);
time_t gtt_date_edit_get_time(GttDateEdit *gde);
void gtt_date_edit_set_popup_range(GttDateEdit *gde, int low_hour, int up_hour);
void gtt_date_edit_set_flags(GttDateEdit *gde, GttDateEditFlags flags);
int gtt_date_edit_get_flags(GttDateEdit *gde);

time_t gtt_date_edit_get_initial_time(GttDateEdit *gde);

#ifndef GTT_DISABLE_DEPRECATED
time_t gtt_date_edit_get_date(GttDateEdit *gde);
#endif // GTT_DISABLE_DEPRECATED

#endif // GTT_DATE_EDIT_H
