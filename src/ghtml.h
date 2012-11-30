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

#include <qof.h>

#include "proj.h"

/* GHTML == guile-parsed html.  These routines will read in html
 * files with embedded scheme code, evaluate the scheme, and output
 * plain-old html to the indicated stream.
 *
 * By appropriately supplying the stream structure, gtt HTML data
 * can be sent anywhere desired. For example, this could, in theory
 * be used inside a cgi-bin script.  (This is a plannned, multi-user,
 * web-based version that we hope to code up someday).  Currently,
 * the stream is used to push data into GtkHTML, and also to fwrite()
 * for the save-to-file function.
 *
 * The X that can be Y is not the true X.
 */

typedef struct gtt_ghtml_s GttGhtml;

struct gtt_ghtml_s
{
	/* stream interface for writing */
	void (*open_stream) (GttGhtml *, gpointer);
	void (*write_stream) (GttGhtml *, const char *, size_t len, gpointer);
	void (*close_stream) (GttGhtml *, gpointer);
	void (*error) (GttGhtml *, int errcode, const char * msg, gpointer);
	gpointer user_data;

	/* open_count and ref_path used for recursive file includes */
	int open_count;
	const char * ref_path;

	/* Key-Value Pair data; includes HTML form GET/POST results. */
	KvpFrame *kvp;
	
	/* The 'linked' project */
	GttProject *prj;
	
	/* List of projects, returned as query result */
	GList *query_result;
	gboolean did_query; /* TRUE if query was run */
	
	gboolean show_links; /* Flag -- show internal <a href> links */
	gboolean really_hide_links; /* Flag -- show internal <a href> links */

	time_t last_ivl_time;  /* hack for pretty-printing interval dates */

	/* ------------------------------------------------------ */
	/* Deprecated portion of this struct -- will go away someday. */
	/* Used only by ghtml-deprecated.c */
	/* Table layout info */

	gboolean show_html;  /* Flag -- add html markup, or not */

	/* field delimiter, for tab/comma delim */
	char * delim;

#define NCOL 30
	int ntask_cols;
	int task_cols[NCOL];
	char * task_titles[NCOL];

	int ninvl_cols;
	int invl_cols[NCOL];
	char * invl_titles[NCOL];

	char **tp;
};

extern GttGhtml *ghtml_guile_global_hack;


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

/** The gtt_ghtml_display() routine will parse the indicated gtt file,
 *     and output standard HTML to the indicated stream.
 */
void gtt_ghtml_display (GttGhtml *, const char *path_frag, GttProject *prj);

/** The gtt_gthml_show_links() routine will set a flag indicating whether
 *     the output html should include internal <a href> links.  Normally,
 *     this should be set to TRUE when displaying in the internal browser,
 *     and FALSE when printing.
 */
void gtt_ghtml_show_links (GttGhtml *, gboolean);

/** The gtt_ghtml_resolve_path() routine helps find the fully-qualified
 *     path name to the indicated filename, so that the file can be opened.
 *     The 'reference path', if not null, is checked first.  It is checked
 *     by finding its trailing slash, and appending the path_frag to it,
 *     and checking for existance.  If the reference is NULL, or the file
 *     is not found, then the standard gnotime data dirs are checked.
 *     The checked data dirs are locale-dependent.
 */
char * gtt_ghtml_resolve_path (const char *path_frag, const char *reference_path);

#endif /* __GTT_GHTML_H__ */

