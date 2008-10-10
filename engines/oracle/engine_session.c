/***************************************************************************
 *            engine_session.c
 *
 *  Wed Sep  5 01:07:06 2007
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

#include <libgsql/engines.h>
#include <libgsql/common.h>
#include <libgsql/editor.h>
#include <libgsql/cursor.h>
#include <libgsql/cvariable.h>
#include <libgsql/menu.h>
#include <string.h>
#include "engine_session.h"
#include "engine_menu.h"
#include "nav_tree_static.h"


static void on_session_close (GSQLSession *session, gpointer user_data);
static void on_session_reopen (GSQLSession *session, gpointer user_data);
static void on_session_duplicate (GSQLSession *session, gpointer user_data);
static void on_session_rollback (GSQLSession *session, gpointer user_data);
static void on_session_commit (GSQLSession *session, gpointer user_data);
static void on_session_switch (GSQLSession *session, gpointer user_data);

gpointer
engine_session_open (GtkWidget *logon_widget, gchar *buffer)
{
	GSQL_TRACE_FUNC;

	GtkWidget *widget;
		
	const gchar *username,
				*password,
				*database;
	gchar info[32];
	
	GSQLSession  *session;
	GSQLSession  *current;
	GSQLEngine   *engine;
	GSQLCursor   *cursor;
	GSQLCursorState c_state;
	GSQLWorkspace *workspace;
	GSQLNavigation *navigation;
	GSQLEOracleSession *spec;
	GList   *lst;
	gchar *sql = "select version from product_component_version "
	 			 "where instr(lower(product),'oracle') >0  "
				 "and rownum <2 ";
	GSQLVariable *variable;
	ub4 mode;

	
	gint connect_as = 0;
	widget = g_object_get_data (G_OBJECT (logon_widget), "username");
	username = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "password");
	password = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "database");
	database = gtk_combo_box_get_active_text (GTK_COMBO_BOX (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "mode");
	
	switch (gtk_combo_box_get_active (GTK_COMBO_BOX (widget)))
	{
		case 2:
			GSQL_DEBUG ("Session mode: OCI_SYSOPER");
			mode = OCI_SYSOPER;
			break;
		case 1:
			GSQL_DEBUG ("Session mode: OCI_SYSDBA");
			mode = OCI_SYSDBA;
			break;
			
		case 0:
		default:
			GSQL_DEBUG ("Session mode: OCI_DEFAULT");
			mode = OCI_DEFAULT;
	}
	
	session = gsql_session_new_with_attrs ("session-username", 
										   g_strdup(username),
										   "session-password",
										   g_strdup (password),
										   "session-database",
										   g_strdup (database),
										   NULL);
	
	if (!oracle_session_open (session, mode, buffer)) {
		
		g_object_unref(session);
		return NULL;
	}
	
	spec = session->spec;
	
	/* 
	 I know, this is the hack, but i think the 'workaround' sounds better :).
	 It is need for the cursor run at this place.
	 However the variable 'session->engine' will set to the original value.
	 */
	// <workaround>
	engine = g_object_get_data (G_OBJECT (logon_widget), "engine");
	session->engine = engine;
	// </workaround>
	
	cursor = gsql_cursor_new (session, sql);
	c_state = gsql_cursor_open (cursor, FALSE); 
	
	memset ((void *) info, 0, 32);
	
	if ((c_state == GSQL_CURSOR_STATE_OPEN) && (gsql_cursor_fetch (cursor, 1)))
	{
		lst = g_list_first (cursor->var_list);
		variable = GSQL_VARIABLE (lst->data);
		g_snprintf (info, 32, "%s", variable->value);
		
	} else {
		g_snprintf (info, 32, "%s", "0.0.0.0");
	}
	
	gsql_cursor_close (cursor);
	
	gsql_session_set_attrs (session, "session-info",
							info,
							NULL);	 
	
	workspace = gsql_workspace_new (session);
	navigation = gsql_workspace_get_navigation (workspace);
	
	gsql_navigation_set_root (navigation, GSQLE_ORACLE_STOCK_ORACLE, g_strdup (username), 
							  root_objects, G_N_ELEMENTS (root_objects));
	
	g_signal_connect (G_OBJECT (session), "close",
					  G_CALLBACK (on_session_close), session);
	g_signal_connect (G_OBJECT (session), "reopen",
					  G_CALLBACK (on_session_reopen), session);
	g_signal_connect (G_OBJECT (session), "duplicate",
					  G_CALLBACK (on_session_duplicate), session);
	g_signal_connect (G_OBJECT (session), "commit",
					  G_CALLBACK (on_session_commit), session);
	g_signal_connect (G_OBJECT (session), "rollback",
					  G_CALLBACK (on_session_rollback), session);
	g_signal_connect (G_OBJECT (session), "switch",
					  G_CALLBACK (on_session_switch), session);
	
	g_snprintf(buffer, 256,
			   _("Connect to the Oracle database \"<b>%s</b>\" succesfully\n"
				 "<small>(%s)</small>"), 
			   g_utf8_strup (database, g_utf8_strlen (database, 128)),
			   spec->server_version);
	
	gsql_message_add (workspace, GSQL_MESSAGE_NORMAL, buffer);
	
	GSQL_DEBUG ("New session created with name [%s]", gsql_session_get_name (session));
	
	return session;
}

/* Static section:
 *
 *
 *
 *
 *
 */

static void
on_session_close (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
}

static void
on_session_reopen (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
}

static void
on_session_duplicate (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
}

static void
on_session_commit (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;;
	
	GSQLWorkspace *workspace;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);
	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction commited"));
	

}

static void
on_session_rollback (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);
	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction rolled back"));
	
}


static void
on_session_switch (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *current;
	GSQLEOracleSession *spec_session;
	GtkWidget *widget;
	GtkAction *act;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	current = gsql_session_get_active ();
	
	if (current == session)
	{
		gsql_engine_menu_set_status (session->engine, TRUE);
		
		spec_session = session->spec;
		
		widget = gsql_menu_get_widget ("/MenuMain/PHolderEngines/MenuOracle/OracleServerOutput");

		act = gtk_action_group_get_action (session->engine->action, "OracleActionServerOutput");
		gtk_action_block_activate_from (act, widget);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), spec_session->dbms_output);
		gtk_action_unblock_activate_from (act, widget);

		GSQL_DEBUG ("Oracle engine. Yes, It is mine");
		
	} else {
		
		gsql_engine_menu_set_status (session->engine, FALSE);

			GSQL_DEBUG ("Oracle engine. No, It is not mine");
	}
	
}
