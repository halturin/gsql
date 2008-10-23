/***************************************************************************
 *            nav_tree__recycle.c
 *
 *  Sun Oct 28 17:25:17 2007
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
#include "nav_sql.h"


void
nav_tree_refresh_recycle (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model;
	GtkListStore *details = NULL;
	GSQLNavigation *nav = NULL;
	gchar			*sql = NULL;
	gchar			*realname = NULL;
	gchar			*orig = NULL;
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
	GSQLVariable *var_name;
	GSQLVariable *var_type;
	GSQLVariable *var_orig;
	gchar *name, *tmp;
	gchar key[256];
	gchar *stock;
	
	model = gtk_tree_view_get_model(tv);
	n = gtk_tree_model_iter_n_children(model, iter);
	
	for (; n>1; n--)
	{
		gtk_tree_model_iter_children(model, &child, iter);
		gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
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
	
	cursor = gsql_cursor_new (session, sql);
	state = gsql_cursor_open_with_bind (cursor,
										FALSE,
										GSQL_CURSOR_BIND_BY_NAME,
										G_TYPE_STRING, ":name",
										G_TYPE_STRING, "%",
										-1);
	
	if (state != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;		
	}
	
	var_name = g_list_nth_data(cursor->var_list, 0);
	var_orig = g_list_nth_data(cursor->var_list, 1);
	var_type = g_list_nth_data(cursor->var_list, 4);
	
	i = 0;
	
	while (gsql_cursor_fetch (cursor, 1) > 0)
	{
		if ((var_type->value_type == G_TYPE_INT64) && (var_type->raw_to_value))
		{
			var_type->raw_to_value (var_type);
			n = *(gint *) var_type->value;
			
		} else {	
			
			GSQL_DEBUG ("SQL is incorrect. Bug.");
			n = -1;
		}
		
		i++;
		
		switch (n)
		{
			case 1: // TABLE
				id = TABLE_ID;
				stock = GSQL_STOCK_TABLES;
				break;
		
			case 2: // VIEW
				id = VIEW_ID;
				stock = GSQL_STOCK_VIEWS;
				break;
		
			case 3: // INDEX
				id = INDEX_ID;
				stock = GSQL_STOCK_INDEXES;
				break;
		
			case 4: // PROCEDURE
				id = PROCEDURE_ID;
				stock = GSQL_STOCK_PROCEDURES;
				break;
			case 5: // FUNCTION
				id = FUNCTION_ID;
				stock = GSQL_STOCK_FUNCTIONS;
				break;

			case 6: // PACKAGE
				id = PACKAGE_ID;
				stock = GSQLE_ORACLE_STOCK_PACKAGE;
				break;

			case 7: // PACKAGE BODY
				id = PACKAGE_BODY_ID;
				stock = GSQLE_ORACLE_STOCK_PACKAGE_BODIES;
				break;

			case 8: // TRIGGER
				id = TRIGGER_ID;
				stock = GSQL_STOCK_TRIGGERS;
				break;

			case 9: // SEQUENCE
				id = SEQUENCE_ID;
				stock = GSQL_STOCK_SEQUENCES;
				break;

			case 10: // SYNONYM
				id = SYNONYM_ID;
				stock = GSQLE_ORACLE_STOCK_SYNONYMS;
				break;

			case 11: // CLUSTER
				id = CLUSTER_ID;
				stock = GSQLE_ORACLE_STOCK_CLUSTERS;
				break;

			case 12: // DATABASE LINK
				id = DATABASE_LINK_ID;
				stock = GSQLE_ORACLE_STOCK_DB_LINK;
				break;

			case 13:
			case 14:
				id = MVIEW_ID;
				stock = GSQLE_ORACLE_STOCK_MVIEWS;
				break;

			case 15:
				id = OBJECT_TYPE_ID;
				stock = GSQLE_ORACLE_STOCK_OBJ_TYPES;
				break;

			case 16:
				id = OBJECT_TYPE_BODY_ID;
				stock = GSQLE_ORACLE_STOCK_OBJ_TYPE_BODIES;
				break;

			case 17:
				id = TABLE_PARTITION_ID;
				GSQL_FIXME;
				stock = NULL; // FIXME. Paint me.
				break;

			case 18:
				id = INDEX_PARTITION_ID;
				GSQL_FIXME;
				stock = NULL; // FIXME. Paint me.
				break;

			case 19:
				id = LIBRARY_ID;
				stock = GSQLE_ORACLE_STOCK_LIBRARIES;
				break;

			case 20:
				id = DIRECTORY_ID;
				GSQL_FIXME;
				stock = NULL; // FIXME. Paint me.
				break;

			case 21:
				id = QUEUE_ID;
				stock = GSQLE_ORACLE_STOCK_QUEUES;
				break;

			case 22:
				id = JAVA_SOURCE_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_SOURCE;
				break;

			case 23:
				id = JAVA_CLASS_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_CLASSES;
				break;

			case 24:
				id = JAVA_RESOURCE_ID;
				stock = GSQLE_ORACLE_STOCK_JAVA_RESOURCES;
				break;

			case 25: // INDEXTYPE
				id = INDEX_TYPE_ID;
				stock = GSQLE_ORACLE_STOCK_INDEX_TYPES;
				break;

			case 26: // OPERATOR
				id = OPERATOR_ID;
				stock = GSQLE_ORACLE_STOCK_OPERATORS;
				break;
			
			case 27: // DIMENSION
				id = DIMENSION_ID;
				stock = GSQLE_ORACLE_STOCK_DIMENSIONS;
				break;

			case 28: // RESOURCE PLAN
				id = RESOURCE_PLAN_ID;
				stock = GSQLE_ORACLE_STOCK_RESOURCE_PLANS;
				break;

			case 29: // CONSUMER GROUP
				id = CONSUMER_GROUP_ID;
				stock = GSQLE_ORACLE_STOCK_CONSUMER_GROUPS;
				break;

			default:
				GSQL_DEBUG ("Unhandled type ID %d. use default.", n);
				id = RECYCLE_BIN_ID;
				stock = GSQLE_ORACLE_STOCK_RECYLE_BIN;
		}
		
		if (var_name->value_type != G_TYPE_STRING)
		{
			GSQL_DEBUG ("The name of object should be a string (char *). Is the bug");
			name = N_("Incorrect data");
		} else {
			name = (gchar *) var_name->value;
			orig = (gchar *) var_orig->value;
			
			// make a key for a hash of details
			memset (key, 0, 256);
			g_snprintf (key, 255, "%x%s%d%s",
				   session, name, id, name);
			
			details = gsql_navigation_get_details (navigation, key);
			oracle_navigation_fill_details (cursor, details);
		}
		
		tmp = g_strdup_printf("(%s) %s", orig, name);
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			id,
					GSQL_NAV_TREE_OWNER,		owner,
					GSQL_NAV_TREE_IMAGE,		stock,
					GSQL_NAV_TREE_NAME,			tmp,
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
