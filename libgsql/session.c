/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2010  Taras Halturin  halturin@gmail.com
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


#include <libgsql/common.h>
#include <libgsql/session.h>
#include <gdl/gdl.h>

struct _GSQLSessionPrivate
{
	GdlDock			*dock;
	GdlDockLayout	*layout;
	GtkWidget 		*dockbar;

	GtkWidget		*left_item;
	GtkWidget		*center_item;
	GtkWidget		*right_item;

};

enum {
	SIG_ON_CLOSE,
	
	SIG_ON_COMMIT,
	SIG_ON_ROLLBACK,
	
	SIG_LAST
};

static GtkWidgetClass *parent_class;
static guint session_signals[SIG_LAST] = { 0 };

G_DEFINE_TYPE (GSQLSession, gsql_session, GTK_TYPE_CONTAINER);

static void
gsql_session_destroy (GObject *obj)
{
	GSQLSession *session = GSQL_SESSION (obj);
	
	//GTK_CONTAINER_CLASS (gsql_session_parent_class)->destroy (obj);
}


static void 
gsql_session_finalize (GObject *obj)
{
	GSQLSession *session = GSQL_SESSION (obj);
	
	//GTK_CONTAINER_CLASS (gsql_session_parent_class)->finalize (obj);;
}


static void 
gsql_session_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data)
{
	GSQLSession *session = GSQL_SESSION (container);
	GtkWidget *child = GTK_WIDGET (session->private->dock);
	
	g_return_if_fail (callback != NULL);
	
	if (child)
		(* callback) (child, callback_data);

}
static GType gsql_session_child_type (GtkContainer   *container)
{
	return GTK_TYPE_WIDGET;
}

static void gsql_session_add (GtkContainer *container, GtkWidget *widget)
{

}



static void
gsql_session_class_init (GSQLSessionClass *klass)
{
	GSQL_TRACE_FUNC

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	gtkobject_class->destroy = gsql_session_destroy;
	obj_class->finalize = gsql_session_finalize;
	
	container_class->add = gsql_session_add;
	container_class->forall = gsql_session_forall;
	container_class->child_type = gsql_session_child_type;
	
	widget_class->size_request = gtk_widget_size_request;
	widget_class->size_allocate = gtk_widget_size_allocate;
	
	session_signals [SIG_ON_CLOSE] = 
		g_signal_new ("close", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_close),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__BOOLEAN,
					  G_TYPE_NONE, 
					  1,
					  G_TYPE_BOOLEAN);
	session_signals [SIG_ON_COMMIT] = 
		g_signal_new ("commit", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_commit),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	session_signals [SIG_ON_ROLLBACK] = 
		g_signal_new ("rollback", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_rollback),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
}

static void
gsql_session_init (GSQLSession *session)
{
	GSQL_TRACE_FUNC


	session->private = g_slice_new0 (GSQLSessionPrivate);
	session->private->dock = gdl_dock_new ();
	session->private->layout = gdl_dock_layout_new (GDL_DOCK (session->private->dock));
	session->private->dockbar = gdl_dock_bar_new (GDL_DOCK (session->private->dock));

	session->private->left_item = gdl_dock_item_new_with_stock ("item 1", "item 1 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->left_item), gtk_button_new ());

	g_debug ("aaaa");
	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->left_item), GDL_DOCK_LEFT);
	g_debug ("bbbb");

	session->private->center_item = gdl_dock_item_new_with_stock ("item 2", "item 2 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->center_item), gtk_button_new ());

	g_debug ("cccc");
	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->center_item), GDL_DOCK_LEFT);
	g_debug ("dddd");
	
	session->private->right_item = gdl_dock_item_new_with_stock ("item 3", "item 3 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->right_item), gtk_button_new ());

	g_debug ("eeee");
	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->right_item), GDL_DOCK_LEFT);
	g_debug ("ffff");

	gtk_container_add (GTK_CONTAINER (session), session->private->dock);
	
	//gtk_widget_show_all (GTK_WIDGET (session));

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (session), GTK_NO_WINDOW);
	
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (session), FALSE);
}

gboolean
gsql_session_lock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), FALSE);
	
	return TRUE;	
}

void
gsql_session_unlock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	
	
}

