/*   Implement a catch-throw-like error mechanism for gtt
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

#include <glib.h>
#include <gnome.h>  /* needed only to define the _() macro */

#include "err-throw.h"
 

static GttErrCode err = GTT_NO_ERR;

void 
gtt_err_set_code (GttErrCode code)
{
	/* clear the error if requested. */
	if (GTT_NO_ERR == code) 
	{
		err = GTT_NO_ERR;
		return;
	}
	
	/* if the error code is already set, don't over-write it */
	if (GTT_NO_ERR != err) return;

	/* Otherwise set it. */
	err = code;
}

GttErrCode 
gtt_err_get_code (void)
{
	return err;
}

/* ================================================================ */

const char *
gtt_err_to_string (GttErrCode code, const char * filename)
{
	const char * ret = NULL;
	switch (code)
	{
		case GTT_NO_ERR:
			ret = g_strdup (_("No Error"));
			break;
		case GTT_CANT_OPEN_FILE:
			ret = g_strdup_printf (
				_("Cannot open the project data file\n\t%s\n"),
				filename);
			break;
		case GTT_CANT_WRITE_FILE:
			ret = g_strdup_printf (
				_("Cannot write the project data file\n\t%s\n"),
				filename);
			break;
		case GTT_NOT_A_GTT_FILE:
			ret = g_strdup_printf (
				_("The file\n\t%s\n"
				  "doesn't seem to be a GTT project data file\n"),
				filename);
			break;
		case GTT_FILE_CORRUPT:
			ret = g_strdup_printf (
				_("The file\n\t%s\n"
				  "seems to be corrupt\n"),
				filename);
			break;
		case GTT_UNKNOWN_TOKEN:
			ret = g_strdup_printf (
				_("An unknown token was found during the parsing of\n\t%s\n"),
				filename);
			break;
		case GTT_UNKNOWN_VALUE:
			ret = g_strdup_printf (
				_("An unknown value was found during the parsing of\n\t%s\n"),
				filename);
			break;
		case GTT_CANT_WRITE_CONFIG:
			ret = g_strdup_printf (
				_("Cannot write the config file\n\t%s\n"),
				filename);
			break;
	}
	return ret;
}

/* =========================== END OF FILE ======================== */
