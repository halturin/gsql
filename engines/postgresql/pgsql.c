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

#include <libpq-fe.h>
#include <glib.h>

//#include <libgsql/conf.h>
//#include <libgsql/session.h>
//#include <libgsql/common.h>

#include "engine_conf.h"
#include "engine_session.h"

gboolean
pgsql_session_open (GSQLEPGSQLSession *spec_session, 
		    gchar *username,
		    gchar *password,
		    gchar *database,
		    gchar *hostname,
		    guint port)
{
  GSQL_TRACE_FUNC;
	
  gchar *conninfo = g_strdup_printf ("host = '%s' port='%d' dbname = '%s' user = '%s' password = '%s' connect_timeout = '10'",
  				     hostname, port, database, username, password);
  

  spec_session->pgconn = 
    PQconnectdb ( conninfo );
  
  if (! spec_session->pgconn || PQstatus(spec_session->pgconn) != CONNECTION_OK) {
    GSQL_DEBUG ("Connect failed");
    g_free ( conninfo );
    return FALSE;
  }

  GSQL_DEBUG ("Current Connection: [%p]", spec_session->pgconn);
	
  spec_session->server_version = (gchar *) PQparameterStatus (spec_session->pgconn, "server_version");
  spec_session->use = TRUE;
  g_free ( conninfo );

  return TRUE;
}

gboolean
pgsql_session_close (GSQLSession *session, gchar *buffer)
{
  GSQL_TRACE_FUNC;
  
  GSQLEPGSQLSession *spec_session;
  spec_session = (GSQLEPGSQLSession *) session->spec;

  PQfinish (spec_session->pgconn);

  g_free (spec_session->pgconn);
  g_free (spec_session);
	
  return TRUE;
}

void
pgsql_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	/* GSQLWorkspace *workspace; */
	/* GSQLEMySQLSession *spec_session; */
	/* gchar error_str[2048]; */
	
	/* g_return_if_fail (GSQL_IS_SESSION (session)); */
	
	/* spec_session = session->spec; */
	
	/* workspace = gsql_session_get_workspace (session); */
	
	/* if (!mysql_commit (spec_session->mysql)) */
	/* { */
	/* 	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction commited")); */
		
	/* 	return; */
	/* } */
	
	/* memset (error_str, 0, 2048); */
	
	/* g_sprintf (error_str, "Error occured: [%d]%s",  */
	/* 		   mysql_errno (spec_session->mysql),  */
	/* 		   mysql_error (spec_session->mysql)); */
	
	/* gsql_message_add (workspace, GSQL_MESSAGE_WARNING, error_str); */
}

void
pgsql_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	/* GSQLWorkspace *workspace; */
	/* GSQLEMySQLSession *spec_session; */
	/* gchar error_str[2048]; */
	
	/* g_return_if_fail (GSQL_IS_SESSION (session)); */
	
	/* spec_session = session->spec; */
	
	/* workspace = gsql_session_get_workspace (session); */
	
	/* if (!mysql_rollback (spec_session->mysql)) */
	/* { */
	/* 	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, N_("Transaction rolled back")); */
		
	/* 	return; */
	/* } */
	
	/* memset (error_str, 0, 2048); */
	
	/* g_sprintf (error_str, "Error occured: [%d]%s",  */
	/* 		   mysql_errno (spec_session->mysql),  */
	/* 		   mysql_error (spec_session->mysql)); */
	
	/* gsql_message_add (workspace, GSQL_MESSAGE_WARNING, */
	/* 							  error_str); */
}

gchar *
pgsql_session_get_error (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	/* g_return_if_fail (GSQL_SESSION (session) != NULL); */
	
	/* GSQLEMySQLSession *sess = session->spec; */
	
	/* g_return_if_fail (sess != NULL); */
	
	/* return (gchar *) mysql_error(sess->mysql); */
	
}
