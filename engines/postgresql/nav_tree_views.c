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
#include <gdk/gdkkeysyms.h>
#include <string.h>
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
		NULL, 					// sql
		NULL, 					// object_popup
		NULL,					// object_handler
		(GSQLNavigationHandler)
		nav_tree_refresh_columns,		// expand_handler
		NULL,					// event_handler
		NULL, 0},				// child, childs
	
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS,
		N_("Triggers"), 
		sql_pgsql_triggers,			// sql
		NULL, 					// object_popup
		NULL,					// object_handler
		(GSQLNavigationHandler) 
		nav_tree_refresh_triggers,		// expand_handler
		NULL,					// event_handler
		NULL, 0},				// child, childs
	
};

static GtkActionEntry view_actions[] = 
	  {

	  	{ "PGSQLActionViewBrowse", NULL, 
			N_("Browse data"), NULL, 
			N_("Open SQL editor to browse the data  [ F3 ]"), 
			G_CALLBACK(on_popup_view_browse) },
		{ "PGSQLActionViewAlter", NULL, 
			N_("Edit"), NULL, 
			N_("Open SQL editor to change the view "), 
			G_CALLBACK(on_popup_view_alter) },

};

static gchar view_ui[] = 
" <ui>							"
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\">	"
"    <placeholder name=\"PHolderNavObjectDo\" >	"
"      <menuitem name=\"PGSQLViewBrowse\" action=\"PGSQLActionViewBrowse\" />"
"      <menuitem name=\"PGSQLViewAlter\" action=\"PGSQLActionViewAlter\" /> "
"  		</placeholder>					"
"  </popup>							"
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
	GtkTreeIter child, child_fake, child_last;
	GSQLCursor *cursor;
	GSQLVariable *var;
	GSQLCursorState state;
	GSQLSession *session;
	gchar *name, key[256], *sql = NULL, *realname = NULL, *owner = NULL,
		*currentdb = NULL;
	gint id, i,n;

	
	model = gtk_tree_view_get_model(tv);
	n = gtk_tree_model_iter_n_children(model, iter);
	
	for (; n>1; n--) {
		gtk_tree_model_iter_children(model, &child, iter);
		gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
	}
	
	gtk_tree_model_iter_children(model, &child_last, iter);
	
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
	var = g_list_nth_data(cursor->var_list,0);
	
	if (state != GSQL_CURSOR_STATE_OPEN) {
		gsql_cursor_close (cursor);
		return;		
	}
	
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
				    session, owner, VIEW_ID, name);
			
			details = gsql_navigation_get_details (navigation,
							       key);
			pgsql_navigation_fill_details (cursor, details);
		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
				    GSQL_NAV_TREE_ID,		VIEW_ID,
				    GSQL_NAV_TREE_OWNER,	owner,
				    GSQL_NAV_TREE_IMAGE,GSQL_STOCK_VIEWS,
				    GSQL_NAV_TREE_NAME,		name,
				    GSQL_NAV_TREE_REALNAME, 	name,
				    GSQL_NAV_TREE_ITEM_INFO, 	NULL,
				    GSQL_NAV_TREE_SQL,			NULL,
				    GSQL_NAV_TREE_OBJECT_POPUP,
				    nav_tree_views_popup,
				    GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
				    GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
				    GSQL_NAV_TREE_EVENT_HANDLER, 
				    nav_tree_views_event,
				    GSQL_NAV_TREE_STRUCT, views,
				    GSQL_NAV_TREE_DETAILS, details,
				    GSQL_NAV_TREE_NUM_ITEMS, 
				    G_N_ELEMENTS(views),
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

static void
nav_tree_views_event (GSQLNavigation *navigation,
		      GtkTreeView *tv,
		      GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	
	switch (event) {
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
	GtkActionGroup *actions = NULL;
	
	if (!gsql_navigation_get_action (navigation, "PGSQLActionViewBrowse"))
	  {
		actions = gtk_action_group_new ("PGSQLPopupViewActions");
		gtk_action_group_add_actions (actions, view_actions, 
					      G_N_ELEMENTS (view_actions), 
					      NULL);
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
	GSQL_FIXME;
}

static void
on_popup_view_create (GtkMenuItem * menuitem, 
		      gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_popup_view_drop (GtkMenuItem * menuitem, 
		    gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_popup_view_alter (GtkMenuItem * menuitem, 
		     gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	GSQLWorkspace *workspace = NULL;
	GSQLNavigation *navigation = NULL;
	GSQLCursor *cursor = NULL;
	GSQLContent *content = NULL;
	GSQLEditor *editor = NULL;
	GtkTreeIter *iter = NULL, det_iter;
	GtkTreeModel *model = NULL;
	GtkTreeModel *details = NULL;
	GtkWidget *source = NULL;
	gchar *view_code = NULL, *name, *owner;
	gboolean valid = FALSE;

	session = gsql_session_get_active();
	g_return_if_fail(GSQL_IS_SESSION(session));

	workspace = gsql_session_get_workspace(session);
	g_return_if_fail(GSQL_IS_WORKSPACE(workspace));

	navigation = gsql_workspace_get_navigation (workspace);
	g_return_if_fail(GSQL_IS_NAVIGATION(navigation));

	iter = gsql_navigation_get_active_iter (navigation);

	if (! iter) {
		GSQL_DEBUG ("No selection");
		return;
	}

	model = gsql_navigation_get_model(navigation);
	gtk_tree_model_get (model, iter,
			    GSQL_NAV_TREE_NAME, &name, 
			    GSQL_NAV_TREE_OWNER, &owner, 
			    GSQL_NAV_TREE_DETAILS, &details, 
			    -1);

	valid = gtk_tree_model_get_iter_first (details, &det_iter);
	while (valid) {
		gchar *det_name, *value;
		gtk_tree_model_get (details, &det_iter,
				    GSQL_NAV_DETAILS_NAME, &det_name,
				    GSQL_NAV_DETAILS_VALUE, &value,
				    -1);
		//GSQL_DEBUG ("Reading [%s] from details", det_name);
		if ( ! g_strcmp0 (det_name, "view_definition") ) {
			view_code = g_strdup_printf("create or replace view "\
						    "%s.%s as\n%s", owner, 
						    name, value);
			//GSQL_DEBUG("[%s] = [%s]", det_name, view_code);
			break;
		}
		valid = gtk_tree_model_iter_next (details, &det_iter);
	}

	content = gsql_content_new(session, GTK_STOCK_FILE);
	gsql_content_set_name_full(content, name, name);

	source = gsql_source_editor_new(view_code);
	editor = gsql_editor_new(session, source);
	gsql_content_set_child(content, GTK_WIDGET(editor));
	
	gsql_workspace_add_content(workspace, content);

	g_free (view_code);
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
	
	if (!iter) {
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
