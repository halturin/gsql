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
gsql_session_destroy (GtkObject *obj)
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
	// stub
	return;
}

static void
gsql_session_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GSQLSession *session = GSQL_SESSION (widget);
	GtkWidget 	*child = GTK_WIDGET (session->private->dock);
	GtkAllocation child_allocation;
	gint width, height;
	
	widget->allocation = *allocation;
	width = allocation->width - GTK_CONTAINER (widget)->border_width*2;
	height = allocation->height - GTK_CONTAINER (widget)->border_width*2;

	child_allocation.width = width;
	child_allocation.height = height;
	child_allocation.x = allocation->x;
	child_allocation.y = allocation->y;
	
	gtk_widget_size_allocate (child, &child_allocation);
	gtk_widget_show (child);
	
}

static void
gsql_session_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GSQLSession *session = GSQL_SESSION (widget);
	GtkWidget 	*child = GTK_WIDGET (session->private->dock);
	
	GtkRequisition req;
	
	gtk_widget_size_request (child, &req);
	widget->requisition.width = 0;
	widget->requisition.height = 0;
	
	widget->requisition.width = MAX (widget->requisition.width, req.width);
	widget->requisition.height = MAX (widget->requisition.height, req.height);
	
	widget->requisition.width += GTK_CONTAINER (widget)->border_width * 2;
	widget->requisition.height += GTK_CONTAINER (widget)->border_width * 2;
	
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
	
	widget_class->size_request = gsql_session_size_request;
	widget_class->size_allocate = gsql_session_size_allocate;
	
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

	session->private->dock = GDL_DOCK (gdl_dock_new ());
	session->private->layout = gdl_dock_layout_new (GDL_DOCK (session->private->dock));
	session->private->dockbar = gdl_dock_bar_new (GDL_DOCK (session->private->dock));

	session->private->left_item = gdl_dock_item_new_with_stock ("item 1", "item 1 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->left_item), gtk_button_new ());


	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->left_item), GDL_DOCK_LEFT);


	session->private->center_item = gdl_dock_item_new_with_stock ("item 2", "item 2 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->center_item), gtk_button_new ());


	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->center_item), GDL_DOCK_LEFT);

	
	session->private->right_item = gdl_dock_item_new_with_stock ("item 3", "item 3 long name", GTK_STOCK_NETWORK,
	    			GDL_DOCK_ITEM_BEH_NORMAL);
gtk_container_add (GTK_CONTAINER (session->private->right_item), gtk_button_new ());


	gdl_dock_add_item (GDL_DOCK (session->private->dock), GDL_DOCK_ITEM (session->private->right_item), GDL_DOCK_LEFT);

	gtk_widget_set_parent (GTK_WIDGET (session->private->dock), GTK_WIDGET (session));
	
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

