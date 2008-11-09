/***************************************************************************
 *            nav_tree__procedures.c
 *
 *  Mon Oct 15 22:45:46 2007
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
#include <libgsql/editor.h>
#include "nav_objects.h"
#include "engine_stock.h"
#include "nav_sql.h"

#include "nav_tree__depend.h"
#include "nav_tree__arguments.h"

#include "nav_tree__procedures.h"

static GSQLNavigationItem procedures[] = {
	{   ARGUMENTS_ID,
		GSQL_STOCK_ARGUMENTS,
		N_("Arguments"), 
		sql_oracle_arguments,						// sql
		NULL, 						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_refresh_arguments,						// expand_handler
		NULL,						// event_handler
		NULL, 0},
	
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


static void on_code_editor_cb_revert (GSQLContent *content, gpointer user_data);
static void on_code_editor_cb_save (GSQLContent *content, gpointer user_data);
static void on_code_editor_cb_close (GSQLContent *content, 
									 gpointer user_data);
static void on_buffer_changed (GtkWidget *widget, gpointer user_data);


void
nav_tree_refresh_procedures (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter)
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
	GSQLCursor *cursor;
	GSQLCursorState state;
	GSQLVariable *var;
	GSQLSession *session;
	GtkListStore *details;
	gchar *name;
	gchar key[256];
	gchar *parent_type = NULL;
	gint  child_type;
	gchar *stock;


	model = gtk_tree_view_get_model(tv);
	n = gtk_tree_model_iter_n_children(model, iter);
	
	for (; n>1; n--)
	{
		gtk_tree_model_iter_children (model, &child, iter);
		gtk_tree_store_remove (GTK_TREE_STORE(model), &child);
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

	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_ID, 
						&id, -1);
	
	switch (id)
	{
		case FUNCTIONS_ID:
			parent_type = "FUNCTION";
			child_type = FUNCTION_ID;
			stock = GSQL_STOCK_FUNCTIONS;
			break;
		
		case PROCEDURES_ID:
			parent_type = "PROCEDURE";
			child_type = PROCEDURE_ID;
			stock = GSQL_STOCK_PROCEDURES;
			break;
			
		default:
			GSQL_DEBUG ("PROCEDURES: unhandled type");
			return;
	}
	
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
										G_TYPE_STRING, parent_type,
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
				   session, owner, child_type, name);
			
			details = gsql_navigation_get_details (navigation, key);
			oracle_navigation_fill_details (cursor, details);
		}
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child, iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child,
					GSQL_NAV_TREE_ID,			child_type,
					GSQL_NAV_TREE_OWNER,		owner,
					GSQL_NAV_TREE_IMAGE,		stock,
					GSQL_NAV_TREE_NAME,			name,
					GSQL_NAV_TREE_REALNAME, 	name,
					GSQL_NAV_TREE_ITEM_INFO, 	NULL,
					GSQL_NAV_TREE_SQL,			NULL,
					GSQL_NAV_TREE_OBJECT_POPUP, NULL,
					GSQL_NAV_TREE_OBJECT_HANDLER, nav_tree_code_editor,
					GSQL_NAV_TREE_EXPAND_HANDLER, NULL,
					GSQL_NAV_TREE_EVENT_HANDLER, NULL,
					GSQL_NAV_TREE_STRUCT, procedures,
					GSQL_NAV_TREE_DETAILS, details,
					GSQL_NAV_TREE_NUM_ITEMS, G_N_ELEMENTS(procedures),
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


/* 
	the code below needs to rework. 
	it is temporary solution.
 */


