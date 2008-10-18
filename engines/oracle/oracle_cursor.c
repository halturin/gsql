/***************************************************************************
 *            oracle_cursor.c
 *
 *  Thu Sep 20 22:26:23 2007
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

#include <stdarg.h>
#include <string.h>
#include <libgsql/common.h>
#include <libgsql/session.h>
#include <libgsql/workspace.h>
#include <libgsql/utils.h>
#include "engine_session.h"
#include "oracle.h"
#include "oracle_cursor.h"
#include "oracle_var.h"

static gboolean
oracle_sql_exec(GSQLCursor *cursor);

static gboolean 
oracle_sql_prepare (GSQLCursor * cursor, gchar * sql);

static void
on_cursor_close (GSQLCursor *cursor, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQLEOracleCursor  *spec;
	GSQLEOracleVariable *var;
	sword ret;
	
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	
	if (cursor->spec != NULL)
	{
		spec = (GSQLEOracleCursor *)cursor->spec;
		
		ret = OCIStmtRelease(spec->statement,
							spec->errhp, NULL, 0, OCI_DEFAULT);
		
		OCIHandleFree ((dvoid *)spec->errhp, OCI_HTYPE_ERROR);
		
	}
	
}

GSQLCursorState 
oracle_cursor_open (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	GSQLEOracleCursor	*spec_cursor;
	GSQLEOracleSession	*spec_session;
	GSQLEOracleVariable *spec_var;
	GSQLVariable		*var;
	gint var_count;
	sword ret;
	OCIParam *param;
	unsigned char op[2000];
	gint i;
	gchar buffer[GSQL_MESSAGE_LEN];
    
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = g_malloc0 (sizeof (GSQLEOracleCursor));	
	cursor->spec = spec_cursor;
	
	ret = OCIHandleAlloc ((dvoid *)(spec_session->envhp), 
					   (dvoid **)&(spec_cursor->errhp),
						OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	if (ret == OCI_ERROR)
	{

		GSQL_DEBUG("oracle_cursor_open: OCIHandleAlloc (allocate an error handle)... failed");
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}

	if ((!oracle_sql_prepare (cursor, cursor->sql)) || 
		(!oracle_sql_exec (cursor)))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	ret = OCIAttrGet (spec_cursor->statement, OCI_HTYPE_STMT, (dvoid*) &(var_count), 0,
						OCI_ATTR_PARAM_COUNT, spec_cursor->errhp);
	
	if (oracle_check_error (cursor, ret))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	};
		
	
	for (i = 0; i < var_count; i++)
	{

		ret = OCIParamGet (spec_cursor->statement, OCI_HTYPE_STMT,
						spec_cursor->errhp, (void**) &param, i+1);
                
 		if (oracle_check_error (cursor, ret))
		{
			 g_free (spec_cursor);
			 return GSQL_CURSOR_STATE_ERROR;
		};

		var = gsql_variable_new ();
		oracle_variable_init (cursor, var, param, i+1);
		cursor->var_list = g_list_append (cursor->var_list, var);
		OCIDescriptorFree (param, OCI_DTYPE_PARAM);

	}
	
	
	
	return GSQL_CURSOR_STATE_OPEN;
};

GSQLCursorState
oracle_cursor_open_bind (GSQLCursor *cursor, GList *args)
{
	GSQL_TRACE_FUNC;

	GSQLEOracleCursor	*spec_cursor;
	GSQLEOracleSession	*spec_session;
	GSQLEOracleVariable *spec_var;
	GSQLVariable		*var;
	GList *vlist = args;
	GType vtype;
	guint  n, var_count=0;
	sword ret;
	OCIParam *param;
	static OCIBind *bindhp = 0;
	unsigned char op[2000];
	gint i;
	gchar buffer[GSQL_MESSAGE_LEN];
    
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = g_malloc0 (sizeof (GSQLEOracleCursor));	
	cursor->spec = spec_cursor;
	
	ret = OCIHandleAlloc ((dvoid *)(spec_session->envhp), 
					   (dvoid **)&(spec_cursor->errhp),
						OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	if (ret == OCI_ERROR)
	{

		GSQL_DEBUG("oracle_cursor_open: OCIHandleAlloc (allocate an error handle)... failed");
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}

	if (!oracle_sql_prepare (cursor, cursor->sql))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}

	n = 0;
	
	while (vlist)
	{
		vtype = (GType) vlist->data;
		vlist = g_list_next (vlist);
		
		switch (vtype)
		{
			case G_TYPE_STRING:
			case G_TYPE_POINTER:
				ret = OCIBindByPos (spec_cursor->statement, &bindhp, spec_cursor->errhp, (ub4) n+1,
							(dvoid *) vlist->data, (sb4)strlen(vlist->data)+1, (ub2) SQLT_STR,
							(dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
							(ub4) OCI_DEFAULT);
				
				break;
			
			case G_TYPE_INT:
			case G_TYPE_UINT:
				GSQL_FIXME;
				
				break;
			
			case G_TYPE_UINT64:
			case G_TYPE_INT64:
				GSQL_FIXME;
			
				break;
			
			case G_TYPE_DOUBLE:
				GSQL_FIXME;
				
				break;
				
		};
		vlist = g_list_next (vlist);
		n++;
	}

	
	if (!oracle_sql_exec (cursor))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	ret = OCIAttrGet (spec_cursor->statement, OCI_HTYPE_STMT, (dvoid*) &(var_count), 0,
						OCI_ATTR_PARAM_COUNT, spec_cursor->errhp);
	
	if (oracle_check_error (cursor, ret))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}
			
	for (i = 0; i < var_count; i++)
	{

		ret = OCIParamGet (spec_cursor->statement, OCI_HTYPE_STMT,
						spec_cursor->errhp, (void**) &param, i+1);
                
 		if (oracle_check_error (cursor, ret))
		{
			 g_free (spec_cursor);
			 return GSQL_CURSOR_STATE_ERROR;
		};

		var = gsql_variable_new ();
		oracle_variable_init (cursor, var, param, i+1);
		cursor->var_list = g_list_append (cursor->var_list, var);
		OCIDescriptorFree (param, OCI_DTYPE_PARAM);

	}
		
	return GSQL_CURSOR_STATE_OPEN;
}

GSQLCursorState 
oracle_cursor_open_bind_by_name (GSQLCursor *cursor, GList *args)
{
	GSQL_TRACE_FUNC;

	GSQLEOracleCursor	*spec_cursor;
	GSQLEOracleSession	*spec_session;
	GSQLEOracleVariable *spec_var;
	GSQLVariable		*var;
	GList *vlist = args;
	GList *vlist_value;
	GType vtype;
	guint  n, var_count=0;
	sword ret;
	OCIParam *param;
	static OCIBind *bindhp = 0;
	unsigned char op[2000];
	gint i;
	gchar *holder;
	gchar buffer[GSQL_MESSAGE_LEN];
    
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = g_malloc0 (sizeof (GSQLEOracleCursor));	
	cursor->spec = spec_cursor;
	
	ret = OCIHandleAlloc ((dvoid *)(spec_session->envhp), 
					   (dvoid **)&(spec_cursor->errhp),
						OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	
	if (ret == OCI_ERROR)
	{

		GSQL_DEBUG("oracle_cursor_open: OCIHandleAlloc (allocate an error handle)... failed");
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}

	if (!oracle_sql_prepare (cursor, cursor->sql))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	n = 0;
	
	while (vlist)
	{
		vtype = (GType) vlist->data;
		
		/* bind by name means first item are name of bind point - it is always are string (gchar *) */
		if (vtype != G_TYPE_STRING)
		{
			GSQL_DEBUG ("Wrong GSQL_CURSOR_BIND_BY_NAME usage");
			return GSQL_CURSOR_STATE_ERROR;
		}
		
		vlist = g_list_next (vlist);
		holder = g_strdup ( (gchar *) vlist->data);
		
		vlist = g_list_next (vlist);
		vtype = (GType) vlist->data;
		vlist = g_list_next (vlist);
		
		switch (vtype)
		{
			case G_TYPE_STRING:
			case G_TYPE_POINTER:
			
				ret = OCIBindByName (spec_cursor->statement, &bindhp, spec_cursor->errhp,
							(CONST text *) holder,-1,
							(dvoid *) vlist->data, (sb4)strlen(vlist->data)+1, (ub2) SQLT_STR,
							(dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
							(ub4) OCI_DEFAULT);
				GSQL_DEBUG ("Bind by name: [\'%s\' = %s] [ret = %d]", holder, vlist->data, ret);
				if (oracle_check_error (cursor, ret))
				{
					GSQL_DEBUG ("bind error");
				}
				
				break;
			
			case G_TYPE_INT:
			case G_TYPE_UINT:
				GSQL_FIXME;
				
				break;
			
			case G_TYPE_UINT64:
			case G_TYPE_INT64:
				GSQL_FIXME;
			
				break;
			
			case G_TYPE_DOUBLE:
				GSQL_FIXME;
				
				break;
				
		}
		
		g_free (holder);
		
		vlist = g_list_next (vlist);
		n++;
	}

	
	if (!oracle_sql_exec (cursor))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	ret = OCIAttrGet (spec_cursor->statement, OCI_HTYPE_STMT, (dvoid*) &(var_count), 0,
						OCI_ATTR_PARAM_COUNT, spec_cursor->errhp);
	
	if (oracle_check_error (cursor, ret))
	{
		
		OCIHandleFree ((dvoid *)spec_cursor->errhp, OCI_HTYPE_ERROR);
		cursor->spec = NULL;
		g_free (spec_cursor);
		return GSQL_CURSOR_STATE_ERROR;
	}
			
	for (i = 0; i < var_count; i++)
	{

		ret = OCIParamGet (spec_cursor->statement, OCI_HTYPE_STMT,
						spec_cursor->errhp, (void**) &param, i+1);
                
 		if (oracle_check_error (cursor, ret))
		{
			 g_free (spec_cursor);
			 return GSQL_CURSOR_STATE_ERROR;
		};

		var = gsql_variable_new ();
		oracle_variable_init (cursor, var, param, i+1);
		cursor->var_list = g_list_append (cursor->var_list, var);
		OCIDescriptorFree (param, OCI_DTYPE_PARAM);

	}
		
	return GSQL_CURSOR_STATE_OPEN;

}


