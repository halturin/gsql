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
"SELECT SCHEMA_NAME FROM information_schema.schemata order by schema_name";

static const gchar sql_mysql_users[] =
"select  replace(GRANTEE, '\'','') disp, GRANTEE \
 from information_schema.USER_PRIVILEGES group by replace(GRANTEE, '\'',''), GRANTEE";

static const gchar sql_mysql_privileges[] =
"select * from  information_schema.USER_PRIVILEGES where grantee = \"'root'@'think'\"";

static const gchar sql_mysql_processes[]=
"show full PROCESSLIST";

/*********** User's objects *************/
static const gchar sql_mysql_tables[] = 
"select table_name, table_catalog, table_schema, table_type, engine, version, \
 row_format, table_rows, avg_row_length, data_length, max_data_length, \
 index_length, data_free, auto_increment, create_time, update_time, check_time,\
 table_collation, checksum, create_options, table_comment \
 from information_schema.tables where table_schema = ? ";

static const gchar sql_mysql_table_columns[] =
"select column_name, table_catalog, table_schema, table_name, ordinal_position, \
column_default, is_nullable, data_type, character_maximum_length, \
character_octet_length, numeric_precision, numeric_scale, character_set_name, \
collation_name, column_type, column_key, extra, privileges, column_comment \
from information_schema.columns where table_schema = ? and table_name = ? ";

static const gchar sql_mysql_constraints[]=
"select * from information_schema.TABLE_CONSTRAINTS where CONSTRAINT_SCHEMA = :schema";

static const gchar sql_mysql_views[] =
"select * from information_schema.views where TABLE_SCHEMA = :schema";

static const gchar sql_mysql_indexes[] =
"select index_name, TABLE_NAME, INDEX_TYPE from information_schema.statistics  \
 where table_schema = :schema \
 group by index_name, TABLE_NAME";
 
static const gchar sql_mysql_triggers[] =
"select * from information_schema.triggers where TRIGGER_SCHEMA = :schema";
 
static const gchar sql_mysql_routines[] =
"select * from information_schema.routines where routine_type = 'PROCEDURE' and ROUTINE_SCHEMA =:schema";


#endif /* _NAV_SQL_H */

 
