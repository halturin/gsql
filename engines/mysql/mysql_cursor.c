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

 
#include <glib.h>
#include <libgsql/session.h>
#include <libgsql/utils.h>
#include "mysql_cursor.h"
#include "engine_session.h"
#include "mysql_var.h"
#include <pthread.h>

static void mysql_cursor_statement_detect (GSQLCursor *cursor);


static void
on_cursor_close (GSQLCursor *cursor, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GSQLEMySQLCursor  *e_cursor;
	GSQLEMySQLVariable *var;
	gint i;
	
	g_return_if_fail (GSQL_IS_CURSOR(cursor));
	
	if (cursor->spec != NULL)
	{
		e_cursor = (GSQLEMySQLCursor *)cursor->spec;
		
		if (e_cursor->statement)
		{
			mysql_stmt_free_result(e_cursor->statement);
			mysql_stmt_close(e_cursor->statement);
		}
		
		g_free (e_cursor->binds);
		g_free (e_cursor);
	}
	
}

static gboolean
mysql_cursor_prepare (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	GSQLEMySQLSession *e_session = NULL;
	GSQLEMySQLCursor  *e_cursor = NULL;
	GSQLWorkspace *workspace = NULL;
	gchar error_str[2048];
	MYSQL_STMT *stmt;

	e_session = cursor->session->spec;
	
	if (cursor->spec == NULL)
	{
		stmt = mysql_stmt_init (e_session->mysql);

		if (!stmt)
		{
			g_debug ("Couldn't initiate a statement [mysql_stmt_init]");
			return FALSE;
		}
		e_cursor = g_new0 (GSQLEMySQLCursor, 1);
		e_cursor->statement = stmt;
		
		cursor->spec = e_cursor;
		g_signal_connect (G_OBJECT (cursor), "close", G_CALLBACK (on_cursor_close), NULL);
	}

	
	if (mysql_stmt_prepare(e_cursor->statement, cursor->sql, g_utf8_strlen(cursor->sql, 1048576)))
	{
		g_snprintf (error_str, 2048, "Prepare failed: %s", 
		            gsql_utils_escape_string (mysql_stmt_error (e_cursor->statement)));

		workspace = gsql_session_get_workspace (cursor->session);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);

		return FALSE;
	}

	return TRUE;	
}

