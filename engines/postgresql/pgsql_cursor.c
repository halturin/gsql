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
#include <libgsql/session.h>

#include "engine_session.h"
#include "pgsql_cursor.h"
#include "pgsql_var.h"

#include <pthread.h>

typedef struct _GSQLEPGSQLCursor GSQLEPGSQLCursor;

struct _GSQLEPGSQLCursor {
	PGresult *result;
	int count;
};


static void pgsql_cursor_statement_detect (GSQLCursor *cursor);

static void
on_cursor_close (GSQLCursor *cursor, gpointer user_data) {
	GSQL_TRACE_FUNC;
	GSQLEPGSQLCursor  *e_cursor;
	GSQLEPGSQLVariable *var;
	gint i;
	
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	
	if (cursor->spec != NULL) {
		e_cursor = (GSQLEPGSQLCursor *)cursor->spec;
		
		PQclear (e_cursor->result);
		g_free (e_cursor);
	}
  
	return;
}

static gboolean
pgsql_cursor_prepare (GSQLCursor *cursor) {
	GSQL_TRACE_FUNC;

	GSQLEPGSQLSession *e_session = NULL;
	GSQLEPGSQLCursor  *e_cursor = NULL;
	GSQLWorkspace *workspace = NULL;
	gchar error_str[2048];

	e_session = cursor->session->spec;

	if (cursor->spec == NULL) {
		e_cursor = g_new0 (GSQLEPGSQLCursor, 1);
		cursor->spec = e_cursor;
		g_signal_connect (G_OBJECT (cursor), "close", 
				  G_CALLBACK (on_cursor_close), NULL);
	};
	
	return TRUE;	
}

GSQLCursorState
pgsql_cursor_open_bind (GSQLCursor *cursor, GList *args) {
	GSQL_TRACE_FUNC;
	GSQLEPGSQLSession *e_session = NULL;
	GSQLEPGSQLCursor  *e_cursor = NULL;
	GSQLVariable *var;
	GSQLWorkspace *workspace = NULL;
	PGSQL_FIELD *field;

	gulong binds_arg, n, n_fields, is_null = 1;
	// store parameters information
	Oid *paramTypes = NULL;
	const char **paramValues = NULL;
	int *paramLengths = NULL;
	int *paramFormats = NULL;

	GList *vlist = args;
	GType vtype;
	gchar error_str[2048];
	
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	
	g_return_if_fail(GSQL_IS_SESSION(cursor->session));
	e_session = cursor->session->spec;

	workspace = gsql_session_get_workspace (cursor->session);
	g_return_if_fail(GSQL_IS_WORKSPACE(workspace));
  
	if (!pgsql_cursor_prepare (cursor)) {
		return GSQL_CURSOR_STATE_ERROR;
	}

	e_cursor = cursor->spec;
	
	binds_arg = g_list_length (args) / 2;

	paramTypes = g_malloc0 ( sizeof(Oid) * binds_arg );
	paramValues = g_malloc0 ( sizeof (char *) * binds_arg );
	paramLengths = g_malloc0 ( sizeof(int) * binds_arg );
	paramFormats = g_malloc0 ( sizeof(int) * binds_arg );
	n = 0;

	GSQL_DEBUG ( "Executing [%s]...", cursor->sql );
	while (vlist) {
		vtype = (GType) vlist->data;
		vlist = g_list_next (vlist);
		if (vlist->data == NULL)
			is_null = 1;
		else 
			is_null = 0;
		switch (vtype) {
			case G_TYPE_STRING:
			case G_TYPE_POINTER:
				paramTypes[n] = VARCHAROID;
				paramValues[n] = (char *) vlist->data;
				paramLengths[n] = g_utf8_strlen((gchar *) 
								vlist->data,
								1048576);
				paramFormats[n] = 0;
				break;
	
			case G_TYPE_INT:
			case G_TYPE_UINT:
				paramTypes[n] = INT4OID;
				paramValues[n] = (char *) &vlist->data;
				paramLengths[n] = 0;
				paramFormats[n] = 0;
				break;
				
			case G_TYPE_UINT64:
			case G_TYPE_INT64:
				paramTypes[n] = INT8OID;
				paramValues[n] = (char *) &vlist->data;
				paramLengths[n] = 0;
				paramFormats[n] = 0;
				break;
	
			case G_TYPE_DOUBLE:
				paramTypes[n] = FLOAT4OID;
				paramValues[n] = (char *) &vlist->data;
				paramLengths[n] = 0;
				paramFormats[n] = 0;
				break;
		}

		//GSQL_DEBUG ("Parms [%d] = [%s]", n, paramValues[n]);
		vlist = g_list_next (vlist);
		n++;
	}

	if (! e_session->pgconn ) {
		GSQL_DEBUG ("BIND: pgconn empty!");
	}

	if ( PQstatus(e_session->pgconn) != CONNECTION_OK ) {
		GSQL_DEBUG("BIND: lost connection!");
	}

	e_cursor->result = PQexecParams(e_session->pgconn, cursor->sql,
					binds_arg, paramTypes, paramValues,
					paramLengths, paramFormats, 0);

	if ( ! e_cursor->result ) {
		g_sprintf ( error_str, "Error occured: %s", 
			    pgsql_session_get_error(cursor->session) );
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		g_free (paramTypes);
		g_free (paramValues);
		g_free (paramLengths);
		g_free (paramFormats);
    
		return GSQL_CURSOR_STATE_ERROR;
	}


	pgsql_cursor_statement_detect (cursor);
  
	g_free (paramTypes);
	g_free (paramValues);
	g_free (paramLengths);
	g_free (paramFormats);
  
	n_fields =  PQnfields (e_cursor->result);

	if (n_fields == 0)
		return GSQL_CURSOR_STATE_OPEN;
  
	for (n = 0; n < n_fields; n++) {
		// TODO: free this in on_variable_delete
		field = g_malloc0 ( sizeof(PGSQL_FIELD) );
		field->name = PQfname(e_cursor->result, n);
		field->type = PQftype(e_cursor->result, n);
		field->size = PQfsize(e_cursor->result, n);

		var = gsql_variable_new ();
		pgsql_variable_init (var, field);
		cursor->var_list = g_list_append (cursor->var_list, var);
	}

	return GSQL_CURSOR_STATE_OPEN;
}

