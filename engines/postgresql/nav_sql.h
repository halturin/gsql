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
"select "\
"  nsp.nspname, "\
"  nsp.oid, "\
"  pg_get_userbyid(nsp.nspowner) AS namespaceowner, "\
"  nsp.nspacl, "\
"  des.description "\
"FROM pg_namespace nsp "\
"LEFT OUTER JOIN pg_description des ON des.objoid=nsp.oid "\
"where nspname not in ('information_schema', 'pg_catalog', 'pg_temp_1', 'pg_toast') " \
"order by lower(nspname)";

static const gchar sql_pgsql_languages[] =
"SELECT "\
"  lan.lanname, lanpltrusted, lanacl,  "\
"  hp.proname as lanproc, vp.proname as lanval,  "\
"  description "\
"FROM pg_language lan "\
"JOIN pg_proc hp on hp.oid=lanplcallfoid "\
"LEFT OUTER JOIN pg_proc vp on vp.oid=lanvalidator "\
"LEFT OUTER JOIN pg_description des ON des.objoid=lan.oid AND des.objsubid=0 "\
"WHERE lanispl IS TRUE "\
"ORDER BY lanname ";

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
"SELECT "\
"  rel.relname, "\
"  ns.nspname,"\
"  pg_get_userbyid(rel.relowner) AS relowner, "\
"  rel.relacl, "\
"  rel.relhasoids, "\
"  rel.relhassubclass, "\
"  rel.reltuples, "\
"  des.description, "\
"  c.conname, "\
"  c.conkey "\
"from pg_class rel "\
"LEFT OUTER JOIN pg_description des "\
"ON (des.objoid=rel.oid AND des.objsubid=0) "\
"LEFT OUTER JOIN pg_constraint c "\
"ON c.conrelid=rel.oid AND c.contype='p' "\
"join pg_namespace ns on rel.relnamespace = ns.oid "\
"WHERE rel.relkind IN ('r','s','t') "\
"and nspname = $1 "\
"order by relname ";

