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

static const gchar sql_oracle_all_schemas[] = 
"select username \
from sys.all_users \
where exists (select 'x' from sys.all_objects where owner=username) or username = user \
order by username";

static const gchar sql_oracle_all_users[] = 
"select username,user_id,created,null default_tablespace, \
null temporary_tablespace,to_date(null) lock_date, \
to_date(null) expiry_date,null external_name,null account_status, \
null initial_rsrc_consumer_group,null profile,null password \
from all_users where username like '%' order by username";


/************ Privileges *************/

static const gchar sql_oracle_session_privileges[] =
"select privilege from session_privs where privilege like :name order by privilege";

static const gchar sql_oracle_system_privileges[] =
"select privilege,decode(admin_option,'YES',1,0) admin_option \
from sys.role_sys_privs \
where role = :name order by privilege";

static const gchar sql_oracle_object_privileges[] =
"select t.privilege||' on '||o.object_type||' '||t.table_schema||'.'||t.table_name privilege, \
t.grantor,t.table_schema owner,t.table_name, \
t.grantable, o.object_type \
from all_tab_privs t, all_objects o \
where grantee = :name \
and o.owner=t.table_schema \
and o.object_name=t.table_name \
order by owner,table_name,privilege";

static const gchar sql_oracle_privileges[] = 
"select privilege,grantor,grantee,grantable \
from all_tab_privs \
where table_schema = UPPER(:owner) \
and table_name = :name";


/************ Roles *************/

static const gchar sql_oracle_roles[] = 
"select role,password_required from sys.dba_roles \
where role like :name order by role";

static const gchar sql_oracle_enabled_roles[] =
"select role from session_roles where role like :name order by role";

static const gchar sql_oracle_granted_roles[] =
"select granted_role,admin_option \
from sys.dba_role_privs \
where grantee = :name order by granted_role";

/************ Tables *************/

static const gchar sql_oracle_tables_owner[] = 
"select a.object_name,a.object_id,a.created, \
		a.last_ddl_time,a.status, b.partitioned, b.nested, \
		b.IOT_Type,b.IOT_Name,b.temporary, :1 owner \
from user_objects a,user_tables b \
where b.table_name like :2 \
and a.object_name=b.table_name \
and a.object_type='TABLE' \
and b.dropped='NO' \
order by a.object_name";

static const gchar sql_oracle_table_columns_owner[] =
"select a.column_name, \
a.data_type, \
a.data_type_owner type_owner,a.data_length, \
a.data_precision,a.data_scale,a.nullable,a.column_id, \
a.data_default,a.num_distinct, \
cast(a.low_value as varchar2(500)) low_value, \
cast(a.high_value as varchar2(500)) high_value, \
a.char_length,UPPER(:owner) object_owner,a.table_name object_name \
from user_tab_columns a \
where a.table_name = :name \
order by a.column_id";

static const gchar sql_oracle_tables[] = 
"select a.object_name,a.object_id,a.created,a.last_ddl_time, \
a.status, b.partitioned, b.nested , b.IOT_Type, \
b.IOT_Name,b.temporary \
from sys.dba_objects a,sys.dba_tables b \
where b.owner = :1 \
and b.table_name like :2 \
and a.owner=:1 \
and a.object_name = b.table_name \
and a.object_type='TABLE' \
and b.dropped='NO' \
order by a.object_name";

static const gchar sql_oracle_table_columns[] = 
"select a.column_name, \
a.data_type, \
a.data_type_owner type_owner,a.data_length,a.data_precision,a.data_scale, \
a.nullable,a.column_id,a.data_default,a.num_distinct, \
cast(a.low_value as varchar2(500)) low_value, \
cast(a.high_value as varchar2(500)) high_value, \
a.char_length,a.owner object_owner,a.table_name object_name \
from sys.all_tab_columns a \
where a.owner=UPPER(:1) \
and a.table_name=UPPER(:2) \
order by a.column_id";

static const gchar sql_oracle_table_triggers[] = 
"select trigger_name object_name, UPPER(:owner) owner,\
trigger_type, triggering_event, base_object_type, \
table_owner,table_name, status \
from sys.all_triggers \
where table_owner=UPPER(:owner) and table_name=:name ";

static const gchar sql_oracle_table_indexes[] = 
"select index_name object_name, UPPER(:owner) owner, \
uniqueness,index_type,table_owner,table_name, \
partitioned, status,temporary \
from sys.all_indexes \
where table_owner=UPPER(:owner) \
and table_name=:name \
and index_type<>'LOB'";

