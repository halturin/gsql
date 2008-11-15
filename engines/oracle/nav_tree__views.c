/***************************************************************************
 *            nav_tree__views.c
 *
 *  Sun Oct 14 20:59:11 2007
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
#include <string.h>
#include <libgsql/common.h>
#include <libgsql/stock.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/cvariable.h>
#include "nav_objects.h"
#include "engine_stock.h"

#include "nav_tree__tables.h"
#include "nav_tree__indexes.h"
#include "nav_tree__columns.h"
#include "nav_tree__triggers.h"
#include "nav_tree__depend.h"
#include "nav_sql.h"

static void nav_tree_views_event (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);
static void nav_tree_views_popup (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);
static void nav_tree_views_editor (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event);


static void on_popup_view_create (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_view_drop (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_view_alter (GtkMenuItem * menuitem, 
								 gpointer user_data);
static void on_popup_view_browse (GtkMenuItem * menuitem, 
								 gpointer user_data);


static GSQLNavigationItem views[] = {
	
	{	COLUMNS_ID,
		GSQL_STOCK_COLUMNS,
		N_("Columns"), 
		sql_oracle_table_columns_owner,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_columns,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS,
		N_("Triggers"), 
		sql_oracle_triggers_owner,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_triggers,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	DEPENDSON_ID,
		GSQLE_ORACLE_STOCK_DEPENDS_ON,
		N_("Depends On"), 
		sql_oracle_depends_on,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_depend,						// expand_handler
		NULL,						// event_handler
		NULL, 0},					// child, childs
	
	{	DEPENDENT_ID,
		GSQLE_ORACLE_STOCK_DEPENDENT,
		N_("Dependent Objects"), 
		sql_oracle_dependent_objects,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_depend,						// expand_handler
		NULL,						// event_handler
		NULL, 0}					// child, childs
};

static GtkActionEntry view_actions[] = 
{
/*	{ "OracleActionPopupNavViewCreate", GTK_STOCK_NEW, 
		N_("Create..."), NULL, 
		N_("Create a new view  [ Ins ]"), 
		G_CALLBACK(on_popup_view_create) },
	
	{ "OracleActionPopupNavViewDrop", GTK_STOCK_DELETE, 
		N_("Drop..."), NULL, 
		N_("Drop this view  [ Del ]"), 
		G_CALLBACK(on_popup_view_drop) },
	
	{ "OracleActionPopupNavViewAlter", NULL, 
		N_("Alter..."), NULL, 
		N_("Alter table"), 
		G_CALLBACK(on_popup_view_alter) },
*/	
	{ "OracleActionPopupNavViewBrowse", NULL, 
		N_("Browse data"), NULL, 
		N_("Open SQL editor to browse the data  [ F3 ]"), 
		G_CALLBACK(on_popup_view_browse) },

};

static gchar view_ui[] = 
" <ui>																									"
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\">												"
" 		<placeholder name=\"PHolderNavObjectDo\" >													"
//"  				<menuitem name=\"OracleNavViewCreate\" action=\"OracleActionPopupNavViewCreate\" />		"
//"  				<menuitem name=\"OracleNavViewDrop\" action=\"OracleActionPopupNavViewDrop\" />			"
//"  				<menuitem name=\"OracleNavViewAlter\" action=\"OracleActionPopupNavViewAlter\" />		"
"  				<menuitem name=\"OracleNavViewBrowse\" action=\"OracleActionPopupNavViewBrowse\" />		"
"  		</placeholder>																				"
"  </popup>																								"
"</ui> ";

void
nav_tree_refresh_views (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel *model;
	GtkListStore *details;
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
	GSQLVariable *var;
	GSQLCursorState state;
	GSQLSession *session;
	gchar *name;
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
	
	if (strncmp (owner, gsql_session_get_username (session), 64))
		sql = (gchar *) sql_oracle_users_objects;
	
	cursor = gsql_cursor_new (session, sql);
	
	state = gsql_cursor_open_with_bind (cursor,
										FALSE,
										GSQL_CURSOR_BIND_BY_NAME,
										G_TYPE_STRING, ":owner",
										G_TYPE_STRING, owner,
										G_TYPE_STRING, ":object_name",
										G_TYPE_STRING, "%",
										G_TYPE_STRING, ":object_type",
										G_TYPE_STRING, "VIEW",
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
						session, owner, VIEW_ID, name);
			
			details = gsql_navigation_get_details (navigation, key);
			oracle_navigation_fill_details (cursor, details);
		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			VIEW_ID,
					GSQL_NAV_TREE_OWNER,		owner,
					GSQL_NAV_TREE_IMAGE,		GSQL_STOCK_VIEWS,
					GSQL_NAV_TREE_NAME,			name,
					GSQL_NAV_TREE_REALNAME, 	name,
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_POPUP, nav_tree_views_popup,
					GSQL_NAV_TREE_OBJECT_HANDLER, NULL, // FIXME:nav_tree_views_editor,
					GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
					GSQL_NAV_TREE_EVENT_HANDLER, nav_tree_views_event,
					GSQL_NAV_TREE_STRUCT, views,
					GSQL_NAV_TREE_DETAILS, details,
					GSQL_NAV_TREE_NUM_ITEMS, G_N_ELEMENTS(views),
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
	};
	
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

static void
nav_tree_views_event (GSQLNavigation *navigation,
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
			on_popup_view_browse (NULL, NULL);
			break;
			
		case GDK_Delete:
			GSQL_DEBUG ("Delete pressed");
			break;
	}
}


static void
nav_tree_views_popup (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	static GtkActionGroup *actions = NULL;
	
	if (!actions)
	{
		actions = gtk_action_group_new ("OraclePopupViewActions");
		gtk_action_group_add_actions (actions, view_actions, 
								  G_N_ELEMENTS (view_actions), NULL);
		gsql_navigation_menu_merge (navigation, view_ui, actions);
	}
	
	gsql_navigation_menu_popup (navigation, actions);
	
}

static void
nav_tree_views_editor (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
}





static void
on_popup_view_create (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
}

static void
on_popup_view_drop (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
}

static void
on_popup_view_alter (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
}

static void
on_popup_view_browse (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session = NULL;
	GSQLContent *content = NULL;
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
