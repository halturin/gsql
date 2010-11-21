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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

 
#ifndef _NAV_SQL_H
#define _NAV_SQL_H

#include <glib.h>
#include "nav_objects.h"

/************ Root objects *************/

static const gchar sql_firebird_users[] =
"select distinct RDB$USER\
        from RDB$USER_PRIVILEGES";

static const gchar sql_firebird_generators[] =
"execute block returns (gen_name char(31), gen_value int) \
as \
  declare variable n varchar(32); \
  declare variable n2 varchar(32); \
  declare variable v int; \
begin \
  for \
    select rdb$generator_name from rdb$generators where rdb$system_flag = 0 or rdb$system_flag is null into :n \
  do \
  begin \
    for \
      execute statement 'select rdb$generator_name, gen_id(' || :n ||', 0)  from rdb$generators where rdb$system_flag = 0 or rdb$system_flag is null' into :n2, :v \
      do \
begin \
        gen_name = :n; \
        gen_value = :v; \
      end \
      suspend; \
   end \
end";

static const gchar sql_firebird_exceptions[] =
"select rdb$exception_name, RDB$MESSAGE \
from rdb$exceptions \
where ((rdb$system_flag = 0) or (rdb$system_flag is null))";

static const gchar sql_mysql_privileges[] =
"select privilege_type, grantee, table_catalog, table_schema, is_grantable \
from information_schema.SCHEMA_PRIVILEGES \
where table_schema = ?";

static const gchar sql_firebird_tables[] = 
"SELECT\
 t.rdb$relation_name as \"table_name\", \
 t.rdb$field_id as \"field_count\", \
 t.rdb$owner_name as \"owner_name\" \
from rdb$relations t \
where (t.RDB$SYSTEM_FLAG = 0 \
or t.RDB$SYSTEM_FLAG is null) \
and t.RDB$VIEW_SOURCE is null";

static const gchar sql_firebird_views[] = 
"SELECT \
t.rdb$relation_name as table_name, \
t.rdb$field_id as field_count, \
t.rdb$owner_name as owner_name \
from rdb$relations t \
where (t.RDB$SYSTEM_FLAG = 0 \
or t.RDB$SYSTEM_FLAG is null) \
and t.RDB$VIEW_SOURCE is not null";

static const gchar sql_firebird_table_columns[] =
"select r.rdb$field_name, r.rdb$null_flag, r.rdb$field_source, t.rdb$type_name, \
         l.rdb$collation_name,f.rdb$field_length \
         from rdb$fields f\
		 join RDB$TYPES t \
			 on (f.RDB$FIELD_TYPE = t.RDB$TYPE) \
         join rdb$relation_fields r \
             on f.rdb$field_name=r.rdb$field_source \
         left outer join rdb$collations l \
             on l.rdb$collation_id = r.rdb$collation_id \
             and l.rdb$character_set_id = f.rdb$character_set_id \
         where r.rdb$relation_name = ? and \
		 t.RDB$FIELD_NAME = 'RDB$FIELD_TYPE' \
         order by r.rdb$field_position";

static const gchar sql_firebird_all_columns[] =
"select distinct r.rdb$field_source,r.rdb$null_flag, t.rdb$type_name, \
         l.rdb$collation_name,f.rdb$field_length \
		 from rdb$fields f \
		 join RDB$TYPES t \
			 on (f.RDB$FIELD_TYPE = t.RDB$TYPE) \
         join rdb$relation_fields r \
             on f.rdb$field_name=r.rdb$field_source \
         left outer join rdb$collations l \
             on l.rdb$collation_id = r.rdb$collation_id \
             and l.rdb$character_set_id = f.rdb$character_set_id \
         where not (r.rdb$field_source starting with 'RDB$' OR r.rdb$field_source starting with 'MON$')  \
		 and t.RDB$FIELD_NAME = 'RDB$FIELD_TYPE' \
         order by r.rdb$field_position";

static const gchar sql_firebird_table_constraints[]=
"select RDB$CONSTRAINT_NAME, RDB$CONSTRAINT_TYPE, RDB$INDEX_NAME \
from rdb$relation_constraints \
where rdb$relation_name = ?";

static const gchar sql_firebird_all_constraints[]=
"select RDB$CONSTRAINT_NAME, RDB$CONSTRAINT_TYPE, RDB$INDEX_NAME \
from rdb$relation_constraints";

static const gchar sql_firebird_table_indexes[] =
"select RDB$INDEX_NAME, RDB$UNIQUE_FLAG, RDB$INDEX_INACTIVE, RDB$STATISTICS \
from rdb$indices \
where rdb$relation_name = ?";

static const gchar sql_firebird_all_indexes[] =
"select RDB$INDEX_NAME, RDB$UNIQUE_FLAG, RDB$INDEX_INACTIVE, RDB$STATISTICS \
from rdb$indices";

static const gchar sql_firebird_index_columns[]=
"select RDB$FIELD_NAME, RDB$FIELD_POSITION \
from rdb$index_segments where rdb$index_name = ? \
order by rdb$field_position;";

static const gchar sql_firebird_table_triggers[] =
"SELECT RDB$TRIGGER_NAME AS TRIGGER_NAME, \
RDB$TRIGGER_SEQUENCE AS TRIGGER_SEQUENCE, \
RDB$TRIGGER_TYPE AS TRIGGER_TYPE, RDB$TRIGGER_INACTIVE AS TRIGGER_INACTIVE \
from RDB$TRIGGERS \
where ((RDB$SYSTEM_FLAG = 0) or (RDB$SYSTEM_FLAG is NULL)) and \
(RDB$TRIGGER_NAME not in \
(select RDB$TRIGGER_NAME from RDB$CHECK_CONSTRAINTS) and \
(RDB$RELATION_NAME = ?))";

