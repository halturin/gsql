/***************************************************************************
 *            oracle_cursor.h
 *
 *  Thu Sep 20 22:26:36 2007
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
 
#ifndef _ORACLE_CURSOR_H
#define _ORACLE_CURSOR_H


typedef struct _GSQLEOracleCursor GSQLEOracleCursor;

#include <glib.h>
#include <libgsql/session.h>
#include <libgsql/cursor.h>
#include "oracle.h"




struct _GSQLEOracleCursor
{
	OCIStmt		*statement;
	OCIError 	*errhp;
	ub2			statement_type;
	gint		row_count;
	gint		row_num;
};

G_BEGIN_DECLS

GSQLCursorState 
oracle_cursor_open (GSQLCursor *cursor);

GSQLCursorState 
oracle_cursor_stop (GSQLCursor *cursor);

GSQLCursorState 
oracle_cursor_open_bind (GSQLCursor *cursor, GList *args);

GSQLCursorState 
oracle_cursor_open_bind_by_name (GSQLCursor *cursor, GList *args);

gint 
oracle_cursor_fetch (GSQLCursor *cursor, gint rows);

G_END_DECLS

#endif /* _ORACLE_CURSOR_H */

 