static const gchar sql_oracle_table_constraints[] = 
"select constraint_name, constraint_type, \
decode(constraint_type,'C','Check','P','Primary','U','Unique','R','Foreign','...') constraint_type_name, \
table_name, search_condition, r_owner, r_constraint_name, delete_rule, \
status , deferrable, deferred, validated, generated, bad, last_change, rely \
from sys.dba_constraints where owner = UPPER(:owner) \
and table_name = :name \
order by constraint_name";

static const gchar sql_oracle_table_constraints_owner[] = 
"select constraint_name, constraint_type, \
decode(constraint_type,'C','Check','P','Primary','U','Unique','R','Foreign','...') constraint_type_name, \
table_name, search_condition, r_owner, r_constraint_name, delete_rule, \
status , deferrable, deferred, validated, generated, bad, last_change, rely \
from sys.all_constraints where owner = UPPER(:owner) \
and table_name = :name \
order by constraint_name";


/************ Indexes *************/

static const gchar sql_oracle_indexes_owner[] = 
"select a.object_name, UPPER(:owner) owner, a.object_id,a.created,a.last_ddl_time, \
a.status,b.uniqueness,b.index_type,b.table_owner,b.table_name, \
b.partitioned, b.temporary \
from user_objects a,user_indexes b \
where b.index_name like :name \
and b.index_type<>'LOB' \
and a.object_name=b.index_name \
and a.object_type='INDEX' \
and b.dropped='NO' \
order by a.object_name";

static const gchar sql_oracle_index_columns_owner[] = 
"select a.column_name,a.column_length,a.column_position,b.column_expression, \
 a.descend, UPPER(:owner) object_owner,a.index_name object_name \
 from user_ind_columns a,user_ind_expressions b \
where a.index_name like :name \
and b.index_name(+)=a.index_name \
and b.column_position(+)=a.column_position \
order by a.column_position";

static const gchar sql_oracle_indexes[] = 
"select a.object_name, a.object_id, a.created, a.last_ddl_time, \
a.status,b.uniqueness,b.index_type, b.table_owner,b.table_name, \
b.partitioned, b.temporary \
from (select a.owner owner,a.object_name,a.object_id,a.created, \
	  a.last_ddl_time, a.status from sys.dba_objects a \
	  where a.owner like UPPER(:owner) \
	  and a.object_type = 'INDEX') a, sys.dba_indexes b \
where b.owner like UPPER(:owner) \
and b.index_name like :name \
and b.index_type<>'LOB' \
and a.object_name=b.index_name \
and b.dropped='NO' \
order by a.object_name";

static const gchar sql_oracle_index_columns[] = 
"select a.column_name,a.column_length,a.column_position, \
 b.column_expression, a.descend, a.index_owner object_owner, \
 a.index_name object_name \
 from sys.dba_ind_columns a,sys.dba_ind_expressions b \
where a.index_owner=:1 \
and a.index_name=:2 \
and b.index_owner(+)=a.index_owner \
and b.index_name(+)=a.index_name \
and b.column_position(+)=a.column_position \
order by a.column_position";

/************ Triggers *************/

static const gchar sql_oracle_triggers_owner[] =
"select a.object_name, UPPER(:owner) owner, a.object_id,a.created,a.last_ddl_time, \
a.status, b.trigger_type, b.triggering_event, b.base_object_type, \
b.table_owner,b.table_name, b.status \
from user_objects a,user_triggers b \
where b.trigger_name like :name \
and a.object_name=b.trigger_name \
 and a.object_type='TRIGGER' \
and b.trigger_name not in (select object_name \
				from user_recyclebin \
				where object_name=b.trigger_name) \
order by a.object_name";

static const gchar sql_oracle_trigger_columns_owner[] = 
"select a.column_name,a.column_usage, \
a.column_list,UPPER(:owner) object_owner,a.trigger_name object_name \
from user_trigger_cols a \
where a.trigger_name=:name \
and a.table_owner=UPPER(:owner) \
and a.table_name= (select table_name \
					from user_triggers \
					where trigger_name = :name)";

static const gchar sql_oracle_triggers[] =
"select a.object_name, a.owner, a.object_id,a.created,a.last_ddl_time,a.status, \
b.trigger_type, b.triggering_event,b.base_object_type,b.table_owner,\
b.table_name,b.status \
from (select a.owner,a.object_name,a.object_id,a.created,a.last_ddl_time,\
		a.status  from sys.dba_objects a  \
	  where a.owner like UPPER(:owner) \
	  and a.object_type='TRIGGER') a, \
     sys.dba_triggers b \
where b.owner like UPPER(:owner) \
and b.trigger_name like :name \
and a.object_name=b.trigger_name \
and b.trigger_name not in (select object_name \
			from DBA_recyclebin  \
			where object_name=b.trigger_name) \
order by a.object_name";

