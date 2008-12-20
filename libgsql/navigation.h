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

 
#ifndef _NAVIGATION_H
#define _NAVIGATION_H

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _GSQLNavigation		GSQLNavigation;
typedef struct _GSQLNavigationClass GSQLNavigationClass;
typedef struct _GSQLNavigationPrivate   GSQLNavigationPrivate;
typedef struct _GSQLNavigationItem  GSQLNavigationItem;

#define GSQL_NAVIGATION_TYPE 			(gsql_navigation_get_type ())
#define GSQL_NAVIGATION(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_NAVIGATION_TYPE, GSQLNavigation))
#define GSQL_NAVIGATION_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_NAVIGATION_TYPE, GSQLNavigationClass))

#define GSQL_IS_NAVIGATION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_NAVIGATION_TYPE))
#define GSQL_IS_NAVIGATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_NAVIGATION_TYPE))

typedef void (*GSQLNavigationHandler) (GSQLNavigation *, GtkTreeView*, 
										GtkTreeIter *, guint);


typedef enum
{
	GSQL_NAV_TREE_ID = 0,
	GSQL_NAV_TREE_OWNER,
	GSQL_NAV_TREE_IMAGE,
	GSQL_NAV_TREE_NAME,
	GSQL_NAV_TREE_REALNAME,
	GSQL_NAV_TREE_ITEM_INFO,
	GSQL_NAV_TREE_SQL,
	GSQL_NAV_TREE_OBJECT_POPUP,
	GSQL_NAV_TREE_OBJECT_HANDLER,
	GSQL_NAV_TREE_EXPAND_HANDLER,
	GSQL_NAV_TREE_EVENT_HANDLER,
	GSQL_NAV_TREE_STRUCT,
	GSQL_NAV_TREE_DETAILS,
	GSQL_NAV_TREE_NUM_ITEMS,
	GSQL_NAV_TREE_N_COLUMNS
} GSQLNavigationTreeItem;

typedef enum
{
	GSQL_NAV_DETAILS_STOCK = 0,
	GSQL_NAV_DETAILS_NAME,
	GSQL_NAV_DETAILS_VALUE,
	GSQL_NAV_DETAILS_N_COLUMNS
} GSQLNavigationDetailsItem;

struct _GSQLNavigation
{
	GtkContainer parent;

	GSQLNavigationPrivate *private;
};

struct _GSQLNavigationClass
{
	GtkContainerClass   parent;

	/* signals */
	void (*on_expand)   (GSQLNavigation *navigation);
	void (*on_activate) (GSQLNavigation *navigation);
	
};


struct _GSQLNavigationItem
{
	gint 				id;
	gchar 				*stock_name;
	gchar				*name;
	const gchar			*sql;
	GSQLNavigationHandler	object_popup;
	GSQLNavigationHandler	object_handler;
	GSQLNavigationHandler	expand_handler;
	GSQLNavigationHandler	event_handler;
	GSQLNavigationItem	*child;
	gint				childs;
};

#define NAV_TREE_ROOT_ID 0

G_BEGIN_DECLS

GType gsql_navigation_get_type (void);

GSQLNavigation *
gsql_navigation_new (GSQLSession *session);

void
gsql_navigation_set_root (GSQLNavigation *navigation, gchar *stock, gchar *owner,
							GSQLNavigationItem *items, gint len);

GtkListStore *
gsql_navigation_get_details (GSQLNavigation *navigation, gchar *key);

guint
gsql_navigation_menu_merge (GSQLNavigation *navigation, gchar *ui, 
							GtkActionGroup *actions);

GtkAction *
gsql_navigation_get_action (GSQLNavigation *navigation, gchar *action);

void
gsql_navigation_menu_popup (GSQLNavigation *navigation, GtkActionGroup *action);

GtkTreeIter *
gsql_navigation_get_active_iter (GSQLNavigation *navigation);

GtkTreeModel *
gsql_navigation_get_model (GSQLNavigation* navigation);

G_END_DECLS

#endif /* _NAVIGATION_H */

 
