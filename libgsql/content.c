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


/*


#include <libgsql/content.h>
#include <libgsql/utils.h>
#include <libgsql/cursor.h>

struct _GSQLContentPrivate
{
	const gchar *stock;
	gchar		*name;
	gchar		*display_name;
	gboolean	changed;		// have changes
	GtkWidget	*child;

	GtkWidget	*header;	// tab header widget
	GtkImage	*header_icon;
	GtkWidget	*menuitem;	// popup menu lable
	GtkImage	*menuitem_icon; 
	GtkWidget	*close;		// close button
	
	GSQLCursor	*cursor;

};

static void gsql_content_class_init (GSQLContentClass *klass);
static void gsql_content_init (GSQLContent *obj);
static void gsql_content_finalize (GObject *obj);
static void gsql_content_destroy (GtkObject *obj);
static void on_content_close_button_activate (GtkButton *button,
											gpointer user_data);
static void gsql_content_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data);
static GType gsql_content_child_type (GtkContainer   *container);
static void gsql_content_add (GtkContainer *container, GtkWidget *widget);
static void gsql_content_size_allocate (GtkWidget *widget, 
										GtkAllocation *allocation);
static void gsql_content_size_request (GtkWidget *widget, 
									   GtkRequisition *requisition);
static void gsql_content_update_labels (GSQLContent *content);

static void on_content_status_changed (GSQLContent *content, gboolean status,
									   gpointer user_data);


static GtkWidgetClass *parent_class;

enum {
	SIG_ON_CHANGED,
	SIG_ON_CLOSE,
	
	SIG_ON_SAVE,
	SIG_ON_REVERT,
	
	SIG_LAST
};

static guint content_signals[SIG_LAST] = { 0 };


GType
gsql_content_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLContentClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_content_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLContent),
			0,
			(GInstanceInitFunc) gsql_content_init,
			NULL
		};
		obj_type = g_type_register_static (GTK_TYPE_CONTAINER,
										   "GSQLContent", &obj_info, 0);
		
	}
	
	return obj_type;
}


GSQLContent *
gsql_content_new (GSQLSession *session, const gchar *stock)
{
	GSQL_TRACE_FUNC;
	
	GSQLContent *content;
	GtkImage *image = NULL;
	GtkWidget *close;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);
	
	content = g_object_new (GSQL_CONTENT_TYPE, NULL);	
	content->session = session;
	content->private->cursor = NULL;

	if (stock)
	{
		content->private->stock = g_strdup (stock);
		image = (GtkImage *) gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
		content->private->header_icon = image;
	}

	content->private->header = gsql_utils_header_new (GTK_WIDGET (image),
									   "test",
									   NULL,
									   TRUE, 0);
	content->private->close = (GtkWidget *) g_object_get_data (G_OBJECT(content->private->header), 
															   "close_button");

	if (stock)
	{
		image = (GtkImage *) gtk_image_new_from_stock (stock, 
													GTK_ICON_SIZE_MENU);
		content->private->menuitem_icon = image;
	}

	content->private->menuitem = gsql_utils_header_new (GTK_WIDGET (image),
									   NULL, 
									   NULL,
									   FALSE, 0);

	g_signal_connect ((gpointer) content->private->close, "clicked",
						G_CALLBACK (on_content_close_button_activate),
						content);
	return content;
}


GtkWidget *
gsql_content_get_header_label (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_CONTENT (content) != NULL, NULL);
	
	return content->private->header;
}

GtkWidget *
gsql_content_get_menu_label (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_CONTENT (content), NULL);
	
	return content->private->menuitem;
}

void
gsql_content_set_child (GSQLContent *content, GtkWidget *child)
{
	GSQL_TRACE_FUNC;
		
	g_return_if_fail (GSQL_IS_CONTENT (content));
	g_return_if_fail (GTK_IS_WIDGET (child));
	
	content->private->child = child;
	
	gtk_widget_freeze_child_notify (GTK_WIDGET (content));
	gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (content));
	gtk_widget_thaw_child_notify (GTK_WIDGET (content));

}

void 
gsql_content_set_name (GSQLContent *content, gchar *name)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_CONTENT (content));
	g_return_if_fail (name != NULL);

	if (content->private->name)
		g_free (content->private->name);
	
	content->private->name = g_strdup (name);
	gsql_content_update_labels (content);

}

void 
gsql_content_set_display_name (GSQLContent *content, gchar *name)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_CONTENT (content));

	if (content->private->display_name)
	{
		GSQL_DEBUG ("content->private->display_name = [%s]", content->private->display_name);
		gsql_session_release_title (content->session, content->private->display_name);
	}

	content->private->display_name = gsql_session_get_title (content->session, name);
	gsql_content_update_labels (content);

}


void 
gsql_content_set_name_full (GSQLContent *content, gchar *name, gchar *display_name)
{
	GSQL_TRACE_FUNC;
	
	gchar *t_name;
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	if (!name)
	{
		gsql_content_set_display_name (content, NULL);
		t_name = g_strdup(content->private->display_name);
		gsql_content_set_name (content, t_name);
		
	} else {
		
		gsql_content_set_display_name (content, display_name);
		gsql_content_set_name (content, name);
	}

};

void
gsql_content_set_stock (GSQLContent *content, const gchar *stock)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (GSQL_IS_CONTENT (content));
	g_return_if_fail (stock != NULL);
	
	if (content->private->stock)
		g_free ((gchar *) content->private->stock);
	
	gtk_image_set_from_stock (content->private->header_icon, stock,
							  GTK_ICON_SIZE_MENU);
	gtk_image_set_from_stock (content->private->menuitem_icon, stock,
							  GTK_ICON_SIZE_MENU);
	content->private->stock = g_strdup (stock);

}

gchar *
gsql_content_get_stock (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
	
	return (gchar *) content->private->stock;
}

gboolean
gsql_content_get_changed (GSQLContent *content)
{
	GSQL_TRACE_FUNC;

	g_return_val_if_fail (GSQL_IS_CONTENT (content), FALSE);
	
	return content->private->changed;	
}

void
gsql_content_set_changed (GSQLContent *content, gboolean changed)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	if (content->private->changed == changed)
		return;
	
	content->private->changed = changed;
	
	gsql_content_update_labels (content);
	
	g_signal_emit_by_name (G_OBJECT (content), "changed", changed);
	
}




gchar *
gsql_content_get_display_name (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
		
	g_return_val_if_fail (content != NULL, NULL);

	return g_strdup (content->private->display_name);
}

gchar *
gsql_content_get_name (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
		
	g_return_val_if_fail (content != NULL, NULL);

	return g_strdup (content->private->name);
}

*/