static const gchar sql_oracle_trigger_columns[] = 
"select a.column_name,a.column_usage, \
a.column_list,a.trigger_owner object_owner, \
a.trigger_name object_name \
from sys.dba_trigger_cols a \
where a.trigger_owner=UPPER(:owner) \
and a.trigger_name=:nane";

/************ Depends On *************/

static const gchar sql_oracle_depends_on[] = 
"select a.owner,a.object_name, \
decode(a.object_type, \
'TABLE', 1 , \
'VIEW',2, \
'INDEX',3, \
'PROCEDURE',4, \
'FUNCTION',5, \
'PACKAGE',6, \
'PACKAGE BODY',7, \
'TRIGGER',8, \
'SEQUENCE',9, \
'SYNONYM',10, \
'CLUSTER',11, \
'DATABASE LINK',12, \
'SNAPSHOT',13, \
'MATERIALIZED VIEW',14, \
'TYPE',15, \
'TYPE BODY',16, \
'TABLE PARTITION',17, \
'INDEX PARTITION',18, \
'LIBRARY',19, \
'DIRECTORY',20, \
'QUEUE',21, \
'JAVA SOURCE',22, \
'JAVA CLASS',23, \
'JAVA RESOURCE',24, \
'INDEXTYPE',25, \
'OPERATOR',26, \
'DIMENSION',27, \
'RESOURCE PLAN',28, \
'CONSUMER GROUP',29,0) object_type \
from sys.all_objects a \
where a.object_id in (select DISTINCT referenced_object_id object_id \
						from public_dependency \
						where object_id=(select object_id \
				 from sys.all_objects \
				 where owner = UPPER (:owner) \
				 and object_name=:object_name \
				 and object_type like :object_type )) \
order by object_type, a.object_name";

/************ Dependent Objects *************/

static const gchar sql_oracle_dependent_objects[] = 
"select a.owner,a.object_name, \
decode(a.object_type, \
'TABLE', 1 , \
'VIEW',2, \
'INDEX',3, \
'PROCEDURE',4, \
'FUNCTION',5, \
'PACKAGE',6, \
'PACKAGE BODY',7, \
'TRIGGER',8, \
'SEQUENCE',9, \
'SYNONYM',10, \
'CLUSTER',11, \
'DATABASE LINK',12, \
'SNAPSHOT',13, \
'MATERIALIZED VIEW',14, \
'TYPE',15, \
'TYPE BODY',16, \
'TABLE PARTITION',17, \
'INDEX PARTITION',18, \
'LIBRARY',19, \
'DIRECTORY',20, \
'QUEUE',21, \
'JAVA SOURCE',22, \
'JAVA CLASS',23, \
'JAVA RESOURCE',24, \
'INDEXTYPE',25, \
'OPERATOR',26, \
'DIMENSION',27, \
'RESOURCE PLAN',28, \
'CONSUMER GROUP',29,0) object_type \
from sys.all_objects a \
where a.object_id in (select DISTINCT object_id from public_dependency \
where referenced_object_id = (select object_id \
				 from sys.all_objects \
				 where owner = UPPER (:owner) \
				 and object_name=:object_name \
				 and object_type like :object_type)) \
order by object_type, a.object_name";


/************ User's Objects *************/

static const gchar sql_oracle_users_objects_owner[] = 
"select a.object_name, UPPER(:owner) owner, a.object_id,a.created, \
a.last_ddl_time,a.status \
from user_objects a \
where object_name like :object_name \
and object_type = :object_type \
order by a.object_name";

static const gchar sql_oracle_users_objects[] = 
"select a.object_name, owner, a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status \
from sys.dba_objects a \
where a.owner like UPPER(:owner) \
and object_name like :object_name \
and object_type=:object_type \
order by a.object_name";

/************ Constraints *************/

static const gchar sql_oracle_constraints[] = 
"select constraint_name, constraint_type, \
decode(constraint_type,'C','Check','P','Primary','U','Unique','R','Foreign','...') constraint_type_name, \
table_name, search_condition, r_owner, r_constraint_name, delete_rule, \
status , deferrable, deferred, validated, generated, bad, last_change, rely \
from sys.dba_constraints where owner=UPPER(:owner) \
and constraint_name like :name \
and not constraint_type in ('V','O') \
order by constraint_name";

