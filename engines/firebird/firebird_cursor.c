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
#include <libgsql/session.h>
#include <ibase.h>
#include <string.h>

#include "engine_session.h"
#include "firebird_cursor.h"
#include "firebird_var.h"

#include <pthread.h>

typedef struct _GSQLEFirebirdCursor GSQLEFirebirdCursor;

struct _GSQLEFirebirdCursor {
	XSQLDA *out_sqlda;
	XSQLDA *in_sqlda;
	isc_stmt_handle stmt;	
};

static void firebird_cursor_statement_detect (GSQLCursor *cursor);

static void
on_cursor_close (GSQLCursor *cursor, gpointer user_data) {
	GSQL_TRACE_FUNC;
	GSQLEFirebirdCursor  *e_cursor;
	GSQLEFirebirdVariable *var;
	ISC_STATUS status_vector[20];
	
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	
	if (cursor->spec != NULL) {
		e_cursor = (GSQLEFirebirdCursor *)cursor->spec;		
		isc_dsql_free_statement (status_vector, &e_cursor->stmt,DSQL_drop);
		free (e_cursor->out_sqlda);
		free (e_cursor->in_sqlda);
		g_free (e_cursor);
	}
  	return;
}

static gboolean
firebird_cursor_prepare (GSQLCursor *cursor, gboolean with_binds) {
	GSQL_TRACE_FUNC;

	GSQLEFirebirdSession *e_session = NULL;
	GSQLEFirebirdCursor  *e_cursor = NULL;
	GSQLWorkspace *workspace = NULL;
	gchar error_str[2048];
	ISC_STATUS status_vector[20];
	e_session = cursor->session->spec;
	long SQLCODE = 0;
	short col_num = 0;

	workspace = gsql_session_get_workspace (cursor->session);
	g_return_if_fail(GSQL_IS_WORKSPACE(workspace));
	
	if (cursor->spec == NULL) {
		e_cursor = g_new0 (GSQLEFirebirdCursor, 1);
		e_cursor->out_sqlda = (XSQLDA *)malloc (XSQLDA_LENGTH(1));
		e_cursor->out_sqlda->version = 1;
		e_cursor->out_sqlda->sqln = 1;
		e_cursor->out_sqlda->sqld = 1;
		e_cursor->in_sqlda = (XSQLDA *)malloc (XSQLDA_LENGTH(1));
		e_cursor->in_sqlda->version = 1;
		e_cursor->in_sqlda->sqln = 1;
		e_cursor->in_sqlda->sqld = 1;
		e_cursor->stmt = NULL;
		isc_dsql_allocate_statement (status_vector,&e_session->dbhandle,&e_cursor->stmt);
		if (status_vector[0] == 1 && status_vector[1])
		{
			firebird_cursor_show_error (workspace, status_vector, FALSE);
			return FALSE;
		}
		isc_dsql_prepare (status_vector, &e_session->trhandle, &e_cursor->stmt, 0, cursor->sql, e_session->dialect, NULL);
		if (status_vector[0] == 1 && status_vector[1])
		{
			firebird_cursor_show_error (workspace, status_vector, TRUE);		
			return FALSE;
		}
		if (with_binds)
		{
			isc_dsql_describe_bind (status_vector, &e_cursor->stmt, e_session->dialect, e_cursor->in_sqlda);
			if (status_vector[0] == 1 && status_vector[1])
			{
				firebird_cursor_show_error (workspace, status_vector, TRUE);			
				return FALSE;
			}
			if (e_cursor->in_sqlda->sqln < (col_num = e_cursor->in_sqlda->sqld))
    		{
				free (e_cursor->in_sqlda);
				e_cursor->in_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(col_num));
        		e_cursor->in_sqlda->sqln = col_num;
        		e_cursor->in_sqlda->version = 1;
                   
        		isc_dsql_describe_bind (status_vector, &e_cursor->stmt, e_session->dialect, e_cursor->in_sqlda);
				if (status_vector[0] == 1 && status_vector[1])
				{
					firebird_cursor_show_error (workspace, status_vector, TRUE);
					return FALSE;
				}
			e_cursor->in_sqlda->sqlvar->sqllen = strlen((char *)e_cursor->in_sqlda->sqlvar->sqldata);
    		}
		}
		isc_dsql_describe (status_vector, &e_cursor->stmt, e_session->dialect, e_cursor->out_sqlda);
		if (status_vector[0] == 1 && status_vector[1])
		{
			firebird_cursor_show_error (workspace, status_vector, TRUE);			
			return FALSE;
		}
		if (e_cursor->out_sqlda->sqln < (col_num = e_cursor->out_sqlda->sqld))
    	{
			free (e_cursor->out_sqlda);
			e_cursor->out_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(col_num));
        	e_cursor->out_sqlda->sqln = col_num;
        	e_cursor->out_sqlda->version = 1;
                   
        	isc_dsql_describe (status_vector, &e_cursor->stmt, e_session->dialect, e_cursor->out_sqlda);
			if (status_vector[0] == 1 && status_vector[1])
			{
				firebird_cursor_show_error (workspace, status_vector, TRUE);
				return FALSE;
			}
    	}
		cursor->spec = e_cursor;
		g_signal_connect (G_OBJECT (cursor), "close", 
				  G_CALLBACK (on_cursor_close), NULL);
	};
	
	return TRUE;	
}

