/* GSettings configuration handling for GnoTime - a time tracker
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
 * @brief Retrieve a maybe string GSettings option
 * @param settings The GSettings object to  retrieve the value from
 * @param key The key of the value to be retrieved
 * @param[in,out] value Pointer to hold the string value, will be `g_free`'d in case it already
 *   holds a value
 */
void gtt_gsettings_get_maybe_str(
    GSettings *const settings, const gchar *const key, gchar **const value
)
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
void gtt_gsettings_get_str(
    GSettings *const settings, const gchar *const key, gchar **const value
)
{
    if (NULL != *value)
    {
        g_free(*value);
        *value = NULL;
    }

    *value = g_settings_get_string(settings, key);
}

/**
 * @brief Set a boolean GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_bool(
    GSettings *const settings, const gchar *const key, const gboolean value
)
{
    if (FALSE == g_settings_set_boolean(settings, key, value))
    {
        g_warning(
            "Failed to set boolean option \"%s\" to value: %s", key,
            (TRUE == value) ? "true" : "false"
        );
    }
}

/**
 * @brief Set an integer GSettings option and log a message on error
 * @param settings The GSettings object to set the value on
 * @param key The key of the value to be set
 * @param value The actual value to be set
 */
void gtt_gsettings_set_int(GSettings *const settings, const gchar *const key, const gint value)
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
void gtt_gsettings_set_maybe_str(
    GSettings *const settings, const gchar *const key, const gchar *const value
)
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
void gtt_gsettings_set_str(
    GSettings *const settings, const gchar *const key, const gchar *const value
)
{
    if (FALSE == g_settings_set_string(settings, key, value))
    {
        g_warning("Failed to set string option \"%s\" to value: \"%s\"", key, value);
    }
}
