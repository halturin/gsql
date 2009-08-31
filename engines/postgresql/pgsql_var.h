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

 
#ifndef PGSQL_VAR_H
#define PGSQL_VAR_H

#include <libpq-fe.h>
#include <c.h>
#include <catalog/pg_type.h>

#include <libgsql/cvariable.h>

#include "pgsql_cursor.h"

typedef struct _GSQLEPGSQLVariable  GSQLEPGSQLVariable;

typedef struct _PGSQL_FIELD PGSQL_FIELD;

struct _PGSQL_FIELD {
  char *name;
  int type;
  char *value;
  int size;
};

/* typedef struct _PGSQL_BIND PGSQL_BIND; */

/* struct _PGSQL_BIND { */
/*   Oid paramType; */
/*   char *paramValue; */
/*   int paramLength; */
/* }; */

struct _GSQLEPGSQLVariable 
{
  //PGSQL_BIND *bind;
  PGSQL_FIELD *field;
	
  //my_bool is_null;
  //my_bool err;
  //gulong  length;
};


G_BEGIN_DECLS

/* gboolean */
/* pgsql_variable_init (GSQLVariable *variable, PGSQL_FIELD *field, PGSQL_BIND *bind); */

void
pgsql_variable_free (GSQLEPGSQLVariable *var);

void
pgsql_variable_clear (GSQLCursor *cursor);

/*void *
pgsql_variable_data_to_display_format (GSQLEPGSQLVariable *var);
*/

/*
GSQLEPGSQLVariable * 
pgsql_cursor_get_variable (GSQLCursor *cursor, gint column);
*/

G_END_DECLS

#endif /* PGSQL_VAR_H */