static const gchar sql_oracle_constraints_owner[] = 
"select constraint_name, constraint_type, \
decode(constraint_type,'C','Check','P','Primary','U','Unique','R','Foreign','...') constraint_type_name, \
table_name, search_condition, r_owner, r_constraint_name, delete_rule, \
status , deferrable, deferred, validated, generated, bad, last_change, rely \
from sys.all_constraints where owner=UPPER(:owner) \
and constraint_name like :name \
and not constraint_type in ('V','O')";

static const gchar sql_oracle_constraints_columns[] = 
" select column_name,decode(position,null,0,position) position \
from sys.all_cons_columns \
where owner = UPPER(:owner) \
and constraint_name = :name \
and table_name = (select table_name \
				  from sys.all_constraints \
				  where owner = UPPER(:owner) \
				  and constraint_name = :name )";

/************ Arguments *************/

static const gchar sql_oracle_arguments[] = 
"select nvl(argument_name, '...OUT') argument_name,position,sequence,data_level,overload, \
in_out, data_type, data_length,data_precision,data_scale, \
object_name,package_name,default_value,radix,type_owner, \
type_name,type_subname,type_link,pls_type \
from all_arguments \
where object_id = ( select a.object_id \
					from sys.all_objects a \
					where a.owner = UPPER(:owner) \
					and object_name = :object_name \
					and object_type = :object_type ) \
and data_type is not null \
order by sequence";

/************ Entries *************/

static const gchar sql_oracle_entries[] = 
"select object_name,decode(sum(decode(position,0,1,0)),0,0,1) entry_type, overload \
from all_arguments \
where object_id = ( select a.object_id \
					from sys.all_objects a \
					where a.owner like UPPER(:owner) \
					and object_name like :object_name \
					and object_type=:object_type ) \
group by object_name,overload";


/************ Materialized views (aka Snapshots) *************/

static const gchar sql_oracle_mviews[] =
"select /*+ QCO9i QCOALL */ sn.name object_name,sn.owner,sn.table_name,sn.master_view, \
sn.master_owner,sn.master,sn.master_link,sn.can_use_log, sn.updatable, sr.last_refresh, \
sn.error,sn.type, sn.next, sn.start_with, sn.refresh_group,sn.update_trig,sn.update_log, \
sn.query, sn.refresh_method, sn.fr_operations, sn.cr_operations, sn.master_rollback_seg, \
sn.status,sn.refresh_mode, sn.prebuilt \
from sys.dba_snapshots sn, sys.dba_mview_refresh_times sr \
where sn.owner like UPPER(:owner) \
and sr.owner = sn.owner \
and sr.name = sn.name \
and sr.master_owner = sn.master_owner \
and sr.master = sn.master \
order by sn.name";

static const gchar sql_oracle_mviews_owner[] =
"select /*+ QCO9i QCOALL */ sn.name object_name,sn.owner,sn.table_name,sn.master_view, \
sn.master_owner,sn.master,sn.master_link,sn.can_use_log, sn.updatable, sr.last_refresh, \
sn.error,sn.type, sn.next, sn.start_with, sn.refresh_group,sn.update_trig,sn.update_log, \
sn.query, sn.refresh_method, sn.fr_operations, sn.cr_operations, sn.master_rollback_seg, \
sn.status,sn.refresh_mode, sn.prebuilt \
from sys.all_snapshots sn, sys.all_mview_refresh_times sr \
where sn.owner like UPPER(:owner) \
and sr.owner = sn.owner \
and sr.name = sn.name \
and sr.master_owner = sn.master_owner \
and sr.master = sn.master \
order by sn.name ";

static const gchar sql_oracle_mview_logs[] =
"select log_table,log_trigger,current_snapshots, \
rowids,primary_key,filter_columns,snapshot_id \
from sys.dba_snapshot_logs \
where log_owner=UPPER(:owner) \
and master=:name";

static const gchar sql_oracle_mview_logs_owner[] =
"select log_table,log_trigger,current_snapshots, \
rowids,primary_key,filter_columns,snapshot_id \
from sys.all_snapshot_logs \
where log_owner=UPPER(:owner) \
and master=:name";


/************ Synonyms *************/

static const gchar sql_oracle_synonyms[] =
"select a.object_name,a.owner,a.object_id,a.created,a.last_ddl_time, a.status, \
s.table_owner,s.table_name,s.db_link \
from (select a.owner,a.object_name,a.object_id,a.created,a.last_ddl_time, \
		a.status from sys.dba_objects a where a.owner = UPPER(:owner)) a, sys.dba_synonyms s \
where s.owner = UPPER(:owner) \
and s.synonym_name like :name \
and a.object_name=s.synonym_name \
order by a.object_name";

