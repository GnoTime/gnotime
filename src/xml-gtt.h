/*   Convert gtt in-memory project structures to/from XML.
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

#ifndef __XML_GTT_H__
#define __XML_GTT_H__

#include <glib.h>

/* The gtt_xml_write() routine will all gtt data to xml file.
 *    If an error occurs, one of teh err-throw.h errors will
 *    be set.
 *
 * The gtt_xml_read_projects() will read a gtt XML file, and
 *    return a list of the projects that it found.  Note that
 *    this list has *not* been mashed into the global list of
 *    projects that gtt maintains.
 *
 * The gtt_xml_read_file() routine will read a gtt XML file,
 *    merging the data into the global list of projects
 *    that gtt maintains.
 */

void gtt_xml_read_file (const char * filename);
void gtt_xml_write_file (const char * filename);

GList * gtt_xml_read_projects (const char * filename);

#endif /* __XML_GTT_H__ */
