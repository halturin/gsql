/***************************************************************************
 *            nav_tree__tables.c
 *
 *  Sun Sep 23 02:12:14 2007
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
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <string.h>

#include <libgsql/session.h>
#include <libgsql/stock.h>
#include <libgsql/sqleditor.h>

#include "oracle_var.h"
#include "nav_objects.h"

#include "nav_sql.h"
#include "engine_stock.h"

#include "nav_tree__columns.h"
#include "nav_tree__constraints.h"
#include "nav_tree__indexes.h"

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
		sql_oracle_table_columns_owner,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_columns,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	INDEXES_ID,
		GSQL_STOCK_INDEXES,
		N_("Indexes"), 
		sql_oracle_indexes_owner,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_indexes,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS,
		N_("Triggers"), 
		NULL,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	CONSTRAINTS_ID,
		GSQL_STOCK_CONSTRAINT,
		N_("Constraints"), 
		sql_oracle_table_constraints_owner,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_constraints,					// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	MVIEWS_LOG_ID,
		GSQLE_ORACLE_STOCK_MVIEW_LOGS,
		N_("MView Logs"), 
		NULL,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	DEPENDSON_ID,
		GSQLE_ORACLE_STOCK_DEPENDS_ON,
		N_("Depends On"), 
		NULL,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	DEPENDENT_ID,
		GSQLE_ORACLE_STOCK_DEPENDENT,
		N_("Dependent Objects"), 
		NULL,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0}					// child, childs
};


static gchar table_ui[] = 
" <ui> "
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\"> "
"  		<placeholder name=\"PHolderNavObjectDo\"> "
"  				<menuitem name=\"OracleTableCreate\" action=\"OracleActionTableCreate\" />	"
"  				<menuitem name=\"OracleTableDrop\" action=\"OracleActionTableDrop\" />		"
"  				<menuitem name=\"OracleTableAlter\" action=\"OracleActionTableAlter\" />		"
"  				<menuitem name=\"OracleTableBrowse\" action=\"OracleActionTableBrowse\" />	"
"	    </placeholder> "
"  </popup> "
"</ui>";

static GtkActionEntry table_acts[] = 
{
	{ "OracleActionTableCreate", GTK_STOCK_NEW, 
		N_("Create..."), NULL, 
		N_("Create table"), 
		G_CALLBACK(on_popup_table_create) },
	
	{ "OracleActionTableDrop", GTK_STOCK_DELETE, 
		N_("Drop..."), NULL, 
		N_("Drop table"), 
		G_CALLBACK(on_popup_table_drop) },
	
	{ "OracleActionTableAlter", NULL, 
		N_("Alter..."), NULL, 
		N_("Alter table"), 
		G_CALLBACK(on_popup_table_alter) },
	
	{ "OracleActionTableBrowse", NULL, 
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
								   G_TYPE_STRING, "%",
								   -1) == GSQL_CURSOR_STATE_ERROR)
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
			memset (key, 0, 256);
			g_snprintf (key, 255, "%s%d%s",
				   name, TABLE_ID, name);
			
			details = gsql_navigation_get_details (navigation, key);
			oracle_navigation_fill_details (cursor, details);
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
	}
	
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
		actions = gtk_action_group_new ("OraclePopupTableActions");
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
							  (gchar *) sql_oracle_table_columns);
	GSQL_DEBUG ("owner [%s] name [%s]", owner, name);
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
	editor = gsql_editor_new (source);
	gsql_content_set_child (content, GTK_WIDGET (editor));
	
	workspace = gsql_session_get_workspace (session);
	gsql_workspace_add_content (workspace, content);
	gsql_content_set_name_full (content, name, name);
	
	gsql_cursor_close (cursor);
	g_free (sql);
	
	gsql_editor_run_sql (editor);

}






















// ===============================================
/*
static GtkActionEntry nav_tree_popup_table_actions[] = 
{
	{ "OracleActionPopupNavTableCreate", GTK_STOCK_NEW, 
		N_("Create..."), NULL, 
		N_("Create table"), 
		G_CALLBACK(on_nav_tree_popup_tables_create) },
	
	{ "OracleActionPopupNavTableDrop", GTK_STOCK_DELETE, 
		N_("Drop..."), NULL, 
		N_("Drop table"), 
		G_CALLBACK(on_nav_tree_popup_tables_drop) },
	
	{ "OracleActionPopupNavTableAlter", NULL, 
		N_("Alter..."), NULL, 
		N_("Alter table"), 
		G_CALLBACK(on_nav_tree_popup_tables_alter) },
	
	{ "OracleActionPopupNavTableBrowse", NULL, 
		N_("Browse data"), NULL, 
		N_("Browse data"), 
		G_CALLBACK(on_nav_tree_popup_tables_browse) },
	
	{ "OracleActionPopupNavTableEditData", NULL, 
		N_("Browse with update"), NULL, 
		N_("Browse with update"), 
		G_CALLBACK(on_nav_tree_popup_tables_browse_update) },
	
	{ "OracleActionPopupNavTableAnalyze", NULL, 
		N_("Analyze..."), NULL, 
		N_("Analyze table"), 
		G_CALLBACK(on_nav_tree_popup_tables_analyze) }
};

static const gchar *nav_tree_popup_ui = 
" <ui>																									\n"
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\">												\n"
"  	<placeholder name=\"PHolderNavObjects\">															\n"
"  			<placeholder name=\"PHolderNavObjectDo\" >													\n"
"  				<menuitem name=\"OracleNavTableCreate\" action=\"OracleActionPopupNavTableCreate\" />	\n"
"  				<menuitem name=\"OracleNavTableDrop\" action=\"OracleActionPopupNavTableDrop\" />		\n"
"  				<menuitem name=\"OracleNavTableAlter\" action=\"OracleActionPopupNavTableAlter\" />		\n"
"  				<separator name=\"separator01\" />														\n"
"  				<menuitem name=\"OracleNavTableBrowse\" action=\"OracleActionPopupNavTableBrowse\" />	\n"
"  				<menuitem name=\"OracleNavTableEditData\" action=\"OracleActionPopupNavTableEditData\" />	\n"
"  				<separator name=\"separator02\" />														\n"
"  				<menuitem name=\"OracleNavTableAnalyze\" action=\"OracleActionPopupNavTableAnalyze\" />	\n"
"  			</placeholder>																				\n"
"  	</placeholder>																						\n"
"  </popup>																								\n"
"</ui> ";

void
nav_tree_refresh_tables (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
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
	GSQLEOracleCursor *cursor;
	GSQLEOracleVariable *var;
	gchar *name;
	gchar key[256];

	
	/*model = gtk_tree_view_get_model(tv);
	n = gtk_tree_model_iter_n_children(model, iter);
	for (; n>1; n--)
	{
		gtk_tree_model_iter_children(model, &child, iter);
		gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
	};
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
	g_return_if_fail (owner != NULL);
	
	if (strncmp (owner, session->username, 64))
		sql = (gchar *) sql_oracle_tables;
	
	cursor = oracle_cursor_open_bind (session, 
						                  sql, FALSE,
						                  2,
						                  owner, "%");
	if (cursor == NULL)
		return;
	
	var = oracle_cursor_get_variable(cursor, 0);
	i=0;
	while (oracle_cursor_fetch(cursor)>0)		
	{
		i++;			

		name = oracle_variable_data_to_display_format (var);
		// make a key for a hash of details
		g_snprintf (key, 256, "%s%d%s%x",
				   name, TABLE_ID, name, session);
		detail = nav_tree_fill_detail (cursor->var_list, 
									   key, NULL);

		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			TABLE_ID,
					GSQL_NAV_TREE_OWNER,		owner,
					GSQL_NAV_TREE_IMAGE,		GSQL_STOCK_TABLES,
					GSQL_NAV_TREE_NAME,			name,
					GSQL_NAV_TREE_REALNAME, 	name,
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
					GSQL_NAV_TREE_EXPAND_HANDLER, nav_tree_refresh_static,
					GSQL_NAV_TREE_STRUCT, tables,
					GSQL_NAV_TREE_DETAILS, detail,
					GSQL_NAV_TREE_NUM_ITEMS, G_N_ELEMENTS(tables),
					-1);
		g_free (name);	
		memset(var->data, 0, var->max_len);
		gtk_tree_store_append (GTK_TREE_STORE (model), &child_fake, &child);
		gtk_tree_store_set (GTK_TREE_STORE (model), &child_fake,
				GSQL_NAV_TREE_ID,				-1,
				GSQL_NAV_TREE_IMAGE,			NULL,
				GSQL_NAV_TREE_NAME,				N_("Processing..."),
				GSQL_NAV_TREE_REALNAME,			NULL,
				GSQL_NAV_TREE_ITEM_INFO,		NULL,
				GSQL_NAV_TREE_SQL,				NULL,
				GSQL_NAV_TREE_OBJECT_HANDLER,	NULL,
				GSQL_NAV_TREE_EXPAND_HANDLER,	NULL,
				GSQL_NAV_TREE_STRUCT,			NULL,
				GSQL_NAV_TREE_NUM_ITEMS, 		NULL,
				-1);
	};
	
		
	if (i > 0)
	{
		name = g_strdup_printf("%s<span weight='bold'> [%d]</span>", 
												realname, i);
		gtk_tree_store_set (GTK_TREE_STORE(model), iter,
							GSQL_NAV_TREE_NAME, 
							name,
							-1);
		g_free (name);
	};
	gtk_tree_store_remove(GTK_TREE_STORE(model), &child_last);
	
	oracle_cursor_free(cursor);
	
	return;
};


GtkActionGroup *
nav_tree_tables_popup_actions ()
{
	GSQL_TRACE_FUNC;

	static GtkActionGroup *table_actions = NULL;
	static gint menu_id;

	if (table_actions == NULL)
	{		
		table_actions = gtk_action_group_new ("ActionNavTables");
	
		gtk_action_group_add_actions (table_actions, nav_tree_popup_table_actions, 
									G_N_ELEMENTS (nav_tree_popup_table_actions), NULL);
	
		menu_id = gsql_menu_merge_from_string (nav_tree_popup_ui, 
								   table_actions);
		GSQL_DEBUG ("Table's popup menu merged. actions = 0x%x", table_actions);
		
	};

	return table_actions;
};
	
void
nav_tree_tables_browse (gchar *name, gchar *owner, gboolean updatable)
{
	GSQL_TRACE_FUNC;

	GtkWidget *tmp_widget;
	GSQLSession *session;
	GSQLContent *content;
	GSQLEOracleCursor *cursor;
	GSQLEOracleVariable *var;
	gint tmp_int = 0;
	GtkTextBuffer *tbuffer;
	GtkTextIter   iter;
	gchar * sql = "select ";
	gchar * tmp = NULL;

	
	/*session = gsql_session_get_current ();
	
	cursor = oracle_cursor_open_bind (session, 
									  (gchar *)	sql_oracle_table_columns,
									  FALSE, 
									  2, owner, name);
	var = oracle_cursor_get_variable(cursor, 0);
	
	while (oracle_cursor_fetch (cursor) > 0)
	{
		if (tmp_int == 0)
		{
			tmp = g_utf8_strdown (var->data, var->data_len);
			sql = g_strconcat (sql, "a.", tmp, NULL);
			GSQL_DEBUG ("making SQL: [clumns = %s] [sql = %s]", tmp, sql);
			g_free (tmp);
		}
		else
		{                                                        
			tmp = g_utf8_strdown (var->data, var->data_len);
			sql = g_strconcat (sql, ((tmp_int % 4) == 0) ? ", \n\t ": ", ",
									"a.", tmp, NULL);
			GSQL_DEBUG ("making SQL: [clumns = %s] [sql = %s]", tmp, sql);
			g_free (tmp);
		};
		tmp_int++;
		memset(var->data, 0, var->data_len);
	};
	sql = g_strconcat (sql,"\nfrom ", g_utf8_strdown (owner, strlen (owner)),
					   		".", g_utf8_strdown (name, strlen (name)),
										 " a\n", NULL);
	
	content = engine_sql_editor_new (session, name, sql, TRUE);
	gsql_content_add (session, content);

	gtk_window_set_focus (GTK_WINDOW (gsql_window), content->source);
	tbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (content->source));
	gtk_text_buffer_get_start_iter (tbuffer, &iter);
	gtk_text_buffer_place_cursor (tbuffer, &iter);
	
	if (sql != NULL) g_free (sql);
	oracle_cursor_free (cursor);	
	
	return;
};
*/
