/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2008  Taras Halturin  halturin@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

 
#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/common.h>
#include <libgsql/content.h>
#include <libgsql/workspace.h>
#include <libgsql/session.h>
#include <string.h>

void
exporter_export_to_xml (GSQLContent *content, gchar *file, gchar *encoding, gboolean headers)
{
	GSQL_TRACE_FUNC;
	
	GIOChannel *ioc = NULL;
	GIOStatus status;
	gchar msg[GSQL_MESSAGE_LEN];
	GError *err = NULL;
	gboolean finished = FALSE;
	GtkTreeView *tv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *buffer;
	gint ncolumns;
	GList *columns = NULL;
	GtkTreeViewColumn *column;
	GtkWidget *column_header;
	gchar *name;
	GSQLWorkspace *workspace;
	
	
	g_return_if_fail (content != NULL);
	name = gsql_content_get_name (content);
	workspace = gsql_session_get_workspace (content->session);
	
	
	GSQL_DEBUG ("Open [%s] for writing", file);
	ioc = g_io_channel_new_file (name, "w+", &err);
	
	GSQL_DEBUG ("11");
	
	if (ioc == NULL)
	{
		memset (msg, 0, GSQL_MESSAGE_LEN);
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Export failed:['%s'] %s"), 
					name, err->message);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
		return;
	};
	GSQL_DEBUG ("22");
	if (encoding)
		g_io_channel_set_encoding (ioc, encoding, &err);
	
/*	tv = (GtkTreeView *) g_object_get_data (G_OBJECT (content->widget),
												"sql_result_treeview");
	if (tv == NULL)
	{
		GSQL_DEBUG ("Content widget have no 'sql_result_treeview'");
		g_io_channel_unref (ioc);
	};
	
	gtk_widget_set_sensitive (content, FALSE);
	
	model = gtk_tree_view_get_model (tv);
//	ncolumns = gtk_tree_model_get_n_columns (tv);
	columns = gtk_tree_view_get_columns (tv);
	columns = g_list_first (columns);
	while (columns)
	{
		column = (GtkTreeViewColumn *) columns->data;
		column_header = gtk_tree_view_column_get_widget (column);
		GSQL_DEBUG ("Column: [%s]", gtk_label_get_text (GTK_LABEL (column_header)));
		columns = g_list_next (columns);
	};
	gtk_tree_model_get_iter_first (model, &iter);
	while (!finished)
	{
//		gtk_tree_model_get_value (model, &iter, column, value);
	//	gtk_tree_model_iter_next (model, &iter);
		finished = TRUE;
	};
*/	

	if (columns)
		g_list_free (columns);

	g_io_channel_flush (ioc, &err);
	g_io_channel_unref (ioc);
	// FIXME
	gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
					  "The XML routines is not implemented yet. Please select other type.");
	
	return;
};

