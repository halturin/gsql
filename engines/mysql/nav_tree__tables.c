/***************************************************************************
 *            nav_tree__tables.c
 *
 *  Sun Apr  6 18:05:32 2008
 *  Copyright  2008  fantom
 *  <fantom@<host>>
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
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <string.h>

#include <libgsql/session.h>
#include <libgsql/stock.h>
#include <libgsql/sqleditor.h>

#include "mysql_cursor.h"
#include "mysql_var.h"
#include "mysql.h"
#include "nav_objects.h"
#include "nav_sql.h"

#include "nav_tree__constraints.h"
#include "nav_tree__triggers.h"
#include "nav_tree__indexes.h"
#include "nav_tree__columns.h"


static void nav_tree_tables_event (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);
static void nav_tree_tables_popup (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);
static void nav_tree_tables_editor (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);
static void nav_tree_tables_browse (gchar *name, gchar *owner);

static void on_popup_table_create (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_table_drop (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_table_alter (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_table_browse (GtkMenuItem * menuitem, 
								 gpointer user_data);


static GSQLNavigationItem tables[] = {
	{	COLUMNS_ID,
		GSQL_STOCK_COLUMNS,
		N_("Columns"), 
		NULL,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_columns,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	INDEXES_ID,
		GSQL_STOCK_INDEXES,
		N_("Indexes"), 
		sql_mysql_indexes,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_indexes,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS,
		N_("Triggers"), 
		sql_mysql_triggers,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_triggers,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	CONSTRAINTS_ID,
		GSQL_STOCK_CONSTRAINT,
		N_("Constraints"), 
		sql_mysql_constraints,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_constraints,						// expand_handler
		NULL,						// event_handler
		NULL, 0}					// child, childs
};

static gchar table_ui[] = 
" <ui> "
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\"> "
"  		<placeholder name=\"PHolderNavObjectDo\"> "
//"  				<menuitem name=\"MySQLTableCreate\" action=\"MySQLActionTableCreate\" />	"
//"  				<menuitem name=\"MySQLTableDrop\" action=\"MySQLActionTableDrop\" />		"
//"  				<menuitem name=\"MySQLTableAlter\" action=\"MySQLActionTableAlter\" />		"
"  				<menuitem name=\"MySQLTableBrowse\" action=\"MySQLActionTableBrowse\" />	"
"	    </placeholder> "
"  </popup> "
"</ui>";

static GtkActionEntry table_acts[] = 
{
/*	{ "MySQLActionTableCreate", GTK_STOCK_NEW, 
		N_("Create..."), NULL, 
		N_("Create table"), 
		G_CALLBACK(on_popup_table_create) },
	
	{ "MySQLActionTableDrop", GTK_STOCK_DELETE, 
		N_("Drop..."), NULL, 
		N_("Drop table"), 
		G_CALLBACK(on_popup_table_drop) },
	
	{ "MySQLActionTableAlter", NULL, 
		N_("Alter..."), NULL, 
		N_("Alter table"), 
		G_CALLBACK(on_popup_table_alter) },
*/	
	{ "MySQLActionTableBrowse", NULL, 
		N_("Browse data"), NULL, 
		N_("Browse data"), 
		G_CALLBACK(on_popup_table_browse) }
};


