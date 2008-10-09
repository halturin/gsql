/***************************************************************************
 *            engine_stock.c
 *
 *  Fri Sep  7 22:35:44 2007
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


#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgsql/stock.h>
#include <libgsql/common.h>

#include "engine_stock.h"

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLE_ORACLE_STOCK_CLUSTERS,				"clusters.png" },
	{ GSQLE_ORACLE_STOCK_COLLECTIONS_TYPES,		"collection.types.png" },
	{ GSQLE_ORACLE_STOCK_CONSUMER_GROUPS,		"consumer.groups.png" },
	{ GSQLE_ORACLE_STOCK_CURRENT_INSTANCE,		"current.instans.png" },
	{ GSQLE_ORACLE_STOCK_DATA_FILES,			"data.files.png" },
	{ GSQLE_ORACLE_STOCK_DB_LINK,				"database.links.png" },
	{ GSQLE_ORACLE_STOCK_DEBUG,					"debug.png" },
	{ GSQLE_ORACLE_STOCK_DEPENDENT,				"dependent.objects.png" },
	{ GSQLE_ORACLE_STOCK_DEPENDS_ON,			"depends.on.png" },
	{ GSQLE_ORACLE_STOCK_DIMENSIONS,			"dimensions.png" },
	{ GSQLE_ORACLE_STOCK_DISK_GROUPS,			"disk.groups.png" },
	{ GSQLE_ORACLE_STOCK_ENABLED_ROLES,			"enabled.roles.png" },
	{ GSQLE_ORACLE_STOCK_FREE_SPACE,			"free.space.png" },
	{ GSQLE_ORACLE_STOCK_GRANTED_ROLES,			"granted.roles.png" },
	{ GSQLE_ORACLE_STOCK_INDEX_TYPES,			"index.types.png" },
	{ GSQLE_ORACLE_STOCK_INST_PROP_MODIFIABLE,	"instance.properties.modifiable.png" },
	{ GSQLE_ORACLE_STOCK_INST_PROP,				"instance.properties.png" },
	{ GSQLE_ORACLE_STOCK_JAVA_CLASSES,			"java.classes.png" },
	{ GSQLE_ORACLE_STOCK_JAVA_RESOURCES,		"java.resources.png" },
	{ GSQLE_ORACLE_STOCK_JAVA_SOURCE,			"java.source.png" },
	{ GSQLE_ORACLE_STOCK_LIBRARIES,				"libraries.png" },
	{ GSQLE_ORACLE_STOCK_OBJ_PRIVILEGES,		"object.privileges.png" },
	{ GSQLE_ORACLE_STOCK_OBJ_TYPE_BODIES,		"object.type.bodies.png" },
	{ GSQLE_ORACLE_STOCK_OBJ_TYPES,				"object.types.png" },
	{ GSQLE_ORACLE_STOCK_OPERATORS,				"operators.png" },
	{ GSQLE_ORACLE_STOCK_ORACLE,				"oracle.png" },
	{ GSQLE_ORACLE_STOCK_PACKAGE_BODIES,		"package.bodies.png" },
	{ GSQLE_ORACLE_STOCK_PACKAGE,				"packages.png" },
	{ GSQLE_ORACLE_STOCK_PACKAGE_ENTRIES,		"package.entries.png" },
	{ GSQLE_ORACLE_STOCK_PRIVILEDES,			"privileges.png" },
	{ GSQLE_ORACLE_STOCK_PROFILES,				"profiles.png" },
	{ GSQLE_ORACLE_STOCK_QUEUES,				"queues.png" },
	{ GSQLE_ORACLE_STOCK_RECYLE_BIN,			"recycle.bin.png" },
	{ GSQLE_ORACLE_STOCK_REDO_LOG_GROUPS,		"redo.log.groups.png" },
	{ GSQLE_ORACLE_STOCK_REDO_LOG_MEMBERS,		"redo.log.members.png" },
	{ GSQLE_ORACLE_STOCK_RESOURCE_LIMITS,		"resource.limits.png" },
	{ GSQLE_ORACLE_STOCK_RESOURCE_PLANS,		"resource.plans.png" },
	{ GSQLE_ORACLE_STOCK_RESOURCES,				"resources.png" },
	{ GSQLE_ORACLE_STOCK_ROLES,					"roles.png" },
	{ GSQLE_ORACLE_STOCK_ROLLBACK_SEGS,			"rollback.segments.png" },
	{ GSQLE_ORACLE_STOCK_SRV_OPTIONS,			"server.options.png" },
	{ GSQLE_ORACLE_STOCK_SRV_OUTPUT,			"server.output.png" },
	{ GSQLE_ORACLE_STOCK_SESSION_PRIVILEGES,	"session.privileges.png" },
	{ GSQLE_ORACLE_STOCK_MVIEWS,				"snapshots.png" },
	{ GSQLE_ORACLE_STOCK_MVIEW_LOGS,				"snapshot.logs.png" },
	{ GSQLE_ORACLE_STOCK_STORE_TABLES,			"store.tables.png" },
	{ GSQLE_ORACLE_STOCK_SYNONYMS,				"synonyms.png" },
	{ GSQLE_ORACLE_STOCK_SYS_PRIVILEGES,		"system.privileges.png" },
	{ GSQLE_ORACLE_STOCK_TABLESPC_QUOTAS,		"tablespace.quotas.png" },
	{ GSQLE_ORACLE_STOCK_TABLESPCS,				"tablespaces.png" }
};

void
engine_stock_init ()
{
	GSQL_TRACE_FUNC;

	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));
	return;
}
