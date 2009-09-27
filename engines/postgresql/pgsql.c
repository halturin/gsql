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
#include <libpq-fe.h>

#include "engine_conf.h"
#include "engine_session.h"

void
pgsql_session_hash_table_destroy (gpointer key, gpointer value, gpointer data);

gboolean
pgsql_session_open (GSQLEPGSQLSession *spec_session, 
		    gchar *username,
		    gchar *password,
		    gchar *database,
		    gchar *hostname,
		    gchar *port)
{
	GSQL_TRACE_FUNC;
	
	gchar *conninfo = g_strdup_printf ("host = '%s' port='%s' "\
					   "dbname = '%s' " \
					   "user = '%s' password = '%s' " \
					   "connect_timeout = '10'",
					   hostname, port, database, username,
					   password);
  

	spec_session->pgconn = PQconnectdb ( conninfo );
  
	if (! spec_session->pgconn || 
	    PQstatus(spec_session->pgconn) != CONNECTION_OK) 
	  {
	  	GSQL_DEBUG ("Connect failed");
		g_free ( conninfo );
		return FALSE;
	  }

	spec_session->hash_conns = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert(spec_session->hash_conns, g_strdup(database),
			    spec_session->pgconn);

	spec_session->server_version = 
	  (gchar *) PQparameterStatus (spec_session->pgconn, "server_version");
	spec_session->use = TRUE;
	g_free ( conninfo );

	if ( ! PQexec(spec_session->pgconn, "BEGIN;") ) {
		GSQL_DEBUG ("Trans: Transaction Not Started!!!");
	}

	return TRUE;
}

gboolean
pgsql_session_close (GSQLSession *session, gchar *buffer)
{
	GSQL_TRACE_FUNC;
  
	GSQLEPGSQLSession *spec_session;
	spec_session = (GSQLEPGSQLSession *) session->spec;

	g_hash_table_foreach(spec_session->hash_conns,
			     pgsql_session_hash_table_destroy, NULL);

	gsql_session_close (session);

	g_free (spec_session);
	
	return TRUE;
}

void
pgsql_session_hash_table_destroy (gpointer key, gpointer value, gpointer data)
{
	GSQL_TRACE_FUNC;
	PGconn *conn = (PGconn *)value;

	g_free(key);
	PQfinish (conn);
}

gchar *
pgsql_session_get_database(GSQLSession *session) {
	GSQL_TRACE_FUNC;
	GValue gdb = {0, };
	char *database = NULL;
	
	g_return_if_fail(GSQL_IS_SESSION(session));

	// Retrieving database from session
	g_value_init (&gdb, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (session), "session-database", &gdb);
	database = g_value_get_string(&gdb);

	GSQL_DEBUG ("Database: Session database: [%s]", database);
	return database;
}

void
pgsql_session_switch_database(GSQLSession *session, gchar *database) {
	GSQL_TRACE_FUNC;
	GSQLEPGSQLSession *spec_session;
	GValue gpass={0, }, ghost={0, };
	PGconn *conn, *newconn;
	gchar *username = NULL, *password = NULL, *hostname = NULL,
	  *currentdb = NULL, *port = "5432";

	g_return_if_fail(GSQL_IS_SESSION(session));
	spec_session = session->spec;
	conn = spec_session->pgconn;

	// Retrieving current database from connection. Session holds the
	// initial database, not the current one.
	currentdb = PQdb(conn);

	if ( ! g_strcmp0(currentdb, database) ) {
		GSQL_DEBUG("Database: Already connected to requested db.");
		return;
	}

	// Retrieving hostname from session
	g_value_init (&ghost, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (session), "session-hostname", &ghost);
	hostname = g_value_get_string(&ghost);

	// Retrieving password from session
	g_value_init (&gpass, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (session), "session-password", &gpass);
	password = g_value_get_string(&gpass);

	// Retrieving username from connection (retrieving it from session
	// returns NULL)
	username = PQuser(conn);

	if ( ! ( newconn = 
		g_hash_table_lookup(spec_session->hash_conns, database) ) ) 
	  {
		newconn = PQsetdbLogin(hostname, port, NULL, NULL, database,
				       username, password);

		if ( newconn &&
		     PQstatus(newconn) == CONNECTION_OK) {
			GSQL_DEBUG("Database: Successfully switched to [%s]",
				   database);
			g_hash_table_insert (spec_session->hash_conns,
					     database, newconn);
		} else {
			GSQL_DEBUG("Database: Switching to [%s] fail. "\
				   "Keep with [%s].", database, currentdb);
			newconn = conn;
		}
	  } else 
	  {
	  	GSQL_DEBUG("Database: using previously opened connection.");
	  }
	spec_session->pgconn = newconn;
}



void
pgsql_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	GSQLEPGSQLSession *spec = NULL;
	
	g_return_if_fail (GSQL_IS_SESSION(session));
	spec = (GSQLEPGSQLSession *)session->spec;
	if ( ! PQexec (spec->pgconn, "COMMIT") ) {
	  GSQL_DEBUG ("Trans: Unable to COMMIT");
	  return;
	}

	if ( ! PQexec (spec->pgconn, "BEGIN") ) {
	  GSQL_DEBUG ("Trans: Unable Start a NEW Transaction");
	  return;
	}

	GSQL_DEBUG ("Trans: transaction COMMITTED and RESTARTED");
	pgsql_session_workspace_info(session, "Changes committed.");
}

void
pgsql_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	GSQLEPGSQLSession *spec = NULL;
	
	g_return_if_fail (GSQL_IS_SESSION(session));
	spec = (GSQLEPGSQLSession *)session->spec;
	if ( ! PQexec (spec->pgconn, "ROLLBACK") ) {
	  GSQL_DEBUG ("Trans: Unable to ROLLBACK");
	  return;
	}

	if ( ! PQexec (spec->pgconn, "BEGIN") ) {
	  GSQL_DEBUG ("Trans: Unable Start a NEW Transaction");
	  return;
	}

	GSQL_DEBUG ("Trans: transaction ROLLED BACK and RESTARTED");
	pgsql_session_workspace_info(session, "Changes rolled back.");
}

void
pgsql_session_workspace_info (GSQLSession *session, gchar *msg) {
	GSQLWorkspace *workspace = NULL;
	g_return_if_fail ( GSQL_IS_SESSION (session) );

	workspace = gsql_session_get_workspace (session);
	g_return_if_fail (GSQL_IS_WORKSPACE(workspace));

	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, msg);
}

gchar *
pgsql_session_get_error (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	gchar *err = NULL;
	g_return_if_fail (GSQL_SESSION (session) != NULL);
	
	GSQLEPGSQLSession *sess = session->spec;
	g_return_if_fail (sess != NULL);
	
	err = g_strdup((gchar *) PQerrorMessage (sess->pgconn));

	pgsql_session_rollback (session);
	return err;
}
