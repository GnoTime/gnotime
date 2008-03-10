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
 * Filename:      projects-tree.c
 * Author:        Goedson Teixeira Paixao <goedson@debian.org>
 * Description:   Implementation of the ProjectsTree component for
 *                Gnotime
 *                
 * Created at:    Thu Nov 22 18:23:49 2007
 * Modified at:   Sat Dec  1 17:56:41 2007
 * Modified by:   Goedson Teixeira Paixao <goedson@debian.org>
 ********************************************************************/

#include <glib.h>
#include <glib/gi18n.h>

#include "projects-tree.h"
#include "timer.h"

#define GTT_PROJECTS_TREE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTT_TYPE_PROJECTS_TREE, GttProjectsTreePrivate))

static void gtt_projects_tree_finalize (GObject *obj);


typedef struct _ColumnDefinition {
	gchar            *name;
	gint             model_column;
	GtkCellRenderer  *renderer;
	gchar            *value_property_name;
	gchar            *label;
	gint             default_width;
} ColumnDefinition;

typedef struct _ExpanderStateHelper {
	GtkTreeView *view;
	gchar *states;
	int   *row;
} ExpanderStateHelper;

/* Columns for the model */
typedef enum {
	/* data columns */
	TIME_EVER_COLUMN,
	TIME_YEAR_COLUMN,
	TIME_MONTH_COLUMN,
	TIME_WEEK_COLUMN,
	TIME_LASTWEEK_COLUMN,
	TIME_YESTERDAY_COLUMN,
	TIME_TODAY_COLUMN,
	TIME_TASK_COLUMN,
	TITLE_COLUMN,
	DESCRIPTION_COLUMN,
	TASK_COLUMN,
	ESTIMATED_START_COLUMN,
	ESTIMATED_END_COLUMN,
	DUE_DATE_COLUMN,
	SIZING_COLUMN,
	PERCENT_COLUMN,
	URGENCY_COLUMN,
	IMPORTANCE_COLUMN,
	STATUS_COLUMN,
	/* row configuration colums */
	BACKGROUND_COLOR_COLUMN,        /* Custom background color */
	WEIGHT_COLUMN,
	/* pointer to the project structure */
	GTT_PROJECT_COLUMN,
	/* total number of columns in model */
	NCOLS
} ModelColumn;

enum {
	COLUMNS_SETUP_DONE,
	LAST_SIGNAL
};


#define N_VIEWABLE_COLS (NCOLS - 3)

typedef struct _GttProjectsTreePrivate GttProjectsTreePrivate;



struct _GttProjectsTreePrivate {
	GtkCellRenderer  *text_renderer;
	GtkCellRenderer  *date_renderer;
	GtkCellRenderer  *time_renderer;
	GtkCellRenderer  *progress_renderer;
	gchar            *active_bgcolor;
	gboolean         show_seconds;
	gboolean         highlight_active;
	ColumnDefinition column_definitions[N_VIEWABLE_COLS];
	GTree            *row_references;
	GTree            *column_references;
	gulong            row_changed_handler;
	char *expander_states;
};

static guint projects_tree_signals[LAST_SIGNAL] = {0};

static void gtt_projects_tree_row_expand_collapse_callback (GtkTreeView *view, GtkTreeIter *iter, GtkTreePath *path, gpointer data);

static void gtt_projects_tree_model_row_changed_callback (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);

static void project_changed (GttProject *prj, gpointer user_data);

static void
gtt_projects_tree_create_model (GttProjectsTree *gpt)
{
	GtkTreeStore *tree_model;
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	tree_model = gtk_tree_store_new (NCOLS,
									 G_TYPE_STRING,      /* TIME_EVER_COLUMN */
									 G_TYPE_STRING,      /* TIME_YEAR_COLUMN */
									 G_TYPE_STRING,      /* TIME_MONTH_COLUMN */
									 G_TYPE_STRING,      /* TIME_WEEK_COLUMN */
									 G_TYPE_STRING,      /* TIME_LASTWEEK_COLUMN */
									 G_TYPE_STRING,      /* TIME_YESTERDAY_COLUMN */
									 G_TYPE_STRING,      /* TIME_TODAY_COLUMN */
									 G_TYPE_STRING,      /* TIME_TASK_COLUMN */
									 G_TYPE_STRING,      /* TITLE_COLUMN */
									 G_TYPE_STRING,      /* DESCRIPTION_COLUMN */
									 G_TYPE_STRING,      /* TASK_COLUMN */
									 G_TYPE_STRING,      /* ESTIMATED_START_COLUMN */
									 G_TYPE_STRING,      /* ESTIMATED_END_COLUMN */
									 G_TYPE_STRING,      /* DUE_DATE_COLUMN */
									 G_TYPE_INT,      /* SIZING_COLUMN */
									 G_TYPE_INT,      /* PERCENT_COLUMN */
									 G_TYPE_STRING,      /* URGENCY_COLUMN */
									 G_TYPE_STRING,      /* IMPORTANCE_COLUMN */
									 G_TYPE_STRING,      /* STATUS_COLUMN */
									 G_TYPE_STRING,  /* BACKGROUND_COLOR_COLUMN */
									 G_TYPE_INT,
									 G_TYPE_POINTER   /* GTT_POINTER_COLUMN */
		);
	gtk_tree_view_set_model (GTK_TREE_VIEW (gpt), GTK_TREE_MODEL (tree_model));

	priv->row_changed_handler = g_signal_connect (GTK_TREE_MODEL(tree_model),
												 "row-changed",
												  G_CALLBACK (gtt_projects_tree_model_row_changed_callback),
												 gpt);

}


