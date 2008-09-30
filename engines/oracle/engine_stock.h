/***************************************************************************
 *            engine_stock.h
 *
 *  Fri Sep  7 22:36:21 2007
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
 
#ifndef _ENGINE_STOCK_H
#define _ENGINE_STOCK_H

#include <glib.h>

#define GSQLE_ORACLE_STOCK_CLUSTERS				"gsqle-oracle-clusters"
#define GSQLE_ORACLE_STOCK_COLLECTIONS_TYPES	"gsqle-oracle-collection-types"
#define GSQLE_ORACLE_STOCK_CONSUMER_GROUPS		"gsqle-oracle-consumer-groups"
#define GSQLE_ORACLE_STOCK_CURRENT_INSTANCE		"gsqle-oracle-current-instans"
#define GSQLE_ORACLE_STOCK_DATA_FILES			"gsqle-oracle-data-files"
#define GSQLE_ORACLE_STOCK_DB_LINK				"gsqle-oracle-database-links"
#define GSQLE_ORACLE_STOCK_DEBUG				"gsqle-oracle-debug"
#define GSQLE_ORACLE_STOCK_DEPENDENT			"gsqle-oracle-dependent"
#define GSQLE_ORACLE_STOCK_DEPENDS_ON			"gsqle-oracle-depends-on"
#define GSQLE_ORACLE_STOCK_DIMENSIONS			"gsqle-oracle-dimensions"
#define GSQLE_ORACLE_STOCK_DISK_GROUPS			"gsqle-oracle-disk-groups"
#define GSQLE_ORACLE_STOCK_ENABLED_ROLES		"gsqle-oracle-enabled-roles"
#define GSQLE_ORACLE_STOCK_FREE_SPACE			"gsqle-oracle-free-space"
#define GSQLE_ORACLE_STOCK_GRANTED_ROLES		"gsqle-oracle-granted-roles"
#define GSQLE_ORACLE_STOCK_INDEX_TYPES			"gsqle-oracle-index-types"
#define GSQLE_ORACLE_STOCK_INST_PROP_MODIFIABLE	"gsqle-oracle-instance-properties-modifiable"
#define GSQLE_ORACLE_STOCK_INST_PROP			"gsqle-oracle-instance-properties"
#define GSQLE_ORACLE_STOCK_JAVA_CLASSES			"gsqle-oracle-java-classes"
#define GSQLE_ORACLE_STOCK_JAVA_RESOURCES		"gsqle-oracle-java-resources"
#define GSQLE_ORACLE_STOCK_JAVA_SOURCE			"gsqle-oracle-java-source"
#define GSQLE_ORACLE_STOCK_LIBRARIES			"gsqle-oracle-libraries"
#define GSQLE_ORACLE_STOCK_OBJ_PRIVILEGES		"gsqle-oracle-object-privileges"
#define GSQLE_ORACLE_STOCK_OBJ_TYPE_BODIES		"gsqle-oracle-object-type-bodies"
#define GSQLE_ORACLE_STOCK_OBJ_TYPES			"gsqle-oracle-object-types"
#define GSQLE_ORACLE_STOCK_OPERATORS			"gsqle-oracle-operators"
#define GSQLE_ORACLE_STOCK_ORACLE				"gsqle-oracle-oracle"
#define GSQLE_ORACLE_STOCK_PACKAGE_BODIES		"gsqle-oracle-package-bodies"
#define GSQLE_ORACLE_STOCK_PACKAGE				"gsqle-oracle-packages"
#define GSQLE_ORACLE_STOCK_PACKAGE_ENTRIES		"gsqle-oracle-package-entries"
#define GSQLE_ORACLE_STOCK_PRIVILEDES			"gsqle-oracle-privileges"
#define GSQLE_ORACLE_STOCK_PROFILES				"gsqle-oracle-profiles"
#define GSQLE_ORACLE_STOCK_QUEUES				"gsqle-oracle-queues"
#define GSQLE_ORACLE_STOCK_RECYLE_BIN			"gsqle-oracle-recycle-bin"
#define GSQLE_ORACLE_STOCK_REDO_LOG_GROUPS		"gsqle-oracle-redo-log-groups"
#define GSQLE_ORACLE_STOCK_REDO_LOG_MEMBERS		"gsqle-oracle-redo-log-members"
#define GSQLE_ORACLE_STOCK_RESOURCE_LIMITS		"gsqle-oracle-resource-limits"
#define GSQLE_ORACLE_STOCK_RESOURCE_PLANS		"gsqle-oracle-resource-plans"
#define GSQLE_ORACLE_STOCK_RESOURCES			"gsqle-oracle-resources"
#define GSQLE_ORACLE_STOCK_ROLES				"gsqle-oracle-roles"
#define GSQLE_ORACLE_STOCK_ROLLBACK_SEGS		"gsqle-oracle-rollback-segments"
#define GSQLE_ORACLE_STOCK_SRV_OPTIONS			"gsqle-oracle-server-options"
#define GSQLE_ORACLE_STOCK_SRV_OUTPUT			"gsqle-oracle-server-output"
#define GSQLE_ORACLE_STOCK_SESSION_PRIVILEGES	"gsqle-oracle-session-privileges"
#define GSQLE_ORACLE_STOCK_MVIEWS				"gsqle-oracle-snapshots"
#define GSQLE_ORACLE_STOCK_MVIEW_LOGS			"gsqle-oracle-snapshot-logs"
#define GSQLE_ORACLE_STOCK_STORE_TABLES			"gsqle-oracle-store-tables"
#define GSQLE_ORACLE_STOCK_SYNONYMS				"gsqle-oracle-synonyms"
#define GSQLE_ORACLE_STOCK_SYS_PRIVILEGES		"gsqle-oracle-system-privileges"
#define GSQLE_ORACLE_STOCK_TABLESPC_QUOTAS		"gsqle-oracle-tablespace-quotas"
#define GSQLE_ORACLE_STOCK_TABLESPCS			"gsqle-oracle-tablespaces"


G_BEGIN_DECLS

void
engine_stock_init ();

G_END_DECLS

#endif /* _ENGINE_STOCK_H */

 
