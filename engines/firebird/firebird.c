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

#include <glib.h>
#include <ibase.h>
#include <string.h>
#include "engine_conf.h"
#include "engine_session.h"
/*
void
firebird_session_hash_table_destroy (gpointer key, gpointer value, gpointer data);*/

gboolean
firebird_session_open (GSQLEFirebirdSession *spec_session, 
		    gchar *username,
		    gchar *password,
		    gchar *database,
		    gchar *hostname,
		    gchar *port,
            gchar *buffer)
{
	GSQL_TRACE_FUNC;
	gchar *dpb, *start;
	gshort dpb_lenth = 0;
	gchar *host;

	gchar db_items[] = { isc_info_db_sql_dialect, isc_info_version, isc_info_end};
	gchar res_buffer[40], *p, item;
	gint length = 0;
	
	
	ISC_STATUS status_vector[20];
	static char isc_tpb[] = {isc_tpb_version3,
		isc_tpb_write, isc_tpb_concurrency,
		isc_tpb_wait};

	char spb_buffer[6], *spb = spb_buffer;
	char request_buffer[]= {
		isc_info_svc_server_version};
	char result_buffer[1024];
	
	dpb = (gchar*) 0;
	host = (gchar*) 0;
	start = (gchar*) 0;
	host = g_malloc0 (strlen(hostname)+strlen(port)+strlen(database)+3);
	strcpy(host, hostname);
	strcat(host, "/");
	strcat(host, port);
	strcat(host, ":");
	strcat(host, database);
	isc_expand_dpb (&dpb, &dpb_lenth, isc_dpb_user_name, username, isc_dpb_password, password,NULL);
	isc_attach_database (status_vector,strlen(host), host, &spec_session->dbhandle,dpb_lenth,dpb);
	if (status_vector[0] == 1 && status_vector[1])
	{
		const ISC_STATUS *pvector = status_vector;
		GSQL_DEBUG ("Connect failed");
		isc_print_status (status_vector);
		fb_interpret (buffer, 150, &pvector);
		return FALSE;
	}

	isc_database_info (status_vector, &spec_session->dbhandle,
	                   sizeof (db_items), db_items,
	                   sizeof (res_buffer), res_buffer);
	if (status_vector[0] == 1 && status_vector[1])
	{
		const ISC_STATUS *pvector = status_vector;
		GSQL_DEBUG ("Connect failed");
		isc_print_status (status_vector);
		fb_interpret (buffer, 150, &pvector);
		return FALSE;
	}

	for (p = res_buffer; *p != isc_info_end;)
	{
		item = *p++;
		length = isc_vax_integer (p, 2);
		p += 2;
		if (item == isc_info_db_sql_dialect)
		{
			spec_session->dialect = isc_vax_integer (p, length);
		}
		if (item == isc_info_version)
		{
			spec_session->server_version = g_malloc0(length-1);
			strncpy(spec_session->server_version, p+2, length-2);
		}
		p += length;
	}
	
	isc_start_transaction (status_vector, &spec_session->trhandle, 1, 
	                       &spec_session->dbhandle, 
	                       (unsigned short) sizeof (isc_tpb), isc_tpb);

	p = result_buffer;
	*spb++ = isc_info_svc_timeout;
	ADD_SPB_NUMERIC(spb, 30);
	
	
	
	spec_session->hash_conns = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert(spec_session->hash_conns, g_strdup(database),
			    spec_session->dbhandle);

	g_free(host);
	spec_session->use = TRUE;

	return TRUE;
} 

gboolean
firebird_session_close (GSQLSession *session, gchar *buffer)
{
	GSQL_TRACE_FUNC;
	ISC_STATUS status_vector[20];
	GSQLEFirebirdSession *spec_session;
	spec_session = (GSQLEFirebirdSession *) session->spec;
	gsql_session_close (session);
	isc_detach_database (status_vector,&spec_session->dbhandle);
	g_free (spec_session->server_version);
	g_free (spec_session);
	return TRUE;
}
/*
void
firebird_session_hash_table_destroy (gpointer key, gpointer value, gpointer data)
{

}
*/
gchar *
firebird_session_get_database(GSQLSession *session) {
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
firebird_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	GSQLEFirebirdSession *spec = NULL;
	ISC_STATUS status_vector[20];
	static char isc_tpb[] = {isc_tpb_version3,
		isc_tpb_write, isc_tpb_concurrency,
		isc_tpb_wait};
	
	g_return_if_fail (GSQL_IS_SESSION(session));
	spec = (GSQLEFirebirdSession *)session->spec;
	isc_commit_transaction(status_vector, &spec->trhandle);
	if (status_vector[0] == 1 && status_vector[1])
	{
		GSQL_DEBUG ("Unable To Commit Transaction");
		firebird_session_workspace_info(session, "Unable To Rollback Transaction.");
		isc_print_status (status_vector);
		return;
	}

	isc_start_transaction (status_vector, &spec->trhandle, 1, 
	                       &spec->dbhandle, 
	                       (unsigned short) sizeof (isc_tpb), isc_tpb);

	GSQL_DEBUG ("Trans: transaction COMMITTED and RESTARTED");
	firebird_session_workspace_info(session, "Changes committed.");
}

void
firebird_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	GSQLEFirebirdSession *spec = NULL;
	ISC_STATUS status_vector[20];
	static char isc_tpb[] = {isc_tpb_version3,
		isc_tpb_write, isc_tpb_concurrency,
		isc_tpb_wait};
	
	g_return_if_fail (GSQL_IS_SESSION(session));

	spec = (GSQLEFirebirdSession *)session->spec;
	
	
	isc_rollback_transaction (status_vector, &spec->trhandle);
	if (status_vector[0] == 1 && status_vector[1])
	{
		GSQL_DEBUG ("Unable To Rollback Transaction");
		firebird_session_workspace_info(session, "Unable To Rollback Transaction.");
		isc_print_status (status_vector);
		return;
	}

	GSQL_DEBUG ("Trans: transaction ROLLED BACK and RESTARTED");
	firebird_session_workspace_info(session, "Changes rolled back.");
	
	isc_start_transaction (status_vector, &spec->trhandle, 1, 
	                       &spec->dbhandle, 
	                       (unsigned short) sizeof (isc_tpb), isc_tpb);
	
}

void
firebird_session_workspace_info (GSQLSession *session, gchar *msg) {
	GSQLWorkspace *workspace = NULL;
	g_return_if_fail ( GSQL_IS_SESSION (session) );

	workspace = gsql_session_get_workspace (session);
	g_return_if_fail (GSQL_IS_WORKSPACE(workspace));

	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, msg);
}