GSQLCursorState
pgsql_cursor_open (GSQLCursor *cursor) {
	GSQL_TRACE_FUNC;
	PGconn *pgconn;
	GSQLEPGSQLSession *e_session = NULL;
	GSQLEPGSQLCursor  *e_cursor = NULL;
	GSQLEPGSQLVariable *e_var;
	GSQLVariable *var;
	GSQLWorkspace *workspace = NULL;
	PGSQL_FIELD *field;
	gulong n, n_fields, is_null = 1;
	gchar error_str[2048];
  
	g_return_val_if_fail (GSQL_IS_CURSOR(cursor), GSQL_CURSOR_STATE_ERROR);
	g_return_val_if_fail (GSQL_IS_SESSION(cursor->session), 
	                      GSQL_CURSOR_STATE_ERROR);
	e_session = cursor->session->spec;

	workspace = gsql_session_get_workspace (cursor->session);
	g_return_val_if_fail(GSQL_IS_WORKSPACE(workspace), 
	                     GSQL_CURSOR_STATE_ERROR);
	
	pgconn = e_session->pgconn;
	
	if (!pgsql_cursor_prepare (cursor)) {
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	e_cursor = cursor->spec;
	e_cursor->result = PQexec(e_session->pgconn, cursor->sql);

	if ( ! e_cursor->result ) {
		g_sprintf ( error_str, "Error occured: %s",
			    pgsql_session_get_error(cursor->session) );
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
    
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	pgsql_cursor_statement_detect (cursor);

	n_fields =  PQnfields (e_cursor->result);
	if (n_fields == 0 && 
	    PQresultStatus(e_cursor->result) == PGRES_COMMAND_OK)
		return GSQL_CURSOR_STATE_OPEN;

	if (n_fields == 0 && 
	    PQresultStatus(e_cursor->result) != PGRES_COMMAND_OK) {
		g_sprintf ( error_str, "Error occured: %s", 
			    pgsql_session_get_error(cursor->session) );
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	for (n = 0; n < n_fields; n++) {
		// where to free this?!?
		field = g_malloc0 ( sizeof(PGSQL_FIELD) );
		field->name = PQfname(e_cursor->result, n);
		field->type = PQftype(e_cursor->result, n);
		field->size = PQfsize(e_cursor->result, n);

		var = gsql_variable_new ();
		pgsql_variable_init (var, field);
		cursor->var_list = g_list_append (cursor->var_list, var);
	}

	return GSQL_CURSOR_STATE_OPEN;
}

GSQLCursorState 
pgsql_cursor_stop (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	GSQLEPGSQLSession *spec_session;
	GSQLEPGSQLCursor *spec_cursor;
	PGcancel *cancel = NULL;
	gchar buff[256];
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);

	session = cursor->session;
	g_return_val_if_fail (GSQL_IS_SESSION (session), 
	                      GSQL_CURSOR_STATE_ERROR);

	spec_session = session->spec;
	spec_cursor = cursor->spec;

	cancel = PQgetCancel (spec_session->pgconn);
	if ( ! PQcancel (cancel, buff, 256) ) {
		pgsql_session_workspace_info (session, buff);
		PQfreeCancel (cancel);
		return GSQL_CURSOR_STATE_ERROR;
	}
	PQfreeCancel (cancel);
	return GSQL_CURSOR_STATE_STOP;
}

gint
pgsql_cursor_fetch (GSQLCursor *cursor, gint rows) {
	GSQL_TRACE_FUNC;

	static int curr = 0;
	int nvar=0, nfields=0;
	GSQLEPGSQLCursor *e_cursor = NULL;
	GSQLVariable *var = NULL;
	GSQLEPGSQLVariable *pgvar = NULL;
	gchar error_str[2048];
	gint ret = 1;
	GList *vlist = cursor->var_list;

	g_return_if_fail (GSQL_CURSOR (cursor) != NULL);
	e_cursor = (GSQLEPGSQLCursor *) cursor->spec;
	if ( ! ( curr < e_cursor->count ) ) return curr = 0;
  
	nfields = PQnfields(e_cursor->result);

	for ( vlist = g_list_first(vlist), nvar=0; 
	      vlist && (nvar < nfields); 
	      vlist = g_list_next(vlist), nvar++ ) 
	  {
		var = (GSQLVariable *)vlist->data;
		var->value = PQgetvalue(e_cursor->result, curr, nvar);
	  }


	curr++;

	return 1;
}

static void
pgsql_cursor_statement_detect (GSQLCursor *cursor) {
	GSQL_TRACE_FUNC;
	
	gchar *stmt_char;
	gchar *affect = NULL;
	GSQLEPGSQLCursor  *e_cursor = NULL;
	
	e_cursor = cursor->spec;
	e_cursor->count = 0;
	cursor->stmt_affected_rows = 0;
	
	affect = PQcmdTuples(e_cursor->result);
	
	stmt_char = PQcmdStatus (e_cursor->result);
	GSQL_DEBUG("STMT [%s]", stmt_char);
	
	switch (0) {
		case 0:
			if (g_str_has_prefix (stmt_char, "SELECT")) {
				GSQL_DEBUG ("'select' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
				e_cursor->count = PQntuples(e_cursor->result);
				break;
			}
      
			if (g_str_has_prefix (stmt_char, "INSERT")) {
				GSQL_DEBUG ("'insert' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_INSERT;
				cursor->stmt_affected_rows = strtoull (affect,
								       NULL,
								       10);
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "UPDATE")) {
				GSQL_DEBUG ("'update' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_UPDATE;
				cursor->stmt_affected_rows = strtoull (affect, 
								       NULL,
								       10);
				break;
			}
      
			if (g_str_has_prefix (stmt_char, "DELETE")) {
				GSQL_DEBUG ("'delete' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DELETE;
				cursor->stmt_affected_rows = 
					PQcmdTuples(e_cursor->result);
				cursor->stmt_affected_rows = strtoull (affect,
								       NULL,
								       10);
				break;
			}
				
			if (g_str_has_prefix (stmt_char, "CREATE")) {
				GSQL_DEBUG ("'create' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_CREATE;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "DROP")) {
				GSQL_DEBUG ("'drop' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DROP;
				break;
			}
      
			if (g_str_has_prefix (stmt_char, "ALTER")) {
				GSQL_DEBUG ("'alter' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_ALTER;
				break;
			}
			
		default:
			GSQL_DEBUG ("default 'exec' statement");
			cursor->stmt_type = GSQL_CURSOR_STMT_EXEC;
	}
}