gint 
oracle_cursor_fetch(GSQLCursor * cursor, gint rows)
{
	GSQL_TRACE_FUNC;

	sword ret;
	gint fetched;
	GSQLEOracleCursor *cursor_spec;
	

	g_return_if_fail (GSQL_IS_CURSOR (cursor));
	
	cursor_spec = cursor->spec;
	oracle_variable_clear (cursor);
	ret = OCIStmtFetch2 (cursor_spec->statement,
						 cursor_spec->errhp,
						 1,
						 OCI_FETCH_NEXT,
						 (sb4) 0,
						 OCI_DEFAULT);
	if (oracle_check_error (cursor, ret))
		return 0;

	fetched = 0;

	ret = OCIAttrGet (cursor_spec->statement, OCI_HTYPE_STMT,
						(void *) &fetched, (uint *) 0, 
						OCI_ATTR_ROWS_FETCHED, cursor_spec->errhp);
	GSQL_DEBUG ("ORACLE Fetched [%d]", fetched);
	return fetched;
}


static gboolean
oracle_sql_prepare (GSQLCursor *cursor, gchar *sql)
{
	GSQL_TRACE_FUNC;

	sword ret;
	unsigned char op[2000];
	GSQLEOracleSession *spec_session;
	GSQLEOracleCursor  *spec_cursor;
    
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), FALSE);
	g_return_val_if_fail (GSQL_IS_SESSION (cursor->session), FALSE);
	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = (GSQLEOracleCursor *) cursor->spec;
	
	ret = OCIStmtPrepare2(spec_session->svchp, 
							&(spec_cursor->statement), 
							spec_cursor->errhp, 
							(text *) sql, 
							strlen(sql),
							NULL, 0,
							OCI_NTV_SYNTAX,
							OCI_DEFAULT);

	if ( oracle_check_error (cursor, ret))
			 return FALSE;
	
	OCIAttrGet(spec_cursor->statement, OCI_HTYPE_STMT,
				(dvoid*) &(spec_cursor->statement_type), 0, OCI_ATTR_STMT_TYPE,
				spec_cursor->errhp);

	g_signal_connect (G_OBJECT (cursor), "close", G_CALLBACK (on_cursor_close), NULL);

	return TRUE;
}