static const gchar sql_oracle_synonyms_owner[] =
"select a.object_name, UPPER(:owner) owner, a.object_id,a.created,a.last_ddl_time, \
a.status,s.table_owner, \
s.table_name,s.db_link \
from user_objects a,user_synonyms s \
where s.synonym_name like :name \
 and a.object_name=s.synonym_name \
order by a.object_name";


/************ Database links *************/

static const gchar sql_oracle_dblinks[] =
"select owner,db_link object_name, created, username, \
host, owner dblink_type,created last_ddl_time \
from all_db_links \
where owner like UPPER(:owner) \
and db_link like :name";

/************* Object types ************/
static const gchar sql_oracle_object_types_owner[] = 
/*
select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time,
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection,
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods,
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,
c.upper_bound,c.elem_type_owner,c.elem_type_name,
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,
c.length,c.precision,c.scale,c.elem_storage,
decode(c.nulls_stored,'YES','Y','N') nulls_stored
from user_coll_types c,user_types b,user_objects a
where b.type_name like :object_name
and a.object_name=b.type_name
and a.object_type='TYPE'
and c.type_name(+)=a.object_name
and c.elem_type_owner is not null
UNION ALL select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time,
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection,
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods,
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,c.upper_bound,c.elem_type_owner
,decode(c.elem_type_name, 'TIMESTAMP WITH TZ','TIMESTAMP WITH TIME ZONE',
'TIMESTAMP WITH LOCAL TZ','TIMESTAMP WITH LOCAL TIME ZONE',c.elem_type_name) elem_type_name,
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,
c.length,c.precision,c.scale,c.elem_storage,
decode(c.nulls_stored,'YES','Y','N') nulls_stored
from user_coll_types c,user_types b,user_objects a
where b.type_name like :object_name
and a.object_name=b.type_name
and a.object_type='TYPE'
and c.type_name(+)=a.object_name
and c.elem_type_owner is null*/


"select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type, \
c.upper_bound,c.elem_type_owner,c.elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod, \
c.length,c.precision,c.scale,c.elem_storage, \
decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from user_coll_types c,user_types b,user_objects a \
where b.type_name like :object_name \
and a.object_name=b.type_name \
and b.typecode != 'COLLECTION' \
and a.object_type='TYPE' \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is not null \
UNION ALL select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,c.upper_bound,c.elem_type_owner \
,decode(c.elem_type_name, 'TIMESTAMP WITH TZ','TIMESTAMP WITH TIME ZONE', \
'TIMESTAMP WITH LOCAL TZ','TIMESTAMP WITH LOCAL TIME ZONE',c.elem_type_name) elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod, \
c.length,c.precision,c.scale,c.elem_storage, \
decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from user_coll_types c,user_types b,user_objects a \
where b.type_name like :object_name \
and b.typecode != 'COLLECTION' \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is null \
order by 1";

static const gchar sql_oracle_object_types[] = 
"select a.object_name, a.owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
decode(b.typecode,'COLLECTION','Y','N') collection,decode(b.incomplete,'YES','Y','N') incomplete, \
b.attributes,b.methods,decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type, \
c.upper_bound,c.elem_type_owner,c.elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,c.length, \
c.precision,c.scale,c.elem_storage,decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from sys.dba_coll_types c,sys.dba_types b,sys.dba_objects a \
where b.owner like :owner \
and b.type_name like :object_name \
and b.typecode != 'COLLECTION' \
and a.owner=:owner \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.owner(+)=:owner \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is not null \
UNION ALL select a.object_name, a.owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,c.upper_bound,c.elem_type_owner, \
decode(c.elem_type_name, 'TIMESTAMP WITH TZ','TIMESTAMP WITH TIME ZONE', \
'TIMESTAMP WITH LOCAL TZ','TIMESTAMP WITH LOCAL TIME ZONE',c.elem_type_name) elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,c.length, \
c.precision,c.scale,c.elem_storage,decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from sys.dba_coll_types c,sys.dba_types b,sys.dba_objects a \
where b.owner like :owner \
and b.type_name like :object_name \
and b.typecode != 'COLLECTION' \
and a.owner=:owner \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.owner(+)=:owner \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is null \
order by 1";

/************ Collection Types *************/