G_DEFINE_TYPE(GttProjectsTree, gtt_projects_tree, GTK_TYPE_TREE_VIEW)


/* GDataCompareFunc for use in the row_references GTree */
static gint
project_cmp (gconstpointer a, gconstpointer b, gpointer user_data)
{
	GttProject *prj_a = (GttProject *) a;
	GttProject *prj_b = (GttProject *) b;
	return gtt_project_get_id (prj_a) - gtt_project_get_id (prj_b);
}

static void
gtt_projects_tree_init (GttProjectsTree* gpt)
{

	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);


	/* default properties values */
	priv->active_bgcolor = g_strdup("green");
	priv->show_seconds = TRUE;
	priv->highlight_active = TRUE;

	/* references to the rows */
	priv->row_references = g_tree_new_full (project_cmp, NULL, NULL, (GDestroyNotify) gtk_tree_row_reference_free);

	/* cell renderers used to render the tree */
	priv->text_renderer = gtk_cell_renderer_text_new ();
	g_object_ref (priv->text_renderer);

	priv->date_renderer = gtk_cell_renderer_text_new ();
	g_object_set (priv->date_renderer, "xalign", 0.5, NULL);
	g_object_ref (priv->date_renderer);

	priv->time_renderer = gtk_cell_renderer_text_new ();
	g_object_set (priv->time_renderer, "xalign", 0.5, NULL);
	g_object_ref (priv->time_renderer);

	priv->progress_renderer = gtk_cell_renderer_progress_new ();
	g_object_ref (priv->progress_renderer);


	/* Definition of the possible data columns to be displayed */
	priv->column_definitions[0].name = "time_ever";
	priv->column_definitions[0].model_column = TIME_EVER_COLUMN;
	priv->column_definitions[0].renderer = priv->time_renderer;
	priv->column_definitions[0].value_property_name = "text";
	priv->column_definitions[0].label = _("Total");
	priv->column_definitions[0].default_width = 72;

	priv->column_definitions[1].name = "time_year";
	priv->column_definitions[1].model_column = TIME_YEAR_COLUMN;
	priv->column_definitions[1].renderer = priv->time_renderer;
	priv->column_definitions[1].value_property_name = "text";
	priv->column_definitions[1].label = _("Year");
	priv->column_definitions[1].default_width = 72;

	priv->column_definitions[2].name = "time_month";
	priv->column_definitions[2].model_column = TIME_MONTH_COLUMN;
	priv->column_definitions[2].renderer = priv->time_renderer;
	priv->column_definitions[2].value_property_name = "text";
	priv->column_definitions[2].label = _("Month");
	priv->column_definitions[2].default_width = 72;

	priv->column_definitions[3].name = "time_week";
	priv->column_definitions[3].model_column = TIME_WEEK_COLUMN;
	priv->column_definitions[3].renderer = priv->time_renderer;
	priv->column_definitions[3].value_property_name = "text";
	priv->column_definitions[3].label = _("Week");
	priv->column_definitions[3].default_width = 72;

	priv->column_definitions[4].name = "time_lastweek";
	priv->column_definitions[4].model_column = TIME_LASTWEEK_COLUMN;
	priv->column_definitions[4].renderer = priv->time_renderer;
	priv->column_definitions[4].value_property_name = "text";
	priv->column_definitions[4].label = _("Last Week");
	priv->column_definitions[4].default_width = 72;

	priv->column_definitions[5].name = "time_yesterday";
	priv->column_definitions[5].model_column = TIME_YESTERDAY_COLUMN;
	priv->column_definitions[5].renderer = priv->time_renderer;
	priv->column_definitions[5].value_property_name = "text";
	priv->column_definitions[5].label = _("Yesterday");
	priv->column_definitions[5].default_width = 72;

	priv->column_definitions[6].name = "time_today";
	priv->column_definitions[6].model_column = TIME_TODAY_COLUMN;
	priv->column_definitions[6].renderer = priv->time_renderer;
	priv->column_definitions[6].value_property_name = "text";
	priv->column_definitions[6].label = _("Today");
	priv->column_definitions[6].default_width = 72;

	priv->column_definitions[7].name = "time_task";
	priv->column_definitions[7].model_column = TIME_TASK_COLUMN;
	priv->column_definitions[7].renderer = priv->time_renderer;
	priv->column_definitions[7].value_property_name = "text";
	priv->column_definitions[7].label = _("Entry");
	priv->column_definitions[7].default_width = 72;

	priv->column_definitions[8].name = "title";
	priv->column_definitions[8].model_column = TITLE_COLUMN;
	priv->column_definitions[8].renderer = priv->text_renderer;
	priv->column_definitions[8].value_property_name = "text";
	priv->column_definitions[8].label = _("Title");
	priv->column_definitions[8].default_width = 72;

	priv->column_definitions[9].name = "description";
	priv->column_definitions[9].model_column = DESCRIPTION_COLUMN;
	priv->column_definitions[9].renderer = priv->text_renderer;
	priv->column_definitions[9].value_property_name = "text";
	priv->column_definitions[9].label = _("Description");
	priv->column_definitions[9].default_width = 72;

	priv->column_definitions[10].name = "task";
	priv->column_definitions[10].model_column = TASK_COLUMN;
	priv->column_definitions[10].renderer = priv->text_renderer;
	priv->column_definitions[10].value_property_name = "text";
	priv->column_definitions[10].label = _("Diary Entry");
	priv->column_definitions[10].default_width = 72;

	priv->column_definitions[11].name = "estimated_start";
	priv->column_definitions[11].model_column = ESTIMATED_START_COLUMN;
	priv->column_definitions[11].renderer = priv->date_renderer;
	priv->column_definitions[11].value_property_name = "text";
	priv->column_definitions[11].label = _("Start");
	priv->column_definitions[11].default_width = 72;

	priv->column_definitions[12].name = "estimated_end";
	priv->column_definitions[12].model_column = ESTIMATED_END_COLUMN;
	priv->column_definitions[12].renderer = priv->date_renderer;
	priv->column_definitions[12].value_property_name = "text";
	priv->column_definitions[12].label = _("End");
	priv->column_definitions[12].default_width = 72;

	priv->column_definitions[13].name = "due_date";
	priv->column_definitions[13].model_column = DUE_DATE_COLUMN;
	priv->column_definitions[13].renderer = priv->date_renderer;
	priv->column_definitions[13].value_property_name = "text";
	priv->column_definitions[13].label = _("Due");
	priv->column_definitions[13].default_width = 72;

	priv->column_definitions[14].name = "sizing";
	priv->column_definitions[14].model_column = SIZING_COLUMN;
	priv->column_definitions[14].renderer = priv->date_renderer;
	priv->column_definitions[14].value_property_name = "text";
	priv->column_definitions[14].label = _("Size");
	priv->column_definitions[14].default_width = 72;

	priv->column_definitions[15].name = "percent_done";
	priv->column_definitions[15].model_column = PERCENT_COLUMN;
	priv->column_definitions[15].renderer = priv->progress_renderer;
	priv->column_definitions[15].value_property_name = "value";
	priv->column_definitions[15].label = _("Done");
	priv->column_definitions[15].default_width = 72;
	
	priv->column_definitions[16].name = "urgency";
	priv->column_definitions[16].model_column = URGENCY_COLUMN;
	priv->column_definitions[16].renderer = priv->text_renderer;
	priv->column_definitions[16].value_property_name = "text";
	priv->column_definitions[16].label = _("Urgency");
	priv->column_definitions[16].default_width = 72;
	
	priv->column_definitions[17].name = "importance";
	priv->column_definitions[17].model_column = IMPORTANCE_COLUMN;
	priv->column_definitions[17].renderer = priv->text_renderer;
	priv->column_definitions[17].value_property_name = "text";
	priv->column_definitions[17].label = _("Importance");
	priv->column_definitions[17].default_width = 72;
	
	priv->column_definitions[18].name = "status";
	priv->column_definitions[18].model_column = STATUS_COLUMN;
	priv->column_definitions[18].renderer = priv->text_renderer;
	priv->column_definitions[18].value_property_name = "text";
	priv->column_definitions[18].label = _("Status");
	priv->column_definitions[18].default_width = 72;

	gtt_projects_tree_create_model (gpt);
	gtt_projects_tree_set_visible_columns (gpt, NULL);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gpt), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (gpt), TRUE);
	gtk_tree_view_set_show_expanders (GTK_TREE_VIEW (gpt), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gpt), TRUE);
	gtk_tree_view_set_enable_tree_lines (GTK_TREE_VIEW (gpt), TRUE);

	g_signal_connect (GTK_TREE_VIEW (gpt),
					  "row-expanded",
					  G_CALLBACK (gtt_projects_tree_row_expand_collapse_callback),
					  NULL);
	g_signal_connect (GTK_TREE_VIEW (gpt),
					  "row-collapsed",
					  G_CALLBACK (gtt_projects_tree_row_expand_collapse_callback),
					  NULL);

}

