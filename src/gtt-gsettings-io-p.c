/* GSettings input/output handling for GnoTime - a time tracker
 * Copyright (C) 2023      Markus Prasser
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

#include "gtt-gsettings-io-p.h"

/**
 * @brief Retrieve an array of integer GSettings option
 * @param settings The GSettings object to  retrieve the value from
 * @param key The key of the value to be retrieved
 * @return A newly-allocated single-linked list representing the array of integers
 */
GSList *gtt_gsettings_get_array_int(GSettings *settings, const gchar *key)
{
    GSList *ret = NULL;

    GVariant *var = g_settings_get_value(settings, key);

    const gsize n_items = g_variant_n_children(var);
    gsize i;
    for (i = 0; i < n_items; ++i)
    {
        gint val;
        g_variant_get_child(var, i, "i", &val);
        ret = g_slist_prepend(ret, GINT_TO_POINTER(val));
    }

    g_variant_unref(var);
    var = NULL;

    ret = g_slist_reverse(ret);

    return ret;
}

/**
 * @brief Retrieve an array of string GSettings option
 * @param settings The GSettings object to  retrieve the value from
 * @param key The key of the value to be retrieved
 * @return A newly-allocated single-linked list representing the array of strings
 */
GSList *gtt_gsettings_get_array_string(GSettings *const settings, const gchar *const key)
{
    GSList *ret = NULL;

    GVariant *var = g_settings_get_value(settings, key);

    const gsize n_items = g_variant_n_children(var);
    gsize i;
    for (i = 0; i < n_items; ++i)
    {
        GVariant *child_val = g_variant_get_child_value(var, i);

        ret = g_slist_prepend(ret, g_strdup(g_variant_get_string(child_val, NULL)));

        g_variant_unref(child_val);
        child_val = NULL;
    }

    g_variant_unref(var);
    var = NULL;

    ret = g_slist_reverse(ret);

    return ret;
}

/**
 * @brief Retrieve a maybe string GSettings option
 * @param settings The GSettings object to  retrieve the value from
 * @param key The key of the value to be retrieved
 * @param[in,out] value Pointer to hold the string value, will be `g_free`'d in case it already
 *   holds a value
 */
void gtt_gsettings_get_maybe_string(GSettings *settings, const gchar *key, gchar **value)
{
    if (NULL != *value)
    {
        g_free(*value);
        *value = NULL;
    }

    GVariant *val = g_settings_get_value(settings, key);

    GVariant *maybe_val = g_variant_get_maybe(val);
    if (NULL != maybe_val)
    {
        gsize len;
        *value = g_strdup(g_variant_get_string(maybe_val, &len));

        g_variant_unref(maybe_val);
        maybe_val = NULL;
    }

    g_variant_unref(val);
    val = NULL;
}

/**
 * @brief Retrieve a string GSettings option
 * @param settings The GSettings object to  retrieve the value from
 * @param key The key of the value to be retrieved
 * @param[in,out] value Pointer to hold the string value, will be `g_free`'d in case it already
 *   holds a value
 */
void gtt_gsettings_get_string(GSettings *settings, const gchar *key, gchar **value)
{
    if (NULL != *value)
    {
        g_free(*value);
        *value = NULL;
    }

    *value = g_settings_get_string(settings, key);
}

/**
 * @brief Set an array of integer GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_array_int(GSettings *settings, const gchar *key, GSList *value)
{
    GVariantType *variant_type = g_variant_type_new("ai");

    GVariantBuilder bldr;
    g_variant_builder_init(&bldr, variant_type);

    GSList *node;
    for (node = value; node != NULL; node = node->next)
    {
        g_variant_builder_add(&bldr, "i", GPOINTER_TO_INT(node->data));
    }

    GVariant *val = g_variant_builder_end(&bldr);
    if (G_UNLIKELY(FALSE == g_settings_set_value(settings, key, val)))
    {
        g_warning("Failed to set integer array option \"%s\"", key);
    }

    g_variant_type_free(variant_type);
    variant_type = NULL;
}

/**
 * @brief Set an array of string GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_array_string(
    GSettings *const settings, const gchar *const key, GSList *const value
)
{
    GVariantType *variant_type = g_variant_type_new("as");

    GVariantBuilder bldr;
    g_variant_builder_init(&bldr, variant_type);

    GSList *node;
    for (node = value; node != NULL; node = node->next)
    {
        g_variant_builder_add(&bldr, "s", node->data);
    }

    GVariant *val = g_variant_builder_end(&bldr);
    if (G_UNLIKELY(FALSE == g_settings_set_value(settings, key, val)))
    {
        g_warning("Failed to set string array option \"%s\"", key);
    }

    g_variant_type_free(variant_type);
    variant_type = NULL;
}

/**
 * @brief Set a boolean GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_bool(GSettings *settings, const gchar *key, gboolean value)
{
    if (FALSE == g_settings_set_boolean(settings, key, value))
    {
        g_warning(
            "Failed to set boolean option \"%s\" to value: %s", key,
            (FALSE == value) ? "false" : "true"
        );
    }
}

/**
 * @brief Set an integer GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_int(GSettings *settings, const gchar *key, gint value)
{
    if (FALSE == g_settings_set_int(settings, key, value))
    {
        g_warning("Failed to set integer option \"%s\" to value: %d", key, value);
    }
}

/**
 * @brief Set a maybe string GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_maybe_string(GSettings *settings, const gchar *key, const gchar *value)
{
    if (FALSE == g_settings_set(settings, key, "ms", value))
    {
        g_warning(
            "Failed to set maybe string option \"%s\" to value: \"%s\"", key,
            (NULL != value) ? value : "<nothing>"
        );
    }
}

/**
 * @brief Set a string GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_string(GSettings *settings, const gchar *key, const gchar *value)
{
    if (FALSE == g_settings_set_string(settings, key, value))
    {
        g_warning("Failed to set string option \"%s\" to value: \"%s\"", key, value);
    }
}
