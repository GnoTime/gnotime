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


typedef enum {
	GTT_NO_ERR = 0,
	GTT_CANT_OPEN_FILE,
	GTT_CANT_WRITE_FILE,
	GTT_NOT_A_GTT_FILE,
	GTT_FILE_CORRUPT,
	GTT_UNKNOWN_TOKEN,
	GTT_UNKNOWN_VALUE,
	GTT_CANT_WRITE_CONFIG
} GttErrCode;


/* 
 * These two routines can be used to implement a poor-man's 
 * try-catch block by doing as follows:
 *
 *  gtt_err_set_code (GTT_NO_ERR);  // start of try block
 *  { do stuff ... }
 *  switch (gtt_err_get_code()) {     // catch block
 *     case GTT_NO ERR: break;
 *     case GTT_BOGUS_ERROR: { try to recover...}
 *  }
 */

GttErrCode gtt_err_get_code (void);

void gtt_err_set_code (GttErrCode);


/* The gtt_err_to_string() routine returns a handy-dandy human-readable
 *    error message, suitable for framing.  Be sure to free the returned 
 *    string using g_free when done.
 */
	
const char * gtt_err_to_string (GttErrCode code, const char * filename);
 

/* =========================== END OF FILE ======================== */