/* Static section:
 *  gsql_content_class_init
 *  gsql_content_init
 *  gsql_content_finalize
 *  on_content_close_button_activate
 *  gsql_content_forall
 *  gsql_content_child_type 
 *  gsql_content_size_allocate
 *  gsql_content_size_request
 *  gsql_content_update_labels
 */

/*

static void
gsql_content_class_init (GSQLContentClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	gtkobject_class->destroy = gsql_content_destroy;
	obj_class->finalize = gsql_content_finalize;
	
	container_class->add = gsql_content_add;
	container_class->forall = gsql_content_forall;
	container_class->child_type = gsql_content_child_type;
	
	widget_class->size_request = gsql_content_size_request;
	widget_class->size_allocate = gsql_content_size_allocate;
	
	content_signals [SIG_ON_CHANGED] = 
		g_signal_new ("changed", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLContentClass,
									   on_changed),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__BOOLEAN,
					  G_TYPE_NONE, 
					  1,
					  G_TYPE_BOOLEAN);
	content_signals [SIG_ON_CLOSE] = 
		g_signal_new ("close", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLContentClass,
									   on_close),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__BOOLEAN,
					  G_TYPE_NONE, 
					  1,
					  G_TYPE_BOOLEAN);
	content_signals [SIG_ON_SAVE] = 
		g_signal_new ("save", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLContentClass,
									   on_save),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__BOOLEAN,
					  G_TYPE_NONE, 
					  1,
					  G_TYPE_BOOLEAN);
	content_signals [SIG_ON_REVERT] = 
		g_signal_new ("revert", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLContentClass,
									   on_revert),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);

}

static void 
gsql_content_init (GSQLContent *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLContentPrivate, 1);
	obj->private->changed = FALSE;
	obj->private->name = NULL;
	obj->private->display_name = NULL;
	
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (obj), GTK_NO_WINDOW);
	
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (obj), FALSE);
	
}


static void
gsql_content_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLContent *content = GSQL_CONTENT (obj);

	if (content->private->stock)
		g_free ((gchar *) content->private->stock);
	
	if (content->private->name)
		g_free (content->private->name);

	if (content->private->display_name)
		gsql_session_release_title (content->session, content->private->display_name);

	g_free (content->private);
	
	(* G_OBJECT_CLASS (parent_class)->finalize) (obj);
	
}

static void
gsql_content_destroy (GtkObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLContent *content = GSQL_CONTENT (obj);
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
	
}


static void
on_content_close_button_activate (GtkButton *button, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GSQLContent *content = GSQL_CONTENT (user_data);
	
	g_signal_emit_by_name (G_OBJECT (content), "close", FALSE);

}

static void
gsql_content_add (GtkContainer *container, GtkWidget *widget)
{
	// stub
	return;
}

static GType
gsql_content_child_type (GtkContainer   *container)
{
	return GTK_TYPE_WIDGET;
}

static void
gsql_content_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data)
{
	GSQLContent *content = GSQL_CONTENT (container);
	GtkWidget *child = GTK_WIDGET (content->private->child);
	
	g_return_if_fail (callback != NULL);
	
	if (child)
		(* callback) (child, callback_data);
	
}

static void
gsql_content_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GSQLContent *content = GSQL_CONTENT (widget);
	GtkWidget 	*child = GTK_WIDGET (content->private->child);
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
gsql_content_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GSQLContent *content = GSQL_CONTENT (widget);
	GtkWidget 	*child = GTK_WIDGET (content->private->child);
	
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
gsql_content_update_labels (GSQLContent *content)
{
	GSQL_TRACE_FUNC;
		
	GtkLabel *lbl;
	gchar *disp;
	
	if (!content->private->changed)
		pango_parse_markup (content->private->display_name, -1, 0, NULL, 
							&disp, NULL, NULL);
	else
		disp = g_strdup_printf ("<span color='red'>%s</span>", 
								content->private->display_name);

	// set tab header label
	lbl = (GtkLabel *) g_object_get_data (G_OBJECT (content->private->header),
							 "label");
	gtk_label_set_text (lbl, disp);
	// FIXME: we must set markup again o_O but why? is the bug?
		gtk_label_set_use_markup(lbl, TRUE);

	gtk_widget_set_tooltip_text (GTK_WIDGET (lbl), content->private->name);
	// set menu label
	lbl = (GtkLabel *) g_object_get_data (G_OBJECT (content->private->menuitem),
							 "label");
	gtk_label_set_text (lbl, disp);
	// FIXME: we must set markup again o_O but why? is the bug?
		gtk_label_set_use_markup(lbl, TRUE);
	
	
	g_free (disp);
	
}

*/