GSQLCursorState
firebird_cursor_open_bind (GSQLCursor *cursor, GList *args) {
	GSQL_TRACE_FUNC;
	GSQLEFirebirdSession *e_session = NULL;
	GSQLEFirebirdCursor  *e_cursor = NULL;
	GSQLEFirebirdVariable *e_var = NULL;
	GSQLVariable *var;
	GSQLWorkspace *workspace = NULL;
	Firebird_FIELD *field;
	gshort            length = 0, alignment = 0, type = 0, offset;
	XSQLVAR	*vary;
	gint* buffer = g_malloc0 (4096);	
	gint i = 0;
	ISC_STATUS status_vector[20];
	gulong binds_count,binds_arg, n, n_fields, is_null = 1;
	// store parameters information
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
  
	if (!firebird_cursor_prepare (cursor, TRUE)) {
		return GSQL_CURSOR_STATE_ERROR;
	}

	e_cursor = cursor->spec;
	for (vary = e_cursor->out_sqlda->sqlvar, offset = 0, i = 0; i < e_cursor->out_sqlda->sqld; vary++, i++)
    {
			
        length = alignment = vary->sqllen;
        type = vary->sqltype & ~1;

        if (type == SQL_TEXT)
            alignment = 1;
        else if (type == SQL_VARYING)
        {   
            length += sizeof (short) + 1;
            alignment = sizeof (short);
        }
        offset = FB_ALIGN(offset, alignment);
        vary->sqldata = (char *) buffer + offset;
        offset += length;
        offset = FB_ALIGN(offset, sizeof (short));
        vary->sqlind = (short*) ((char *) buffer + offset);
        offset += sizeof  (short);
    }
	binds_count = e_cursor->in_sqlda->sqln;
	binds_arg = g_list_length (args) / 2;

	if (binds_arg != binds_count)
	{
		GSQL_DEBUG ("Bind count is wrong. Need [%d]. Got [%f]", binds_count, binds_arg);

		return GSQL_CURSOR_STATE_ERROR;
	}

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
				e_cursor->in_sqlda->sqlvar[n].sqltype = SQL_TEXT;
				e_cursor->in_sqlda->sqlvar[n].sqldata = (char *) vlist->data;
				break;
				
			case G_TYPE_INT:
			case G_TYPE_UINT:
				e_cursor->in_sqlda->sqlvar[n].sqltype = SQL_LONG;
				e_cursor->in_sqlda->sqlvar[n].sqldata = (char *) vlist->data;
				break;
				
			case G_TYPE_UINT64:
			case G_TYPE_INT64:
				e_cursor->in_sqlda->sqlvar[n].sqltype = SQL_INT64;
				e_cursor->in_sqlda->sqlvar[n].sqldata = (char *) vlist->data;
				break;
	
			case G_TYPE_DOUBLE:
				e_cursor->in_sqlda->sqlvar[n].sqltype = SQL_DOUBLE;
				e_cursor->in_sqlda->sqlvar[n].sqldata = (char *) vlist->data;
				break;
		}

		vlist = g_list_next (vlist);
		n++;
	}
	isc_dsql_execute2 (status_vector, &e_session->trhandle, &e_cursor->stmt,
	                   e_session->dialect, e_cursor->in_sqlda, NULL);
	if (status_vector[0] == 1 && status_vector[1])
	{
		firebird_cursor_show_error (workspace, status_vector, FALSE);
		return GSQL_CURSOR_STATE_ERROR;		
	}
	
	firebird_cursor_statement_detect (cursor);
	n_fields = e_cursor->out_sqlda->sqld;
	if (n_fields == 0)
		return GSQL_CURSOR_STATE_OPEN;
		
	for (n = 0; n < n_fields; n++) {
		field = g_malloc0 ( sizeof(Firebird_FIELD) );
		if (e_cursor->out_sqlda->sqlvar[n].aliasname != NULL)
			field->name = e_cursor->out_sqlda->sqlvar[n].aliasname;
		else
			field->name = e_cursor->out_sqlda->sqlvar[n].sqlname;
		field->type = e_cursor->out_sqlda->sqlvar[n].sqltype;
		field->size = e_cursor->out_sqlda->sqlvar[n].sqllen;
		field->scale = e_cursor->out_sqlda->sqlvar[n].sqlscale;
		field->ind = e_cursor->out_sqlda->sqlvar[n].sqlind;
		field->subtype = e_cursor->out_sqlda->sqlvar[n].sqlsubtype;
		if ((field->type & ~1) == SQL_BLOB)
			e_cursor->out_sqlda->sqlvar[n].sqldata = (char *) &field->blob_id;
				
		var = gsql_variable_new ();
		firebird_variable_init (var, field);
		e_var = (GSQLEFirebirdVariable *) var->spec;

		e_var->workspace = workspace;
		e_var->dbhandle = e_session->dbhandle;
		e_var->trhandle = e_session->trhandle;
		cursor->var_list = g_list_append (cursor->var_list, var);
	}
	//g_free (buffer);
	return GSQL_CURSOR_STATE_OPEN;
}

