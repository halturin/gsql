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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <libgsql/common.h>
#include <libgsql/sqleditor.h>
#include <libgsql/stock.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/cvariable.h>

#include "nav_objects.h"
#include "engine_stock.h"
#include "nav_tree_tables.h"
#include "nav_tree_columns.h"
#include "nav_tree_triggers.h"
#include "nav_sql.h"


static GSQLNavigationItem types[] = {
	{	COLUMNS_ID,
		GSQL_STOCK_COLUMNS,
		N_("Columns"), 
		NULL, 					// sql
		NULL, 					// object_popup
		NULL,					// object_handler
		(GSQLNavigationHandler)
		nav_tree_refresh_columns,		// expand_handler
		NULL,					// event_handler
		NULL, 0},				// child, childs
};

void
nav_tree_refresh_types (GSQLNavigation *navigation,
			GtkTreeView *tv,
			GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model = NULL;
	GtkTreeIter child, child_fake, child_last;
	GtkListStore *details;
	GSQLSession *session = NULL;
	GSQLCursor *cursor = NULL;
	GSQLVariable *var = NULL;
	GSQLCursorState state;
	gchar *realname = NULL, *sql = NULL, *owner = NULL, *currentdb = NULL,
		*name = NULL, key[256];
	gint n, i, id;

	model = gtk_tree_view_get_model(tv);

	for (n = gtk_tree_model_iter_n_children(model, iter); n>1; n--) {
		gtk_tree_model_iter_children(model, &child, iter);
		gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
	}

	gtk_tree_model_get (model, iter, GSQL_NAV_TREE_REALNAME, &realname,
			    -1);
	
	gtk_tree_model_get (model, iter, GSQL_NAV_TREE_SQL, &sql, -1);
	
	gtk_tree_model_get (model, iter, GSQL_NAV_TREE_OWNER, &owner, -1);

	session = gsql_session_get_active ();
	g_return_if_fail (GSQL_IS_SESSION(session));

	currentdb = pgsql_navigation_get_database (navigation, tv, iter);
	GSQL_DEBUG("Database: switching to [%s]", currentdb);
	pgsql_session_switch_database(session, currentdb);
	
	cursor = gsql_cursor_new (session, sql);
	
	state = gsql_cursor_open_with_bind (cursor, FALSE,
					    GSQL_CURSOR_BIND_BY_POS,
					    G_TYPE_STRING, owner,
					    -1);

	if (state != GSQL_CURSOR_STATE_OPEN) {
		gsql_cursor_close (cursor);
		return;		
	}
	
	var = g_list_nth_data(cursor->var_list,0);
	i = 0;

	while (gsql_cursor_fetch (cursor, 1) > 0) {
		i++;			

		if (var->value_type != G_TYPE_STRING) {
			GSQL_DEBUG ("The name of object should be a string "\
				    "(char *). Is the bug");
			name = N_("Incorrect data");
		} else {
			name = (gchar *) var->value;
			// make a key for a hash of details
			memset (key, 0, 256);
			g_snprintf (key, 255, "%x%s%d%s",
				    session, owner, TYPE_ID, name);
			
			details = gsql_navigation_get_details (navigation,
							       key);
			pgsql_navigation_fill_details (cursor, details);
		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), 	&child,
				    GSQL_NAV_TREE_ID,		TYPE_ID,
				    GSQL_NAV_TREE_OWNER,	owner,
				    GSQL_NAV_TREE_IMAGE,GSQL_STOCK_VIEWS,
				    GSQL_NAV_TREE_NAME,		name,
				    GSQL_NAV_TREE_REALNAME, 	name,
				    GSQL_NAV_TREE_ITEM_INFO, 	NULL,
				    GSQL_NAV_TREE_SQL,		NULL,
				    GSQL_NAV_TREE_OBJECT_POPUP,	NULL,
				    GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
				    GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
				    GSQL_NAV_TREE_EVENT_HANDLER, NULL,
				    GSQL_NAV_TREE_STRUCT, types,
				    GSQL_NAV_TREE_DETAILS, details,
				    GSQL_NAV_TREE_NUM_ITEMS, 
				    G_N_ELEMENTS(types),
				    -1);
		
		gtk_tree_store_append (GTK_TREE_STORE (model), &child_fake, 
				       &child);
		gtk_tree_store_set (GTK_TREE_STORE (model), &child_fake,
				    GSQL_NAV_TREE_ID,		-1,
				    GSQL_NAV_TREE_IMAGE,	NULL,
				    GSQL_NAV_TREE_NAME,	N_("Processing..."),
				    GSQL_NAV_TREE_REALNAME,	NULL,
				    GSQL_NAV_TREE_ITEM_INFO,	NULL,
				    GSQL_NAV_TREE_SQL,		NULL,
				    GSQL_NAV_TREE_OBJECT_POPUP,	NULL,
				    GSQL_NAV_TREE_OBJECT_HANDLER,NULL,
				    GSQL_NAV_TREE_EXPAND_HANDLER,NULL,
				    GSQL_NAV_TREE_EVENT_HANDLER,NULL,
				    GSQL_NAV_TREE_STRUCT,	NULL,
				    GSQL_NAV_TREE_NUM_ITEMS, 	NULL,
				    -1);		
	}

	GSQL_DEBUG ("Items fetched: [%d]", i);
	
	if (i > 0) {
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
