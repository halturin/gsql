/***************************************************************************
 *            mysql_var.h
 *
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
 
#ifndef MYSQL_VAR_H
#define MYSQL_VAR_H

#include <glib.h>
#include "mysql_cursor.h"
#include <libgsql/cvariable.h>

typedef struct _GSQLEMySQLVariable  GSQLEMySQLVariable;

struct _GSQLEMySQLVariable 
{
	MYSQL_BIND *bind;
	MYSQL_FIELD *field;
	
	my_bool is_null;
	my_bool err;
	gulong  length;
};


G_BEGIN_DECLS

gboolean
mysql_variable_init (GSQLVariable *variable, MYSQL_FIELD *field, MYSQL_BIND *bind);

void
mysql_variable_free (GSQLEMySQLVariable *var);

void
mysql_variable_clear (GSQLCursor *cursor);

/*void *
mysql_variable_data_to_display_format (GSQLEMySQLVariable *var);
*/

/*
GSQLEMySQLVariable * 
mysql_cursor_get_variable (GSQLCursor *cursor, gint column);
*/

G_END_DECLS

#endif /* MYSQL_VAR_H */