static void
gtt_projects_tree_finalize (GObject *obj)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (obj);
	g_object_unref (priv->text_renderer);
	g_object_unref (priv->date_renderer);
	g_object_unref (priv->time_renderer);
	g_object_unref (priv->progress_renderer);
	g_tree_destroy (priv->row_references);
	g_tree_destroy (priv->column_references);
}

GttProjectsTree *
gtt_projects_tree_new (void)
{
	return g_object_new (gtt_projects_tree_get_type(), NULL);
}

static void
gtt_projects_tree_class_init (GttProjectsTreeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gtt_projects_tree_finalize;
	g_type_class_add_private (object_class, sizeof (GttProjectsTreePrivate));
	projects_tree_signals[COLUMNS_SETUP_DONE] = 
		g_signal_new ("columns_setup_done",
					  G_TYPE_FROM_CLASS (object_class),
					  G_SIGNAL_RUN_LAST,
					  0, NULL, NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
}

static void
gtt_projects_tree_set_time_value (GttProjectsTree *gpt, GtkTreeStore *tree_model, GtkTreeIter *iter, gint column, gint value)
{

	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	char buff[13];
	memset(buff, 0, 13);
	/* Format time and set column value */
	if (value == 0)
	{
		buff[0] = '-';
	}
	else
	{
		if (priv->show_seconds)
		{
			sprintf (buff, "%02d:%02d:%02d", value / 3600, (value / 60) % 60, value % 60);
		}
		else
		{
			sprintf (buff, "%02d:%02d", value / 3600, (value / 60) % 60);
		}
	}
	gtk_tree_store_set (tree_model, iter, column, buff, -1);
}


static void
gtt_projects_tree_set_date_value (GttProjectsTree *gpt, GtkTreeStore *tree_model, GtkTreeIter *iter, gint column, time_t value)
{
	gchar buff[100];
	memset(buff, 0, 100);
	if (value > -1)
	{
		strftime (buff, 100, "%x", localtime(&value));
	}
	else
	{
		buff[0] = '-';
	}
	gtk_tree_store_set (tree_model, iter, column, buff, -1);
}


static void
gtt_projects_tree_set_project_times (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (tree_model), iter);

	if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (gpt), path))
	{
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_EVER_COLUMN, gtt_project_get_secs_ever (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_YEAR_COLUMN, gtt_project_get_secs_year (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_MONTH_COLUMN, gtt_project_get_secs_month (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_WEEK_COLUMN, gtt_project_get_secs_week (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_LASTWEEK_COLUMN, gtt_project_get_secs_lastweek (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_YESTERDAY_COLUMN, gtt_project_get_secs_yesterday (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_TODAY_COLUMN, gtt_project_get_secs_day (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_TASK_COLUMN, gtt_project_get_secs_current (prj));
	}
	else
	{
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_EVER_COLUMN, gtt_project_total_secs_ever (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_YEAR_COLUMN, gtt_project_total_secs_year (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_MONTH_COLUMN, gtt_project_total_secs_month (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_WEEK_COLUMN, gtt_project_total_secs_week (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_LASTWEEK_COLUMN, gtt_project_total_secs_lastweek (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_YESTERDAY_COLUMN, gtt_project_total_secs_yesterday (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_TODAY_COLUMN, gtt_project_total_secs_day (prj));
		gtt_projects_tree_set_time_value (gpt, tree_model, iter, TIME_TASK_COLUMN, gtt_project_total_secs_current (prj));
	}
}


static void
gtt_projects_tree_set_project_dates (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{
	gtt_projects_tree_set_date_value (gpt, tree_model, iter, ESTIMATED_START_COLUMN, gtt_project_get_estimated_start (prj));
	gtt_projects_tree_set_date_value (gpt, tree_model, iter, ESTIMATED_END_COLUMN, gtt_project_get_estimated_end (prj));
	gtt_projects_tree_set_date_value (gpt, tree_model, iter, DUE_DATE_COLUMN, gtt_project_get_due_date (prj));
}

static void
gtt_projects_tree_set_style (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	PangoWeight weight = PANGO_WEIGHT_NORMAL;
	gchar *bgcolor = NULL;

	if (priv->highlight_active)
	{
		if (timer_project_is_running (prj))
		{
			bgcolor = priv->active_bgcolor;
			weight = PANGO_WEIGHT_BOLD;
		}
	}
	gtk_tree_store_set (tree_model,
						iter,
						BACKGROUND_COLOR_COLUMN, bgcolor,
						WEIGHT_COLUMN, weight,
						-1);
}

static void
gtt_projects_tree_set_project_urgency (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{
	gchar *value;
	switch (gtt_project_get_urgency (prj)) {
	case GTT_UNDEFINED: value = "-"; break;
	case GTT_LOW:    value = _("Low"); break;
	case GTT_MEDIUM: value = _("Med"); break;
	case GTT_HIGH:   value = _("High"); break;
	default: value = "-"; break;
	}


	gtk_tree_store_set (tree_model,
						iter,
						URGENCY_COLUMN, value,
						-1);
}

static void
gtt_projects_tree_set_project_importance (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{
	gchar *value;
	switch (gtt_project_get_importance (prj)) {
	case GTT_UNDEFINED: value = "-"; break;
	case GTT_LOW:    value = _("Low"); break;
	case GTT_MEDIUM: value = _("Med"); break;
	case GTT_HIGH:   value = _("High"); break;
	default: value = "-"; break;
	}

	gtk_tree_store_set (tree_model,
						iter,
						IMPORTANCE_COLUMN, value,
						-1);
}

static void
gtt_projects_tree_set_project_status (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{

	gchar *value;
	switch (gtt_project_get_status (prj)) {
	case GTT_NO_STATUS:   value = "-"; break;
	case GTT_NOT_STARTED: value = _("Not Started"); break;
	case GTT_IN_PROGRESS: value = _("In Progress"); break;
	case GTT_ON_HOLD:     value = _("On Hold"); break;
	case GTT_CANCELLED:   value = _("Cancelled"); break;
	case GTT_COMPLETED:   value = _("Completed"); break;
	default: value = "-"; break;
	}

	gtk_tree_store_set (tree_model,
						iter,
						STATUS_COLUMN, value,
						-1);
}

static void
gtt_projects_tree_set_project_data (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *iter)
{

	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	if (priv->row_changed_handler)
	{
		g_signal_handler_disconnect (tree_model, priv->row_changed_handler);
		priv->row_changed_handler = 0;
	}
	gtk_tree_store_set (tree_model,
						iter,
						TITLE_COLUMN, gtt_project_get_title (prj),
						DESCRIPTION_COLUMN, gtt_project_get_desc (prj),
						TASK_COLUMN, gtt_task_get_memo (gtt_project_get_first_task (prj)),
						SIZING_COLUMN, gtt_project_get_sizing (prj),
						PERCENT_COLUMN, gtt_project_get_percent_complete (prj),
						GTT_PROJECT_COLUMN, prj,
						-1);
	gtt_projects_tree_set_project_times (gpt, tree_model, prj, iter);
	gtt_projects_tree_set_project_dates (gpt, tree_model, prj, iter);
	gtt_projects_tree_set_style (gpt, tree_model, prj, iter);
	gtt_projects_tree_set_project_urgency (gpt, tree_model, prj, iter);
	gtt_projects_tree_set_project_importance (gpt, tree_model, prj, iter);
	gtt_projects_tree_set_project_status (gpt, tree_model, prj, iter);

	priv->row_changed_handler = g_signal_connect (GTK_TREE_MODEL(tree_model),
												  "row-changed",
												  G_CALLBACK (gtt_projects_tree_model_row_changed_callback),
												  gpt);
}

static void
gtt_projects_tree_add_project (GttProjectsTree *gpt, GtkTreeStore *tree_model, GttProject *prj, GtkTreeIter *parent, GtkTreePath *path, gboolean recursive)
{
	GtkTreeIter iter;
	GList *node;
	GtkTreePath *child_path;
	GtkTreeRowReference *row_reference;
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	
	gtk_tree_store_append (tree_model, &iter, parent);
	gtt_projects_tree_set_project_data (gpt, tree_model, prj, &iter);
	row_reference = gtk_tree_row_reference_new ( GTK_TREE_MODEL(tree_model), path);
	g_tree_insert (priv->row_references, prj, row_reference);
	gtt_project_add_notifier (prj, project_changed, gpt);
	if (recursive)
	{
		child_path = gtk_tree_path_copy (path);
		gtk_tree_path_down (child_path);
		for (node = gtt_project_get_children (prj); node; node = node->next, gtk_tree_path_next (child_path))
		{
			GttProject *sub_prj = node->data;
			gtt_projects_tree_add_project(gpt, tree_model, sub_prj, &iter, child_path, recursive);
		}
			gtk_tree_path_free (child_path);
	}
}


static void
gtt_projects_tree_populate_tree_store (GttProjectsTree *gpt,
									   GtkTreeStore *tree_model,
									   GList *prj_list,
									   gboolean recursive)
{
	GList *node;
	GtkTreePath *path = gtk_tree_path_new_first ();
	if (prj_list)
	{
		for (node = prj_list; node; node = node->next, gtk_tree_path_next (path))
		{
			GttProject *prj = node->data;
			gtt_projects_tree_add_project(gpt, tree_model, prj, NULL, path, recursive);
		}
	}
	gtk_tree_path_free (path);

}

void
gtt_projects_tree_populate (GttProjectsTree *proj_tree,
							GList *plist,
							gboolean recursive)
{
	GtkTreeStore *tree_model = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (proj_tree)));

	gtk_tree_store_clear (tree_model);
	gtt_projects_tree_populate_tree_store (proj_tree, tree_model, plist, recursive);
	gtk_tree_view_set_model (GTK_TREE_VIEW (proj_tree), GTK_TREE_MODEL(tree_model));
}


static void
gtt_projects_tree_add_column (GttProjectsTree *project_tree, gchar *column_name)
{

	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (project_tree);
	int index;
	GtkTreeViewColumn *column;

	for (index = 0; index < N_VIEWABLE_COLS; index ++)
	{
		if (!strcmp (column_name, priv->column_definitions[index].name))
		{
			ColumnDefinition *c = &(priv->column_definitions[index]);
			column = gtk_tree_view_column_new_with_attributes (c->label,
															   c->renderer,
															   c->value_property_name, c->model_column,
															   "cell-background", BACKGROUND_COLOR_COLUMN,
															   NULL);
			if (GTK_IS_CELL_RENDERER_TEXT (c->renderer))
			{
				gtk_tree_view_column_add_attribute (column, c->renderer, "weight", WEIGHT_COLUMN);
			}
			gtk_tree_view_column_set_resizable (column, TRUE);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
			gtk_tree_view_column_set_fixed_width (column, c->default_width);
			gtk_tree_view_column_set_clickable (column, TRUE);
			gtk_tree_view_append_column (GTK_TREE_VIEW (project_tree), column);
			if (!strcmp(c->name, "title"))
			{
				gtk_tree_view_set_expander_column (GTK_TREE_VIEW (project_tree), column);
			}
			g_tree_insert (priv->column_references, column_name, column);
			return;
		}
	}
	g_warning ("Illegal column name '%s' requested", column_name);
}

void
gtt_projects_tree_set_visible_columns (GttProjectsTree *project_tree,
									   GList *columns)
{
	GtkTreeViewColumn *column = NULL;
	GtkTreeView * tree_view = GTK_TREE_VIEW (project_tree);
	GList *p;
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (project_tree);

	g_tree_destroy (priv->column_references);
	priv->column_references = g_tree_new ((GCompareFunc) strcmp);


	/* remove all columns */
	while ((column = gtk_tree_view_get_column (tree_view, 0)))
	{
		gtk_tree_view_remove_column (tree_view, column);
	}


	/* The title column is mandatory. If it's not on the list, 
	   we add it as the first column. */

	if (!g_list_find_custom (columns, "title", (GCompareFunc) strcmp))
	{
		gtt_projects_tree_add_column (project_tree, "title");
	}


	for (p = columns; p; p = p->next)
	{
		gtt_projects_tree_add_column (project_tree, p->data);
	}
	
	g_signal_emit (project_tree, projects_tree_signals[COLUMNS_SETUP_DONE], 0);
}

static void
gtt_projects_tree_update_row_data (GttProjectsTree *gpt, GttProject *prj, GtkTreeRowReference *row_ref)
{
	g_return_if_fail (gtk_tree_row_reference_valid (row_ref));
	GtkTreePath *path = gtk_tree_row_reference_get_path (row_ref);
	if (path)
	{
		GtkTreeStore *tree_model = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (gpt)));
		
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_model), &iter, path))
		{
			gtt_projects_tree_set_project_data (gpt, tree_model, prj, &iter);
		}
		else
		{
			gchar *path_str = gtk_tree_path_to_string (path);
			g_warning ("Invalid path %s", path_str);
			g_free (path_str);
		}
		gtk_tree_path_free (path);
	}
}


void
gtt_projects_tree_update_project_data (GttProjectsTree *gpt, GttProject *prj)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	GtkTreeRowReference *row_ref = (GtkTreeRowReference *) g_tree_lookup (priv->row_references, prj);
	
	if (row_ref)
	{
		gtt_projects_tree_update_row_data (gpt, prj, row_ref);
	}
	else
	{
		g_warning ("Updating non-existant project");
	}
}


static gboolean
update_row_data (gpointer key, gpointer value, gpointer data)
{
	GttProject *prj = (GttProject *) key;
	GtkTreeRowReference *row_ref = (GtkTreeRowReference *) value;
	GttProjectsTree *gpt = GTT_PROJECTS_TREE (data);

	gtt_projects_tree_update_row_data (gpt, prj, row_ref);
	return FALSE;
}

void
gtt_projects_tree_update_all_rows (GttProjectsTree *gpt)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);

	g_tree_foreach (priv->row_references, update_row_data, gpt);

}

void
gtt_projects_tree_set_active_bgcolor (GttProjectsTree *gpt, gchar *color)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);

	if (priv->active_bgcolor)
	{
		g_free (priv->active_bgcolor);
	}

	if (color)
	{
		priv->active_bgcolor = g_strdup (color);
	}
	else
	{
		priv->active_bgcolor = NULL;
	}

	gtt_projects_tree_update_all_rows (gpt);

}

gchar *
gtt_projects_tree_get_active_bgcolor (GttProjectsTree *gpt)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	return priv->active_bgcolor;
}

void
gtt_projects_tree_set_show_seconds (GttProjectsTree *gpt, gboolean show_seconds)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	priv->show_seconds = show_seconds;
	gtt_projects_tree_update_all_rows (gpt);
}