GSQLCursorState
mysql_cursor_open_bind (GSQLCursor *cursor, GList *args)
{
	GSQL_TRACE_FUNC;
	MYSQL *mysql;
	GSQLEMySQLSession *e_session = NULL;
	GSQLEMySQLCursor  *e_cursor = NULL;
	GSQLEMySQLVariable *e_var;
	GSQLVariable *var;
	GSQLWorkspace *workspace = NULL;
	MYSQL_BIND *binds;
	MYSQL_FIELD *fields;
	gulong binds_count = 0, binds_arg, n, n_fields, is_null = 1;
	gulong str_len;
	GList *vlist = args;
	GType vtype;
	gdouble affect = 0;
	gchar error_str[2048];
	
	e_session = cursor->session->spec;
	workspace = gsql_session_get_workspace (cursor->session);
	
	mysql = e_session->mysql;
	
	if (!mysql_cursor_prepare (cursor))
	{
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	e_cursor = cursor->spec;
	
	binds_count = mysql_stmt_param_count(e_cursor->statement);
	binds_arg = g_list_length (args) / 2;
	
	if (binds_arg != binds_count)
	{
		GSQL_DEBUG ("Bind count is wrong. Need [%d]. Got [%f]", binds_count, binds_arg);
		mysql_stmt_reset (e_cursor->statement);

		return GSQL_CURSOR_STATE_ERROR;
	}

	binds = g_new0 (MYSQL_BIND, binds_count);
	n = 0;

	while (vlist)
	{
		vtype = (GType) vlist->data;
		vlist = g_list_next (vlist);
		if (vlist->data == NULL)
			is_null = 1;
		else 
			is_null = 0;
		switch (vtype)
		{
			case G_TYPE_STRING:
			case G_TYPE_POINTER:
				binds[n].buffer_type = MYSQL_TYPE_STRING;
				binds[n].buffer = (char *) vlist->data;
				binds[n].buffer_length = g_utf8_strlen((gchar *) vlist->data, 1048576);
				binds[n].is_null= 0;
				binds[n].length = NULL;
				break;
			
			case G_TYPE_INT:
			case G_TYPE_UINT:
				binds[n].buffer_type = MYSQL_TYPE_LONG;
				binds[n].buffer = (char *) &vlist->data;
				binds[n].is_null= (my_bool*) &is_null;
				break;
			
			case G_TYPE_UINT64:
			case G_TYPE_INT64:
				binds[n].buffer_type = MYSQL_TYPE_LONGLONG;
				binds[n].buffer = (char *) &vlist->data;
				binds[n].is_null= (my_bool*) &is_null;
				break;
			
			case G_TYPE_DOUBLE:
				binds[n].buffer_type = MYSQL_TYPE_DOUBLE;
				binds[n].buffer = (char *) &vlist->data;
				binds[n].is_null= (my_bool*) &is_null;
				break;
				
		}
		
		vlist = g_list_next (vlist);
		n++;
	}

	if ((mysql_stmt_bind_param (e_cursor->statement, binds)) ||
		(!(e_cursor->result = mysql_stmt_result_metadata(e_cursor->statement))) ||
		(mysql_stmt_execute(e_cursor->statement)) )
		
	{
		g_sprintf (error_str, "Error occured: %s", mysql_stmt_error (e_cursor->statement));
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		g_free (binds);
		mysql_stmt_reset (e_cursor->statement);

		return GSQL_CURSOR_STATE_ERROR;		
	}
	
	mysql_cursor_statement_detect (cursor);
	
	g_free (binds);
	
	n_fields =  mysql_field_count (mysql);
	
	if (n_fields == 0)
		return GSQL_CURSOR_STATE_OPEN;
	
	fields = e_cursor->statement->fields;
	binds = g_new0 (MYSQL_BIND, n_fields);
	
	e_cursor->binds = binds;
	
	for (n = 0; n < n_fields; n++)
	{
		GSQL_DEBUG ("field[%d] = %s", n, fields[n].name);
		var = gsql_variable_new ();
		mysql_variable_init (var, &fields[n], &binds[n]);
		cursor->var_list = g_list_append (cursor->var_list, var);
	}
	
	if (mysql_stmt_bind_result (e_cursor->statement, binds))
	{
		g_sprintf (error_str, "Error occured: %s", mysql_stmt_error (e_cursor->statement));
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		g_free (binds);
		mysql_stmt_reset (e_cursor->statement);

		return GSQL_CURSOR_STATE_ERROR;
	}
	
	return GSQL_CURSOR_STATE_OPEN;
}


GSQLCursorState
mysql_cursor_open (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
	MYSQL *mysql;
	GSQLEMySQLSession *e_session = NULL;
	GSQLEMySQLCursor  *e_cursor = NULL;
	GSQLEMySQLVariable *e_var;
	GSQLVariable *var;
	GSQLWorkspace *workspace = NULL;
	MYSQL_BIND *binds;
	MYSQL_FIELD *fields;
	gulong n, n_fields, is_null = 1;
	gchar error_str[2048];
	
	
	e_session = cursor->session->spec;
	workspace = gsql_session_get_workspace (cursor->session);
	
	mysql = e_session->mysql;
	
	if (!mysql_cursor_prepare (cursor))
	{
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	e_cursor = cursor->spec;
	
	e_cursor->result = mysql_stmt_result_metadata(e_cursor->statement);

	if (mysql_stmt_execute(e_cursor->statement)) 
	{
		g_sprintf (error_str, "Error occured: %s", mysql_stmt_error (e_cursor->statement));
		GSQL_DEBUG (error_str);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		mysql_stmt_reset (e_cursor->statement);

		return GSQL_CURSOR_STATE_ERROR;		
	}
	
	mysql_cursor_statement_detect (cursor);
	
	n_fields =  mysql_field_count (mysql);	
	fields = e_cursor->statement->fields;
	
	if (n_fields == 0)
		return GSQL_CURSOR_STATE_OPEN;
	
	binds = g_new0 (MYSQL_BIND, n_fields);
	e_cursor->binds = binds;
	
	for (n = 0; n < n_fields; n++)
	{
		GSQL_DEBUG ("field[%d] = %s", n, fields[n].name);
		var = gsql_variable_new ();
		mysql_variable_init (var, &fields[n], &binds[n]);
		cursor->var_list = g_list_append (cursor->var_list, var);
	}
	
	
	
	if (mysql_stmt_bind_result (e_cursor->statement, binds))
	{
		g_sprintf (error_str, "Error occured: %s", mysql_stmt_error (e_cursor->statement));
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
		g_free (binds);
		mysql_stmt_reset (e_cursor->statement);

		return GSQL_CURSOR_STATE_ERROR;
	}

	return GSQL_CURSOR_STATE_OPEN;
}


gint
mysql_cursor_fetch (GSQLCursor *cursor, gint rows)
{
	GSQL_TRACE_FUNC;
	
	GSQLEMySQLCursor  *e_cursor = NULL;
	gchar error_str[2048];
	gint ret = 1;
	
	g_return_if_fail (GSQL_CURSOR (cursor) != NULL);
	e_cursor = cursor->spec;
	
	mysql_variable_clear (cursor);
	
	if (ret = mysql_stmt_fetch (e_cursor->statement))
	{
		switch (ret)
		{
			case MYSQL_NO_DATA:
				return 0;
			case MYSQL_DATA_TRUNCATED:
				gsql_message_add (gsql_session_get_workspace (cursor->session), GSQL_MESSAGE_WARNING,
								  N_("Data truncated. It is mean that internal error occured. Please, report this bug."));
				return 0;
			default:
				g_sprintf (error_str, "Error occured: [%d]%s", ret, mysql_stmt_error (e_cursor->statement));
				gsql_message_add (gsql_session_get_workspace (cursor->session), GSQL_MESSAGE_WARNING,
								  error_str);
				return -1;
		}
	} 
	
	return 1;
}

static void
mysql_cursor_statement_detect (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
	
	gchar *stmt_char;
	gdouble affect = 0;
	GSQLEMySQLCursor  *e_cursor = NULL;
	
	
	/*  FIXME
		How to recognize the type of the statement?
		Does any body know?  it is realy defective!! 
	*/
	
	e_cursor = cursor->spec;
	
	affect = mysql_stmt_affected_rows(e_cursor->statement);
	
	stmt_char = g_ascii_strdown (cursor->sql, 10);
	
	switch (0)
	{
		case 0:
			if (g_str_has_prefix (stmt_char, "select"))
			{
				GSQL_DEBUG ("'select' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "show"))
			{
				GSQL_DEBUG ("'show' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "desc"))
			{
				GSQL_DEBUG ("'desc' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_SELECT;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "insert"))
			{
				GSQL_DEBUG ("'insert' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_INSERT;
				cursor->stmt_affected_rows = affect;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "update"))
			{
				GSQL_DEBUG ("'update' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_UPDATE;
				cursor->stmt_affected_rows = affect;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "delete"))
			{
				GSQL_DEBUG ("'delete' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DELETE;
				cursor->stmt_affected_rows = affect;
				break;
			}
				
			if (g_str_has_prefix (stmt_char, "create"))
			{
				GSQL_DEBUG ("'create' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_CREATE;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "drop"))
			{
				GSQL_DEBUG ("'drop' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_DROP;
				break;
			}
			
			if (g_str_has_prefix (stmt_char, "alter"))
			{
				GSQL_DEBUG ("'alter' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_ALTER;
				break;
			}
			
		default:
				GSQL_DEBUG ("default 'exec' statement");
				cursor->stmt_type = GSQL_CURSOR_STMT_EXEC;
	}
	
	
}