static const gchar sql_oracle_collection_types_owner[] =
"select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type, \
c.upper_bound,c.elem_type_owner,c.elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod, \
c.length,c.precision,c.scale,c.elem_storage, \
decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from user_coll_types c,user_types b,user_objects a \
where b.type_name like :object_name \
and a.object_name=b.type_name \
and b.typecode = 'COLLECTION' \
and a.object_type='TYPE' \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is not null \
UNION ALL select a.object_name, :owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,c.upper_bound,c.elem_type_owner \
,decode(c.elem_type_name, 'TIMESTAMP WITH TZ','TIMESTAMP WITH TIME ZONE', \
'TIMESTAMP WITH LOCAL TZ','TIMESTAMP WITH LOCAL TIME ZONE',c.elem_type_name) elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod, \
c.length,c.precision,c.scale,c.elem_storage, \
decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from user_coll_types c,user_types b,user_objects a \
where b.type_name like :object_name \
and b.typecode = 'COLLECTION' \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is null \
order by 1";

static const gchar sql_oracle_collection_types[] = 
"select a.object_name, a.owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
decode(b.typecode,'COLLECTION','Y','N') collection,decode(b.incomplete,'YES','Y','N') incomplete, \
b.attributes,b.methods,decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type, \
c.upper_bound,c.elem_type_owner,c.elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,c.length, \
c.precision,c.scale,c.elem_storage,decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from sys.dba_coll_types c,sys.dba_types b,sys.dba_objects a \
where b.owner like :owner \
and b.type_name like :object_name \
and b.typecode = 'COLLECTION' \
and a.owner=:owner \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.owner(+)=:owner \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is not null \
UNION ALL select a.object_name, a.owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,decode(b.typecode,'COLLECTION','Y','N') collection, \
decode(b.incomplete,'YES','Y','N') incomplete,b.attributes,b.methods, \
decode(c.coll_type,'TABLE',0,'VARYING ARRAY',1,2) coll_type,c.upper_bound,c.elem_type_owner, \
decode(c.elem_type_name, 'TIMESTAMP WITH TZ','TIMESTAMP WITH TIME ZONE', \
'TIMESTAMP WITH LOCAL TZ','TIMESTAMP WITH LOCAL TIME ZONE',c.elem_type_name) elem_type_name, \
decode(c.elem_type_mod,'REF',1,'POINTER',2,0) elem_type_mod,c.length, \
c.precision,c.scale,c.elem_storage,decode(c.nulls_stored,'YES','Y','N') nulls_stored \
from sys.dba_coll_types c,sys.dba_types b,sys.dba_objects a \
where b.owner like :owner \
and b.type_name like :object_name \
and b.typecode = 'COLLECTION' \
and a.owner=:owner \
and a.object_name=b.type_name \
and a.object_type='TYPE' \
and c.owner(+)=:owner \
and c.type_name(+)=a.object_name \
and c.elem_type_owner is null \
order by 1";

/************ Store tables ************/

static const gchar sql_oracle_store_tables_owner[]=
"select a.object_name,:owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
decode(b.partitioned,'YES','Y','NO','N') partitioned,'Y' object_table, \
'N' external_table,decode(b.nested,'YES','Y','N') nested, \
decode(b.IOT_Type,'IOT',1,'IOT_OVERFLOW',2,0) IOT_Type,b.IOT_Name,b.temporary \
from user_objects a,user_object_tables b \
where b.table_name like :object_name \
and a.object_name=b.table_name \
and a.object_type='TABLE' \
and ( b.nested = 'YES' or b.IOT_Type = 'IOT_OVERFLOW' ) \
union all \
select a.object_name,:owner owner,a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status,'N' partitioned,'N' object_table, \
'Y' external_table,'N' nested,0 IOT_Type,'' IOT_Name,'N' temporary \
from user_objects a,user_external_tables b \
where b.table_name like :object_name \
and a.object_name=b.table_name \
and a.object_type='TABLE'";

