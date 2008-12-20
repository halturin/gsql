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

 
#ifndef _ORACLE_H
#define _ORACLE_H

#include <glib.h>
#include <libgsql/session.h>
#include <libgsql/cursor.h>
#include <oci.h>
#include "engine_session.h"
#include "oracle_cursor.h"


G_BEGIN_DECLS

gboolean
oracle_session_open (GSQLEOracleSession *oracle_session, 
					 gchar *username,
					 gchar *password,
					 gchar *database, 
					 gchar *buffer);
void 
oracle_session_reopen (GSQLSession *session);

gboolean
oracle_session_close (GSQLSession *session, gchar *buffer);


void
oracle_session_commit (GSQLSession *session);

void
oracle_session_rollback (GSQLSession *session);

gboolean 
oracle_check_error (GSQLCursor *cursor, gint ret);

gchar *
oracle_get_error_string (OCIError *error);


G_END_DECLS

#endif /* _ORACLE_H */

 
