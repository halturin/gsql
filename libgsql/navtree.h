/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2009  Taras Halturin  halturin@gmail.com
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

 
#ifndef _NAVTREE_H
#define _NAVTREE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GSQLNavTree GSQLNavTree;
typedef struct _GSQLNavTreeClass GSQLNavTreeClass;
typedef struct _GSQLNavTreePrivate GSQLNavTreePrivate;



#define GSQL_NAVTREE_TYPE 			(gsql_navtree_get_type ())
#define GSQL_NAVTREE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_NAVTREE_TYPE, GSQLNavTree))
#define GSQL_NAVTREE_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_NAVTREE_TYPE, GSQLNavTreeClass))

#define GSQL_IS_NAVTREE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_NAVTREE_TYPE))
#define GSQL_IS_NAVTREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_NAVTREE_TYPE))
#define GSQL_NAVTREE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQL_NAVTREE_TYPE, GSQLNavTreeClass))


#define GSQL_NAVTREE_REGISTER_ID(ItemID)  \
	g_quark_from_string ( ItemID )
	

#define GSQL_NAVTREE_GET_ID_BY_NAME(ItemName) \
	g_quark_try_string (ItemName)


#define GSQL_NAVTREE_GET_NAME_BY_ID(ItemID) \
	g_quark_to_string (ItemID)


struct _GSQLNavTree
{
	GObject parent;

	// item id
	gint id;

	// stock and name of the item
	const gchar *stock_name;
	const gchar *name;

	// if hasn't a child the tree will build from SQL
	GList *queries;
	// child id for query's items
	guint child_id;
		
	// set of GSQLNavTree childs
	GList *childs;	

};

struct _GSQLNavTreeClass
{
	GObjectClass parent;
	
	/* Signals */
	void (*on_object_open) (GSQLNavTree *navtree);
	void (*on_object_expand) (GSQLNavTree *navtree);
	void (*on_object_event) (GSQLNavTree *navtree);
	void (*on_object_popup) (GSQLNavTree *navtree);
	
};





GType gsql_navtree_get_type (void);

GSQLNavTree *
gsql_navtree_new ();

G_END_DECLS

#endif /* _NAVTREE_H */

 
