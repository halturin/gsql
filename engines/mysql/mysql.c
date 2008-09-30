/***************************************************************************
 *            mysql.c
 *
 *  Sun Mar 30 17:18:05 2008
 *  Copyright  2008  Taras Halturin
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
 
#include <mysql/mysql.h>
#include <glib.h>
#include <libgsql/session.h>
#include <libgsql/common.h>
#include "engine_session.h"


gboolean
mysql_session_open (GSQLSession *session, gint mode, gchar *buffer)
{
	GSQL_TRACE_FUNC;
	
	MYSQL *mysql;
	GValue hostname = {0, };
	GValue database = {0, };
	GValue username = {0, };
	GValue password = {0, };
	GSQLEMySQLSession *spec;
	
	g_return_if_fail (session != NULL);
	
	mysql = g_malloc0 (sizeof (MYSQL));
	mysql_init (mysql);

	spec = (GSQLEMySQLSession *) session->spec;
	spec->mysql = mysql;

	g_value_init (&hostname, G_TYPE_STRING);
	g_value_init (&database, G_TYPE_STRING);
	g_value_init (&username, G_TYPE_STRING);
	g_value_init (&password, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (session), "session-database", &database);
	g_object_get_property (G_OBJECT (session), "session-hostname", &hostname);
	g_object_get_property (G_OBJECT (session), "session-username", &username);
	g_object_get_property (G_OBJECT (session), "session-password", &password);
	GSQL_DEBUG ("try connect");
	if (!mysql_real_connect(mysql, g_value_get_string (&hostname),
								   g_value_get_string (&username),
								   g_value_get_string (&password),
								   g_value_get_string (&database),
								   0,NULL,0)) 
	{
		GSQL_DEBUG ("Connect failed");
		return FALSE;
	};
	
	spec->server_version = (gchar *) mysql_get_server_info (mysql);
	
	return TRUE;
};

gboolean
mysql_session_close (GSQLSession *session, gchar *buffer)
{
	GSQL_TRACE_FUNC;
	
	return TRUE;
};

void
mysql_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	return;
};

void
mysql_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	return;
};

gchar *
mysql_session_get_error (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	g_return_if_fail (GSQL_SESSION (session) != NULL);
	
	GSQLEMySQLSession *sess = session->spec;
	g_return_if_fail (sess != NULL);
	
	return (gchar *) mysql_error(sess->mysql);
	
	
};