gboolean
gtt_projects_tree_get_show_seconds (GttProjectsTree *gpt)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	return priv->show_seconds;
}

void
gtt_projects_tree_set_highlight_active (GttProjectsTree *gpt, gboolean highlight_active)
{
		GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
		priv->highlight_active = highlight_active;
		gtt_projects_tree_update_all_rows (gpt);
}

gboolean
gtt_projects_tree_get_highlight_active (GttProjectsTree *gpt)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	return priv->highlight_active;
}


GttProject *
gtt_projects_tree_get_selected_project (GttProjectsTree *gpt)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gpt));
		GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));
	GList *list = gtk_tree_selection_get_selected_rows(selection, NULL);
	GttProject *prj = NULL;
	if (list)
	{
		GtkTreePath *path = (GtkTreePath *)list->data;
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter (model, &iter, path))
		{
			gtk_tree_model_get (model, &iter, GTT_PROJECT_COLUMN, &prj, -1);
		}
		g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);
	}
	return prj;
}

static void
gtt_projects_tree_remove_project_recursively (GttProjectsTree *gpt,
											  GttProject *prj,
											  GtkTreeModel *model,
											  GTree *row_references)
{
	GtkTreeRowReference *row = g_tree_lookup (row_references, prj);

	if (row)
	{
		GtkTreePath *path = gtk_tree_row_reference_get_path (row);
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter (model, &iter, path))
		{
			gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
		}
		g_tree_remove (row_references, prj);
		gtk_tree_path_free (path);

		GList *node;
		for (node = gtt_project_get_children (prj); node; node = node->next)
		{
			GttProject *sub_prj = node->data;
			gtt_projects_tree_remove_project_recursively(gpt, sub_prj, model, row_references);
		}
	}
}

