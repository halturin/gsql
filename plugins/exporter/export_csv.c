/***************************************************************************
 *            export_csv.c
 *
 *  Mon Mar  3 01:08:53 2008
 *  Copyright  2007  Taras Halturin
 *  <halturin@gmail.com>
 ****************************************************************************/

/*
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
#include <glade/glade.h>
#include <libgsql/common.h>
#include <libgsql/content.h>
#include <libgsql/sqleditor.h>
#include <libgsql/session.h>
#include <libgsql/type_datetime.h>
#include <libgsql/utils.h>
#include "plugin_exporter.h"


/*
	FIXME:
 
	DO REWORK ALL OF THIS!!!!!!!!!!!!!!
	DO REWORK ALL OF THIS!!!!!!!!!!!!!!
	DO REWORK ALL OF THIS!!!!!!!!!!!!!!
	DO REWORK ALL OF THIS!!!!!!!!!!!!!!
	DO REWORK ALL OF THIS!!!!!!!!!!!!!!
	... will do it in the next release.
	
	it is just temporary solution and veeeeeeery stupid.
	i feel sick from it :-!
 
 */



static gboolean
export_for_each (GtkTreeModel *model, GtkTreePath *path, 
				 GtkTreeIter *iter, gpointer data);

static void
export_bg (GSQLContent *content);


void
exporter_export_to_csv (GSQLContent *content, gchar *file, gchar *encoding, gboolean headers)
{
	GSQL_TRACE_FUNC;
	
	GList *list, *hlist;
	GSQLEditor *editor;
	GtkTreeView *rtv;
	GtkTreeModel *model;
	GtkWidget *hlabel;
	const gchar *htext;
	GError *err = NULL;
	GIOChannel *ioc;
	GIOStatus status;
	GSQLWorkspace *workspace;
	gsize  data_written;
	gchar *data;
	gchar buffer[1024];
	gchar msg[GSQL_MESSAGE_LEN];
	GThread *thread = NULL;
	
	list = gtk_container_get_children (GTK_CONTAINER (content));
	
	if ((!list) || (!GSQL_IS_EDITOR (list->data)))
		return;
	
	editor = list->data;
	
	rtv = gsql_editor_get_result_treeview (editor);
	model = gtk_tree_view_get_model (rtv);
	
	// get current workspace
	workspace = gsql_session_get_workspace (NULL);
	
	ioc = g_io_channel_new_file (file, "w+", &err);
	
	if (!ioc)
	{
		memset (msg, 0, GSQL_MESSAGE_LEN);
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Export failed. file :'%s'. %s"), 
					file, err->message);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
		
		return;
	}
	
	if (encoding)
		g_io_channel_set_encoding (ioc, encoding, &err);
	
	if (headers)
	{
		hlist = gtk_tree_view_get_columns (rtv);
		
		while (hlist)
		{	
			hlabel = gtk_tree_view_column_get_widget (hlist->data);
			htext = gtk_label_get_text (GTK_LABEL (hlabel));
			
			GSQL_DEBUG ("Header: [%s]", htext);
			memset (buffer, 0, 1024);
			
			hlist = g_list_next (hlist);
			
			if (!hlist)
			{
				g_snprintf (buffer, 1024, "%s\n", htext);
				status = g_io_channel_write_chars (ioc, buffer, -1, &data_written, &err);
			} else {
				
				g_snprintf (buffer, 1024, "%s;", htext);
				status = g_io_channel_write_chars (ioc, buffer, -1, &data_written, &err);
			}
			
			if (status != G_IO_STATUS_NORMAL)
			{
		
				g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Export failed. file :'%s'. %s"), 
							file, err->message);
				gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
				
				g_io_channel_unref (ioc);
				return;
			}
			
		}
	}
	
	g_object_set_data (G_OBJECT (content), "ioc", ioc);
	g_object_set_data (G_OBJECT (content), "model", model);
	
	
	thread = g_thread_create ((GThreadFunc) export_bg,
							  content, 
							  FALSE,
							  &err);
	if (!thread)
		g_warning ("Couldn't create thread");	
	
	
	
	
}

static gboolean
on_close (GtkWidget *dialog, gpointer userdata)
{
	gboolean *stop = userdata;
	
	*stop = TRUE;
	
	return FALSE;
}

static void
export_bg (GSQLContent *content)
{
	gpointer *progress;
	GtkTreeModel *model;
	GtkDialog *dialog;
	GIOChannel *ioc;
	gboolean stop = FALSE;
	gint childs;
	
	
	progress = g_object_get_data (G_OBJECT (content), "progress");
	model = g_object_get_data (G_OBJECT (content), "model");
	dialog = g_object_get_data (G_OBJECT (content), "dialog");
	ioc = g_object_get_data (G_OBJECT (content), "ioc");
	
	g_object_set_data (G_OBJECT (model), "ioc", ioc);
	g_object_set_data (G_OBJECT (model), "progress", progress);
	
	childs = gtk_tree_model_iter_n_children (model, NULL);
	
	if (!childs)
		return;
	
	g_signal_connect (G_OBJECT (dialog), "close", G_CALLBACK (on_close), &stop);
	
	g_object_set_data (G_OBJECT (progress), "childs", (gpointer) childs);
	g_object_set_data (G_OBJECT (progress), "current", (gpointer) 0);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 0.01);
	
	GSQL_DEBUG ("FOR EACH 1");
	gtk_tree_model_foreach (model, export_for_each, (gpointer) stop);
	GSQL_DEBUG ("FOR EACH 1");
	
	if (!stop)
		gtk_dialog_response (dialog, 3);
	
	g_io_channel_unref (ioc);
}

