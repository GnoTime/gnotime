/*   Generate guile-parsed html output for GTimeTracker - a time tracker
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

#ifndef __GTT_GHTML_H__
#define __GTT_GHTML_H__

#include "proj.h"

/* GHTML == guile-parsed html.  These routines will read in html
 * files with embedded scheme code, evaluate the scheme, and output 
 * plain-old html to the indicated stream.
 *
 * By appropriately supplying the stream structure, gtt HTML data
 * can be sent anywhere desired. For example, this could, in theory
 * be used inside a cgi-bin script.  (this is a plannned, multi-user,
 * web-based version that we hope to code up someday).  Currently, 
 * the only user of this interface is GtkHTML
 */

typedef struct gtt_ghtml_s GttGhtml;

GttGhtml * gtt_ghtml_new (void);
void gtt_ghtml_destroy (GttGhtml *p);

typedef void (*GttGhtmlOpenStream) (GttGhtml *, gpointer);
typedef void (*GttGhtmlWriteStream) (GttGhtml *, const char *, size_t len, gpointer);
typedef void (*GttGhtmlCloseStream) (GttGhtml *, gpointer);
typedef void (*GttGhtmlError) (GttGhtml *, int errcode, const char * msg, gpointer);

void gtt_ghtml_set_stream (GttGhtml *, gpointer user_data,
                                       GttGhtmlOpenStream, 
                                       GttGhtmlWriteStream,
                                       GttGhtmlCloseStream, 
                                       GttGhtmlError);

/* The gtt_ghtml_display() routine will parse the indicated gtt file, 
 * and output standard HTML to the indicated stream
 */
void gtt_ghtml_display (GttGhtml *, const char *path_frag, GttProject *prj);

#endif /* __GTT_GHTML_H__ */