void
gtt_projects_tree_remove_project (GttProjectsTree *gpt, GttProject *prj)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));

	gtt_projects_tree_remove_project_recursively (gpt, prj, model, priv->row_references);
}


static void
gtt_projects_tree_append_projects_recursively (GttProjectsTree * gpt,
											   GtkTreeModel *model, GTree *row_references,
											   GttProject *prj, GttProject *parent)
{
	GtkTreeIter parent_iter;
	GtkTreeIter prj_iter;
	GtkTreePath *path = NULL;
	if (parent)
	{
		GtkTreeRowReference *row = g_tree_lookup (row_references, parent);
		path = gtk_tree_row_reference_get_path (row);
		gtk_tree_model_get_iter (model, &parent_iter, path);
		gtk_tree_store_append (GTK_TREE_STORE (model), &prj_iter, &parent_iter);
		gtk_tree_path_free (path);
	}
	else
	{
		gtk_tree_store_append (GTK_TREE_STORE (model), &prj_iter, NULL);
	}

	gtt_projects_tree_set_project_data (gpt, GTK_TREE_STORE (model),
										prj, &prj_iter);
	path = gtk_tree_model_get_path (model, &prj_iter);

	GtkTreeRowReference *row_reference = gtk_tree_row_reference_new ( model,
																	  path);
	g_tree_insert (row_references, prj, row_reference);
	gtk_tree_path_free (path);
	GList *children;
	for (children = gtt_project_get_children (prj); children; children = children->next)
	{
		gtt_projects_tree_append_projects_recursively (gpt, model,
													  row_references,
													  children->data, prj);
	}
}

