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
#include "nav_tree__constraints.h"
#include "nav_tree__indexes.h"

/*
#include "nav_tree__users.h"


#include "nav_tree__triggers.h"
#include "nav_tree__views.h"

#include "nav_tree__procedures.h"
#include "nav_tree__packages.h"
#include "nav_tree__sequences.h"
#include "nav_tree__mviews.h"
#include "nav_tree__synonyms.h"
#include "nav_tree__dblinks.h"
#include "nav_tree__libraries.h"
#include "nav_tree__operators.h"
#include "nav_tree__indextypes.h"
#include "nav_tree__roles.h"
#include "nav_tree__privileges.h"
#include "nav_tree__tablespaces.h"
#include "nav_tree__srv_inst.h"
#include "nav_tree__recycle.h"
#include "nav_tree__rollback.h"
#include "nav_tree__redo_logs.h"
*/

static GSQLNavigationItem current_instance[] = {
	{	INSTANCE_PROPERTIES_ID,					// id
		GSQLE_ORACLE_STOCK_INST_PROP,			// stock
		N_("Instance Properties"),				// name
		NULL,//sql_oracle_instance_props,			// sql
		NULL,						// object_popup
		NULL,						// object_handler
		NULL,	// expand_handler
		NULL,						// event_handler
		NULL, 0 },
		
	{	SERVER_OPTIONS_ID,						// id
		GSQLE_ORACLE_STOCK_SRV_OPTIONS,			// stock
		N_("Server Options"),					// name
		NULL, //sql_oracle_server_opts,			// sql
		NULL,									// object_popup
		NULL,									// object_handler
		NULL, //nav_tree_refresh_srv_inst,		// expand_handler
		NULL,									// event_handler
		NULL, 0}
};


