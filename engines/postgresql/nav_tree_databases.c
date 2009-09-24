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
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <string.h>

#include <libgsql/session.h>
#include <libgsql/stock.h>
#include <libgsql/sqleditor.h>
#include <libgsql/cvariable.h>

#include "nav_objects.h"
#include "nav_sql.h"

#include "nav_tree_databases.h"

static void nav_tree_databases_event (GSQLNavigation *navigation,
				      GtkTreeView *tv,
				      GtkTreeIter *iter, guint event);
static void nav_tree_databases_popup (GSQLNavigation *navigation,
				      GtkTreeView *tv,
				      GtkTreeIter *iter, guint event);
static void nav_tree_databases_editor (GSQLNavigation *navigation,
				       GtkTreeView *tv,
				       GtkTreeIter *iter, guint event);


static void on_popup_database_create (GtkMenuItem * menuitem, 
				      gpointer user_data);
static void on_popup_database_drop (GtkMenuItem * menuitem, 
				    gpointer user_data);
static void on_popup_database_alter (GtkMenuItem * menuitem, 
				     gpointer user_data);



static gchar database_ui[] = 
" <ui> "
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\"> "
"  		<placeholder name=\"PHolderNavObjectDo\"> "
//"<menuitem name=\"PGSQLDatabaseCreate\" action=\"PGSQLActionDatabaseCreate\" />	"
//"<menuitem name=\"PGSQLDatabaseDrop\" action=\"PGSQLActionDatabaseDrop\" />		"
"	    </placeholder> "
"  </popup> "
"</ui>";

static GtkActionEntry database_acts[] = 
{
	{ "PGSQLActionDatabaseCreate", GTK_STOCK_NEW, 
		N_("Create..."), NULL, 
		N_("Create database"), 
		G_CALLBACK(on_popup_database_create) },
	
	{ "PGSQLActionDatabaseDrop", GTK_STOCK_DELETE, 
		N_("Drop..."), NULL, 
		N_("Drop database"), 
		G_CALLBACK(on_popup_database_drop) },
	
/*	{ "PGSQLActionDatabaseAlter", NULL, 
		N_("Alter..."), NULL, 
		N_("Alter database"), 
		G_CALLBACK(on_popup_database_alter) },
*/	
};


void
nav_tree_refresh_databases (GSQLNavigation *navigation,
			 GtkTreeView *tv,
			 GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	GtkTreeModel *model;
	GtkTreeIter child, child_fake, child_last;
	GtkListStore *details;
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GSQLCursor * cursor;
	GSQLVariable *var;
	gint n;
	gchar key[256], *sql = NULL, *realname = NULL, *name = NULL,
		*owner = NULL;
  
	model = gtk_tree_view_get_model(tv);
	
	n = gtk_tree_model_iter_n_children(model, iter);
  
	for (; n>1; n--) {
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
	g_return_if_fail (owner != NULL);
	
	session = gsql_session_get_active ();
	g_return_if_fail (GSQL_IS_SESSION(session));
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
  
	GSQL_DEBUG ("cursor state [%d]. Start fetching",
		    gsql_cursor_get_state (cursor));
  
	if (var == NULL) {
		GSQL_DEBUG ("var is NULL");
		return;
	}
	 
	GSQL_DEBUG ("var->data = [%s]", (gchar *) var->value);
	n = 0;
	
	while (gsql_cursor_fetch (cursor, 1) > 0) {
		n++;
		if (var->value_type != G_TYPE_STRING) {
		  GSQL_DEBUG ("The name of object should be a string "\
			      "(char *). Is the bug");
		  name = N_("Incorrect data");
		} else {
		  name = (gchar *) var->value;
		  // make a key for a hash of details
		  g_snprintf (key, 256, "%s%d%s%x",
			      name, DATABASE_ID, name, session);
		  details = gsql_navigation_get_details (navigation, key);
		  pgsql_navigation_fill_details (cursor, details);
		}
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
				    GSQL_NAV_TREE_ID,		DATABASE_ID,
				    GSQL_NAV_TREE_OWNER,	owner,
				    GSQL_NAV_TREE_IMAGE,GSQL_STOCK_ALL_SCHEMAS,
				    GSQL_NAV_TREE_NAME,		name,
				    GSQL_NAV_TREE_REALNAME, 	name,
				    GSQL_NAV_TREE_ITEM_INFO, 	NULL,
				    GSQL_NAV_TREE_SQL,		NULL,
				    GSQL_NAV_TREE_OBJECT_POPUP,
				    nav_tree_databases_popup,
				    GSQL_NAV_TREE_OBJECT_HANDLER, NULL,
				    //FIXME: nav_tree_databases_editor,
				    GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
				    GSQL_NAV_TREE_EVENT_HANDLER,
				    nav_tree_databases_event,
				    GSQL_NAV_TREE_STRUCT, databases,
				    GSQL_NAV_TREE_DETAILS, details,
				    GSQL_NAV_TREE_NUM_ITEMS,
				    G_N_ELEMENTS(databases),
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
	
	GSQL_DEBUG ("Items fetched: [%d]", n);
	
	if (n > 0) {
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
 *  nav_tree_databases_event
 *  nav_tree_databases_popup
 *  nav_tree_databases_editor
 *
 *  on_popup_database_create
 *  on_popup_database_drop
 *  on_popup_database_alter
 *
 */


static void
nav_tree_databases_event (GSQLNavigation *navigation,
			  GtkTreeView *tv,
			  GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	
	switch (event)
	{
		
		case GDK_Insert:
			GSQL_DEBUG ("Insert pressed");
			break;
		case GDK_Delete:
			GSQL_DEBUG ("Delete pressed");
			break;
	}
}

static void
nav_tree_databases_popup (GSQLNavigation *navigation,
			  GtkTreeView *tv,
			  GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	GtkActionGroup *actions = NULL;
	
	if (!gsql_navigation_get_action (navigation, 
					 "PGSQLActionDatabaseBrowse"))
	{
		actions = gtk_action_group_new ("PGSQLPopupDatabaseActions");
		gtk_action_group_add_actions (actions, database_acts, 
					      G_N_ELEMENTS (database_acts), 
					      NULL);
		gsql_navigation_menu_merge (navigation, database_ui, actions);
	}
	
	gsql_navigation_menu_popup (navigation, actions);
	
}

static void
nav_tree_databases_editor (GSQLNavigation *navigation,
			   GtkTreeView *tv,
			   GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_popup_database_create (GtkMenuItem * menuitem, 
			  gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_popup_database_drop (GtkMenuItem * menuitem, 
			gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_popup_database_alter (GtkMenuItem * menuitem, 
			 gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}
