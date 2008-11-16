/***************************************************************************
 *            nav_tree__schemas.c
 *
 *  Tue Sep 18 22:20:44 2007
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

#include <libgsql/common.h>
#include <libgsql/stock.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/cursor.h>
#include <libgsql/cvariable.h>

#include "engine_stock.h"
#include "nav_objects.h"
#include "nav_sql.h"

#include "nav_tree__tables.h"
#include "nav_tree__users.h"
#include "nav_tree__constraints.h"
#include "nav_tree__triggers.h"
#include "nav_tree__indexes.h"
#include "nav_tree__views.h"
#include "nav_tree__procedures.h"
#include "nav_tree__privileges.h"
#include "nav_tree__schemas.h"
#include "nav_tree__variables.h"
#include "nav_tree__processlist.h"

/* 	
	The users_objects struct used as child for "My Scheme" 
	and as child of each users schemas 
*/
static GSQLNavigationItem users_objects[] = {
	{	TABLES_ID,					// id
		GSQL_STOCK_TABLES,			// stock
		N_("Tables"),				// name
		sql_mysql_tables,			// sql
		NULL,						// object_popup
		NULL,						// object_handler
		nav_tree_tables_refresh,	// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	CONSTRAINTS_ID,
		GSQL_STOCK_CONSTRAINT, 
		N_("Constraints"), 
		sql_mysql_constraints, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_constraints,					// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	VIEWS_ID, 
		GSQL_STOCK_VIEWS, 
		N_("Views"), 
		sql_mysql_views, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_views,							// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	INDEXES_ID,
		GSQL_STOCK_INDEXES, 
		N_("Indexes"), 
		sql_mysql_indexes, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_indexes,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS, 
		N_("Triggers"), 
		sql_mysql_triggers, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_triggers,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	PROCEDURES_ID, 
		GSQL_STOCK_PROCEDURES, 
		N_("Procedures"), 
		sql_mysql_routines, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_procedures,					// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	FUNCTIONS_ID, 
		GSQL_STOCK_FUNCTIONS, 
		N_("Functions"), 
		sql_mysql_routines, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_procedures,					// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{
		PRIVILEGES_ID,
		GSQL_STOCK_PRIVILEGES,
		N_("Privileges"),
		sql_mysql_privileges,
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_privileges,					// expand_handler
		NULL,						// event_handler
		NULL, 0 }					// child, childs
};


static GSQLNavigationItem variables[] = {
	{	SESSION_VARIABLES_ID,					// id
		GSQLE_MYSQL_STOCK_SESSION_VARIABLES,			// stock
		N_("Session Variables"),				// name
		sql_mysql_session_variables,			// sql
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_variables ,					// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	GLOBAL_VARIABLES_ID,
		GSQLE_MYSQL_STOCK_SESSION_VARIABLES, 
		N_("Global Variables"), 
		sql_mysql_global_variables, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_variables ,					// expand_handler
		NULL,						// event_handler
		NULL, 0 }
};

/* 	
	The root_objects struct used as root tree
*/
static GSQLNavigationItem root_objects[] = {
// root. my schema
	{	MY_SCHEMA_ID,
		GSQL_STOCK_MY_SCHEMA,
		N_("My Schema"), 
		NULL,						// sql
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		users_objects, G_N_ELEMENTS (users_objects) },
// root. all schemas	
	{	ALL_SCHEMAS_ID,
		GSQL_STOCK_ALL_SCHEMAS,
		N_("All Schemas"), 
		sql_mysql_all_schemas, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_schemas,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
// root. all users
	{	USERS_ID,
		GSQL_STOCK_USERS,
		N_("Users"), 
		sql_mysql_users, 
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_users,							// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs

// process list
    {   PROCESS_LIST_ID,
		GSQLE_MYSQL_STOCK_PROCESS_LIST,
		N_("Process List"),
		sql_mysql_processes,
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_processlist,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
		
// variables
    {   VARIABLES_ID,
		GSQLE_MYSQL_STOCK_SESSION_VARIABLES,
		N_("Variables"),
		NULL,
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		variables, G_N_ELEMENTS (variables) }					// child, childs
};

void
nav_tree_refresh_schemas (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model;
	GtkListStore *detail;
	GSQLNavigation *nav = NULL;
	gchar			*sql = NULL;
	gchar			*realname = NULL;
	gint 		id;
	gint		i,n;
	GtkTreeIter child, parent;
	GtkTreeIter child_fake;
	GtkTreeIter	child_last;
	GSQLCursor *cursor;
	GSQLCursorState state;
	GSQLVariable *var;
	GSQLSession *session;
	gchar *name, *parent_realname;

	
	model = gtk_tree_view_get_model(tv);
	n = gtk_tree_model_iter_n_children(model, iter);
	
	for (; n>1; n--)
	{
		gtk_tree_model_iter_children (model, &child, iter);
		gtk_tree_store_remove (GTK_TREE_STORE(model), &child);
	}
	
	gtk_tree_model_iter_children(model, &child_last, iter);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_SQL, 
						&sql, -1);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	
	session = gsql_session_get_active ();

	gtk_tree_model_iter_parent (model, &parent, iter);	
	gtk_tree_model_get (model, &parent,  
						GSQL_NAV_TREE_REALNAME, 
						&parent_realname, -1);
	
	cursor = gsql_cursor_new (session, sql);
	state = gsql_cursor_open (cursor, FALSE);

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
		name = (gchar *) var->value;
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			ALL_SCHEMAS_ID,
					GSQL_NAV_TREE_OWNER,		name,
					GSQL_NAV_TREE_IMAGE,		GSQL_STOCK_ALL_SCHEMAS,
					GSQL_NAV_TREE_NAME,			name,
					GSQL_NAV_TREE_REALNAME, 	name,
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_POPUP, NULL,
					GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
					GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
					GSQL_NAV_TREE_EVENT_HANDLER, NULL,
					GSQL_NAV_TREE_STRUCT, users_objects,
					GSQL_NAV_TREE_DETAILS, NULL,
					GSQL_NAV_TREE_NUM_ITEMS, G_N_ELEMENTS(users_objects),
					-1);

		gtk_tree_store_append (GTK_TREE_STORE (model), &child_fake, &child);
		gtk_tree_store_set (GTK_TREE_STORE (model), &child_fake,
				GSQL_NAV_TREE_ID,				-1,
				GSQL_NAV_TREE_IMAGE,			NULL,
				GSQL_NAV_TREE_NAME,				N_("Processing..."),
				GSQL_NAV_TREE_REALNAME,			NULL,
				GSQL_NAV_TREE_ITEM_INFO,		NULL,
				GSQL_NAV_TREE_SQL,				NULL,
				GSQL_NAV_TREE_OBJECT_POPUP,		NULL,
				GSQL_NAV_TREE_OBJECT_HANDLER,	NULL,
				GSQL_NAV_TREE_EXPAND_HANDLER,	NULL,
				GSQL_NAV_TREE_EVENT_HANDLER,	NULL,
				GSQL_NAV_TREE_STRUCT,			NULL,
				GSQL_NAV_TREE_NUM_ITEMS, 		NULL,
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


void
nav_tree_set_root (GSQLNavigation *navigation,  gchar *username)
{
	GSQL_TRACE_FUNC;
	
	gsql_navigation_set_root (navigation, GSQLE_MYSQL_STOCK_MYSQL, username, 
							  root_objects, G_N_ELEMENTS (root_objects));
	
}
