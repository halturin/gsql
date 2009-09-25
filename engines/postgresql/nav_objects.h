
/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2009  Estêvão Samuel Procópio <tevaum@gmail.com>
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

 
#ifndef _NAV_OBJECTS_H
#define _NAV_OBJECTS_H

#include <glib.h>

enum {
  PG_SCHEMA_ID  = NAV_TREE_ROOT_ID + 1,

  DATABASES_ID,
  DATABASE_ID,

  SCHEMAS_ID,
  LANGUAGES_ID,
	
  USERS_ID,
  USER_ID,

  TABLES_ID,
  TABLE_ID,

  COLUMNS_ID,
  COLUMN_ID,

  CONSTRAINTS_ID,
  CONSTRAINT_ID,
	
  VIEWS_ID,
  VIEW_ID,

  TYPES_ID,
  TYPE_ID,
	
  INDEXES_ID,
  INDEX_ID,
	
  TRIGGERS_ID,
  TRIGGER_ID,
	
  PROCEDURES_ID,
  PROCEDURE_ID,
  
  FUNCTIONS_ID,
  FUNCTION_ID,
  
  PACKAGES_ID,
  PACKAGE_ID,
  
  SEQUENCES_ID,
  SEQUENCE_ID,
  
  PRIVILEGES_ID,
  PRIVILEGE_ID,
  
  PROCESSES_ID,
  PROCESS_ID,

  PROCESS_LIST_ID,
  
  VARIABLES_ID,

  GLOBAL_VARIABLES_ID,
  GLOBAL_VARIABLE_ID,
  
  SESSION_VARIABLES_ID,
  SESSION_VARIABLE_ID,
  
  UNKNOWN_OBJECT_ID
};

G_BEGIN_DECLS

void pgsql_navigation_fill_details (GSQLCursor *cursor, GtkListStore *store);

G_END_DECLS

#endif /* _NAV_OBJECTS_H */