static const gchar sql_pgsql_table_columns[] = 
"SELECT "\
" attname, attlen, attnum, attnotnull, atthasdef, "\
" pg_catalog.pg_get_expr(def.adbin, def.adrelid) AS defval,  "\
" CASE WHEN att.attndims > 0 THEN 1 ELSE 0 END AS isarray, "\
" attndims, format_type(ty.oid,NULL) AS typname, tn.nspname as typnspname,  "\
" et.typname as elemtypname, cl.relname, na.nspname, att.attstattarget,  "\
" des.description, cs.relname AS sername, ns.nspname AS serschema, "\
" (SELECT count(1) FROM pg_type t2 WHERE t2.typname=ty.typname) > 1 AS isdup,"\
" indkey, "\
"  CASE "\
"   WHEN EXISTS ( SELECT inhparent FROM pg_inherits  "\
"        WHERE inhrelid=att.attrelid ) THEN att.attrelid::regclass "\
"   ELSE NULL "\
"  END as inhrelname, "\
"  EXISTS( "\
"    SELECT 1  "\
"    FROM  pg_constraint  "\
"    WHERE conrelid=att.attrelid  "\
"    AND contype='f' AND att.attnum=ANY(conkey) "\
"   ) As isfk "\
"FROM pg_attribute att "\
"JOIN pg_type ty ON ty.oid=att.atttypid "\
"JOIN pg_namespace tn ON tn.oid=ty.typnamespace "\
"JOIN pg_class cl ON cl.oid=att.attrelid "\
"JOIN pg_namespace na ON na.oid=cl.relnamespace "\
"LEFT OUTER JOIN pg_type et ON et.oid=ty.typelem "\
"LEFT OUTER JOIN pg_attrdef def ON adrelid=att.attrelid AND adnum=att.attnum "\
"LEFT OUTER JOIN pg_description des "\
"ON des.objoid=att.attrelid AND des.objsubid=att.attnum "\
"LEFT OUTER JOIN (pg_depend JOIN pg_class cs  "\
"ON objid=cs.oid AND cs.relkind='S')  "\
"ON refobjid=att.attrelid AND refobjsubid=att.attnum "\
"LEFT OUTER JOIN pg_namespace ns ON ns.oid=cs.relnamespace "\
"LEFT OUTER JOIN pg_index pi ON pi.indrelid=att.attrelid AND indisprimary "\
"WHERE na.nspname = $1 "\
"and cl.relname = $2 "\
"AND att.attisdropped IS FALSE "\
"and attnum > 0 ";

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
"SELECT  "\
"att.attname, att.attlen, att.attnum, att.attnotnull, att.atthasdef,   "\
"pg_catalog.pg_get_expr(def.adbin, def.adrelid) AS defval,    "\
"CASE WHEN att.attndims > 0 THEN 1 ELSE 0 END AS isarray,   "\
"att.attndims, format_type(ty.oid,NULL) AS typname,  "\
"tn.nspname as typnspname,   et.typname as elemtypname,  "\
"cl.relname, n.nspname, att.attstattarget, des.description,  "\
"cs.relname AS sername, ns.nspname AS serschema,   "\
"(SELECT count(1) FROM pg_type t2 WHERE t2.typname=ty.typname) > 1 AS isdup, "\
"  i.indkey, CASE WHEN EXISTS ( SELECT inhparent FROM pg_inherits WHERE  "\
"inhrelid=att.attrelid ) THEN att.attrelid::regclass    ELSE NULL    "\
"END as inhrelname, EXISTS( SELECT 1 FROM  pg_constraint WHERE conrelid =  "\
"att.attrelid AND contype='f' AND att.attnum=ANY(conkey) ) As isfk  "\
"from pg_catalog.pg_class c1  "\
"join pg_catalog.pg_index i on i.indexrelid = c1.oid  "\
"join pg_catalog.pg_class c2 on i.indrelid = c2.oid  "\
"left join pg_catalog.pg_user u on u.usesysid = c1.relowner  "\
"left join pg_catalog.pg_namespace n on n.oid = c1.relnamespace  "\
"left join pg_attribute att on c1.relfilenode = att.attrelid  "\
"JOIN pg_class cl ON cl.oid=att.attrelid  "\
"LEFT OUTER JOIN pg_attrdef def ON adrelid=att.attrelid AND adnum=att.attnum "\
"JOIN pg_type ty ON ty.oid=att.atttypid  "\
"JOIN pg_namespace tn ON tn.oid=ty.typnamespace  "\
"LEFT OUTER JOIN pg_description des ON des.objoid=att.attrelid  "\
"AND des.objsubid=att.attnum  "\
"LEFT OUTER JOIN pg_type et ON et.oid=ty.typelem  "\
"LEFT OUTER JOIN (pg_depend JOIN pg_class cs  ON objid=cs.oid  "\
"AND cs.relkind='S') ON refobjid=att.attrelid AND refobjsubid=att.attnum  "\
"LEFT OUTER JOIN pg_namespace ns ON ns.oid=cs.relnamespace  "\
"where c1.relkind in ('i','') and pg_catalog.pg_table_is_visible(c1.oid)  "\
"and att.attrelid in (  "\
"       select relfilenode  "\
"       from pg_class c  "\
"       where relname = $2 "\
"       and relkind in ( 'i', '' )  "\
")  ";

static const gchar sql_pgsql_triggers[] =
"SELECT "\
"  t.tgname, t.tgenabled, t.tgisconstraint, t.tgnargs, "\
"  cl.relname, nspname, des.description, l.lanname, "\
"  p.prosrc "\
"FROM pg_trigger t "\
"JOIN pg_class cl ON cl.oid=tgrelid "\
"JOIN pg_namespace na ON na.oid=relnamespace "\
"LEFT OUTER JOIN pg_description des ON des.objoid=t.oid "\
"LEFT OUTER JOIN pg_proc p ON p.oid=t.tgfoid "\
"LEFT OUTER JOIN pg_language l ON l.oid=p.prolang "\
"WHERE NOT tgisconstraint "\
"and nspname = $1 "\
"AND relname ilike $2 "\
"ORDER BY tgname ";