static gchar *
oracle_get_affected_message (GSQLEOracleCursor *cursor, gdouble elapsed);

static void
oracle_dbms_output (GSQLCursor *cursor);

static gboolean
oracle_sql_exec(GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	sword ret;
	int num_iters = 0;
	unsigned char mess[2000];
	GSQLEOracleSession *spec_session;
	GSQLEOracleCursor  *spec_cursor;
	gint param;

	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = (GSQLEOracleCursor *) cursor->spec;
    
	/* FIXME. 
	iters (IN)
		For non-SELECT statements, the number of times this statement is executed is equal to
		iters - rowoff.
		For SELECT statements, if iters is nonzero, then defines must have been done for the
		statement handle. The execution fetches iters rows into these predefined buffers and
		prefetches more rows depending upon the prefetch row count. If you do not know
		how many rows the SELECT statement will retrieve, set iters to zero.
		This function returns an error if iters=0 for non-SELECT statements.

		Note: For array DML operations, set iters <= 32767 to get better performance.
	*/

	num_iters = (spec_cursor->statement_type != OCI_STMT_SELECT) ? 1: 0;
	
	if ((cursor->scrollable) && (num_iters))
		param = OCI_STMT_SCROLLABLE_READONLY;
	else
		param = OCI_DEFAULT;


	ret = OCIStmtExecute(spec_session->svchp, spec_cursor->statement,
							spec_cursor->errhp, num_iters, 0, 0, 0, 
							param);

	
	if (oracle_check_error (cursor, ret))
	{
		 return FALSE;
	}
	
	OCIAttrGet(spec_cursor->statement, OCI_HTYPE_STMT,
				(dvoid*) &(spec_cursor->row_count), 0, OCI_ATTR_ROW_COUNT,
				spec_cursor->errhp);
	
	 switch (spec_cursor->statement_type)
	{
		case OCI_STMT_UPDATE:
			cursor->stmt_type = GSQL_CURSOR_STMT_UPDATE;
			cursor->stmt_affected_rows = spec_cursor->row_count;
			break;
			
		case OCI_STMT_DELETE:
			cursor->stmt_type = GSQL_CURSOR_STMT_DELETE;
			cursor->stmt_affected_rows = spec_cursor->row_count;
			break;
		
		case OCI_STMT_INSERT:
			cursor->stmt_type = GSQL_CURSOR_STMT_INSERT;
			cursor->stmt_affected_rows = spec_cursor->row_count;
			break;

		case OCI_STMT_SELECT:
			cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
			cursor->stmt_affected_rows = 0;
			break;
		
		case OCI_STMT_CREATE:
			cursor->stmt_type = GSQL_CURSOR_STMT_CREATE;
			cursor->stmt_affected_rows = 0;
			break;
			
		case OCI_STMT_DROP:
			cursor->stmt_type = GSQL_CURSOR_STMT_DROP;
			cursor->stmt_affected_rows = 0;
			break;
			
		case OCI_STMT_ALTER:
			cursor->stmt_type = GSQL_CURSOR_STMT_ALTER;
			cursor->stmt_affected_rows = 0;
			break;
			
		case OCI_STMT_BEGIN:
		case OCI_STMT_DECLARE:
			cursor->stmt_type = GSQL_CURSOR_STMT_EXEC;
			cursor->stmt_affected_rows = 0;
			break;
			
		default:
			GSQL_DEBUG ("Oracle engine: internal error occured. Unknown statement type.");
			return FALSE;
			
	}

	if (spec_session->dbms_output)
		oracle_dbms_output (cursor);

	return TRUE;
};



