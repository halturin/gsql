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

 
#include <libgsql/conf.h>
#include <libgsql/common.h>
#include "engine_conf.h"
#include <gconf/gconf.h>

static void
save_custom_env (GtkListStore *store)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeIter iter;
	gchar *tmp = NULL;
	gchar *tmp1 = NULL;
	gchar *env_type;
	gchar *env_value;
	
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
		return;
	
	do
	{	
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &env_type, -1);
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 1, &env_value, -1);
		
		if (tmp)
		{
			tmp1 = g_strdup_printf ("%s,%s,%s", tmp, env_type, env_value);
			g_free (tmp);
			tmp = tmp1;
		} else {
			tmp1 = g_strdup_printf ("%s,%s", env_type, env_value);
			tmp = tmp1;
		}
		
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));
	
	if (tmp)
	{
		gsql_conf_value_set_string (GSQLE_CONF_ORACLE_ENV, tmp);
		g_free (tmp);
	}
}

void
env_type_renderer_edited_cb (GtkCellRendererText *renderer,
							  gchar		*c_path,
							  gchar		*new_text,
							  gpointer  user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreePath *path = NULL;
	GtkListStore *lstore = user_data;
	GtkTreeIter iter;
	gchar *env_type;

	path = gtk_tree_path_new_from_string (c_path);
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL(lstore), &iter, path);
	gtk_tree_path_free (path);
	GSQL_DEBUG ("new:[%s]", new_text);
	if (!g_utf8_strlen(new_text, 128))
	{
		gtk_list_store_remove (lstore, &iter);

	} else {
		
		gtk_list_store_set(GTK_LIST_STORE(lstore), &iter,
					   0, new_text,
					   -1);
	}
	
	save_custom_env (lstore);
	
}

void
env_type_renderer_canceled_cb (GtkCellRendererText *renderer,							  
							  gpointer  user_data)
{
	GSQL_TRACE_FUNC;
	GtkTreeView *tv = user_data;
	GtkTreeModel *model;
	GtkTreeSelection *sel;
	GtkTreeIter iter;
	gchar *env_type = NULL;

	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);
	
	gtk_tree_selection_get_selected (sel, &model, &iter);
	
	gtk_tree_model_get (model, &iter,
						0, &env_type,
						-1);
	if (!env_type)
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

}

void
env_value_renderer_edited_cb (GtkCellRendererText *renderer,
							  gchar		*c_path,
							  gchar		*new_text,
							  gpointer  user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreePath *path = NULL;
	GtkListStore *lstore = user_data;
	GtkTreeIter iter;
	gchar *env_type;

	path = gtk_tree_path_new_from_string (c_path);
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL(lstore), &iter, path);
	gtk_tree_path_free (path);
	
	gtk_list_store_set(GTK_LIST_STORE(lstore), &iter,
					   1, new_text,
					   -1);
	
	save_custom_env (lstore);
	
}

void
on_conf_use_system_env_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkDialog * dialog = user_data;        
	gboolean status;
	GtkWidget *oracle_env_hbox;
        
	oracle_env_hbox = g_object_get_data (G_OBJECT (togglebutton), "oracle_env_hbox");        
	status = gtk_toggle_button_get_active (togglebutton);
    
	gtk_widget_set_sensitive ( oracle_env_hbox, !status);        
        
	gsql_conf_value_set_boolean (GSQLE_CONF_ORACLE_USE_SYS_ENV,
							status);    
    
}

void
on_conf_oracle_env_add_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GtkTreeIter iter;
	GtkTreeView *tv = user_data;
	GtkTreeModel *model;
	GtkTreePath  *path;
	GtkTreeViewColumn *col;
	
	model = gtk_tree_view_get_model (tv);
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	path = gtk_tree_model_get_path (model, &iter);
	
	col = gtk_tree_view_get_column (tv, 0);
	gtk_tree_view_set_cursor (tv, path, col, TRUE);

}


void
on_conf_oracle_env_del_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeView *tv = user_data;
	GtkTreeModel *model;
	GtkTreeSelection *sel = NULL;
	GtkTreeIter iter;
	
	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);
	
	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;
	
	gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
	
	save_custom_env (GTK_LIST_STORE (model));
}


void
on_conf_oracle_enable_trace_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQLE_CONF_ORACLE_ENABLE_TRACE,
							status);

}

