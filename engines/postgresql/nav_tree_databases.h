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

#ifndef NAV_TREE__DATABASES_H
#define NAV_TREE__DATABASES_H

#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/session.h>

#include "nav_tree_schemas.h"
#include "nav_tree_languages.h"

G_BEGIN_DECLS

static GSQLNavigationItem databases[] = {
	{	SCHEMAS_ID,
		GSQL_STOCK_ALL_SCHEMAS,
		N_("Schemas"), 
		sql_pgsql_all_schemas, 
		NULL,				// object_popup
		NULL,				// object_handler
		(GSQLNavigationHandler)
		nav_tree_refresh_schemas,	// expand_handler
		NULL,				// event_handler
		NULL, 0 },			// child, childs
	{	LANGUAGES_ID,
		GSQL_STOCK_TABLES,
		N_("Languages"), 
		sql_pgsql_languages, 
		NULL,				// object_popup
		NULL,				// object_handler
		(GSQLNavigationHandler) 
		nav_tree_refresh_languages,	// expand_handler
		NULL,				// event_handler
		NULL, 0 },			// child, childs
};

void
nav_tree_refresh_databases (GSQLNavigation *navigation,
			 GtkTreeView *tv,
			 GtkTreeIter *iter, guint event);
						 
void
nav_tree_databases_browse (gchar *name, gchar *owner);

G_END_DECLS

#endif /* NAV_TREE__DATABASES_H */
