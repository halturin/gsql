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
#include "nav_objects.h"
#include "engine_stock.h"
#include "nav_sql.h"


void
nav_tree_refresh_processlist (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model;
	GtkListStore *details;
	GSQLNavigation *nav = NULL;
	gchar			*sql = NULL;
	gchar			*realname = NULL;
	gint 		id;
	gint		i,n;
	GtkTreeIter child;
	GtkTreeIter child_fake;
	GtkTreeIter	child_last;
	GSQLCursor *cursor;
	GSQLVariable *var, *var1, *var2, *var3, *var4;
	GSQLCursorState state;
	GSQLSession *session;
	gchar *name, *display_name, *owner;
	gchar key[256];

	
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
	state = gsql_cursor_open (cursor, FALSE);
	
	var = g_list_nth_data(cursor->var_list, 0); // process id. should by gint64
	var1 = g_list_nth_data(cursor->var_list, 1); // user
	var2 = g_list_nth_data(cursor->var_list, 2); // hostname
	var3 = g_list_nth_data(cursor->var_list, 3); // db
	var4 = g_list_nth_data(cursor->var_list, 4); // status
	
	if (state != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;		
	}
	
	i = 0;
	
	while (gsql_cursor_fetch (cursor, 1) > 0)
	{
		i++;
		
		if (var->value_type != G_TYPE_INT64)
		{
			GSQL_DEBUG ("The name of object should be a gint64. Is the bug");
			name = g_strdup (N_("Incorrect data"));
			display_name = g_strdup (N_("Incorrect data"));
		} else {
			name = g_strdup_printf ("%llu", *((gint64 *) var->value));
			display_name = g_strdup_printf ("%llu %s@%s[%s] %s", 
											*((gint64 *) var->value),
											(gchar *) var1->value,
											(gchar *) var3->value,
											(gchar *) var2->value,
											(gchar *) var4->value);
			// make a key for a hash of details
			memset (key, 0, 256);
			g_snprintf (key, 255, "%x%s%d%s",
				   session, name, PROCESS_LIST_ID, name);
			
			details = gsql_navigation_get_details (navigation, key);
			mysql_navigation_fill_details (cursor, details);
		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			PROCESS_LIST_ID,
					GSQL_NAV_TREE_OWNER,		name,
					GSQL_NAV_TREE_IMAGE,		GSQLE_MYSQL_STOCK_PROCESS_LIST,
					GSQL_NAV_TREE_NAME,			display_name,
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
		
		g_free (name);
		g_free (display_name);
		
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