GSQLCursorState
firebird_cursor_open (GSQLCursor *cursor) {
	GSQL_TRACE_FUNC;
	isc_db_handle *dbhandle;
	GSQLEFirebirdSession *e_session = NULL;
	GSQLEFirebirdCursor  *e_cursor = NULL;	
	GSQLVariable *var;
	GSQLEFirebirdVariable *e_var = NULL;
	GSQLWorkspace *workspace = NULL;
	Firebird_FIELD *field;
	gulong n, n_fields, is_null = 1;
	gchar error_str[2048];
	ISC_STATUS status_vector[20];	
	gshort            length = 0, alignment = 0, type = 0, offset;
	XSQLVAR	*vary;
	gint i = 0;
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	g_return_if_fail (GSQL_IS_SESSION(cursor->session));
	e_session = cursor->session->spec;
	gint* buffer = g_malloc0 (4096);
	workspace = gsql_session_get_workspace (cursor->session);
	g_return_if_fail(GSQL_IS_WORKSPACE(workspace));
	
	dbhandle = e_session->dbhandle;

	if (!firebird_cursor_prepare (cursor, FALSE)) {
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	e_cursor = cursor->spec;
	
	for (vary = e_cursor->out_sqlda->sqlvar, offset = 0, i = 0; i < e_cursor->out_sqlda->sqld; vary++, i++)
    {
			
        length = alignment = vary->sqllen;
        type = vary->sqltype & ~1;

        if (type == SQL_TEXT)
            alignment = 1;
        else if (type == SQL_VARYING)
        {   
            length += sizeof (short) + 1;
            alignment = sizeof (short);
        }
        offset = FB_ALIGN(offset, alignment);
        vary->sqldata = (char *) buffer + offset;
        offset += length;
        offset = FB_ALIGN(offset, sizeof (short));
        vary->sqlind = (short*) ((char *) buffer + offset);
        offset += sizeof  (short);
    }	
	
	isc_dsql_execute (status_vector, &e_session->trhandle, &e_cursor->stmt, e_session->dialect, NULL);
	if (status_vector[0] == 1 && status_vector[1])
	{
		firebird_cursor_show_error (workspace, status_vector, FALSE);
		return GSQL_CURSOR_STATE_ERROR;		
	}
	
	firebird_cursor_statement_detect (cursor);
	n_fields = e_cursor->out_sqlda->sqld;
	if (n_fields == 0)
		return GSQL_CURSOR_STATE_OPEN;
		
	for (n = 0; n < n_fields; n++) {
		field = g_malloc0 ( sizeof(Firebird_FIELD) );
		if (e_cursor->out_sqlda->sqlvar[n].aliasname != NULL)
			field->name = e_cursor->out_sqlda->sqlvar[n].aliasname;
		else
			field->name = e_cursor->out_sqlda->sqlvar[n].sqlname;
		field->type = e_cursor->out_sqlda->sqlvar[n].sqltype;
		field->size = e_cursor->out_sqlda->sqlvar[n].sqllen;
		field->scale = e_cursor->out_sqlda->sqlvar[n].sqlscale;
		field->ind = e_cursor->out_sqlda->sqlvar[n].sqlind;
		field->subtype = e_cursor->out_sqlda->sqlvar[n].sqlsubtype;
		if ((field->type & ~1) == SQL_BLOB)
			e_cursor->out_sqlda->sqlvar[n].sqldata = (char *) &field->blob_id;
				
		var = gsql_variable_new ();
		firebird_variable_init (var, field);
		e_var = (GSQLEFirebirdVariable *) var->spec;

		e_var->workspace = workspace;
		e_var->dbhandle = e_session->dbhandle;
		e_var->trhandle = e_session->trhandle;
		cursor->var_list = g_list_append (cursor->var_list, var);
	}
	//g_free (buffer);
	return GSQL_CURSOR_STATE_OPEN;
}

gint
firebird_cursor_fetch (GSQLCursor *cursor, gint rows) {
	GSQL_TRACE_FUNC;

	static int curr = 0;
	int nvar=0, nfields=0;
	GSQLEFirebirdCursor *e_cursor = NULL;
	GSQLVariable *var = NULL;
	GSQLEFirebirdSession *e_session = NULL;
	GSQLEFirebirdVariable *fbvar = NULL;	
	GSQLWorkspace *workspace = NULL;
	gchar error_str[2048];
	gint ret = 1;
	GList *vlist = cursor->var_list;
	ISC_STATUS status_vector[20];
	glong SQLCODE = 0;
	long fetch_stat = 0, blob_stat = 0;

	
	g_return_if_fail (GSQL_CURSOR (cursor) != NULL);
	g_return_if_fail (GSQL_IS_SESSION(cursor->session));	
	e_cursor = (GSQLEFirebirdCursor *) cursor->spec;
	e_session = cursor->session->spec;

	workspace = gsql_session_get_workspace (cursor->session);
	g_return_if_fail(GSQL_IS_WORKSPACE(workspace));
  
	nfields = e_cursor->out_sqlda->sqld;


	
	if ((fetch_stat = isc_dsql_fetch (status_vector,
		                &e_cursor->stmt,
		                e_session->dialect, e_cursor->out_sqlda)) == 0) {
		for ( vlist = g_list_first(vlist), nvar=0; 
	      	vlist && (nvar < nfields); 
	      	vlist = g_list_next(vlist), nvar++ ) 
			{
				var = (GSQLVariable *)vlist->data;
				var->value = e_cursor->out_sqlda->sqlvar[nvar].sqldata;
    	 	}
							
		curr++;							
		return 1;
	}

	else if (fetch_stat == 100L)
		{GSQL_FIXME; return 0;}
	else if (fetch_stat != 100L){
		firebird_cursor_show_error (workspace, status_vector, TRUE);
		return -1;
	}
}

static void
firebird_cursor_statement_detect (GSQLCursor *cursor) {
	GSQL_TRACE_FUNC;
	gchar count_item[] = { isc_info_sql_records, isc_info_end };
	gchar type_item[] = { isc_info_sql_stmt_type };
	char affect_buffer[33], type_buffer[8], *p, item;
	gint affect = 0;
	gint length = 0;
	gint statement_type;
	ISC_STATUS status_vector[20];
	GSQLEFirebirdCursor  *e_cursor = NULL;
	GSQLWorkspace *workspace = NULL;
	
	e_cursor = cursor->spec;
	workspace = gsql_session_get_workspace (cursor->session);
	cursor->stmt_affected_rows = 0;


	isc_dsql_sql_info (status_vector, &e_cursor->stmt,
	                   sizeof (count_item), count_item,
	                   sizeof (affect_buffer), affect_buffer);

	if (status_vector[0] == 1 && status_vector[1])
	{
		firebird_cursor_show_error (workspace, status_vector, FALSE);		
	}
	
	if (affect_buffer[0] == isc_info_sql_records)
	{
		p=affect_buffer+3;
		while (*p != isc_info_end) {
			int count = 0;
			char item=*p++;
			short len = (short)isc_vax_integer(p,2);
			p+=2;
			count=isc_vax_integer(p,len);
			switch(item) {
		  		case isc_info_req_insert_count:
		  		case isc_info_req_update_count:
		  		case isc_info_req_delete_count:
					affect += count;
					break;
			}


		p += len;
		}			
	}
		
	isc_dsql_sql_info (status_vector, &e_cursor->stmt, 
                   sizeof (type_item), type_item,
                   sizeof (type_buffer), type_buffer);

	if (status_vector[0] == 1 && status_vector[1])
	{
		firebird_cursor_show_error (workspace, status_vector, FALSE);	
	}
	
	if (type_buffer[0] == isc_info_sql_stmt_type)
	{
		length = isc_vax_integer ((char *)type_buffer + 1, 2);
		statement_type = isc_vax_integer ((char *)type_buffer + 3, length);
	}

	switch (statement_type) {
		case isc_info_sql_stmt_select:
				GSQL_DEBUG ("'select' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
				break;

		case isc_info_sql_stmt_insert:
				GSQL_DEBUG ("'insert' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_INSERT;
				cursor->stmt_affected_rows = affect;
				break;
			
		case isc_info_sql_stmt_update:
				GSQL_DEBUG ("'update' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_UPDATE;
				cursor->stmt_affected_rows = affect;
				break;
			
		case isc_info_sql_stmt_delete:
				GSQL_DEBUG ("'delete' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DELETE;
				cursor->stmt_affected_rows = affect;
				break;	
	      
		case isc_info_sql_stmt_ddl:
				GSQL_DEBUG ("'ddl' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DDL;
				cursor->stmt_affected_rows = affect;
				break;

		case isc_info_sql_stmt_get_segment:
				/*In processing;*/
				break;
			
		case isc_info_sql_stmt_put_segment:
				/*In processing*/
				break;

		case isc_info_sql_stmt_exec_procedure:
				GSQL_DEBUG ("'exec' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_EXEC;
				break;

		case isc_info_sql_stmt_start_trans:
				/*In processing*/
				break;

		case isc_info_sql_stmt_commit:
				/*In processing*/
				break;

		case isc_info_sql_stmt_rollback:
				/*In processing*/
				break;

		case isc_info_sql_stmt_select_for_upd:
				/*In processing*/
				break;

		default:
			GSQL_DEBUG ("default 'exec' statement");
			cursor->stmt_type = GSQL_CURSOR_STMT_EXEC;
			break;
	}
}