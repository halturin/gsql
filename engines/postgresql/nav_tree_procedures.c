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
#include <string.h>
#include <libgsql/common.h>
#include <libgsql/stock.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/cvariable.h>
#include <libgsql/editor.h>
#include "nav_objects.h"
#include "engine_stock.h"
#include "nav_sql.h"

void
nav_tree_refresh_procedures (GSQLNavigation *navigation,
			     GtkTreeView *tv,
			     GtkTreeIter *iter)
{
  GSQL_TRACE_FUNC;

  GtkTreeModel *model;
  GtkListStore *detail;
  GSQLNavigation *nav = NULL;
  gchar			*sql = NULL;
  gchar			*realname = NULL;
  gchar			*owner = NULL;
  gint 		id;
  gint		i,n;
  GtkTreeIter child;
  GtkTreeIter child_fake;
  GtkTreeIter	child_last;
  GSQLCursor *cursor;
  GSQLCursorState state;
  GSQLVariable *var;
  GSQLSession *session;
  GtkListStore *details;
  gchar *name;
  gchar key[256];
  gchar *parent_type = NULL;
  gint  child_type;
  gchar *stock;


  model = gtk_tree_view_get_model(tv);
  n = gtk_tree_model_iter_n_children(model, iter);
	
  for (; n>1; n--)
    {
      gtk_tree_model_iter_children (model, &child, iter);
      gtk_tree_store_remove (GTK_TREE_STORE(model), &child);
    }
	
  gtk_tree_model_iter_children(model, &child_last, iter);
	
  gtk_tree_model_get (model, iter,  
		      GSQL_NAV_TREE_REALNAME, 
		      &realname, -1);
	
  gtk_tree_model_get (model, iter,  
		      GSQL_NAV_TREE_SQL, 
		      &sql, -1);
	
  gtk_tree_model_get (model, iter,  
		      GSQL_NAV_TREE_OWNER, 
		      &owner, -1);

  gtk_tree_model_get (model, iter,  
		      GSQL_NAV_TREE_ID, 
		      &id, -1);
	
  switch (id)
    {
    case FUNCTIONS_ID:
      parent_type = "FUNCTION";
      child_type = FUNCTION_ID;
      stock = GSQL_STOCK_FUNCTIONS;
      break;
		
    case PROCEDURES_ID:
      parent_type = "PROCEDURE";
      child_type = PROCEDURE_ID;
      stock = GSQL_STOCK_PROCEDURES;
      break;
			
    default:
      GSQL_DEBUG ("PROCEDURES: unhandled type");
      return;
    }
	
  session = gsql_session_get_active ();
	
  cursor = gsql_cursor_new (session, sql);
  state = gsql_cursor_open_with_bind (cursor,
				      FALSE,
				      GSQL_CURSOR_BIND_BY_POS,
				      G_TYPE_STRING, parent_type,
				      G_TYPE_STRING, owner,
				      -1);
	
  var = g_list_nth_data(cursor->var_list,0);
	
  if (state != GSQL_CURSOR_STATE_OPEN)
    {
      gsql_cursor_close (cursor);
      return;		
    }
	
  i = 0;
	
  while (gsql_cursor_fetch (cursor, 1) > 0)
    {
      i++;
		
      if (var->value_type != G_TYPE_STRING)
	{
	  GSQL_DEBUG ("The name of object should be a string (char *). Is the bug");
	  name = N_("Incorrect data");
	} else {
	name = (gchar *) var->value;
	// make a key for a hash of details
	memset (key, 0, 256);
	g_snprintf (key, 255, "%x%s%d%s",
		    session, owner, child_type, name);
			
	details = gsql_navigation_get_details (navigation, key);
	pgsql_navigation_fill_details (cursor, details);
      }
		
      gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
      gtk_tree_store_set (GTK_TREE_STORE(model), &child,
			  GSQL_NAV_TREE_ID,			child_type,
			  GSQL_NAV_TREE_OWNER,		owner,
			  GSQL_NAV_TREE_IMAGE,		stock,
			  GSQL_NAV_TREE_NAME,			name,
			  GSQL_NAV_TREE_REALNAME, 	name,
			  GSQL_NAV_TREE_ITEM_INFO, 	NULL,
			  GSQL_NAV_TREE_SQL,			NULL,
			  GSQL_NAV_TREE_OBJECT_POPUP, NULL,
			  GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
			  GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
			  GSQL_NAV_TREE_EVENT_HANDLER, NULL,
			  GSQL_NAV_TREE_STRUCT, NULL,
			  GSQL_NAV_TREE_DETAILS, details,
			  GSQL_NAV_TREE_NUM_ITEMS, 0,
			  -1);

    }
	
  GSQL_DEBUG ("Items fetched: [%d]", i);
	
  if (i > 0)
    {
      name = g_strdup_printf("%s<span weight='bold'> [%d]</span>", 
			     realname, i);
      gtk_tree_store_set (GTK_TREE_STORE(model), iter,
			  GSQL_NAV_TREE_NAME, 
			  name,
			  -1);
      g_free (name);
    }
	
  gtk_tree_store_remove(GTK_TREE_STORE(model), &child_last);
	
  gsql_cursor_close (cursor);
}

