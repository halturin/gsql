/***************************************************************************
 *            nav_tree_static.h
 *
 *  Tue Sep 18 22:25:53 2007
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
 
#ifndef _NAV_TREE_STATIC_H
#define _NAV_TREE_STATIC_H

#include <glib.h>

#include <libgsql/common.h>
#include <libgsql/stock.h>
#include <libgsql/navigation.h>
#include "engine_stock.h"
#include "nav_objects.h"
#include "nav_sql.h"

#include "nav_tree__tables.h"


/* 	
	The users_objects struct used as child for "My Scheme" 
	and as child of each users schemas 
*/
static GSQLNavigationItem users_objects[] = {
	{	TABLES_ID,					// id
		GSQL_STOCK_TABLES,			// stock
		N_("Tables"),				// name
		sql_mysql_tables,			// sql
		NULL,						// object_popup
		NULL,						// object_handler
		nav_tree_tables_refresh,	// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	CONSTRAINTS_ID,
		GSQL_STOCK_CONSTRAINT, 
		N_("Constraints"), 
		sql_mysql_constraints, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	VIEWS_ID, 
		GSQL_STOCK_VIEWS, 
		N_("Views"), 
		sql_mysql_views, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	INDEXES_ID,
		GSQL_STOCK_INDEXES, 
		N_("Indexes"), 
		sql_mysql_indexes, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS, 
		N_("Triggers"), 
		sql_mysql_triggers, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	PROCEDURES_ID, 
		GSQL_STOCK_PROCEDURES, 
		N_("Procedures"), 
		sql_mysql_routines, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{	FUNCTIONS_ID, 
		GSQL_STOCK_FUNCTIONS, 
		N_("Functions"), 
		sql_mysql_routines, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
	{
		PRIVILEGES_ID,
		GSQL_STOCK_PRIVILEGES,
		N_("Privileges"),
		sql_mysql_privileges,
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 }					// child, childs
};


/* 	
	The root_objects struct used as root tree
*/
static GSQLNavigationItem root_objects[] = {
// root. my schema
	{	MY_SCHEMA_ID,
		GSQL_STOCK_MY_SCHEMA,
		N_("My Schema"), 
		NULL,						// sql
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		users_objects, G_N_ELEMENTS (users_objects) },
// root. all schemas	
	{	ALL_SCHEMAS_ID,
		GSQL_STOCK_ALL_SCHEMAS,
		N_("All Schemas"), 
		sql_mysql_all_schemas, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
// root. all users
	{	USERS_ID,
		GSQL_STOCK_USERS,
		N_("Users"), 
		sql_mysql_users, 
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs

// process list
    {   PROCESSES_ID,
		GSQL_STOCK_USERS,
		N_("Processes"),
		sql_mysql_processes,
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,						// expand_handler
		NULL,						// event_handler
		NULL, 0 }					// child, childs
};


#endif /* _NAV_TREE_STATIC_H */

 