static const gchar sql_pgsql_constraints[]=
"SELECT "\
"  DISTINCT ON(cls.relname) cls.relname as idxname, contype, "\
"  indrelid, indkey, indisclustered, indisunique, indisprimary, n.nspname, "\
"  indnatts, cls.reltablespace AS spcoid, spcname, "\
"  tab.relname as tabname, indclass, con.oid as conoid, "\
"  CASE contype  "\
"  WHEN 'p' THEN desp.description  "\
"  WHEN 'u' THEN desp.description  "\
"  ELSE des.description  "\
"  END AS description, "\
"  pg_get_expr(indpred, indrelid) as indconstraint, "\
"  condeferrable, condeferred, amname "\
"FROM pg_index idx "\
"JOIN pg_class cls ON cls.oid=indexrelid "\
"JOIN pg_class tab ON tab.oid=indrelid "\
"LEFT OUTER JOIN pg_tablespace ta on ta.oid=cls.reltablespace "\
"JOIN pg_namespace n ON n.oid=tab.relnamespace "\
"JOIN pg_am am ON am.oid=cls.relam "\
"  LEFT JOIN pg_depend dep  "\
"ON (dep.classid = cls.tableoid  "\
"AND dep.objid = cls.oid  "\
"AND dep.refobjsubid = '0') "\
"  LEFT OUTER JOIN pg_constraint con  "\
"ON (con.tableoid = dep.refclassid  "\
"AND con.oid = dep.refobjid) "\
"LEFT OUTER JOIN pg_description des ON des.objoid=cls.oid "\
"  LEFT OUTER JOIN pg_description desp  "\
"ON (desp.objoid=con.oid AND desp.objsubid = 0) "\
"WHERE nspname = $1 "\
"and tab.relname ilike $2 "\
"ORDER BY cls.relname ";

static const gchar sql_pgsql_views[] =
"SELECT "\
"  c.relname as viewname, pg_get_userbyid(c.relowner) AS viewowner,  "\
"  c.relacl, des.description "\
"FROM pg_class c "\
"LEFT OUTER JOIN pg_description des ON (des.objoid=c.oid and des.objsubid=0) "\
"JOIN pg_namespace ns on c.relnamespace = ns.oid "\
"  WHERE ((c.relhasrules AND (EXISTS ( "\
"     SELECT r.rulename FROM pg_rewrite r "\
"     WHERE ((r.ev_class = c.oid) "\
"    AND (bpchar(r.ev_type) = '1'::bpchar)) ))) OR (c.relkind = 'v'::char)) "\
"  AND ns.nspname = $1 "\
"ORDER BY relname ";

static const gchar sql_pgsql_types[] = 
"select "\
"  t.typname, ns.nspname as typnamespace, t.typlen, "\
"  t.typbyval, t.typtype, t.typisdefined, t.typdelim, "\
"  t.typnotnull, t.typbasetype,  "\
"  format_type(t.oid, null) AS alias, "\
"  pg_get_userbyid(t.typowner) as typeowner,  "\
"  e.typname as element, description, ct.oid AS taboid "\
"FROM pg_type t "\
"LEFT OUTER JOIN pg_type e ON e.oid=t.typelem "\
"LEFT OUTER JOIN pg_class ct ON ct.oid=t.typrelid AND ct.relkind <> 'c' "\
"LEFT OUTER JOIN pg_description des ON des.objoid=t.oid "\
"JOIN pg_namespace ns on t.typnamespace = ns.oid "\
"WHERE t.typtype != 'd'  "\
"AND t.typname NOT LIKE E'\\\\_%' "\
"AND ns.nspname = $1";

static const gchar sql_pgsql_routines[] =
"select "\
"  pr.proname as funcname, prons.nspname as funcnamespace,  "\
"  pg_get_userbyid(proowner) as funcowner, "\
"  prolang, "\
"  proisagg, prosecdef, proisstrict, proretset, "\
"  provolatile, pronargs, prorettype, "\
"  proargtypes, proargnames, prosrc, "\
"  probin, proacl, format_type(TYP.oid, NULL) AS typname,  "\
"  typns.nspname AS typnsp, lanname, des.description "\
"FROM pg_proc pr "\
"JOIN pg_type typ ON typ.oid=prorettype "\
"JOIN pg_namespace typns ON typns.oid=typ.typnamespace "\
"JOIN pg_language lng ON lng.oid=prolang "\
"JOIN pg_namespace prons ON prons.oid=pr.pronamespace "\
"LEFT OUTER JOIN pg_description des ON des.objoid=pr.oid "\
"WHERE proisagg = FALSE "\
"AND prons.nspname = $2 "\
"ORDER BY proname ";

#endif /* _NAV_SQL_H */

 
