/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2009  Estêvão Samuel Procópio <tevaum@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */


#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/type_datetime.h>
#include <libgsql/cvariable.h>

#include "pgsql.h"
#include "nav_objects.h"
#include "pgsql_var.h"

const gchar *
pgsql_navigation_get_database (GSQLNavigation *nav, GtkTreeView *treeview,
			       GtkTreeIter *iter) 
{
	GSQL_TRACE_FUNC;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter dbiter;
	const gchar *dbname = NULL;
	gint id;

	g_return_if_fail (GSQL_IS_NAVIGATION(nav));
	model = gtk_tree_view_get_model(treeview);
	path = gtk_tree_model_get_path (model, iter);

	GSQL_DEBUG("Path Depth [%d]", gtk_tree_path_get_depth(path));
	
	for ( ; gtk_tree_path_get_depth(path) > 3 ; gtk_tree_path_up(path) );

	GSQL_DEBUG("Path Depth [%d]", gtk_tree_path_get_depth(path));

	gtk_tree_model_get_iter (model, &dbiter, path);
	gtk_tree_model_get (model, &dbiter,  
			    GSQL_NAV_TREE_ID, 
			    &id, -1);
	GSQL_DEBUG("ID: [%d] - SCHEMAS_ID: [%d]", id, SCHEMAS_ID);
	if ( id == SCHEMAS_ID || id == LANGUAGES_ID || id == TYPES_ID) {
		// In case of being in the Current Database node
		GSQLSession *session = gsql_session_get_active();
		g_return_if_fail (GSQL_IS_SESSION(session));
		return pgsql_session_get_database(session);
	}
	g_return_if_fail (id == DATABASE_ID);

	gtk_tree_model_get (model, &dbiter,  
			    GSQL_NAV_TREE_REALNAME, 
			    &dbname, -1);
	GSQL_DEBUG("DBName: [%s]", dbname);
	g_return_if_fail (dbname != NULL);

	return dbname;
}


void
pgsql_navigation_fill_details (GSQLCursor *cursor, GtkListStore *store)
{
  GSQL_TRACE_FUNC;
  
  gint n = 0;
  GSQLVariable *var = NULL;
  GSQLEPGSQLVariable *spec;
  gchar attr_name[1024];
  gchar attr_value[1024];
  GtkTreeIter iter;
  GList *lst;
	
  g_return_if_fail (GSQL_IS_CURSOR (cursor));
  g_return_if_fail (GTK_IS_LIST_STORE (store));
	
  gtk_list_store_clear (store);
  lst = g_list_first (cursor->var_list);
	
  while (lst) {
    var = lst->data;
    spec = var->spec;
		
    g_snprintf (attr_name, 1024, "%s", spec->field->name);
		
    if (!var->value) {
      GSQL_DEBUG ("Skip variable[%d]", n);
      n++; lst = g_list_next (lst);
      continue;
    }
		
    switch (var->value_type) {
      case G_TYPE_STRING:
	g_snprintf (attr_value, 1024, "%s", 
		    (gchar *) var->value );
	break;
				
      case G_TYPE_INT64:
	g_snprintf (attr_value, 1024, "%d", 
		    *((gint64*) var->value) );
	break;
				
      case G_TYPE_INT:
	g_snprintf (attr_value, 1024, "%d", 
		    *((gint*) var->value));
	break;
				
      case G_TYPE_DOUBLE:
	g_snprintf (attr_value, 1024, "%f", 
		    *((gdouble*) var->value));
	break;
				
      default:
	if (var->value_type == GSQL_TYPE_DATETIME) {
	  if (var->raw_to_value)
	    var->raw_to_value (var);
	  
	  gsql_type_datetime_to_gchar (var->value, attr_value, 1024);
					
	  break;
	}
				
	g_snprintf (attr_value, 1024, N_("GSQL: Unsupported type"));
				
    }
		
    n++; lst = g_list_next (lst);
		
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
		       GSQL_NAV_DETAILS_STOCK, NULL, //FIXME
		       GSQL_NAV_DETAILS_NAME, attr_name,
		       GSQL_NAV_DETAILS_VALUE, attr_value,
		       -1);
  }
}
