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

static void
nav_tree_processlist_popup (GSQLNavigation *navigation,
			    GtkTreeView *tv,
			    GtkTreeIter *iter, guint event);

static void on_popup_table_create (GtkMenuItem * menuitem, 
				   gpointer user_data);


static void
on_popup_process_kill (GtkMenuItem * menuitem, 
		       gpointer user_data);

static gchar process_ui[] = 
" <ui> "
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\"> "
"  		<placeholder name=\"PHolderNavObjectDo\"> "
"  				<menuitem name=\"PGSQLProcessKill\" action=\"PGSQLActionProcessKill\" />	"
"	    </placeholder> "
"  </popup> "
"</ui>";

static GtkActionEntry process_acts[] = 
{
	{ "PGSQLActionProcessKill", GTK_STOCK_DELETE, 
	  N_("Kill"), NULL, 
	  N_("Kill process"), 
	  G_CALLBACK(on_popup_process_kill) },
};


void
nav_tree_refresh_processlist (GSQLNavigation *navigation,
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
  gchar *name, *parent_realname, key[256];
  GtkListStore *details;
  
	
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

  GSQL_DEBUG ("sql:[%s]     realname:[%s]    parent_realname:[%s]", sql, realname, parent_realname);
  
  cursor = gsql_cursor_new (session, sql);
  state = gsql_cursor_open(cursor, FALSE);
  
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
      GSQL_DEBUG("PID [%s]", name);
      g_snprintf (key, 256, "%s%d%s%x",
		  name, PROCESS_ID, name, session);
      details = gsql_navigation_get_details (navigation, key);
      pgsql_navigation_fill_details (cursor, details);
      
      gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
      gtk_tree_store_set (GTK_TREE_STORE(model), &child,
			  GSQL_NAV_TREE_ID,			PROCESS_LIST_ID,
			  GSQL_NAV_TREE_OWNER,		name,
			  GSQL_NAV_TREE_IMAGE,		GSQLE_PGSQL_STOCK_PROCESS_LIST,
			  GSQL_NAV_TREE_NAME,		name,
			  GSQL_NAV_TREE_REALNAME, 	name,
			  GSQL_NAV_TREE_ITEM_INFO, 	NULL,
			  GSQL_NAV_TREE_SQL,			NULL,
			  GSQL_NAV_TREE_OBJECT_POPUP, nav_tree_processlist_popup,
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

static void
nav_tree_processlist_popup (GSQLNavigation *navigation,
			    GtkTreeView *tv,
			    GtkTreeIter *iter, guint event)
{
	GSQL_TRACE_FUNC;
	GtkActionGroup *actions = NULL;
	
	if (!gsql_navigation_get_action (navigation, "PGSQLActionProcessKill"))
	{
		actions = gtk_action_group_new ("PGSQLPopupProcessActions");
		gtk_action_group_add_actions (actions, process_acts, 
					      G_N_ELEMENTS (process_acts), NULL);
		gsql_navigation_menu_merge (navigation, process_ui, actions);
	}

	gsql_navigation_menu_popup (navigation, actions);
	
}

static void
on_popup_process_kill (GtkMenuItem * menuitem, 
		       gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	GSQLWorkspace *workspace = NULL;
	GSQLNavigation *navigation = NULL;
	GSQLCursor *cursor = NULL;
	GtkTreeIter *iter = NULL;
	GtkTreeModel *model = NULL;
	gchar *procpid = NULL, msg[256];

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
			    GSQL_NAV_TREE_NAME,
			    &procpid, -1);

	GSQL_DEBUG ("Killing process [%s]", procpid);

	cursor = gsql_cursor_new(session, (gchar *)"select pg_cancel_backend (?)");
	if (gsql_cursor_open_with_bind(cursor,
				   FALSE,
				   GSQL_CURSOR_BIND_BY_POS,
				   G_TYPE_STRING, procpid,
				   -1) != GSQL_CURSOR_STATE_OPEN) {
		GSQL_DEBUG("Error killing process...");
		gsql_cursor_close (cursor);
		return;
	}

	g_sprintf(msg, _("Killing %s..."), procpid);
	gsql_message_add(workspace, GSQL_MESSAGE_NOTICE, msg);

	//pg_cancel_backend (procpid)
}
