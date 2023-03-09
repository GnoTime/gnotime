/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
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

/* GttEntry widget - combo box with auto-saved history
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GTT_ENTRY_H
#define GTT_ENTRY_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTT_TYPE_ENTRY (gtt_entry_get_type())
#define GTT_ENTRY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTT_TYPE_ENTRY, GttEntry))
#define GTT_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTT_TYPE_ENTRY, GttEntryClass))
#define GTT_IS_ENTRY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTT_TYPE_ENTRY))
#define GTT_IS_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTT_TYPE_ENTRY))
#define GTT_ENTRY_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GTT_TYPE_ENTRY, GttEntryClass))

/* This also supports the GtkEditable interface so
 * to get text use the gtk_editable_get_chars method
 * on this object */

typedef struct _GttEntry GttEntry;
typedef struct _GttEntryPrivate GttEntryPrivate;
typedef struct _GttEntryClass GttEntryClass;

struct _GttEntry
{
    GtkCombo combo;

    /*< private >*/
    GttEntryPrivate *_priv;
};

struct _GttEntryClass
{
    GtkComboClass parent_class;

    /* Like the GtkEntry signals */
    void (*activate)(GttEntry *entry);

    gpointer reserved1, reserved2; /* Reserved for future use,
                      we'll need to proxy insert_text
                      and delete_text signals */
};

GType gtt_entry_get_type(void) G_GNUC_CONST;
GtkWidget *gtt_entry_new(const gchar *history_id);

/* for language bindings and subclassing, use gtt_entry_new */

GtkWidget *gtt_entry_gtk_entry(GttEntry *gentry);

void gtt_entry_set_max_saved(GttEntry *gentry, guint max_saved);

G_END_DECLS

#endif // GTT_ENTRY_H
