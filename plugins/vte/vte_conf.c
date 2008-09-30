/***************************************************************************
 *            vte_conf.c
 *
 *  Sun Sep  9 21:51:00 2007
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

#include <config.h>
#include <glade/glade.h>
#include <libgsql/engines.h>
#include <libgsql/conf.h>
#include <libgsql/common.h>
#include <vte_conf.h>

#define GSQLP_VTE_GLADE_DIALOG PACKAGE_GLADE_DIR"/plugins/vte_dialog_conf.glade"

static void
conf_engines_list_create (gpointer key, 
								gpointer value,
								gpointer user_data);

static void
conf_renderer_edited_cb (GtkCellRendererText *renderer,
						 gchar *path,
						 gchar *new_text,
						 gpointer user_data);

void
plugin_vte_conf_dialog ()
{
	GSQL_TRACE_FUNC

	GladeXML* gxml;
	GtkDialog *dialog;
	GtkWidget *engines_tree_view;
	GtkListStore *engines_list;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	
	gxml = glade_xml_new (GSQLP_VTE_GLADE_DIALOG, "dialog_conf", NULL);
	g_return_if_fail(gxml);
	
	dialog = (GtkDialog *) glade_xml_get_widget (gxml, "dialog_conf");
	engines_tree_view = glade_xml_get_widget (gxml, "treeview_engines_setting");
	
	engines_list = gtk_list_store_new (4,
									   G_TYPE_STRING, // engine id
									   GDK_TYPE_PIXBUF, // engine icon
									   G_TYPE_STRING, // engine name
									   G_TYPE_STRING // command
									   );

	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes ("Icon",
						renderer,
						NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (engines_tree_view), column);
	gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", 1);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes ("Name",
						renderer,
						NULL);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (engines_tree_view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes ("Name",
						renderer,
						NULL);
	g_object_set (G_OBJECT(renderer), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT(renderer), "edited", 
					  G_CALLBACK(conf_renderer_edited_cb), engines_list);
	
	gtk_tree_view_column_add_attribute (column, renderer, "text", 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (engines_tree_view), column);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (engines_tree_view),
							 GTK_TREE_MODEL (engines_list));
	
	glade_xml_signal_autoconnect(gxml);
	
	gsql_engines_foreach (conf_engines_list_create,
							engines_list);
	
	gtk_dialog_run (dialog);
	gtk_widget_destroy ((GtkWidget *) dialog);
	g_object_unref(G_OBJECT(gxml));
	
	return;
};


static void
conf_engines_list_create (gpointer key, 
								gpointer value,
								gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkListStore *engines_list = user_data;
	GtkTreeIter iter;
	GSQLEngine *engine = value;
	gchar tmp[256];
	gchar *conf_value;
	
	g_return_if_fail (engine != NULL);

	gtk_list_store_append (GTK_LIST_STORE(engines_list), 
						   &iter);
	g_snprintf(tmp, 256 ,"%s/vte/%s", GSQL_CONF_PLUGINS_ROOT_KEY, engine->info.id);
	conf_value = gsql_conf_value_get_string (tmp);
	
	gtk_list_store_set(GTK_LIST_STORE(engines_list), &iter,
					   0, engine->info.id,
					   1, create_pixbuf (engine->file_logo),
					   2, engine->info.name,
					   3, conf_value,
					   -1);

	return;
};

static void
conf_renderer_edited_cb (GtkCellRendererText *renderer,
						 gchar *c_path,
						 gchar *new_text,
						 gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkTreePath *path;
	GtkListStore *lstore = user_data;
	GtkTreeIter iter;
	gchar *command;
	gchar tmp[256];
	gchar *engine_id;
	
	path = gtk_tree_path_new_from_string (c_path);
	gtk_tree_model_get_iter (GTK_TREE_MODEL(lstore), &iter, path);
	g_free (path);
	
	gtk_tree_model_get (GTK_TREE_MODEL (lstore), &iter,
						0, &engine_id, -1);
	
	if (new_text != NULL)
	{
		g_snprintf(tmp, 256 ,"%s/vte/%s", GSQL_CONF_PLUGINS_ROOT_KEY, engine_id);
		gsql_conf_value_set_string (tmp, new_text);
		
		gtk_list_store_set(GTK_LIST_STORE(lstore), &iter,
					   3, new_text,
					   -1);
	};
	
	
	return;
};

