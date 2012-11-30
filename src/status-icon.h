/*********************************************************************
 *
 * Copyright (C) 2007,  Goedson Teixeira Paixao
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Filename:      status-icon.h
 * Author:        Goedson Teixeira Paixao <goedson@debian.org>
 * Description:   Interface for GnoTime's status icon implementation
 *
 * Created at:    Fri Oct 12 11:42:48 2007
 * Modified at:   Fri Oct 12 22:44:06 2007
 * Modified by:   Goedson Teixeira Paixao <goedson@debian.org>
 ********************************************************************/

#include <glib.h>
#include "proj.h"

void gtt_status_icon_create();
void gtt_status_icon_destroy();
void gtt_status_icon_start_timer(GttProject *prj);
void gtt_status_icon_stop_timer(GttProject *prj);