void
nav_tree_code_editor (GSQLNavigation *navigation,
					GtkTreeView *tv,
					GtkTreeIter *iter)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GSQLContent *content;
	GSQLCursor *cursor;
	GSQLCursorState state;
	GSQLVariable *var;
	GtkWidget *editor, *scroll;
	GtkTextBuffer *buffer;
	GtkTextIter txtiter;
	GtkTreeModel *model;
	gchar *realname, *owner, *parent_type, *tmp;
	guint id, child_type, i;
	gchar *stock;
	gboolean alien = FALSE;
	
	
	model = gtk_tree_view_get_model(tv);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	
	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);

	gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_ID, 
						&id, -1);
	
	switch (id)
	{
		case FUNCTION_ID:
			parent_type = "FUNCTION";
			child_type = FUNCTION_ID;
			stock = GSQL_STOCK_FUNCTIONS;
			break;
		
		case PROCEDURE_ID:
			parent_type = "PROCEDURE";
			child_type = PROCEDURE_ID;
			stock = GSQL_STOCK_PROCEDURES;
			break;
			
		case PACKAGE_ID:
			parent_type = "PACKAGE";
			child_type = PACKAGE_ID;
			stock = GSQLE_ORACLE_STOCK_PACKAGE;
			break;
		
		case PACKAGE_BODY_ID:
			parent_type = "PACKAGE BODY";
			child_type = PACKAGE_BODY_ID;
			stock = GSQLE_ORACLE_STOCK_PACKAGE_BODIES;
			break;
			
		case TRIGGER_ID:
			parent_type = "TRIGGER";
			child_type = TRIGGER_ID;
			stock = GSQL_STOCK_TRIGGERS;
			break;
			
		default:
			GSQL_DEBUG ("CODE EDITOR: unhandled type");
			return;
	}
	
	session = gsql_session_get_active ();
	
	if (strncmp (owner, gsql_session_get_username (session), 64))
		alien = TRUE;
	
	cursor = gsql_cursor_new (session, (gchar *) sql_oracle_code_source);
	state = gsql_cursor_open_with_bind (cursor,
										FALSE,
										GSQL_CURSOR_BIND_BY_NAME,
										G_TYPE_STRING, ":owner",
										G_TYPE_STRING, owner,
										G_TYPE_STRING, ":object_name",
										G_TYPE_STRING, realname,
										G_TYPE_STRING, ":object_type",
										G_TYPE_STRING, parent_type,
										-1);
	
	var = g_list_nth_data(cursor->var_list,0);
	
	if (state != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;		
	}
	
	i = 0;
	editor = gsql_source_editor_new (NULL);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
	
	while (gsql_cursor_fetch (cursor, 1) > 0)
	{
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &txtiter);
		
		if (i)
		{
			gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &txtiter,
								(gchar *) var->value, -1);
			
		} else {
			
			if (alien)
			{
				tmp = g_strdup_printf ("%s %s.%s", parent_type, 
									   g_utf8_strup (owner, -1),
									   g_utf8_strup (realname, -1));
			
				gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &txtiter,
										tmp,-1);
				g_free (tmp);
				
			} else {
				
				gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &txtiter,
								(gchar *) var->value, -1);
			}
		}
		
		i++;
	}
	
	gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (buffer), FALSE);
	
	gsql_cursor_close (cursor);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);	
	gtk_container_add (GTK_CONTAINER (scroll), editor);
	
	content = gsql_content_new (session, stock);
	gsql_content_set_child (content, GTK_WIDGET (scroll));
	
	workspace = gsql_session_get_workspace (session);
	gsql_workspace_add_content (workspace, content);
	gsql_content_set_name_full (content, realname, realname);
	
	g_signal_connect (G_OBJECT (content),
					  "close",
					  G_CALLBACK (on_code_editor_cb_close),
					  NULL);
	g_signal_connect (G_OBJECT (content),
					  "save",
					  G_CALLBACK (on_code_editor_cb_save),
					  NULL);
	g_signal_connect (G_OBJECT (content),
					  "revert",
					  G_CALLBACK (on_code_editor_cb_revert),
					  NULL);
	g_signal_connect (G_OBJECT (buffer), "modified-changed",
								  G_CALLBACK (on_buffer_changed), content);
	
}


static void
on_code_editor_cb_revert (GSQLContent *content, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	
}

static void
on_code_editor_cb_save (GSQLContent *content, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	
}

static void
on_code_editor_cb_close (GSQLContent *content, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	gboolean changed;
	guint ret;
	
	changed = gsql_content_get_changed (content);
	
	if (changed)
	{
		GSQL_DEBUG ("Ask for save changes");
		
		if (0)
			return;
	}
	
	gtk_widget_destroy (GTK_WIDGET (content));
}

static void 
on_buffer_changed (GtkWidget *widget, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLContent *content;
	
	content = user_data;
	
	gsql_content_set_changed (content, gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (widget)));
	
}
