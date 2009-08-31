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

 
#ifndef _NAV_SQL_H
#define _NAV_SQL_H

#include <glib.h>
#include "nav_objects.h"

/************ Root objects *************/

static const gchar sql_pgsql_all_databases[] = 
"SELECT d.datname as database_name, \
u.usename as database_owner, \
pg_catalog.pg_encoding_to_char(d.encoding) as database_charset \
FROM pg_catalog.pg_database d \
LEFT JOIN pg_catalog.pg_user u ON d.datdba = u.usesysid \
ORDER BY d.datname";

static const gchar sql_pgsql_all_schemas[] = 
"SELECT distinct t.table_schema as schema_name, s.catalog_name, s.schema_owner, \
s.default_character_set_schema, s.default_character_set_name, s.sql_path \
from information_schema.tables t \
left outer join information_schema.schemata s \
on s.schema_name = t.table_schema order by t.table_schema";

static const gchar sql_pgsql_users[] =
"select \
usename, usecreatedb, usesuper, usecatupd \
from pg_user";

static const gchar sql_pgsql_privileges[] =
"select grentee || ' ' || privilege_type || ' ' || table_name || ' ' || grantee as privilege_name, privilege_type, grantee, table_catalog, table_schema, table_name, is_grantable \
from information_schema.table_privileges \
where table_schema = $1 \
order by table_name, privilege_type, grantee";

static const gchar sql_pgsql_processes[]=
"select \
procpid, usename, replace(replace(current_query, '<', ''), '>', '') as current_query, query_start, backend_start, client_addr, client_port \
from pg_stat_activity";

static const gchar sql_pgsql_session_variables[]=
"show session variables";

static const gchar sql_pgsql_global_variables[]=
"show global variables";

/*********** User's objects *************/
static const gchar sql_pgsql_tables[] =
"select table_name, table_catalog, table_schema, table_type \
 from information_schema.tables where table_schema not in ('information_schema', 'pg_catalog') \
 and table_type <>'VIEW' and table_schema = $1 order by table_name";

static const gchar sql_pgsql_table_columns[] = 
"select column_name, table_catalog, table_schema, table_name, ordinal_position, \
 column_default, is_nullable, data_type, character_maximum_length, \
 character_octet_length, numeric_precision, numeric_scale, character_set_name, \
 collation_name \
 from information_schema.columns where table_schema = $1 and table_name = $2 \
 order by ordinal_position";

static const gchar sql_pgsql_indexes[] = 
"select \
c.relname as index_name, \
n.nspname as table_schema, \
case c.relkind when 'r' then 'table' when 'v' then 'view' \
when 'i' then 'index' when 's' then 'sequence' when 's' \
then 'special' end as type, \
u.usename as owner, \
c2.relname as table_name \
from pg_catalog.pg_class c \
join pg_catalog.pg_index i on i.indexrelid = c.oid	\
join pg_catalog.pg_class c2 on i.indrelid = c2.oid \
left join pg_catalog.pg_user u on u.usesysid = c.relowner \
left join pg_catalog.pg_namespace n on n.oid = c.relnamespace \
where c.relkind in ('i','') \
and n.nspname not in ('pg_catalog', 'pg_toast') \
and pg_catalog.pg_table_is_visible(c.oid) \
and n.nspname = $1 \
and c2.relname ilike $2 \
order by c2.relname, c.relname";

static const gchar sql_pgsql_index_columns[]=
"  select column_name, table_catalog, c.table_schema, c.table_name, \
  ordinal_position, column_default, is_nullable, data_type, \
  character_maximum_length, character_octet_length, numeric_precision, \
  numeric_scale, character_set_name, collation_name \
from information_schema.columns c \
  join ( \
	select \
	c1.relname as index_name, n.nspname as table_schema, \
	case c1.relkind \
	when 'r' then 'table' \
	when 'v' then 'view' \
	when 'i' then 'index' \
	when 's' then 'sequence' \
	end as type, \
	u.usename as owner, c2.relname as table_name, \
	att.attname \
	from pg_catalog.pg_class c1 \
	join pg_catalog.pg_index i \
	on i.indexrelid = c1.oid \
	join pg_catalog.pg_class c2 \
	on i.indrelid = c2.oid \
	left join pg_catalog.pg_user u \
	on u.usesysid = c1.relowner \
	left join pg_catalog.pg_namespace n \
	on n.oid = c1.relnamespace \
	left join pg_attribute att \
	on c1.relfilenode = att.attrelid \
	where c1.relkind in ('i','') \
	and n.nspname not in ('pg_catalog', 'pg_toast') \
	and pg_catalog.pg_table_is_visible(c1.oid) \
	and att.attrelid in ( \
			     select relfilenode \
			     from pg_class c \
			     where relname = $2 \
			     and relkind in ( 'i', '' ) \
			     ) \
	) cc \
on c.column_name = cc.attname \
and c.table_name = cc.table_name \
order by ordinal_position";

static const gchar sql_pgsql_triggers[] =
"select \
trigger_name, trigger_catalog, trigger_schema, event_manipulation, \
event_object_catalog, event_object_schema, event_object_table, \
action_order, action_condition, action_orientation, created \
from information_schema.triggers where event_object_schema = $1 \
and event_object_table ilike $2";

static const gchar sql_pgsql_constraints[]=
"select constraint_name, constraint_catalog, constraint_schema, \
table_catalog, table_schema, table_name, constraint_type, \
is_deferrable, initially_deferred \
from information_schema.table_constraints \
where table_schema = $1 \
and table_name ilike $2";

static const gchar sql_pgsql_views[] =
"select table_name, table_schema, table_catalog, view_definition, \
check_option, is_updatable, is_insertable_into from information_schema.views \
where table_schema = $1";

static const gchar sql_pgsql_routines[] =
"select routine_name, * from information_schema.routines where routine_type = $1 \
 and ROUTINE_SCHEMA = $2";


#endif /* _NAV_SQL_H */

 