/*
 * Appends a new project to the Projects Tree. The project is appended to
 * the list of children of its parent, if one is provided, or to the bottom
 * of the tree, otherwise.
 *
 * Parameters:
 *
 * - gpt - The GttProjectsTree to which the project should be appended
 * - prj - The project to be append
 * - parent - The parent of the project to be appended or NULL if it's
 *            a toplevel project.
 */

void
gtt_projects_tree_append_project (GttProjectsTree *gpt, GttProject *prj, GttProject *parent)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));

	gtt_projects_tree_append_projects_recursively (gpt, model, priv->row_references, prj, parent);
}



/*
 * Inserts a new project to the Projects Tree. The project is inserted
 * before its sibling, if one is provided, or to the bottom of the tree,
 * otherwise.
 *
 * Parameters:
 *
 * - gpt - The GttProjectsTree to which the project should be appended
 * - prj - The project to be append
 * - sibling - The sibling before wich the project will be inserted.
 */
void
gtt_projects_tree_insert_project_before (GttProjectsTree *gpt, GttProject *prj, GttProject *sibling)
{
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));

	GtkTreeIter sib_iter;
	GtkTreeIter prj_iter;
	GtkTreePath *path = NULL;
	if (sibling)
	{
		GtkTreeRowReference *row = g_tree_lookup (priv->row_references, sibling);
		path = gtk_tree_row_reference_get_path (row);
		gtk_tree_model_get_iter (model, &sib_iter, path);
		gtk_tree_store_insert_before (GTK_TREE_STORE (model), &prj_iter, NULL, &sib_iter);
		gtk_tree_path_free (path);
	}
	else
	{
		gtk_tree_store_append (GTK_TREE_STORE (model), &prj_iter, NULL);
	}

	gtt_projects_tree_set_project_data (gpt, GTK_TREE_STORE (model),
										prj, &prj_iter);
	path = gtk_tree_model_get_path (model, &prj_iter);

	GtkTreeRowReference *row_reference = gtk_tree_row_reference_new ( model,
																	  path);
	g_tree_insert (priv->row_references, prj, row_reference);
	gtk_tree_path_free (path);

	GList *children;
	for (children = gtt_project_get_children (prj); children; children = children->next)
	{
		gtt_projects_tree_append_projects_recursively (gpt, model,
													   priv->row_references,
													   children->data, prj);
	}

}

