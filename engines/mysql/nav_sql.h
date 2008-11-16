/***************************************************************************
 *            nav_sql.h
 *
 *  Tue Sep 18 00:38:43 2007
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
 
#ifndef _NAV_SQL_H
#define _NAV_SQL_H

#include <glib.h>
#include "nav_objects.h"

/************ Root objects *************/

static const gchar sql_mysql_all_schemas[] = 
"SELECT SCHEMA_NAME, CATALOG_NAME, DEFAULT_CHARACTER_SET_NAME, \
 DEFAULT_COLLATION_NAME, SQL_PATH \
FROM information_schema.schemata order by schema_name";

static const gchar sql_mysql_users[] =
"select replace(GRANTEE, \"'\",\"\") disp, GRANTEE \
 from information_schema.USER_PRIVILEGES \
 group by replace(GRANTEE, \"'\",\"\"), GRANTEE";

static const gchar sql_mysql_privileges[] =
"select privilege_type, grantee, table_catalog, table_schema, is_grantable \
from information_schema.SCHEMA_PRIVILEGES \
where table_schema = ?";

static const gchar sql_mysql_processes[]=
"show full PROCESSLIST";

static const gchar sql_mysql_session_variables[]=
"show session variables";

static const gchar sql_mysql_global_variables[]=
"show global variables";

/*********** User's objects *************/
static const gchar sql_mysql_tables[] = 
"select table_name, table_catalog, table_schema, table_type, engine, version, \
 row_format, table_rows, avg_row_length, data_length, max_data_length, \
 index_length, data_free, auto_increment, create_time, update_time, check_time,\
 table_collation, checksum, create_options, table_comment \
 from information_schema.tables where table_schema = ? \
 and table_type <>'VIEW'";

static const gchar sql_mysql_table_columns[] =
"select column_name, table_catalog, table_schema, table_name, ordinal_position, \
column_default, is_nullable, data_type, character_maximum_length, \
character_octet_length, numeric_precision, numeric_scale, character_set_name, \
collation_name, column_type, column_key, extra, privileges, column_comment \
from information_schema.columns where table_schema = ? and table_name = ? ";

static const gchar sql_mysql_constraints[]=
"select concat(constraint_name,'(',table_schema,'.',table_name,')') cname, \
 substr(constraint_type, 1, 1) c_type, \
 constraint_catalog, constraint_schema, \
 table_schema, table_name, constraint_type \
 from information_schema.TABLE_CONSTRAINTS \
 where table_schema = ? \
 and table_name like ?";

static const gchar sql_mysql_views[] =
"select table_name, table_schema, table_catalog, view_definition, \
check_option, is_updatable, definer, security_type from information_schema.views \
where TABLE_SCHEMA = ?";

static const gchar sql_mysql_indexes[] =
"select concat(index_name,'(',table_schema,'.',table_name,')') c_name, \
 index_name, TABLE_NAME, INDEX_TYPE \
 from information_schema.statistics  \
 where table_schema = ? \
 and table_name like ? \
 group by index_name, TABLE_NAME";

static const gchar sql_mysql_index_columns[]=
"select COLUMN_NAME, TABLE_CATALOG, TABLE_SCHEMA, TABLE_NAME, \
NON_UNIQUE, INDEX_SCHEMA, INDEX_NAME, SEQ_IN_INDEX, \
COLLATION, CARDINALITY, SUB_PART, PACKED, NULLABLE, \
INDEX_TYPE, COMMENT \
 from information_schema.statistics  \
where index_schema = ? \
and concat(index_name,'(',index_schema,'.',table_name,')') = ? \
order by seq_in_index";
 
static const gchar sql_mysql_triggers[] =
"select TRIGGER_NAME, TRIGGER_CATALOG, TRIGGER_SCHEMA, EVENT_MANIPULATION, \
 EVENT_OBJECT_CATALOG, EVENT_OBJECT_SCHEMA, EVENT_OBJECT_TABLE, \
 ACTION_ORDER, ACTION_CONDITION, ACTION_ORIENTATION, ACTION_TIMING, \
 ACTION_REFERENCE_OLD_TABLE, ACTION_REFERENCE_NEW_TABLE, \
 ACTION_REFERENCE_OLD_ROW, ACTION_REFERENCE_NEW_ROW, \
 CREATED, SQL_MODE, DEFINER \
 from information_schema.triggers where EVENT_OBJECT_SCHEMA = ?\
 and EVENT_OBJECT_TABLE like ?";
 
static const gchar sql_mysql_routines[] =
"select * from information_schema.routines where routine_type = ? \
 and ROUTINE_SCHEMA = ?";


#endif /* _NAV_SQL_H */

 