void
nav_tree_tables_refresh (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	GtkTreeModel *model;
	GtkTreeIter child;
	GtkTreeIter child_fake;
	GtkTreeIter	child_last;
	gint n;
	gchar	   key[256];
	gchar		*sql = NULL;
	gchar		*realname = NULL;
	gchar		*name = NULL;
	gchar		*owner = NULL;
	GSQLCursor * cursor;
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GSQLVariable *var;
	GtkListStore *details;

	

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
	g_return_if_fail (sql != NULL);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);
	GSQL_DEBUG ("realname:[%s]    sql:[%s]   owner:[%s]", realname, sql, owner); 
	//g_return_if_fail (owner != NULL);
	
	session = gsql_session_get_active ();
	
	cursor = gsql_cursor_new (session, sql);
	
	if (gsql_cursor_open_with_bind(cursor, 
								   FALSE, 
								   GSQL_CURSOR_BIND_BY_POS, 
								   G_TYPE_STRING, owner, 
								   -1) != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;
	}

	var = g_list_nth_data(cursor->var_list,0);
	
	GSQL_DEBUG ("cursor state [%d]. Start fetching", gsql_cursor_get_state (cursor));
	
	if (var == NULL)
	{
		GSQL_DEBUG ("var is NULL");
		return;
	}
	 
	GSQL_DEBUG ("var->data = [%s]", (gchar *) var->value);
	n = 0;
	
	while (gsql_cursor_fetch (cursor, 1) > 0)
	{
		n++;
		if (var->value_type != G_TYPE_STRING)
		{
			GSQL_DEBUG ("The name of object should be a string (char *). Is the bug");
			name = N_("Incorrect data");
		} else {
			name = (gchar *) var->value;
			// make a key for a hash of details
			g_snprintf (key, 256, "%s%d%s%x",
				   name, TABLE_ID, name, session);
			details = gsql_navigation_get_details (navigation, key);
			mysql_navigation_fill_details (cursor, details);
		}
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			TABLE_ID,
					GSQL_NAV_TREE_OWNER,		owner,
					GSQL_NAV_TREE_IMAGE,		GSQL_STOCK_TABLES,
					GSQL_NAV_TREE_NAME,			name,
					GSQL_NAV_TREE_REALNAME, 	name,
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_POPUP, nav_tree_tables_popup,
					GSQL_NAV_TREE_OBJECT_HANDLER, NULL, //FIXME: nav_tree_tables_editor,
					GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
					GSQL_NAV_TREE_EVENT_HANDLER, nav_tree_tables_event,
					GSQL_NAV_TREE_STRUCT, tables,
					GSQL_NAV_TREE_DETAILS, details,
					GSQL_NAV_TREE_NUM_ITEMS, G_N_ELEMENTS(tables),
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
	
	GSQL_DEBUG ("Items fetched: [%d]", n);
	
	if (n > 0)
	{
		name = g_strdup_printf("%s<span weight='bold'> [%d]</span>", 
												realname, n);
		gtk_tree_store_set (GTK_TREE_STORE(model), iter,
							GSQL_NAV_TREE_NAME, 
							name,
							-1);
		g_free (name);
	};
	
	gtk_tree_store_remove(GTK_TREE_STORE(model), &child_last);
	
	gsql_cursor_close (cursor);

}

/*
 *  Static section:
 *
 *  nav_tree_tables_event
 *  nav_tree_tables_popup
 *  nav_tree_tables_editor
 *
 *  on_popup_table_create
 *  on_popup_table_drop
 *  on_popup_table_alter
 *  on_popup_table_browse
 *
 */


static void
nav_tree_tables_event (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	
	switch (event)
	{
		
		case GDK_Insert:
			GSQL_DEBUG ("Insert pressed");
			break;
		case GDK_F3:
			GSQL_DEBUG ("F3 pressed");
			on_popup_table_browse (NULL, NULL);
			break;
		case GDK_Delete:
			GSQL_DEBUG ("Delete pressed");
			break;
	};
	
	return;
};

static void
nav_tree_tables_popup (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	static GtkActionGroup *actions = NULL;
	
	if (!actions)
	{
		actions = gtk_action_group_new ("MySQLPopupTableActions");
		gtk_action_group_add_actions (actions, table_acts, 
								  G_N_ELEMENTS (table_acts), NULL);
		gsql_navigation_menu_merge (navigation, table_ui, actions);
	}
	gsql_navigation_menu_popup (navigation, actions);
	
	return;
};

