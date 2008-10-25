/***************************************************************************
 *            nav_tree__depend.c
 *
 *  Thu Oct 11 00:09:16 2007
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
#include <string.h>
#include <libgsql/common.h>
#include <libgsql/stock.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/cvariable.h>
#include "nav_objects.h"
#include "engine_stock.h"

void
nav_tree_refresh_depend (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model;
	GtkListStore *details = NULL;
	GSQLNavigation *nav = NULL;
	gchar			*sql = NULL;
	gchar			*realname = NULL;
	gchar			*owner = NULL;
	gint 		id;
	gint		i,n;
	GtkTreeIter child;
	GtkTreeIter child_fake;
	GtkTreeIter	child_last;
	GtkTreeIter parent;
	GtkTreeIter d_iter;
	GSQLCursor *cursor;
	GSQLSession *session;
	GSQLCursorState state;
	GSQLVariable *var_n;
	GSQLVariable *var_t;
	GSQLVariable *var_o;
	gchar *name;
	gchar key[256];
	gchar *tbl = "%";
	gchar *parent_realname = NULL;
	gchar *obj_id_attr;
	gchar *obj_id_value = "0";
	gchar *stock;
	GSQLNavigationHandler object_handler;
	void  *value;
	
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
	
	session = gsql_session_get_active ();
	
	// get parent iter. if this iter are TABLE_ID then
	// we looking for table's triggers only.
	gtk_tree_model_iter_parent (model, &parent, iter);
	
	gtk_tree_model_get (model, &parent,  
						GSQL_NAV_TREE_ID, 
						&id, -1);
	
	gtk_tree_model_get (model, &parent,  
						GSQL_NAV_TREE_REALNAME, 
						&parent_realname, -1);
	
	switch (id)
	{
		case PACKAGE_ID:
			tbl = "PACKAGE";
			break;
			
		case PACKAGE_BODY_ID:
			tbl = "PACKAGE BODY";
			break;
		
		case OBJECT_TYPES_ID:
			tbl = "TYPE";
			break;
		
		case OBJECT_TYPE_BODIES_ID:
			tbl = "TYPE BODY";
			break;
	}
	
	cursor = gsql_cursor_new (session, sql);
	state = gsql_cursor_open_with_bind (cursor,
										FALSE,
										GSQL_CURSOR_BIND_BY_NAME,
										G_TYPE_STRING, ":owner",
										G_TYPE_STRING, owner,
										G_TYPE_STRING, ":object_name",
										G_TYPE_STRING, parent_realname,
										G_TYPE_STRING, ":object_type",
										G_TYPE_STRING, tbl,
										-1);
	
	if (state != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;		
	}
	
	i = 0;
	
	var_o = g_list_nth_data(cursor->var_list, 0);
	var_n = g_list_nth_data(cursor->var_list, 1);
	var_t = g_list_nth_data(cursor->var_list, 2);
	
	while (gsql_cursor_fetch (cursor, 1) > 0)	
	{
		i++;
		
		n = -1;
		
		if (var_t->raw_to_value)
		{
			var_t->raw_to_value (var_t);
			n =  *(gint *)var_t->value;
		}
			
		
		GSQL_FIXME;
		/* associate object_handler with real handler when it will be done */
		
		switch (n)
		{
			case 1: // TABLE
				id = TABLE_ID;
				stock = GSQL_STOCK_TABLES;
				object_handler = NULL;
				break;
		
			case 2: // VIEW
				id = VIEW_ID;
				stock = GSQL_STOCK_VIEWS;
				object_handler = NULL;
				break;
		
			case 3: // INDEX
				id = INDEX_ID;
				stock = GSQL_STOCK_INDEXES;
				object_handler = NULL;
				break;
		
			case 4: // PROCEDURE
				id = PROCEDURE_ID;
				stock = GSQL_STOCK_PROCEDURES;
				object_handler = NULL;
				break;
			
			case 5: // FUNCTION
				id = FUNCTION_ID;
				stock = GSQL_STOCK_FUNCTIONS;
				object_handler = NULL;
				break;

			case 6: // PACKAGE
				id = PACKAGE_ID;
				stock = GSQLE_ORACLE_STOCK_PACKAGE;
				object_handler = NULL;
				break;

			case 7: // PACKAGE BODY
				id = PACKAGE_BODY_ID;
				stock = GSQLE_ORACLE_STOCK_PACKAGE_BODIES;
				object_handler = NULL;
				break;

			case 8: // TRIGGER
				id = TRIGGER_ID;
				stock = GSQL_STOCK_TRIGGERS;
				object_handler = NULL;
				break;

			case 9: // SEQUENCE
				id = SEQUENCE_ID;
				stock = GSQL_STOCK_SEQUENCES;
				object_handler = NULL;
				break;

			case 10: // SYNONYM
				id = SYNONYM_ID;
				stock = GSQLE_ORACLE_STOCK_SYNONYMS;
				object_handler = NULL;
				break;

			case 11: // CLUSTER
				id = CLUSTER_ID;
				stock = GSQLE_ORACLE_STOCK_CLUSTERS;
				object_handler = NULL;
				break;

			case 12: // DATABASE LINK
				id = DATABASE_LINK_ID;
				stock = GSQLE_ORACLE_STOCK_DB_LINK;
				object_handler = NULL;
				break;

			case 13:
			case 14:
				id = MVIEW_ID;
				stock = GSQLE_ORACLE_STOCK_MVIEWS;
				object_handler = NULL;
				break;

			case 15:
				id = OBJECT_TYPE_ID;
				stock = GSQLE_ORACLE_STOCK_OBJ_TYPES;
				object_handler = NULL;
				break;

			case 16:
				id = OBJECT_TYPE_BODY_ID;
				stock = GSQLE_ORACLE_STOCK_OBJ_TYPE_BODIES;
				object_handler = NULL;
				break;

			case 17:
				id = TABLE_PARTITION_ID;
				stock = NULL; // FIXME. Paint me.
				object_handler = NULL;
				break;

			case 18:
				id = INDEX_PARTITION_ID;
				stock = NULL; // FIXME. Paint me.
				object_handler = NULL;
				break;

			case 19:
				id = LIBRARY_ID;
				stock = GSQLE_ORACLE_STOCK_LIBRARIES;
				object_handler = NULL;
				break;

			case 20:
				id = DIRECTORY_ID;
				stock = NULL; // FIXME. Paint me.
				object_handler = NULL;
				break;

			case 21:
				id = QUEUE_ID;
				stock = GSQLE_ORACLE_STOCK_QUEUES;
				object_handler = NULL;
				break;

			case 22:
				id = JAVA_SOURCE_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_SOURCE;
				object_handler = NULL;
				break;

			case 23:
				id = JAVA_CLASS_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_CLASSES;
				object_handler = NULL;
				break;

			case 24:
				id = JAVA_RESOURCE_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_RESOURCES;
				object_handler = NULL;
				break;

			case 25: // INDEXTYPE
				id = INDEX_TYPE_ID;
				stock = GSQLE_ORACLE_STOCK_INDEX_TYPES;
				object_handler = NULL;
				break;

			case 26: // OPERATOR
				id = OPERATOR_ID;
				stock = GSQLE_ORACLE_STOCK_OPERATORS;
				object_handler = NULL;
				break;
			
			case 27: // DIMENSION
				id = DIMENSION_ID;
				stock = GSQLE_ORACLE_STOCK_DIMENSIONS;
				object_handler = NULL;
				break;

			case 28: // RESOURCE PLAN
				id = RESOURCE_PLAN_ID;
				stock = GSQLE_ORACLE_STOCK_RESOURCE_PLANS;
				object_handler = NULL;
				break;

			case 29: // CONSUMER GROUP
				id = CONSUMER_GROUP_ID;
				stock = GSQLE_ORACLE_STOCK_CONSUMER_GROUPS;
				object_handler = NULL;
				break;

			default:
				GSQL_FIXME;
				GSQL_DEBUG ("Unhandled type of depend object");
				
				id = UNKNOWN_OBJECT_ID;
				stock = NULL;
				object_handler = NULL;

		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			id,
					GSQL_NAV_TREE_OWNER,		(gchar *) (var_o->value),
					GSQL_NAV_TREE_IMAGE,		stock,
					GSQL_NAV_TREE_NAME,			(gchar *) (var_n->value),
					GSQL_NAV_TREE_REALNAME, 	(gchar *) (var_n->value),
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_POPUP, NULL,
					GSQL_NAV_TREE_OBJECT_HANDLER, object_handler,
					GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
					GSQL_NAV_TREE_EVENT_HANDLER, NULL,
					GSQL_NAV_TREE_STRUCT, NULL,
					GSQL_NAV_TREE_DETAILS, NULL,
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