static gboolean
export_for_each (GtkTreeModel *model, GtkTreePath *path, 
				 GtkTreeIter *iter, gpointer data)
{
	GError *err = NULL;
	GIOChannel *ioc;
	gboolean stop = (gboolean) data;
	GtkProgressBar *progress;
	GIOStatus status;
	GType ctype;
	gint n;
	gint cn = gtk_tree_model_get_n_columns (model);;
	gchar buffer[81920];
	gchar *format;
	gchar *p;
	gchar *vchar;
	gint64 vint64;
	gint  vint;
	gdouble vdouble;
	GSQLTypeDateTime *vdate;
	gsize  data_written;
	gint childs, current;
	
//	if (*stop)
//		return TRUE;
	
	ioc = (GIOChannel *) g_object_get_data (G_OBJECT (model), "ioc");
	progress = GTK_PROGRESS_BAR (g_object_get_data (G_OBJECT (model), "progress"));
	childs = (gint) (g_object_get_data (G_OBJECT (progress), "childs"));
	current = (gint) (g_object_get_data (G_OBJECT (progress), "current"));
	
	for (n = 0; n < cn; n++)
	{
		GSQL_DEBUG ("Column %d", n);
		ctype = gtk_tree_model_get_column_type (model, n);
		memset (buffer, 0, 81920);
		
		switch (ctype)
		{
			case G_TYPE_STRING:
				GSQL_DEBUG ("G_TYPE_STRING");
			
				gtk_tree_model_get (model, iter,  
						n, 
						&vchar, -1);
				
				format = "%s;";
				if (n == cn-1)
					format = "%s\n";
				
				g_snprintf (buffer, 82920, format, vchar);
				
				break;
			
			case G_TYPE_INT64:
				GSQL_DEBUG ("G_TYPE_INT64");
				
				gtk_tree_model_get (model, iter,  
						n, 
						&vint64, -1);
				
				format = "%llu;";
				if (n == cn-1)
					format = "%llu\n";
				
				g_snprintf (buffer, 82920, format, vint64);
				
				break;
				
			case G_TYPE_INT:
				GSQL_DEBUG ("G_TYPE_INT");
				
				gtk_tree_model_get (model, iter,  
						n, 
						&vint, -1);
				
				format = "%d;";
				if (n == cn-1)
					format = "%d\n";
				
				g_snprintf (buffer, 82920, format, vint);
				
				break;
				
			case G_TYPE_DOUBLE:
				GSQL_DEBUG ("G_TYPE_DOUBLE");
				
				gtk_tree_model_get (model, iter,  
						n, 
						&vdouble, -1);
				
				format = "%f;";
				if (n == cn-1)
					format = "%f\n";
				
				g_snprintf (buffer, 82920, format, vdouble);
				
				break;
				
			default:
			
				// GSQL_TYPE_DATETIME - case label does not reduce to an integer constant
				if (ctype == GSQL_TYPE_DATETIME)
				{
					GSQL_DEBUG ("GSQL_TYPE_DATETIME");
				
					gtk_tree_model_get (model, iter,  
						n, 
						&vdate, -1);
					
					format = "%s;";
					if (n == cn-1)
						format = "%s\n";
					
					g_snprintf (buffer, 82920, format, gsql_type_datetime_to_gchar (vdate, NULL, 0));
				
					break;
				
				}
				
				if (ctype == GSQL_TYPE_UNSUPPORTED)
				{
					GSQL_DEBUG ("GSQL_TYPE_UNSUPPORTED");
					
					format = "%s;";
					if (n == cn-1)
						format = "%s\n";
					
					g_snprintf (buffer, 82920, format, N_("GSQL: Unsupported type"));
					
					break;
				}
				
		}
		
		
		
		status = g_io_channel_write_chars (ioc, buffer, -1, &data_written, &err);
		
		if (status != G_IO_STATUS_NORMAL)
		{
			GSQL_DEBUG (N_("Export failed: %s"), err->message);			
			return TRUE;
		}
		
		
		/*else {
			return FALSE;
		}*/
	}
	
	// has bin canceled?
	if (!G_IS_OBJECT (progress))
		return TRUE;
	
	current++;
	g_object_set_data (G_OBJECT (progress), "current", (gpointer) current);
	
	GSQL_DEBUG("set fraction");
			GSQL_THREAD_ENTER;
			gtk_progress_bar_set_fraction (progress, (gdouble)current/childs);
			GSQL_THREAD_LEAVE;
		
	GSQL_DEBUG ("STOOOOOOOOOOP = %d [%f] %d" , stop, (gdouble) current/childs, childs);
	
	return FALSE;
}