static const gchar sql_firebird_all_triggers[] =
"SELECT RDB$TRIGGER_NAME AS TRIGGER_NAME, RDB$RELATION_NAME AS RELATION_NAME,\
RDB$TRIGGER_SEQUENCE AS TRIGGER_SEQUENCE, \
RDB$TRIGGER_TYPE AS TRIGGER_TYPE, RDB$TRIGGER_INACTIVE AS TRIGGER_INACTIVE \
from RDB$TRIGGERS \
where ((RDB$SYSTEM_FLAG = 0) or (RDB$SYSTEM_FLAG is NULL)) and \
(RDB$TRIGGER_NAME not in\
(select RDB$TRIGGER_NAME from RDB$CHECK_CONSTRAINTS))";

static const gchar sql_firebird_procedures[] =
"select RDB$PROCEDURE_NAME, RDB$PROCEDURE_INPUTS, RDB$PROCEDURE_OUTPUTS, \
RDB$PROCEDURE_SOURCE, RDB$OWNER_NAME \
from rdb$procedures";

static const gchar sql_firebird_functions[] =
"select RDB$FUNCTION_NAME, RDB$MODULE_NAME, \
RDB$ENTRYPOINT, RDB$RETURN_ARGUMENT \
from rdb$functions";


static const gchar sql_firebird_proc_in_arguments[] =
"select a.RDB$PARAMETER_NAME, a.RDB$PARAMETER_NUMBER, \
b.RDB$VALIDATION_SOURCE, b.RDB$COMPUTED_SOURCE, \
b.RDB$DEFAULT_SOURCE, b.RDB$FIELD_LENGTH, \
b.RDB$FIELD_SCALE, b.RDB$FIELD_TYPE, b.RDB$FIELD_SUB_TYPE, b.RDB$NULL_FLAG \
from RDB$PROCEDURE_PARAMETERS a, RDB$FIELDS b \
where (a.RDB$FIELD_SOURCE=b.RDB$FIELD_NAME) and \
(a.RDB$PROCEDURE_NAME = ?) and (a.RDB$PARAMETER_TYPE = 0) \
order by a.RDB$PARAMETER_TYPE, a.RDB$PARAMETER_NUMBER";

static const gchar sql_firebird_proc_out_arguments[] =
"select a.RDB$PARAMETER_NAME, a.RDB$PARAMETER_NUMBER, \
b.RDB$VALIDATION_SOURCE, b.RDB$COMPUTED_SOURCE, \
b.RDB$DEFAULT_SOURCE, b.RDB$FIELD_LENGTH, \
b.RDB$FIELD_SCALE, b.RDB$FIELD_TYPE, b.RDB$FIELD_SUB_TYPE, b.RDB$NULL_FLAG \
from RDB$PROCEDURE_PARAMETERS a, RDB$FIELDS b \
where (a.RDB$FIELD_SOURCE=b.RDB$FIELD_NAME) and \
(a.RDB$PROCEDURE_NAME = ?) and (a.RDB$PARAMETER_TYPE = 1) \
order by a.RDB$PARAMETER_TYPE, a.RDB$PARAMETER_NUMBER";

static const gchar sql_firebird_fun_in_arguments[] =
"SELECT b.RDB$ARGUMENT_POSITION || ' - ' || trim(c.RDB$TYPE_NAME) || ' (' || b.RDB$FIELD_LENGTH || ')' as argument, \
b.RDB$FIELD_TYPE, b.RDB$ARGUMENT_POSITION, b.RDB$MECHANISM, \
b.RDB$FIELD_SCALE, b.RDB$FIELD_LENGTH, b.RDB$FIELD_SUB_TYPE \
FROM RDB$FUNCTIONS a, RDB$FUNCTION_ARGUMENTS b, RDB$TYPES c \
WHERE (a.RDB$FUNCTION_NAME = b.RDB$FUNCTION_NAME) \
and (a.RDB$FUNCTION_NAME = ? ) \
and (b.RDB$ARGUMENT_POSITION<>a.RDB$RETURN_ARGUMENT) \
and (b.RDB$FIELD_TYPE = c.RDB$TYPE) \
and (c.RDB$FIELD_NAME = 'RDB$FIELD_TYPE')";

static const gchar sql_firebird_fun_out_arguments[] =
"SELECT  b.RDB$ARGUMENT_POSITION || ' - ' || trim(c.RDB$TYPE_NAME) || ' (' || b.RDB$FIELD_LENGTH || ')' as argument, \
b.RDB$FIELD_TYPE, b.RDB$ARGUMENT_POSITION, b.RDB$MECHANISM, \
b.RDB$FIELD_SCALE, b.RDB$FIELD_LENGTH, b.RDB$FIELD_SUB_TYPE \
FROM RDB$FUNCTIONS a, RDB$FUNCTION_ARGUMENTS b, RDB$TYPES c \
WHERE (a.RDB$FUNCTION_NAME = b.RDB$FUNCTION_NAME) \
and (a.RDB$FUNCTION_NAME = ? ) \
and (b.RDB$ARGUMENT_POSITION = a.RDB$RETURN_ARGUMENT) \
and (b.RDB$FIELD_TYPE = c.RDB$TYPE) \
and (c.RDB$FIELD_NAME = 'RDB$FIELD_TYPE')";


#endif /* _NAV_SQL_H */

 
