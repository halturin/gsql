/***************************************************************************
 *            engine_menucb.c
 *
 *  Wed Sep  5 02:24:17 2007
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

#include <libgsql/common.h>
#include <libgsql/session.h>
#include <engine_menucb.h>

void
on_charter_set_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GSQLCursor *cursor;
	gchar sql[255];
	gchar *charset = data;
	
	session = gsql_session_get_active ();
	
	if (!GSQL_IS_SESSION (session))
		return;
	
	workspace = gsql_session_get_workspace (session);
	
	memset (sql, 0, 255);
	g_snprintf (sql, 255, "SET CHARACTER SET %s", data);
	
	cursor = gsql_cursor_new (session, sql);
	gsql_cursor_open (cursor, FALSE);
	
	if (gsql_cursor_get_state (cursor) != GSQL_CURSOR_STATE_OPEN)
	{
		gsql_cursor_close (cursor);
		return;
	}
	
	memset (sql, 0, 255);
	g_snprintf (sql, 255, N_("Character set was changed to '%s'"), data);
	
	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, sql);
	
	gsql_cursor_close (cursor);
	
}