static void
oracle_dbms_output (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	GSQLEOracleSession *session;
	GSQLWorkspace *workspace;
	OCIBind *bind_p1 = NULL, *bind_p2 = NULL;
	OCIStmt *sth = NULL;
	sb2 ind = OCI_IND_NULL;
	OCIError *errhp;
	const gchar *sql = "begin dbms_output.get_line (line=>:p1, status=>:p2); end;";
	gchar *mess = NULL;

// FIXME: use gconf
#define DBMS_OUTPUT_BUFFER 2048
	gchar p1_value[DBMS_OUTPUT_BUFFER];
	gint p2_value = 0, ret;
	
	session = (GSQLEOracleSession *) cursor->session->spec;
	workspace = gsql_session_get_workspace (cursor->session);
	
	OCIHandleAlloc ((dvoid *)(session->envhp), 
					   (dvoid **)&(errhp),
						OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	
	OCIStmtPrepare2(session->svchp, 
							&sth, 
							errhp, 
							(text *) sql, 
							strlen(sql),
							NULL, 0,
							OCI_NTV_SYNTAX,
							OCI_DEFAULT);
	
	OCIBindByPos (sth, &bind_p1, errhp, (ub4) 1,
							(dvoid *) p1_value, (sb4) DBMS_OUTPUT_BUFFER,
				  			(ub2) SQLT_STR,
							(dvoid *) &ind, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
							(ub4) OCI_DEFAULT);
	
	OCIBindByPos (sth, &bind_p2, errhp, (ub4) 2,
							(dvoid *) &p2_value, (sb4) sizeof(p2_value),
				  			(ub2) SQLT_INT,
							(dvoid *) &ind, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
							(ub4) OCI_DEFAULT);
	
	while (p2_value != 1)
	{
		ret = OCIStmtExecute(session->svchp, sth,
							errhp, 1, 0, 0, 0, OCI_DEFAULT);
		
		if (ret != OCI_SUCCESS)
		{
			GSQL_DEBUG ("Couldn't get dbms output: %s", 
						oracle_get_error_string (errhp));
			break;
		}
		
		if (g_utf8_strlen (p1_value, DBMS_OUTPUT_BUFFER) > 0)
		{
			mess = gsql_utils_escape_string (p1_value);
			GSQL_DEBUG ("DBMS_OUTPUT: %s", mess);
			gsql_message_add (workspace, GSQL_MESSAGE_OUTPUT, mess);
			g_free (mess);
		}
	}
	
	OCIHandleFree((dvoid *)sth, (ub4)OCI_HTYPE_STMT);
	OCIHandleFree((dvoid *)errhp, (ub4)OCI_HTYPE_ERROR);

}