static const gchar sql_oracle_store_tables[] =
"select a.object_name, :owner owner, \
a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
decode(b.partitioned,'YES','Y','NO','N') partitioned,'N' object_table,'N' external_table, \
decode(b.nested,'YES','Y','N') nested, \
decode(b.IOT_Type,'IOT',1,'IOT_OVERFLOW',2,0) IOT_Type, \
b.IOT_Name,b.temporary \
from sys.dba_objects a,sys.dba_tables b \
where b.owner=:owner \
and b.table_name like :object_name \
and a.owner=:owner \
and a.object_name=b.table_name \
and a.object_type='TABLE' \
and ( b.nested = 'YES' or b.IOT_Type = 'IOT_OVERFLOW' ) \
and b.dropped='NO' \
union all \
select a.object_name, :owner owner, \
a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
decode(b.partitioned,'YES','Y','NO','N') partitioned,'Y' object_table,'N' external_table, \
decode(b.nested,'YES','Y','N') nested, \
decode(b.IOT_Type,'IOT',1,'IOT_OVERFLOW',2,0) IOT_Type, \
b.IOT_Name,b.temporary \
from sys.dba_objects a,sys.dba_object_tables b \
where b.owner=:owner \
and b.table_name like :object_name \
and a.owner=:owner \
and a.object_name=b.table_name \
and a.object_type='TABLE' \
and ( b.nested = 'YES' or b.IOT_Type = 'IOT_OVERFLOW' ) \
union all \
select a.object_name,:owner owner, \
a.object_id,a.created,a.last_ddl_time, \
decode(a.status,'VALID',0,'INVALID',1,2) status, \
'N' partitioned,'N' object_table,'Y' external_table,'N' nested,0 IOT_Type,'' IOT_Name, \
'N' temporary  \
from sys.dba_objects a,sys.dba_external_tables b \
where b.owner=:owner \
and b.table_name like :object_name \
and a.owner=:owner \
and a.object_name=b.table_name \
and a.object_type='TABLE'";

/************ Tablespaces *************/

static const gchar sql_oracle_tablespaces[] = 
"select f.tablespace_name, to_char(f.block_size) block_size, \
to_char(round(f.block_size/1024,2)) block_size_kb, \
f.status, f.contents, f.logging, f.extent_management, \
to_char(f.initial_extent) initial_extent, \
to_char(round(f.initial_extent/1024,2)) initial_extent_kb, \
to_char(round(f.initial_extent/1048576,2)) initial_extent_mb, \
to_char(f.next_extent) next_extent, \
to_char(round(f.next_extent/1024,2)) next_extent_kb, \
to_char(round(f.next_extent/1048576,2)) next_extent_mb, \
to_char(f.min_extents) min_extents, to_char(f.max_extents) max_extents, \
to_char(f.pct_increase) pct_increase, \
to_char(f.min_extlen) min_extent_length, \
to_char(round(f.min_extlen/1024,2)) min_extent_length_kb, \
to_char(round(f.min_extlen/1048576,2)) min_extent_length_mb, \
f.bigfile, f.allocation_type \
from user_tablespaces f \
where f.tablespace_name like :name \
order by f.tablespace_name";

static const gchar sql_oracle_tablespace_data_files[] = 
"select d.file_name, d.temporary,d.blocks, \
d.status, d.relative_fno, d.autoextensible, to_char(d.maxbytes) max_bytes,\
to_char(trunc(d.maxbytes/1024,1)) max_bytes_kb, \
to_char(trunc(d.maxbytes/1048576,1)) max_bytes_mb, \
to_char(d.user_bytes) user_bytes, \
to_char(trunc(d.user_bytes/1024,1)) user_bytes_kb, \
to_char(trunc(d.user_bytes/1048576,1)) user_bytes_mb, \
d.user_blocks, d.maxblocks max_blocks ,\
to_char(d.increment_by) increment_by \
from (select 'N' temporary, a.file_name, a.file_id, \
		a.tablespace_name, a.bytes, a.blocks, a.status, \
		a.relative_fno, a.autoextensible, a.maxbytes, \
		a.maxblocks, a.increment_by, a.user_bytes, a.user_blocks \
		from sys.dba_data_files a \
		where a.tablespace_name= :name \
		UNION ALL \
		select 'Y' temporary, a.* \
		from sys.dba_temp_files a \
		where tablespace_name=:name ) d";

static const gchar sql_oracle_tablespace_disk_groups[] = 
"select a.name, to_char(a.group_number) group_number, \
to_char(a.sector_size) sector_size, \
to_char(a.allocation_unit_size) allocation_unit_size, \
a.state, a.type, to_char(a.total_mb) \
total_mb, to_char(a.free_mb) free_mb \
from V$asm_diskgroup a";

static const gchar sql_oracle_tablespaces_quotas[] = 
"select tablespace_name, to_char(bytes) bytes, \
to_char(trunc(bytes/1024,1)) bytes_kb, \
to_char(trunc(bytes/1048576,1)) bytes_mb,to_char(max_bytes) max_bytes, \
to_char(trunc(max_bytes/1024,1)) max_bytes_kb, \
to_char(trunc(max_bytes/1048576,1)) max_bytes_mb,blocks,max_blocks \
from user_ts_quotas order by tablespace_name";

