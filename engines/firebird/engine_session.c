/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2010  Smyatkin Maxim <smyatkinmaxim@gmail.com>
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


#include <string.h>

#include <libgsql/engines.h>
#include <libgsql/common.h>
#include <libgsql/editor.h>
#include <libgsql/utils.h>

#include "engine_session.h"
#include "engine_menu.h"
#include "nav_tree__schemas.h"

static void on_session_close (GSQLSession *session, gpointer user_data);
static void on_session_reopen (GSQLSession *session, gpointer user_data);
static void on_session_duplicate (GSQLSession *session, gpointer user_data);
static void on_session_rollback (GSQLSession *session, gpointer user_data);
static void on_session_commit (GSQLSession *session, gpointer user_data);

gpointer 
engine_session_open (GtkWidget *logon_widget, gchar *buffer) {
	GSQL_TRACE_FUNC;

	GtkWidget *widget;
	gchar* buf;
	buf = g_malloc0(200);
		
	const gchar *username = NULL, *password = NULL, *database = NULL,
		*hostname = NULL, *port = NULL;
	GSQLSession  *session;
	GSQLWorkspace *workspace;
	GSQLNavigation *navigation;
	GSQLEFirebirdSession *firebird_session = NULL;
  
	widget = g_object_get_data (G_OBJECT (logon_widget), "username");
	username = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "password");
	password = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "database");
	database = gtk_combo_box_get_active_text (GTK_COMBO_BOX (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "hostname");
	hostname = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "port");
	port = gtk_entry_get_text (GTK_ENTRY (widget));
  
	if ((g_utf8_strlen(database,128) <= 0) || 
	    (g_utf8_strlen(username,128) <= 0)) {
		g_strlcpy (buffer, 
			   N_("Database and Username are required."), 256);
		return NULL;
	};
  
	if (g_utf8_strlen(hostname,128) <= 0) 
		hostname = g_strdup("localhost");

	firebird_session = g_malloc0 (sizeof (GSQLEFirebirdSession));
	firebird_session->use = FALSE;
	
	GSQL_FIXME;
	if (!port)
		port = 3050;
  	
	if (!firebird_session_open (firebird_session, username, password, database,
				 hostname, port, buf)) {
		if (buffer)
			g_strlcpy (buffer, 
				   buf, strlen (buf) + 1);
		g_free (firebird_session->dbhandle);
		g_free (firebird_session->trhandle);
		g_free (firebird_session);
		return FALSE;
	}
	GSQL_DEBUG("Database: username [%s]", username);
	session = gsql_session_new_with_attrs ("session-username", 
					       g_strdup(username),
					       "session-password",
					       g_strdup (password),
					       "session-database",
					       g_strdup (database),
					       "session-hostname",
					       g_strdup (hostname), 
					       "session-info",
					       g_strdup (firebird_session->server_version),
					       NULL);
	session->spec = firebird_session;

	workspace = gsql_workspace_new (session);
	navigation = gsql_workspace_get_navigation (workspace);
	
	nav_tree_set_root (navigation, (gchar *) g_strdup (database));
	
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
	
	g_snprintf(buffer, 256,
		   _("Connected to the Firebird database \"<b>%s</b>\".\n"
		     "<small>(%s)</small>"), 
		   g_utf8_strup (database, g_utf8_strlen (database, 128)),
		   firebird_session->server_version);
	
	gsql_message_add (workspace, GSQL_MESSAGE_NORMAL, buffer);
	
	GSQL_DEBUG ("New session created with name [%s]", 
		    gsql_session_get_name (session));

	return session;
}

/*
 * Static section
 */

static void
on_session_close (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	firebird_session_close (session, NULL);
}

static void
on_session_reopen (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_session_duplicate (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQL_FIXME;
}

static void
on_session_commit (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	firebird_session_commit (session);
}

static void
on_session_rollback (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	firebird_session_rollback (session);
}