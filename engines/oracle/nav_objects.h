/***************************************************************************
 *            nav_objects.h
 *
 *  Mon Sep 17 00:41:07 2007
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
 
#ifndef _NAV_OBJECTS_H
#define _NAV_OBJECTS_H

#include <glib.h>

enum {
	
	SESSION_PRIVILEGES_ID = NAV_TREE_ROOT_ID + 1,
	SESSION_PRIVILEGE_ID,
	
	ENABLED_ROLES_ID,
	ENABLED_ROLE_ID,
	
	RESOURCE_LIMITS_ID,
	TABLESPACE_QUOTAS_ID,
	FREE_SPACE_ID,
	MY_SCHEMA_ID,
	ALL_SCHEMAS_ID,
	
	USERS_ID,
	USER_ID,
	
	ROLES_ID,
	ROLE_ID,
	
	PROFILES_ID,
	PROFILE_ID,
	
	TABLESPACES_ID,
	DATA_FILES_ID,
	DISK_GROUPS_ID,
	ROLLBACK_SEGMENTS_ID,
	
	REDO_LOG_GROUPS_ID,
	REDO_LOG_GROUP_ID,
	REDO_LOG_GROUP_MEMBERS_ID,
	
	CURRENT_INSTANCE_ID,
	RECYCLE_BIN_ID,

	GRANTED_ROLES_ID,
	GRANTED_ROLE_ID,
	
	SYSTEM_PRIVILEGES_ID,
	SYSTEM_PRIVILEGE_ID,
	
	OBJECT_PRIVILEGES_ID,
	OBJECT_PRIVILEGE_ID,
	
	TABLES_ID,
	TABLE_ID,
	TABLE_PARTITIONS_ID,
	TABLE_PARTITION_ID,


	COLUMNS_ID,
	COLUMN_ID,
	
	MVIEWS_LOG_ID,

	CONSTRAINTS_ID,
	CONSTRAINT_ID,
	
	VIEWS_ID,
	VIEW_ID,
	
	INDEXES_ID,
	INDEX_ID,
	INDEX_PARTITIONS_ID,
	INDEX_PARTITION_ID,
	
	TRIGGERS_ID,
	TRIGGER_ID,
	
	DEPENDSON_ID,
	DEPENDENT_ID,
	
	ARGUMENTS_ID,
	ARGUMENT_ID,
	
	PROCEDURES_ID,
	PROCEDURE_ID,
	
	FUNCTIONS_ID,
	FUNCTION_ID,
	
	PACKAGES_ID,
	PACKAGE_ID,
	
	PACKAGE_BODIES_ID,
	PACKAGE_BODY_ID,
	
	ENTRIES_ID,
	
	SEQUENCES_ID,
	SEQUENCE_ID,
	
	CLUSTERS_ID,
	CLUSTER_ID,
	
	MVIEWS_ID,
#define SNAPSHOTS_ID MVIEWS_ID
	MVIEW_ID,
#define SNAPSHOT_ID MVIEW_ID
	
	SYNONYMS_ID,
	SYNONYM_ID,
	
	DATABASE_LINKS_ID,
	DATABASE_LINK_ID,
	
	OBJECT_TYPES_ID,
	OBJECT_TYPE_ID,
	OBJECT_TYPE_BODIES_ID,
	OBJECT_TYPE_BODY_ID,
	
	COLLECTION_TYPES_ID,
	
	LIBRARIES_ID,
	LIBRARY_ID,
	
	QUEUES_ID,
	QUEUE_ID,
	
	JAVA_SOURCES_ID,
	JAVA_CLASSES_ID,
	JAVA_RESOURCES_ID,
	JAVA_SOURCE_ID,
	JAVA_CLASS_ID,
	JAVA_RESOURCE_ID,
	
	INDEX_TYPES_ID,
	INDEX_TYPE_ID,
	
	OPERATORS_ID,
	OPERATOR_ID,
	
	DIRECTORY_ID,

	DIMENSIONS_ID,
	DIMENSION_ID,

	RESOURCE_PLANS_ID,
	RESOURCE_PLAN_ID,

	CONSUMER_GROUPS_ID,
	CONSUMER_GROUP_ID,

	STORE_TABLES_ID,
	STORE_TABLE_ID,
	
	INSTANCE_PROPERTIES_ID,
	SERVER_OPTIONS_ID
};

G_BEGIN_DECLS

void oracle_navigation_fill_details (GSQLCursor *cursor, GtkListStore *store);

G_END_DECLS

#endif /* _NAV_OBJECTS_H */

 
