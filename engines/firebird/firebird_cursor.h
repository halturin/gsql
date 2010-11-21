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

 
#ifndef PGSQL_CURSOR_H
#define PGSQL_CURSOR_H

#include <glib.h>
#include <libgsql/cursor.h>

//typedef PARAMVARY VARY2;

G_BEGIN_DECLS

GSQLCursorState
firebird_cursor_open_bind (GSQLCursor *cursor, GList *args);

GSQLCursorState
firebird_cursor_open (GSQLCursor *cursor);

gint
firebird_cursor_fetch (GSQLCursor *cursor, gint rows);

G_END_DECLS

#endif  