static void
gtt_projects_tree_row_expand_collapse_callback (GtkTreeView *view,
												GtkTreeIter *iter,
												GtkTreePath *path,
												gpointer data)
{
	GttProjectsTree *gpt = GTT_PROJECTS_TREE (view);
	GttProject *prj = NULL;
	GtkTreeModel *tree_model = gtk_tree_view_get_model (view);

	gtk_tree_model_get (tree_model, iter, GTT_PROJECT_COLUMN, &prj, -1);

	gtt_projects_tree_set_project_data (gpt, GTK_TREE_STORE (tree_model),
										prj, iter);

}


static void
gtt_projects_tree_model_row_changed_callback (GtkTreeModel *model,
											  GtkTreePath *path,
											  GtkTreeIter *iter,
											  gpointer data)
{
	GttProjectsTree *gpt = GTT_PROJECTS_TREE (data);
	GttProject *prj = NULL;
	GttProject *parent = NULL;

	gtk_tree_model_get (model, iter, GTT_PROJECT_COLUMN, &prj, -1);

	if (prj)
	{
		int path_depth = gtk_tree_path_get_depth (path);
		gint *indices = gtk_tree_path_get_indices (path);
		int position = 0;
		if (indices)
		{
			position = indices[path_depth - 1];
		}
		else
		{
			g_message ("indices is NULL");
		}
		GtkTreeIter parent_iter;
		if (gtk_tree_model_iter_parent (model, &parent_iter, iter))
		{
			gtk_tree_model_get (model, &parent_iter, GTT_PROJECT_COLUMN, &parent, -1);
		}

		GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);
		GtkTreeRowReference *row_ref = (GtkTreeRowReference *) g_tree_lookup (priv->row_references, prj);
		if (row_ref)
		{
			GtkTreePath *ref_path = gtk_tree_row_reference_get_path (row_ref);

			/* If the row has been moved around we reparent it and 
			   update its row reference */
			if (gtk_tree_path_compare (path, ref_path))
			{
				gtt_project_reparent (prj, parent, position);
				row_ref = gtk_tree_row_reference_new (model, path);
				g_tree_insert (priv->row_references, prj, row_ref);
			}
		}
	}
}



