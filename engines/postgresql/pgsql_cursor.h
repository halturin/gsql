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

 
#ifndef PGSQL_CURSOR_H
#define PGSQL_CURSOR_H

#include <glib.h>
#include <libgsql/cursor.h>
#include <libpq-fe.h>

typedef struct _GSQLEPGSQLCursor GSQLEPGSQLCursor;

struct _GSQLEPGSQLCursor
{
  //PGSQL_STMT  *statement;
  //PGSQL_BIND  *binds;
  PGresult   *result;
  int count;
};

G_BEGIN_DECLS

GSQLCursorState
pgsql_cursor_open_bind (GSQLCursor *cursor, GList *args);

GSQLCursorState
pgsql_cursor_open (GSQLCursor *cursor);

gint
pgsql_cursor_fetch (GSQLCursor *cursor, gint rows);

G_END_DECLS

#endif /* PGSQL_CURSOR_H */
