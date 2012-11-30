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
 * Filename:      projects-tree.h
 * Author:        Goedson Teixeira Paixao <goedson@debian.org>
 * Description:   Interface to the ProjectsTree component for Gnotime
 *
 * Created at:    Thu Nov 22 11:55:01 2007
 * Modified at:   Thu Nov 22 18:45:48 2007
 * Modified by:   Goedson Teixeira Paixao <goedson@debian.org>
 ********************************************************************/

#ifndef __PROJECTS_TREE_H__
#define __PROJECTS_TREE_H__

#include <gtk/gtk.h>

#include "proj.h"


#define GTT_TYPE_PROJECTS_TREE		(gtt_projects_tree_get_type ())
#define GTT_PROJECTS_TREE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTT_TYPE_PROJECTS_TREE, GttProjectsTree))
#define GTT_PROJECTS_TREE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTT_TYPE_PROJECTS_TREE, GttProjectsTreeClass))
#define GTT_IS_PROJECTS_TREE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTT_TYPE_PROJECTS_TREE))
#define GTT_IS_PROJECTS_TREE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTT_TYPE_PROJECTS_TREE))
#define GTT_PROJECTS_TREE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GTT_TYPE_PROJECTS_TREE, GttProjectsTreeClass))


typedef struct _GttProjectsTree GttProjectsTree;
typedef struct _GttProjectsTreeClass GttProjectsTreeClass;

struct _GttProjectsTree
{
	GtkTreeView parent;
};

struct _GttProjectsTreeClass
{
	GtkTreeViewClass parent_class;
	
};


/* Creators */
GType gtt_projects_tree_get_type (void);

/* Create a new GttProjectsTree component */
GttProjectsTree *gtt_projects_tree_new (void);

/* Populates the tree model with data of the projects in the
 * give project list.
 */
void gtt_projects_tree_populate (GttProjectsTree *ptree,
								 GList *plist,
								 gboolean recursive);

void gtt_projects_tree_set_visible_columns (GttProjectsTree *project_tree,
											GList *columns);

void gtt_projects_tree_update_project_data (GttProjectsTree *gpt, GttProject *prj);

void gtt_projects_tree_set_active_bgcolor (GttProjectsTree *gpt, gchar *color);
gchar * gtt_projects_tree_get_active_bgcolor (GttProjectsTree *gpt);
void gtt_projects_tree_set_show_seconds (GttProjectsTree *gpt, gboolean show_seconds);
gboolean gtt_projects_tree_get_show_seconds (GttProjectsTree *gpt);
void gtt_projects_tree_set_highlight_active (GttProjectsTree *gpt, gboolean highlight_active);
gboolean gtt_projects_tree_get_highlight_active (GttProjectsTree *gpt);
GttProject *gtt_projects_tree_get_selected_project (GttProjectsTree *gpt);
void gtt_projects_tree_update_all_rows (GttProjectsTree *gpt);
void gtt_projects_tree_remove_project (GttProjectsTree *gpt, GttProject *prj);
void gtt_projects_tree_append_project (GttProjectsTree *gpt, GttProject *prj, GttProject *sibling);
void gtt_projects_tree_insert_project_before (GttProjectsTree *gpt, GttProject *prj, GttProject *sibling);
gchar *gtt_projects_tree_get_expander_state (GttProjectsTree *gpt);
void gtt_projects_tree_set_expander_state (GttProjectsTree *gpt, gchar *states);

gint gtt_projects_tree_get_col_width (GttProjectsTree *gpt, int col);
void gtt_projects_tree_set_col_width (GttProjectsTree *gpt, int col, int width);

GtkTreeViewColumn *gtt_projects_tree_get_column_by_name (GttProjectsTree *gpt, gchar *column_name);

void gtt_projects_tree_set_sorted_column (GttProjectsTree *gpt, GtkTreeViewColumn *column);

#endif // __PROJECTS_TREE_H__
