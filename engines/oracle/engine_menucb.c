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



#include <engine_menucb.h>
#include <libgsql/session.h>
#include <libgsql/common.h>
#include <libgsql/workspace.h>

#include "oracle.h"
#include "engine_session.h"

void
on_server_output (GtkToggleAction *ta, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLCursor *cursor;
	GSQLSession *session;
	GSQLEOracleSession *spec;
	GSQLWorkspace *workspace;
	gboolean status = gtk_toggle_action_get_active (ta);
	static const gchar *sql_enable = "begin \n"
						"sys.dbms_output.enable; \n"
						"end;";
	static const gchar *sql_disable = "begin \n"
						"sys.dbms_output.disable; \n"
						"end;";
	gchar *sql;
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	spec = (GSQLEOracleSession *) session->spec;
	
	if (status)
	{
		GSQL_DEBUG ("set DBMS_OUTPUT enable");
		sql = (gchar *) sql_enable;
		gsql_message_add (workspace, GSQL_MESSAGE_OUTPUT, "DBMS_OUTPUT enable");
	} else {
		GSQL_DEBUG ("set DBMS_OUTPUT disable");
		sql = (gchar *) sql_disable;
		gsql_message_add (workspace, GSQL_MESSAGE_OUTPUT, "DBMS_OUTPUT disable");
	}
	
	spec->dbms_output = status;
	
	cursor = gsql_cursor_new (session, sql);
	gsql_cursor_open (cursor, FALSE);
	gsql_cursor_close (cursor);
}
 
void
on_empty_recycle_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	GSQLCursor *cursor;
	gchar *sql = "purge recyclebin";
	
	session = gsql_session_get_active ();
	
	cursor = gsql_cursor_new (session, sql);
	
	gsql_cursor_open (cursor, FALSE);
	
	gsql_cursor_close (cursor);

}
 
void
on_job_manager_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;
}
 
void
on_find_code_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;
}
 
void
on_find_object_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;
}