static const gchar sql_oracle_tablespaces_free_space[] =
"select tablespace_name,to_char(sum(bytes)) bytes, \
to_char(trunc(sum(bytes)/1024,1)) bytes_kb, \
to_char(trunc(sum(bytes)/1048576,1)) bytes_mb,sum(blocks) blocks \
from user_free_space \
where tablespace_name like :name \
group by tablespace_name \
order by tablespace_name";


/************ Instance props. Server opts. *************/

static const gchar sql_oracle_instance_props[] = 
"select name, type, value, isdefault, isses_modifiable, \
issys_modifiable, ismodified, isadjusted, description  \
from v$parameter where name like :name \
order by name";

static const gchar sql_oracle_server_opts[] = 
"select parameter,decode(value,'TRUE',1,0) value \
from v$option where parameter like :name \
order by parameter";


/************ Recycle Bin *************/

static const gchar sql_oracle_recycle_owner[] = 
"select OBJECT_NAME, ORIGINAL_NAME, OPERATION, \
TYPE, decode(type, \
'TABLE', 1 , \
'VIEW',2, \
'INDEX',3, \
'PROCEDURE',4, \
'FUNCTION',5, \
'PACKAGE',6, \
'PACKAGE BODY',7, \
'TRIGGER',8, \
'SEQUENCE',9, \
'SYNONYM',10, \
'CLUSTER',11, \
'DATABASE LINK',12, \
'SNAPSHOT',13, \
'MATERIALIZED VIEW',14, \
'TYPE',15, \
'TYPE BODY',16, \
'TABLE PARTITION',17, \
'INDEX PARTITION',18, \
'LIBRARY',19, \
'DIRECTORY',20, \
'QUEUE',21, \
'JAVA SOURCE',22, \
'JAVA CLASS',23, \
'JAVA RESOURCE',24, \
'INDEXTYPE',25, \
'OPERATOR',26, \
'DIMENSION',27, \
'RESOURCE PLAN',28, \
'CONSUMER GROUP',29,0) d_type, \
TS_NAME, CREATETIME, DROPTIME, \
to_char(DROPSCN) DROPSCN,PARTITION_NAME, CAN_UNDROP, CAN_PURGE, \
to_char(RELATED) RELATED,to_char(BASE_OBJECT) BASE_OBJECT, \
to_char(PURGE_OBJECT) PURGE_OBJECT,to_char(SPACE) SPACE \
from user_recyclebin \
where OBJECT_NAME like :name \
order by TYPE, ORIGINAL_NAME";


/************ Rollback Segments *************/

static const gchar sql_oracle_rollbacks[] = 
"select s.segment_name,s.owner,s.status,s.tablespace_name,s.segment_id,s.file_id,s.block_id, \
to_char(s.initial_extent) initial_extent,to_char(s.next_extent) next_extent, \
to_char(trunc(s.initial_extent/1024,1)) initial_extent_kb, \
to_char(trunc(s.next_extent/1024,1)) next_extent_kb, \
to_char(round(s.initial_extent/1048576,4)) initial_extent_mb, \
to_char(round(s.next_extent/1048576,4)) next_extent_mb, \
to_char(s.min_extents) min_extents, to_char(s.max_extents) max_extents, \
to_char(s.pct_increase) pct_increase,s.instance_num,s.relative_fno, \
r.extents,to_char(r.rssize) rssize,to_char(r.writes) writes, \
r.xacts,r.gets,r.waits,to_char(r.optsize) optsize, \
to_char(trunc(r.optsize/1024,1)) optsize_kb, \
to_char(trunc(r.optsize/1048576,1)) optsize_mb,to_char(r.hwmsize) hwmsize, \
r.shrinks,r.wraps,r.extends,r.aveshrink,r.aveactive,r.curext,r.curblk \
from dba_rollback_segs s, v$rollstat r \
where s.segment_name like :name \
and r.usn(+)=s.segment_id \
order by s.segment_name";

/************ Redo Log Groups *************/

static const gchar sql_oracle_redo_log_groups[] =
"select to_char(group#) \"group\", status, to_char(bytes) \"size\", \
to_char(trunc(bytes/1024,1)) size_kb, to_char(trunc(bytes/1048576,1)) size_mb, \
sequence# sequence, thread# thread \
from v$log where group# like :name order by group#";

static const gchar sql_oracle_redo_log_members[] =
"select member, type from v$logfile where group# = :name";

#endif /* _NAV_SQL_H */

 