static gboolean
count_rows (GtkTreeModel *model, GtkTreeIter *iter, GtkTreePath *path, gpointer data)
{
	int *rows = (int *)data;
	++*rows;
	return FALSE;
}

static gboolean
get_expander_state (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	ExpanderStateHelper *esh = (ExpanderStateHelper *) data;
	if (gtk_tree_view_row_expanded (esh->view, path))
	{
		esh->states[*esh->row] = 'y';
	}
	else
	{
		esh->states[*esh->row] = 'n';
	}
	++(*esh->row);
	return FALSE;
}


char *
gtt_projects_tree_get_expander_state (GttProjectsTree *gpt)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));
	int rows = 0;

	gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) count_rows, &rows);

	ExpanderStateHelper esh;
	esh.view = GTK_TREE_VIEW (gpt);
	esh.states = g_new0(char, rows + 1);
	rows = 0;
	esh.row = &rows;

	gtk_tree_model_foreach (model, get_expander_state, &esh);

	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);

	if (priv->expander_states)
	{
		g_free (priv->expander_states);
	}

	priv->expander_states = esh.states;

	return priv->expander_states;
}

static gboolean
set_expander_state (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	ExpanderStateHelper *esh = (ExpanderStateHelper *) data;
	if (esh->states[*esh->row] == 'y')
	{
		gtk_tree_view_expand_row (esh->view, path, FALSE);
	}
	else
	{
		gtk_tree_view_collapse_row (esh->view, path);
	}
	++(*esh->row);
	return FALSE;
}

void
gtt_projects_tree_set_expander_state (GttProjectsTree *gpt, gchar *states)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpt));
	ExpanderStateHelper esh;
	int row = 0;

	esh.states = states;
	esh.row = &row;
	esh.view = GTK_TREE_VIEW (gpt);


	gtk_tree_model_foreach (model, set_expander_state, &esh);
}


gint
gtt_projects_tree_get_col_width (GttProjectsTree *gpt, int col)
{
	g_return_val_if_fail (col >= 0, 0);

	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (gpt), col);
	if (column == NULL)
	{
		return -1;
	}
	return gtk_tree_view_column_get_width (column);
}



void
gtt_projects_tree_set_col_width (GttProjectsTree *gpt, int col, int width)
{
	g_return_if_fail (col >= 0);

	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (gpt), col);
	g_return_if_fail (column != NULL);
	gtk_tree_view_column_set_fixed_width (column, width);
}

GtkTreeViewColumn *
gtt_projects_tree_get_column_by_name (GttProjectsTree *gpt, gchar *column_name)
{
	g_return_val_if_fail (gpt, NULL);
	GttProjectsTreePrivate *priv = GTT_PROJECTS_TREE_GET_PRIVATE (gpt);

	g_return_val_if_fail (priv->column_references, NULL);
	return g_tree_lookup (priv->column_references, column_name);
}

void
gtt_projects_tree_set_sorted_column (GttProjectsTree *gpt, GtkTreeViewColumn *column)
{
	GList *columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (gpt));

	GList *p = columns;

	for (; p != NULL; p = p->next)
	{
		if (p->data == column)
		{
			gtk_tree_view_column_set_sort_indicator (GTK_TREE_VIEW_COLUMN(p->data), TRUE);
			gtk_tree_view_column_set_sort_order (GTK_TREE_VIEW_COLUMN(p->data), GTK_SORT_ASCENDING);
		} 
		else
		{
			gtk_tree_view_column_set_sort_indicator (GTK_TREE_VIEW_COLUMN(p->data), FALSE);
		}
	}

	g_list_free (columns);
}

static void project_changed (GttProject *prj, gpointer user_data)
{
	GttProjectsTree *gpt = GTT_PROJECTS_TREE (user_data);
	gtt_projects_tree_update_project_data (gpt, prj);
}