/* 	
	The users_objects struct used as child for "My Scheme" 
	and as child of each users schemas 
*/
static GSQLNavigationItem users_objects[] = {
	{	TABLES_ID,					// id
		GSQL_STOCK_TABLES,			// stock
		N_("Tables"),				// name
		sql_oracle_tables_owner,	// sql
		NULL,						// object_popup
		NULL,						// object_handler
		(GSQLNavigationHandler) nav_tree_tables_refresh,	// expand_handler
		NULL,						// event_handler
		NULL, 0 },					// child, childs
		
	{	CONSTRAINTS_ID,
		GSQL_STOCK_CONSTRAINT, 
		N_("Constraints"), 
		sql_oracle_constraints_owner, 
		NULL, 
		NULL,
		(GSQLNavigationHandler) nav_tree_refresh_constraints,
		NULL,
		NULL, 0 },
		
	{	VIEWS_ID, 
		GSQL_STOCK_VIEWS, 
		N_("Views"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_views,
		NULL,
		NULL, 0 },
		
	{	INDEXES_ID,
		GSQL_STOCK_INDEXES, 
		N_("Indexes"), 
		sql_oracle_indexes_owner, 
		NULL, 
		NULL,
		(GSQLNavigationHandler) nav_tree_refresh_indexes,
		NULL,
		NULL, 0 },
		
	{	TRIGGERS_ID,
		GSQL_STOCK_TRIGGERS, 
		N_("Triggers"), 
		NULL, //sql_oracle_triggers_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_triggers,
		NULL,
		NULL, 0 },
		
	{	PROCEDURES_ID, 
		GSQL_STOCK_PROCEDURES, 
		N_("Procedures"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL, //nav_tree_refresh_procedures,
		NULL,
		NULL, 0 },
		
	{	FUNCTIONS_ID, 
		GSQL_STOCK_FUNCTIONS, 
		N_("Functions"), 
		NULL,//sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_procedures,
		NULL,
		NULL, 0 },
		
	{	PACKAGES_ID, 
		GSQLE_ORACLE_STOCK_PACKAGE, 
		N_("Packages"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_packages,
		NULL,
		NULL, 0 },
		
	{	PACKAGE_BODIES_ID, 
		GSQLE_ORACLE_STOCK_PACKAGE_BODIES, 
		N_("Package Bodies"),
		NULL, //sql_oracle_users_objects_owner,
		NULL,
		NULL,
		NULL, //nav_tree_refresh_packages,
		NULL,
		NULL, 0 },
		
	{	SEQUENCES_ID, 
		GSQL_STOCK_SEQUENCES, 
		N_("Sequences"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_sequences,
		NULL,
		NULL, 0 },
		
	{	CLUSTERS_ID, 
		GSQLE_ORACLE_STOCK_CLUSTERS, 
		N_("Clusters"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	MVIEWS_ID, 
		GSQLE_ORACLE_STOCK_MVIEWS, 
		N_("Materialized Views"), 
		NULL,//sql_oracle_mviews_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_mviews,
		NULL,
		NULL, 0 },
		
	{	SYNONYMS_ID, 
		GSQLE_ORACLE_STOCK_SYNONYMS, 
		N_("Synonyms"), 
		NULL, //sql_oracle_synonyms_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_synonyms,
		NULL,
		NULL, 0 },
		
	{	DATABASE_LINKS_ID, 
		GSQLE_ORACLE_STOCK_DB_LINK, 
		N_("Database Links"), 
		NULL, //sql_oracle_dblinks, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_dblinks,
		NULL,
		NULL, 0 },
		
	{	OBJECT_TYPES_ID, 
		GSQLE_ORACLE_STOCK_OBJ_TYPES, 
		N_("Object Types"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	OBJECT_TYPE_BODIES_ID, 
		GSQLE_ORACLE_STOCK_OBJ_TYPE_BODIES, 
		N_("Object Type Bodies"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	COLLECTION_TYPES_ID, 
		GSQLE_ORACLE_STOCK_COLLECTIONS_TYPES, 
		N_("Collection Types"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	LIBRARIES_ID, 
		GSQLE_ORACLE_STOCK_LIBRARIES, 
		N_("Libraries"), 
		NULL,//sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_libraries,
		NULL,
		NULL, 0 },
		
	{	QUEUES_ID, 
		GSQLE_ORACLE_STOCK_QUEUES, 
		N_("Queues"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	JAVA_SOURCE_ID, 
		GSQLE_ORACLE_STOCK_JAVA_SOURCE, 
		N_("Java Source"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
	{	JAVA_CLASSES_ID, 
		GSQLE_ORACLE_STOCK_JAVA_CLASSES, 
		N_("Java Classes"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	JAVA_RESOURCES_ID, 
		GSQLE_ORACLE_STOCK_JAVA_RESOURCES, 
		N_("Java Resources"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	INDEX_TYPES_ID, 
		GSQLE_ORACLE_STOCK_INDEX_TYPES, 
		N_("Index Types"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL, //nav_tree_refresh_indextypes,
		NULL, 0 },
		
	{	OPERATORS_ID, 
		GSQLE_ORACLE_STOCK_OPERATORS, 
		N_("Operators"), 
		NULL, //sql_oracle_users_objects_owner, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_operators,
		NULL,
		NULL, 0 },
		
	{	DIMENSIONS_ID, 
		GSQLE_ORACLE_STOCK_DIMENSIONS, 
		N_("Dimensions"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
	{	RESOURCE_PLANS_ID, 
		GSQLE_ORACLE_STOCK_RESOURCE_PLANS, 
		N_("Resource Plans"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
	{	CONSUMER_GROUPS_ID, 
		GSQLE_ORACLE_STOCK_CONSUMER_GROUPS, 
		N_("Consumer Groups"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
	{	STORE_TABLES_ID, 
		GSQLE_ORACLE_STOCK_STORE_TABLES, 
		N_("Store Tables"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 }
};


/* 	
	The root_objects struct used as root tree
*/
static GSQLNavigationItem root_objects[] = {
	{	SESSION_PRIVILEGES_ID, 
		GSQLE_ORACLE_STOCK_SESSION_PRIVILEGES, 
		N_("Session Privileges"), 
		NULL, //sql_oracle_session_privileges, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_privileges,
		NULL,
		NULL, 0 },
		
	{	ENABLED_ROLES_ID,
		GSQLE_ORACLE_STOCK_ENABLED_ROLES,
		N_("Enabled Roles"), 
		NULL, //sql_oracle_enabled_roles, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_roles,
		NULL,
		NULL, 0 },
		
	{	RESOURCE_LIMITS_ID,
		GSQLE_ORACLE_STOCK_RESOURCE_LIMITS,
		N_("Resource Limits"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	TABLESPACE_QUOTAS_ID,
		GSQLE_ORACLE_STOCK_TABLESPC_QUOTAS,
		N_("Tablespace Quotas"), 
		NULL, //sql_oracle_tablespaces_quotas, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_tablespaces,
		NULL,
		NULL, 0 },
		
	{	FREE_SPACE_ID,
		GSQLE_ORACLE_STOCK_FREE_SPACE,
		N_("Free Space"), 
		NULL, //sql_oracle_tablespaces_free_space, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_tablespaces,
		NULL,
		NULL, 0 },
		
// root. my schema
	{	MY_SCHEMA_ID,
		GSQL_STOCK_MY_SCHEMA,
		N_("My Schema"), 
		NULL, NULL, 
		NULL, NULL, NULL,
		users_objects, 
		G_N_ELEMENTS (users_objects) },
		
// root. all schemas	
	{	ALL_SCHEMAS_ID,
		GSQL_STOCK_ALL_SCHEMAS,
		N_("All Schemas"), 
		NULL, //sql_oracle_all_schemas, 
		NULL,
		NULL, 
		NULL, //nav_tree_refresh_schemas,
		NULL,
		NULL, 0 },
// root. all users

	{	USERS_ID,
		GSQL_STOCK_USERS,
		N_("Users"), 
		NULL, //sql_oracle_all_users, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_users,
		NULL,
		NULL, 0 },
		
	{	ROLES_ID,
		GSQLE_ORACLE_STOCK_ROLES,
		N_("Roles"), 
		NULL, //sql_oracle_roles, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_roles,
		NULL,
		NULL, 0 },
		
	{	PROFILES_ID,
		GSQLE_ORACLE_STOCK_PROFILES,
		N_("Profiles"), NULL, NULL, NULL, NULL, NULL,
		NULL, 0 },
		
	{	TABLESPACES_ID,
		GSQLE_ORACLE_STOCK_TABLESPCS,
		N_("Tablespaces"), 
		NULL, //sql_oracle_tablespaces, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_tablespaces,
		NULL,
		NULL, 0 },
		
	{	ROLLBACK_SEGMENTS_ID,
		GSQLE_ORACLE_STOCK_ROLLBACK_SEGS,
		N_("Rollback Segments"), 
		NULL, //sql_oracle_rollbacks, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_rollbacks,
		NULL,
		NULL, 0 },
		
	{	REDO_LOG_GROUPS_ID,
		GSQLE_ORACLE_STOCK_REDO_LOG_GROUPS,
		N_("Redo Log Groups"), 
		NULL, //sql_oracle_redo_log_groups, 
		NULL, 
		NULL,
		NULL, //nav_tree_refresh_redologs,
		NULL,
		NULL, 0 },
		
// root. current instance
	{	CURRENT_INSTANCE_ID,
		GSQLE_ORACLE_STOCK_CURRENT_INSTANCE,
		N_("Current Instance"), NULL, NULL,
		NULL,//nav_tree_refresh_static, 
		NULL, NULL,
		current_instance,
		G_N_ELEMENTS(current_instance) },
		
// root. recycle bin
	{	RECYCLE_BIN_ID,
		GSQLE_ORACLE_STOCK_RECYLE_BIN,
		N_("Recycle Bin"), 
		NULL,//sql_oracle_recycle_owner, 
		NULL, 
		NULL,
		NULL,//nav_tree_refresh_recycle,
		NULL,
		NULL, 0 }
};


#endif /* _NAV_TREE_STATIC_H */

 
