/*   mySQL I/O routines for GTimeTracker
 *   Copyright (C) 2001 Thomas Langaas <tlan@initio.no>
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */


#include <mysql/mysql.h>

#include "mysqlinterface.h"

static GteErrCode err = GTT_NO_ERR;


GttProject *gtt_mysql_loadprojectdefaults(void)
{
}

void gtt_mysql_savedata(GList *projectlist)
{
}

GList *gtt_mysql_loaddata(void)
{
}

GttErrCode gtt_mysql_geterror(void)
{
	return err;
}

void gtt_mysql_clearerror(void)
{
	err = GTT_NO_ERR;
}
