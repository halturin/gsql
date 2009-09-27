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
static void on_session_switch (GSQLSession *session, gpointer user_data);

gpointer 
engine_session_open (GtkWidget *logon_widget, gchar *buffer)
{
	GSQL_TRACE_FUNC;

	GtkWidget *widget;
		
	const gchar *username = NULL,
				*password = NULL,
				*database = NULL,
				*hostname = NULL,
				*portnumb = NULL;
	GSQLSession  *session;
	GSQLWorkspace *workspace;
	GSQLNavigation *navigation;
	GSQLEMySQLSession *mysql_session;
	guint port = 0;
	
	widget = g_object_get_data (G_OBJECT (logon_widget), "username");
	username = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "password");
	password = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "database");
	database = gtk_combo_box_get_active_text (GTK_COMBO_BOX (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "hostname");
	hostname = gtk_entry_get_text (GTK_ENTRY (widget));
	widget = g_object_get_data (G_OBJECT (logon_widget), "port");
	portnumb = gtk_entry_get_text (GTK_ENTRY (widget));
	port = atoi (portnumb);

	
	if ((g_utf8_strlen(database,128) <= 0) || (g_utf8_strlen(username,128) <= 0))
	{
		g_strlcpy (buffer, N_("The Database and Username fields are required."), 256);
		return NULL;
	};
	
	if (g_utf8_strlen(hostname,128) <= 0)
		hostname = "localhost";

	
	mysql_session = g_malloc0 (sizeof (GSQLEMySQLSession));
	mysql_session->use = FALSE;
	
	GSQL_FIXME;
	if (!port)
		port = 3306;
	
	if (!mysql_session_open (mysql_session, username, password, database, hostname, port))
	{
		if (buffer)
			g_strlcpy (buffer, (const gchar *) mysql_error (mysql_session->mysql), 256);
		
		g_free (mysql_session->mysql);
		g_free (mysql_session);
		
		return FALSE;
	}
	
	session = gsql_session_new_with_attrs ("session-username", 
										   g_strdup(username),
										   "session-password",
										   g_strdup (password),
										   "session-database",
										   g_strdup (database),
										   "session-hostname",
										   g_strdup (hostname),
										   "session-info",
										   g_strdup (mysql_session->server_version),
										   NULL);
	session->spec = mysql_session;
	
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
	g_signal_connect (G_OBJECT (session), "switch",
					  G_CALLBACK (on_session_switch), session);
	
	g_snprintf(buffer, 256,
			   _("Connect to the MySQL database \"<b>%s</b>\" succesfully\n"
				 "<small>(%s)</small>"), 
			   g_utf8_strup (database, g_utf8_strlen (database, 128)),
			   mysql_session->server_version);
	
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
	
	mysql_session_close (session, NULL);

}

static void
on_session_reopen (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);

	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE,
					  N_("Restoring the connection is automatic"));
}

static void
on_session_duplicate (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *new_session;
	GSQLEMySQLSession *new_spec, *spec;
	gchar *username, *database, *password, *hostname;
	GSQLWorkspace *new_workspace, *workspace;
	GSQLNavigation *navigation;
	gchar buffer[256], info[32];
	GtkWidget   *sessions;
	gchar *session_name;
	GList *lst;
	GtkWidget	*header;
	gint ret;
	guint port = 0;

	GSQL_FIXME;
	/* Do rework this function, not yet, later. It seems like a hack :) */
	
	spec = session->spec;
	
	new_spec = g_malloc0 (sizeof (GSQLEMySQLSession));
	
	username = (gchar *) gsql_session_get_username (session);
	password = (gchar *) gsql_session_get_password (session);
	database = (gchar *) gsql_session_get_database_name (session);
	hostname = (gchar *) gsql_session_get_hostname (session);

	workspace = gsql_session_get_workspace (session);
	
	GSQL_FIXME;
	if (!port)
		port = 3306;
	
	if (!mysql_session_open (new_spec, username, password, database, hostname, port))
	{
		if (buffer)
			g_strlcpy (buffer, (const gchar *) mysql_error (new_spec->mysql), 256);
		
		g_free (new_spec->mysql);
		g_free (new_spec);
		
		return;
	}

	new_session = gsql_session_new_with_attrs ("session-username", 
										   g_strdup(username),
										   "session-password",
										   g_strdup (password),
										   "session-database",
										   g_strdup (database),
										   "session-hostname",
										   g_strdup (hostname),
										   "session-info",
										   g_strdup (new_spec->server_version),
										   NULL);

	new_session->spec = new_spec;
	new_session->engine = session->engine;

	new_workspace = gsql_workspace_new (new_session);
	navigation = gsql_workspace_get_navigation (new_workspace);
	
	nav_tree_set_root (navigation, (gchar *) g_strdup (database));
	
	g_signal_connect (G_OBJECT (new_session), "close",
					  G_CALLBACK (on_session_close), new_session);
	g_signal_connect (G_OBJECT (new_session), "reopen",
					  G_CALLBACK (on_session_reopen), new_session);
	g_signal_connect (G_OBJECT (new_session), "duplicate",
					  G_CALLBACK (on_session_duplicate), new_session);
	g_signal_connect (G_OBJECT (new_session), "commit",
					  G_CALLBACK (on_session_commit), new_session);
	g_signal_connect (G_OBJECT (new_session), "rollback",
					  G_CALLBACK (on_session_rollback), new_session);
	g_signal_connect (G_OBJECT (new_session), "switch",
					  G_CALLBACK (on_session_switch), new_session);
	
	g_snprintf(buffer, 256,
			   _("Connect to the MySQL database \"<b>%s</b>\" succesfully\n"
				 "<small>(%s)</small>"), 
			   g_utf8_strup (database, g_utf8_strlen (database, 128)),
			   new_spec->server_version);
	
	gsql_message_add (new_workspace, GSQL_MESSAGE_NORMAL, buffer);
	
	GSQL_DEBUG ("New session created with name [%s]", gsql_session_get_name (new_session));
	
	sessions = g_object_get_data(G_OBJECT(gsql_window), "sessions");
	
	session_name = gsql_session_get_name (new_session);
	header = gsql_utils_header_new (create_pixmap(new_session->engine->file_logo),
									   session_name, NULL,
									   FALSE, (gint) 1);
	
	gtk_widget_show (GTK_WIDGET (new_session));
	
	ret = gtk_notebook_append_page (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (new_session), 
							  header);

	gtk_notebook_set_current_page (GTK_NOTEBOOK(sessions), ret);
	gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (new_session), TRUE);
}

static void
on_session_commit (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	mysql_session_commit (session);

}

static void
on_session_rollback (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	mysql_session_rollback (session);
}

static void
on_session_switch (GSQLSession *session, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *current;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	current = gsql_session_get_active ();
	
	if (current == session)
	{
		
		GSQL_DEBUG ("MySQL engine. Yes, It is mine");
		gsql_engine_menu_set_status (session->engine, TRUE);
		
		
	} else {
		
		GSQL_DEBUG ("MySQL engine. No, It is not mine");
		gsql_engine_menu_set_status (session->engine, FALSE);
	}
	
}