static void
nav_tree_tables_editor (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	
	
	
	return;	
};

static void
on_popup_table_create (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	return;
};

static void
on_popup_table_drop (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	return;
};

static void
on_popup_table_alter (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	return;
};

static void
on_popup_table_browse (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session = NULL;
	GSQLContent *content = NULL;
	GSQLEditor *editor;
	GtkWidget *source;
	GSQLWorkspace *workspace;
	GSQLNavigation *navigation;
	GtkTreeIter *iter = NULL;
	gchar *name, *realname, *owner;
	GtkTreeModel *model;
	
	session = gsql_session_get_active ();
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);
	navigation = gsql_workspace_get_navigation (workspace);
	iter = gsql_navigation_get_active_iter (navigation);
	
	if (!iter)
	{
		GSQL_DEBUG ("Have no selection");
		return;
	};
	
	model = gsql_navigation_get_model (navigation);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_NAME, 
						&name, -1);
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);
	
	nav_tree_tables_browse (name, owner);
	
	
	
	
}


static void
nav_tree_tables_browse (gchar *name, gchar *owner)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	GSQLCursor  *cursor;
	GSQLContent *content;
	GSQLEditor *editor;
	GSQLWorkspace *workspace;
	GtkWidget *source;
	
	gint tmp_int = 0;
	gchar * sql = "select ";
	gchar * tmp = NULL;
	GSQLVariable *var;
	
	session = gsql_session_get_active ();
	
	cursor = gsql_cursor_new (session, 
							  (gchar *) sql_mysql_table_columns);
	
	if (gsql_cursor_open_with_bind(cursor, 
								   FALSE, 
								   GSQL_CURSOR_BIND_BY_POS, 
								   G_TYPE_STRING, owner,
								   G_TYPE_STRING, name,
								   -1) == GSQL_CURSOR_STATE_ERROR)
	{
		g_object_unref (cursor);
		return;
	}

	var = g_list_nth_data(cursor->var_list,0);
	

	if (var == NULL)
	{
		GSQL_DEBUG ("var is NULL");
		return;
	};
	 
	GSQL_DEBUG ("var->data = [%s]", (gchar *) var->value);
	
	while (gsql_cursor_fetch (cursor, 1) > 0)
	{
		
		if (var->value_type != G_TYPE_STRING)
		{
			GSQL_DEBUG ("The name of object should be a string (char *). Is the bug");
			name = N_("Incorrect data");
		}
		
		tmp = g_utf8_strdown (var->value, var->value_length);
		
		if (tmp_int == 0)
		{

			sql = g_strconcat (sql, "a.", tmp, NULL);
			GSQL_DEBUG ("making SQL: [clumns = %s] [sql = %s]", tmp, sql);

		}
		else
		{                                                        
			sql = g_strconcat (sql, ((tmp_int % 4) == 0) ? ", \n\t ": ", ",
									"a.", tmp, NULL);
			GSQL_DEBUG ("making SQL: [clumns = %s] [sql = %s]", tmp, sql);

		}
		
		g_free (tmp);
		tmp_int++;
		
	}
	
	sql = g_strconcat (sql,"\nfrom ", g_utf8_strdown (owner, strlen (owner)),
					   		".", g_utf8_strdown (name, strlen (name)),
										 " a\n", NULL);
	GSQL_DEBUG ("SQL formed: %s", sql);
	
	content = gsql_content_new (session, GTK_STOCK_FILE);
	
	source = (GtkWidget *) gsql_source_editor_new (sql);
	editor = gsql_editor_new (session, source);
	gsql_content_set_child (content, GTK_WIDGET (editor));
	
	workspace = gsql_session_get_workspace (session);
	gsql_workspace_add_content (workspace, content);
	gsql_content_set_name_full (content, name, name);
	
	gsql_cursor_close (cursor);
	g_free (sql);
	
	gsql_editor_run_sql (editor);
	
}
