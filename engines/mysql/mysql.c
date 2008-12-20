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

 
#include <mysql/mysql.h>
#include <glib.h>
#include <libgsql/conf.h>
#include <libgsql/session.h>
#include <libgsql/common.h>
#include "engine_session.h"
#include "engine_conf.h"


gboolean
mysql_session_open (GSQLEMySQLSession *spec_session, 
					gchar *username,
					gchar *password,
					gchar *database,
					gchar *hostname,
					guint port)
{
	GSQL_TRACE_FUNC;
	
	gchar *charset = NULL;
	gboolean use_custom_charset = FALSE;
	MYSQL *mysql;
	GSQLEMySQLSession *spec;
	my_bool reconnect = 1;
	
	mysql = g_malloc0 (sizeof (MYSQL));
	mysql_init (mysql);	
	spec_session->mysql = mysql;
	
	use_custom_charset = gsql_conf_value_get_boolean (GSQLE_CONF_MYSQL_USE_CUSTOM_CHARSET);
	
	if (use_custom_charset)
	{
		charset = gsql_conf_value_get_string (GSQLE_CONF_MYSQL_CUSTOM_CHARSET);
		if (charset)
			mysql_options(mysql, MYSQL_SET_CHARSET_NAME, charset);
	}
	
	if (!mysql_real_connect(mysql, hostname,
								   username,
								   password,
								   database,
								   port, NULL, 0)) 
	{
		GSQL_DEBUG ("Connect failed");
		return FALSE;
	}
	
	mysql_autocommit(mysql, 0);
	
	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);
	
	spec_session->server_version = (gchar *) mysql_get_server_info (mysql);
	
	return TRUE;
}

gboolean
mysql_session_close (GSQLSession *session, gchar *buffer)
{
	GSQL_TRACE_FUNC;
	
	GSQLEMySQLSession *spec_session;

	spec_session = (GSQLEMySQLSession *) session->spec;

	gsql_session_close (session);

	mysql_close (spec_session->mysql);
	
	g_free (spec_session);

	return TRUE;
}

void
mysql_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	GSQLEMySQLSession *spec_session;
	gchar error_str[2048];
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	spec_session = session->spec;
	
	workspace = gsql_session_get_workspace (session);
	
	if (!mysql_commit (spec_session->mysql))
	{
		gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction commited"));
		
		return;
	}
	
	memset (error_str, 0, 2048);
	
	g_sprintf (error_str, "Error occured: [%d]%s", 
			   mysql_errno (spec_session->mysql), 
			   mysql_error (spec_session->mysql));
	
	gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
								  error_str);
}

void
mysql_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	GSQLEMySQLSession *spec_session;
	gchar error_str[2048];
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	spec_session = session->spec;
	
	workspace = gsql_session_get_workspace (session);
	
	if (!mysql_rollback (spec_session->mysql))
	{
		gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction rolled back"));
		
		return;
	}
	
	memset (error_str, 0, 2048);
	
	g_sprintf (error_str, "Error occured: [%d]%s", 
			   mysql_errno (spec_session->mysql), 
			   mysql_error (spec_session->mysql));
	
	gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
								  error_str);
}

gchar *
mysql_session_get_error (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	g_return_if_fail (GSQL_SESSION (session) != NULL);
	
	GSQLEMySQLSession *sess = session->spec;
	
	g_return_if_fail (sess != NULL);
	
	return (gchar *) mysql_error(sess->mysql);
	
}


