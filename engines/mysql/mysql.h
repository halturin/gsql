/***************************************************************************
 *            mysql.h
 *
 *  Sun Mar 30 17:18:58 2008
 *  Copyright  2008  Taras Halturin
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
 
#ifndef MYSQL_H
#define MYSQL_H

#include <glib.h>
#include "engine_session.h"

G_BEGIN_DECLS

gboolean
mysql_session_open (GSQLSession *session, gint mode, gchar *buffer);

gboolean
mysql_session_close (GSQLSession *session, gchar *buffer);

void
mysql_session_commit (GSQLSession *session);

void
mysql_session_rollback (GSQLSession *session);

gchar *
mysql_session_get_error (GSQLSession *session);


G_END_DECLS

#endif /* MYSQL_H */

