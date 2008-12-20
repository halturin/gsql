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


#include "session.h"
#include "navigation.h"
#include "common.h"

#include <gdk/gdkkeysyms.h>

extern GtkWidget *statusbar;

struct _GSQLNavigationPrivate
{
	GtkWidget		*root;
	GSQLSession		*session;
	GtkTreeStore	*navstore;
	GtkTreeView		*navtree;
	GHashTable		*details_hash;
	
	GtkAccelGroup   *accel;
	GtkUIManager	*ui;
	GtkWidget		*menu;
	
	gint reserved;
};

static void gsql_navigation_class_init (GSQLNavigationClass *klass);
static void gsql_navigation_init (GSQLNavigation *obj);
static void gsql_navigation_destroy (GtkObject *obj);
static void gsql_navigation_finalize (GObject *obj);
static void gsql_navigation_size_request (GtkWidget *widget, 
										 GtkRequisition *requisition);
static void gsql_navigation_size_allocate (GtkWidget *widget, 
										  GtkAllocation *allocation);
static void gsql_navigation_forall (GtkContainer *container,
									gboolean      include_internals,
									GtkCallback   callback,
									gpointer      callback_data);

static void on_navigation_expand (GtkTreeView *tv, GtkTreeIter *iter,
								  GtkTreePath *path, gpointer data);
static void on_navigation_activate (GtkTreeView *tv, GtkTreePath *path,
									GtkTreeViewColumn *col, gpointer data);
static void on_navigation_cursor_changed (GtkTreeView *tv,
										  gpointer user_data);
static gboolean on_navigation_button_press (GtkWidget *tv, GdkEventButton *event,
										gpointer data);
static gboolean on_navigation_key_press (GtkWidget *tv, GdkEventKey *event, 
									 gpointer data);
static gboolean on_navigation_popup_menu (GtkWidget *tv, void *data);

static void details_hash_remove_key_notify (gpointer data);
static void details_hash_remove_value_notify (gpointer data);

static void on_navigation_popup_copyname (GtkMenuItem *menuitem, 
											gpointer user_data);

static void on_navigation_popup_refresh (GtkMenuItem *menuitem, 
											gpointer user_data);

static void ui_connect_proxy_cb (GtkUIManager *manager,
								 GtkAction *action,
								 GtkWidget *proxy,
								 gpointer userdata);

static void ui_disconnect_proxy_cb (GtkUIManager *manager,
								 GtkAction *action,
								 GtkWidget *proxy,
								 gpointer userdata);

static GtkWidgetClass *parent_class;

static gchar navigation_ui[] = 
" <ui> "
"  <popup name=\"NavObjects\" action=\"ActionNavObjects\"> "
"  		<placeholder name=\"PHolderNavObjectDo\" /> "
"  		<separator name=\"Separator1\" /> "
"  		<menuitem name=\"NavObjectCopyName\" action=\"NavObjectActionCopyName\" /> "
"  		<menuitem name=\"NavObjectRefresh\" action=\"NavObjectActionRefresh\" /> "
"  </popup> "
"</ui>";

static GtkActionEntry navigation_acts[] = 
{
	{ "NavObjectActionCopyName", NULL, 
		N_("Copy name"), NULL, 
		N_("Copy this name  [ Ctrl+C ]"), 
		G_CALLBACK (on_navigation_popup_copyname) },
	
	{ "NavObjectActionRefresh", GTK_STOCK_REFRESH, 
		N_("Refresh"), NULL, 
		N_("Refresh the current list of objects [ F5 ]"), 
		G_CALLBACK (on_navigation_popup_refresh) }
};


enum {
	SIG_ON_ACTIVATE,
	SIG_ON_EXPAND,
	SIG_LAST
};


static gchar nav_tooltips[] = "Refresh			<b>F5</b>";


static guint navigation_signals[SIG_LAST] = { 0 };

GType
gsql_navigation_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLNavigationClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_navigation_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLNavigation),
			0,
			(GInstanceInitFunc) gsql_navigation_init,
			NULL
		};
		obj_type = g_type_register_static (GTK_TYPE_CONTAINER,
										   "GSQLNavigation", &obj_info, 0);
		
	}
	
	return obj_type;	
}


static GType
gsql_navigation_child_type (GtkContainer   *container)
{
	return GTK_TYPE_WIDGET;
}

#if (GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12)

static gboolean
query_tooltip_tree_view_cb (GtkWidget  *widget,
							gint        x,
							gint        y,
							gboolean    keyboard_tip,
							GtkTooltip *tooltip,
							gpointer    data)
{
	GtkTreeIter iter;
	GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
	GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
	GtkTreePath *path = NULL;
	gchar *tmp;

	char buffer[512];

	if (!gtk_tree_view_get_tooltip_context (tree_view, &x, &y,
											keyboard_tip,
											&model, &path, &iter))
		return FALSE;

	gtk_tree_model_get (model, &iter, GSQL_NAV_TREE_NAME, &tmp, -1);
	
	g_snprintf (buffer, 511, "%s", tmp);
	gtk_tooltip_set_markup (tooltip, buffer);
	
	gtk_tree_view_set_tooltip_row (tree_view, tooltip, path);
	
	gtk_tree_path_free (path);
	g_free (tmp);

	return TRUE;
}

#endif

GSQLNavigation *
gsql_navigation_new (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeStore *navstore;
	GtkTreeView	 *navtree;
	GSQLNavigation *navigation;
	GtkTreeViewColumn 	*column;
	GtkCellRenderer 	*renderer;
	GtkWidget			*scroll;
	
	g_return_val_if_fail (GSQL_SESSION (session) != NULL, NULL);
	
	navigation = g_object_new (GSQL_NAVIGATION_TYPE, NULL);
	
	navstore = gtk_tree_store_new (GSQL_NAV_TREE_N_COLUMNS,
							G_TYPE_INT, 	// GSQL_NAV_TREE_ID
							G_TYPE_STRING,  // GSQL_NAV_TREE_OWNER
							G_TYPE_STRING,  // GSQL_NAV_TREE_IMAGE
							G_TYPE_STRING,  // GSQL_NAV_TREE_NAME
							G_TYPE_STRING,  // GSQL_NAV_TREE_REALNAME
							G_TYPE_STRING,  // GSQL_NAV_TREE_ITEM_INFO
							G_TYPE_POINTER, // GSQL_NAV_TREE_SQL
							G_TYPE_POINTER,	// GSQL_NAV_TREE_OBJECT_POPUP
							G_TYPE_POINTER, // GSQL_NAV_TREE_OBJECT_HANDLER
							G_TYPE_POINTER, // GSQL_NAV_TREE_EXPAND_HANDLER
							G_TYPE_POINTER, // GSQL_NAV_TREE_EVENT_HANDLER
							G_TYPE_POINTER, // GSQL_NAV_TREE_STRUCT
							G_TYPE_POINTER, // GSQL_NAV_TREE_DETAILS
							G_TYPE_INT		// GSQL_NAV_TREE_NUM_ITEMS
						);
	
	
	navtree = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (navstore)));
	gtk_tree_view_set_rules_hint(navtree, TRUE);
	
#if (GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12)
	g_object_set (G_OBJECT(navtree), "has-tooltip", TRUE, NULL);
	g_signal_connect (navtree, "query-tooltip",
                    G_CALLBACK (query_tooltip_tree_view_cb), NULL);
#endif
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_title (column, N_("Sessions"));
	gtk_tree_view_append_column(navtree, column);	
	gtk_tree_view_set_headers_visible(navtree, FALSE);
	
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer,
											"stock-id", GSQL_NAV_TREE_IMAGE);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
											"markup", GSQL_NAV_TREE_NAME);	
	gtk_tree_view_set_search_column (navtree, GSQL_NAV_TREE_NAME);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
											"markup", GSQL_NAV_TREE_ITEM_INFO);
        
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);	

	gtk_container_add (GTK_CONTAINER (scroll),
					   GTK_WIDGET(navtree));
	
	
	navigation->private->root = scroll;
	navigation->private->navtree = navtree;
	navigation->private->navstore = navstore;
	navigation->private->session = session;
	
	gtk_widget_freeze_child_notify (scroll);
	gtk_widget_set_parent (scroll, GTK_WIDGET (navigation));
	gtk_widget_thaw_child_notify (scroll);
	gtk_widget_show_all (GTK_WIDGET (navigation));
	
	return navigation;
}


void
gsql_navigation_set_root (GSQLNavigation *navigation, gchar *stock, gchar *owner,
						  GSQLNavigationItem *items, gint len)
{
	GSQL_TRACE_FUNC;
	
	gchar *session_name;
	GtkTreeStore *ts;
	GtkTreeIter toplevel, child;
	GtkTreeIter *iter;
	g_return_if_fail (navigation != NULL);
	g_return_if_fail (len > 0);
	g_return_if_fail (items != NULL);
	

	session_name = gsql_session_get_name (navigation->private->session);
	GSQL_DEBUG ("current session_name: [%s]", session_name);
	
	ts = navigation->private->navstore;
	gtk_tree_store_clear (ts);
	
	gtk_tree_store_append (ts, &toplevel, NULL);
	
	gtk_tree_store_set(ts, &toplevel,
				GSQL_NAV_TREE_ID,			NAV_TREE_ROOT_ID,
				GSQL_NAV_TREE_IMAGE,		stock,
				GSQL_NAV_TREE_OWNER,		owner,
				GSQL_NAV_TREE_NAME,			session_name,
				GSQL_NAV_TREE_REALNAME,		session_name,
				GSQL_NAV_TREE_ITEM_INFO,	NULL,
				GSQL_NAV_TREE_SQL,			NULL,
				GSQL_NAV_TREE_OBJECT_POPUP, NULL,
				GSQL_NAV_TREE_OBJECT_HANDLER,	NULL,
				GSQL_NAV_TREE_EXPAND_HANDLER,	NULL,
				GSQL_NAV_TREE_EVENT_HANDLER, NULL,
				GSQL_NAV_TREE_STRUCT,	items,
				GSQL_NAV_TREE_NUM_ITEMS, 	len,
				-1);

	/* create a child for able to expand child items */
	gtk_tree_store_append (ts, &child, &toplevel);
	
	gtk_tree_store_set (ts, &child,
                	GSQL_NAV_TREE_ID,				-1,
                	GSQL_NAV_TREE_IMAGE, 			NULL,
                	GSQL_NAV_TREE_NAME,    			N_("Processing..."),
					GSQL_NAV_TREE_REALNAME,			NULL,
					GSQL_NAV_TREE_ITEM_INFO, 		NULL,
					GSQL_NAV_TREE_SQL, 				NULL,
					GSQL_NAV_TREE_OBJECT_POPUP,		NULL,
					GSQL_NAV_TREE_OBJECT_HANDLER,	NULL,
					GSQL_NAV_TREE_EXPAND_HANDLER,	NULL,
					GSQL_NAV_TREE_EVENT_HANDLER,	NULL,
					GSQL_NAV_TREE_STRUCT,	NULL,
					GSQL_NAV_TREE_NUM_ITEMS, 		0,
				-1);
	
	g_signal_connect(G_OBJECT(navigation->private->navtree), "row-expanded",
							G_CALLBACK (on_navigation_expand),
							navigation);
	g_signal_connect(G_OBJECT(navigation->private->navtree), "row-activated",
							G_CALLBACK (on_navigation_activate),
							navigation);
	g_signal_connect (G_OBJECT(navigation->private->navtree), "cursor-changed",
					  	G_CALLBACK(on_navigation_cursor_changed),
					    navigation);
	g_signal_connect (G_OBJECT(navigation->private->navtree), "event-after",
						G_CALLBACK (on_navigation_button_press),
						navigation);
	g_signal_connect (G_OBJECT(navigation->private->navtree), "key-press-event",
						G_CALLBACK (on_navigation_key_press),
						navigation);
	g_signal_connect (G_OBJECT(navigation->private->navtree), "popup-menu",
						G_CALLBACK (on_navigation_popup_menu),
						navigation);
	gtk_tree_view_expand_to_path (navigation->private->navtree, 
								  gtk_tree_path_new_first());
	
}

GtkListStore *
gsql_navigation_get_details (GSQLNavigation *navigation, gchar *key)
{
	GSQL_TRACE_FUNC;

	GtkListStore *details = NULL;
	
	g_return_val_if_fail (GSQL_IS_NAVIGATION (navigation), NULL);
	
	details = (GtkListStore *) g_hash_table_lookup (navigation->private->details_hash, 
												   key);
	if (details != NULL)
		return details;

	details = gtk_list_store_new(3, 
								G_TYPE_STRING, // stock id
								G_TYPE_STRING, // name/attribute
								G_TYPE_STRING);  // comment/value
	
	g_hash_table_insert (navigation->private->details_hash, g_strdup(key), details);
	
	return details;
}


guint
gsql_navigation_menu_merge (GSQLNavigation *navigation, gchar *ui, GtkActionGroup *actions)
{
	GSQL_TRACE_FUNC;
	
	GError **error;
	guint ret = 0;
	
	gtk_ui_manager_insert_action_group (navigation->private->ui, actions, 0);
	ret = gtk_ui_manager_add_ui_from_string (navigation->private->ui, ui, -1, error);
	
	if (ret)
	{
		gtk_ui_manager_ensure_update (navigation->private->ui);
		return ret;
		
	} else	{
		
		gtk_ui_manager_remove_action_group (navigation->private->ui, actions);
	}
	
	return ret;
}

GtkAction *
gsql_navigation_get_action (GSQLNavigation *navigation, gchar *action)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_NAVIGATION (navigation), NULL);
	
	return gtk_ui_manager_get_action (navigation->private->ui, action);
}

void
gsql_navigation_menu_popup (GSQLNavigation *navigation, GtkActionGroup *action)
{
	GSQL_TRACE_FUNC;
	
	static GtkActionGroup *last_action = NULL;
	
	GSQL_DEBUG ("Action[0x%x] LastAction[0x%x]", action, last_action);
	
	if ((last_action != action) && (last_action != NULL))
		gtk_action_group_set_visible (last_action, FALSE);
	
	if (action)
		gtk_action_group_set_visible (action, TRUE);
	
	gtk_menu_popup (GTK_MENU (navigation->private->menu), NULL, NULL, 
				   NULL, NULL, 3, gtk_get_current_event_time());
	
	last_action = action;
	
}

GtkTreeIter *
gsql_navigation_get_active_iter (GSQLNavigation *navigation)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	
	g_return_val_if_fail (GSQL_IS_NAVIGATION (navigation), NULL);
	
	selection = gtk_tree_view_get_selection (navigation->private->navtree);
	if (!gtk_tree_selection_get_selected (selection,
										  (GtkTreeModel **)(&navigation->private->navstore),
										  &iter))
		// have no selection
		return NULL;
	
	return gtk_tree_iter_copy (&iter);
}

GtkTreeModel *
gsql_navigation_get_model (GSQLNavigation *navigation)
{
	GSQL_TRACE_FUNC;
	
	return GTK_TREE_MODEL (navigation->private->navstore);	
}

/*
 *	Static section:
 *	gsql_navigation_size_request
 *	gsql_navigation_size_allocate
 *	gsql_navigation_forall
 *	gsql_navigation_init
 *	gsql_navigation_class_init
 *	gsql_navigation_finalize
 *	gsql_navigation_destroy
 
 *  details_hash_remove_key_notify
 *  details_hash_remove_value_notify
 
 *  Callbacks:
 *  on_navigation_expand
 *  on_navigation_cursor_changed
 *  on_navigation_button_press
 *  on_navigation_key_press
 *  on_navigation_popup_menu
 *  on_navigation_popup_copyname 
 *  on_navigation_popup_refresh
 *  ui_connect_proxy_cb
 *  ui_disconnect_proxy_cb
 */


static void
gsql_navigation_destroy (GtkObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLNavigation *navigation = GSQL_NAVIGATION (obj);
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
	
}

static void
gsql_navigation_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLNavigation *navigation = GSQL_NAVIGATION (obj);
	
	if (navigation->private->details_hash)
		g_hash_table_unref (navigation->private->details_hash);
	
	g_object_unref (navigation->private->ui);
	
	g_free (navigation->private);
	
	(* G_OBJECT_CLASS (parent_class)->finalize) (obj);
	
}

static void
gsql_navigation_class_init (GSQLNavigationClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	
	obj_class->finalize = gsql_navigation_finalize;
	
	gtkobject_class->destroy = gsql_navigation_destroy;
	
	widget_class->size_request = gsql_navigation_size_request;
	widget_class->size_allocate = gsql_navigation_size_allocate;
	
	container_class->forall = gsql_navigation_forall;
	container_class->child_type = gsql_navigation_child_type;
	
	navigation_signals [SIG_ON_ACTIVATE] = 
		g_signal_new ("on-activate", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLNavigationClass,
									   on_activate),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	navigation_signals [SIG_ON_EXPAND] = 
		g_signal_new ("on-expand", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLNavigationClass,
									   on_expand),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	gtkobject_class->destroy = gsql_navigation_destroy;
	
}

static void 
gsql_navigation_init (GSQLNavigation *obj)
{
	GSQL_TRACE_FUNC;
	
	GError **error;
	GtkActionGroup *action;

	g_return_if_fail (obj != NULL);
	obj->private = g_new0 (GSQLNavigationPrivate, 1);
	
	obj->private->details_hash = g_hash_table_new_full (g_str_hash, 
											  g_str_equal,
											  details_hash_remove_key_notify,
											  details_hash_remove_value_notify);
	obj->private->ui = gtk_ui_manager_new ();
	
	g_signal_connect (obj->private->ui, "connect_proxy",
					  G_CALLBACK (ui_connect_proxy_cb),
					  NULL);
	
	g_signal_connect (obj->private->ui, "disconnect_proxy",
					  G_CALLBACK (ui_disconnect_proxy_cb),
					  NULL);
	
	action = gtk_action_group_new ("NavObjectActions");
	gtk_action_group_add_actions (action, navigation_acts, 
								  G_N_ELEMENTS (navigation_acts), NULL);
	gtk_ui_manager_insert_action_group (obj->private->ui, action, 0);
	
	
	gtk_ui_manager_add_ui_from_string (obj->private->ui, navigation_ui,
											 -1, error);
	gtk_ui_manager_ensure_update (obj->private->ui);
	obj->private->menu = gtk_ui_manager_get_widget (obj->private->ui, "/NavObjects");
	
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (obj), GTK_NO_WINDOW);
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (obj), FALSE);

}

static void
gsql_navigation_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GSQLNavigation *navigation = GSQL_NAVIGATION (widget);
	GtkWidget *child = GTK_WIDGET (navigation->private->root);
	
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
gsql_navigation_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GSQLNavigation *navigation = GSQL_NAVIGATION (widget);
	GtkWidget *child = GTK_WIDGET (navigation->private->root);
	
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
gsql_navigation_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data)
{
	GSQLNavigation *navigation = GSQL_NAVIGATION (container);
	GtkWidget *child = GTK_WIDGET (navigation->private->root);
	
	g_return_if_fail (callback != NULL);
	
	if (child)
		(* callback) (child, callback_data);

}

static void 
on_navigation_expand (GtkTreeView *tv, GtkTreeIter *iter,
		       GtkTreePath *path, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLNavigation *navigation = data;
	GSQLNavigationItem *expand_struct;
	GSQLNavigationHandler expand_handler;
	GtkTreeModel *model;
	gint id = 0;
	gint n = 0;
	gint n_items = 0;
	GtkTreeIter  child;
	GtkTreeIter child_fake;
	GtkTreeIter	child_last;
	gchar *mark;
	gchar *esc;
	gchar *owner = NULL;
	
	
	model = GTK_TREE_MODEL (navigation->private->navstore);

	gtk_tree_model_get (model, iter,
						GSQL_NAV_TREE_EXPAND_HANDLER, 
						&expand_handler, -1);
	
	if (expand_handler)
	{
		// get only first child iter.
		if (!gtk_tree_model_iter_nth_child (model, &child, iter, 0))
			return;
		
		// get ID of the object. 
		gtk_tree_model_get (model, &child,  
						GSQL_NAV_TREE_ID, 
						&id, -1);
		if (id < 0)
		// Only if "Processed..." item with ID=-1 
		// ...or manual set ID=-1 for refresh by F5 button.
			expand_handler (data, tv, iter, 0);
		
		return;
	}
	
	gtk_tree_model_get (model, iter,
						GSQL_NAV_TREE_STRUCT, 
						&expand_struct, -1);
	if (expand_struct)
	{
		n = gtk_tree_model_iter_n_children(model, iter);
		
		for (; n>1; n--)
		{
			gtk_tree_model_iter_children(model, &child, iter);
			gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
		}
		
		gtk_tree_model_iter_children(model, &child_last, iter);

		gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_NUM_ITEMS, 
						&n_items, -1);
		
		if (n_items <= 0)
			GSQL_DEBUG ("error: You must set length of GSQLNavigationItem array. Do nothing");
		
		gtk_tree_model_get (model, iter,  
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);
		
		for (n = 0; n < n_items; n++)
		{
			esc = g_markup_escape_text(expand_struct[n].name, -1);
			mark = g_strdup_printf("<span weight='bold'>%s</span>", esc);
			
			gtk_tree_store_append (GTK_TREE_STORE (model), &child, iter);
			
			gtk_tree_store_set (GTK_TREE_STORE (model), &child,
								GSQL_NAV_TREE_ID,				expand_struct[n].id,
								GSQL_NAV_TREE_IMAGE,			expand_struct[n].stock_name,
								GSQL_NAV_TREE_OWNER, 			owner,
								GSQL_NAV_TREE_NAME,				mark,
								GSQL_NAV_TREE_REALNAME,			mark,
								GSQL_NAV_TREE_ITEM_INFO,		NULL,
								GSQL_NAV_TREE_SQL,				expand_struct[n].sql,
								GSQL_NAV_TREE_OBJECT_POPUP,		expand_struct[n].object_popup,
								GSQL_NAV_TREE_OBJECT_HANDLER,	expand_struct[n].object_handler,
								GSQL_NAV_TREE_EXPAND_HANDLER,	expand_struct[n].expand_handler,
								GSQL_NAV_TREE_EVENT_HANDLER,	expand_struct[n].event_handler,
								GSQL_NAV_TREE_STRUCT,			expand_struct[n].child,
								GSQL_NAV_TREE_NUM_ITEMS, 		expand_struct[n].childs,
								-1);
			
			gtk_tree_store_append (GTK_TREE_STORE (model), &child_fake, &child);
			
			gtk_tree_store_set (GTK_TREE_STORE (model), &child_fake,
								GSQL_NAV_TREE_ID,				-1,
								GSQL_NAV_TREE_IMAGE,			NULL,
								GSQL_NAV_TREE_NAME,				N_("Processing..."),
								GSQL_NAV_TREE_REALNAME,			NULL,
								GSQL_NAV_TREE_ITEM_INFO,		NULL,
								GSQL_NAV_TREE_SQL,				NULL,
								GSQL_NAV_TREE_OBJECT_POPUP,		NULL,
								GSQL_NAV_TREE_OBJECT_HANDLER,	NULL,
								GSQL_NAV_TREE_EXPAND_HANDLER,	NULL,
								GSQL_NAV_TREE_STRUCT,			NULL,
								GSQL_NAV_TREE_NUM_ITEMS, 		NULL,
								-1);
		}
		
		gtk_tree_store_remove (GTK_TREE_STORE (model), &child_last);
		
	}
	
}


static void 
on_navigation_activate (GtkTreeView *tv, GtkTreePath *path,
				GtkTreeViewColumn *col, gpointer data)
{
	
	GSQL_TRACE_FUNC;
		
	GSQLNavigation *navigation = data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSQLNavigationHandler object_handler;
	
	model = gtk_tree_view_get_model (tv);
	
	gtk_tree_model_get_iter (model, &iter, path);
	
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_OBJECT_HANDLER, 
						&object_handler, -1);
	if (object_handler)
	{
		object_handler (data, tv, &iter, 0);
		return;
	}
	
	if (gtk_tree_view_row_expanded (tv, path))
		gtk_tree_view_collapse_row (tv, path);
	else
		gtk_tree_view_expand_to_path (tv, path);

}

static void
on_navigation_cursor_changed (GtkTreeView *tv,
					gpointer     user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkListStore *details = NULL;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSQLWorkspace *workspace;
	GSQLNavigation *navigation = user_data;
	
	workspace = gsql_session_get_workspace (navigation->private->session);
	
	model = gtk_tree_view_get_model (tv);
	
	gtk_tree_view_get_cursor (tv, &path, &column);
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	
	gtk_tree_model_get (GTK_TREE_MODEL (model), (GtkTreeIter *) & iter,
						GSQL_NAV_TREE_DETAILS, &details, -1);

	gsql_workspace_set_details (workspace, details);

}


static gboolean 
on_navigation_button_press (GtkWidget *tv, GdkEventButton *event,
										gpointer data)
{
	GSQL_TRACE_FUNC;
	
	if (event->type != GDK_BUTTON_PRESS)
	  return;

	GtkTreePath *path;
	GtkTreeSelection *selection;
	
	if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(tv), 
									    event->x, event->y, &path, 
									    NULL, NULL, NULL))
		return FALSE;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tv));
	
	gtk_tree_selection_select_path (selection, path);
		
	if ((event->button == 3) && (event->type == GDK_BUTTON_PRESS)) 
		on_navigation_popup_menu (tv, data);
	
	return FALSE;
}

static gboolean 
on_navigation_key_press (GtkWidget *tv, GdkEventKey *event, 
									 gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLNavigation *navigation = data;
	GtkTreeIter iter;
	GtkTreeIter parent;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeViewColumn *column;
	GSQLNavigationHandler event_handler = NULL;
	
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	
	gtk_tree_view_get_cursor (GTK_TREE_VIEW (tv), &path, &column);
	
	if (!path)
		return FALSE;
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	
	switch (event->keyval)
	{
		case GDK_F5:
			GSQL_DEBUG ("F5 pressed.. refreshing");
			// 1. get parent of selected iter and set 1-st child ID=-1
			//    for the force refresh
			if (!gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &iter))
				return FALSE;
			
			if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &iter, &parent, 0))
				return FALSE;
			
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
								GSQL_NAV_TREE_ID, -1,
								-1);
			
			// 2. call refresh function
			on_navigation_expand (GTK_TREE_VIEW (tv), &parent, path, data);
			
			return FALSE;
			
		case GDK_c:
			if (event->state & GDK_CONTROL_MASK)
			{
				GSQL_DEBUG ("Ctrl+C process");
				on_navigation_popup_copyname (NULL, NULL);
			}
			
			return FALSE;

		default:
			// check for event handler
			gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_EVENT_HANDLER, 
						&event_handler, -1);
			
			if (event_handler)
				event_handler (data, GTK_TREE_VIEW (tv), &iter, event->keyval);
				
			break;
	}

	return FALSE;
}

static gboolean 
on_navigation_popup_menu (GtkWidget *tv, void *data)
{
	GSQL_TRACE_FUNC;

	GSQLNavigation *navigation = data;
	GtkTreePath *path = NULL;
	GtkTreeViewColumn *column;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSQLNavigationHandler handler = NULL;
	
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	
	gtk_tree_view_get_cursor (GTK_TREE_VIEW (tv), &path, &column);
	
	if (!path)
		return FALSE;
		
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	
	gtk_tree_model_get (GTK_TREE_MODEL (model), (GtkTreeIter *) & iter,
						GSQL_NAV_TREE_OBJECT_POPUP, &handler, -1);
	
	if (handler)
		handler (navigation, GTK_TREE_VIEW (tv), &iter, 0);
	else
	{
		GSQL_DEBUG ("Popup default navigation menu");
		gsql_navigation_menu_popup (navigation, NULL);
	}
	
	return FALSE;
}

static void 
on_navigation_popup_copyname (GtkMenuItem *menuitem, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GSQLSession *session = NULL;
	GSQLWorkspace *workspace = NULL;
	GSQLNavigation *navigation = NULL;
	gchar *realname = NULL;
	gchar *name;
	GdkDisplay* disp;
	GtkClipboard* clip;
	
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	navigation = gsql_workspace_get_navigation (workspace);
	
	model = gtk_tree_view_get_model (navigation->private->navtree);
	selection = gtk_tree_view_get_selection (navigation->private->navtree);
	
	if (!gtk_tree_selection_get_selected (selection,
										  &model,
										  &iter))
		// have no one selected. 
		return;
	
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	if (!realname)
		return;
	
	pango_parse_markup (realname, -1, 0, NULL, &name, NULL, NULL);

	disp = gdk_display_get_default();
	clip = gtk_clipboard_get_for_display (disp, GDK_SELECTION_CLIPBOARD);
	
	gtk_clipboard_set_text (clip, name, 256);
	
	g_free (name);
}

static void 
on_navigation_popup_refresh (GtkMenuItem *menuitem, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session = NULL;
	GSQLWorkspace *workspace = NULL;
	GSQLNavigation *navigation = NULL;
	GdkEventKey *event = g_new0 (GdkEventKey, 1);
	
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	navigation = gsql_workspace_get_navigation (workspace);

	g_return_if_fail (GSQL_IS_NAVIGATION (navigation));
	
	// simulate F5 pressing
	event->keyval = GDK_F5;
	
	on_navigation_key_press (GTK_WIDGET (navigation->private->navtree),
							 event, navigation);
	g_free (event);

}

static void
details_hash_remove_key_notify (gpointer data)
{
	GSQL_TRACE_FUNC;

	g_free (data);

}

static void
details_hash_remove_value_notify (gpointer data)
{
	GSQL_TRACE_FUNC;

	g_object_unref (G_OBJECT (data));

}

static void
menu_item_select_cb (GtkMenuItem *proxy, gpointer userdata)
{
	GtkAction *action;
	gchar *tip = NULL;
	
	action = g_object_get_data (G_OBJECT (proxy),  "gtk-action");
	g_return_if_fail (action != NULL);
	
	g_object_get (G_OBJECT (action), "tooltip", &tip, NULL);
	
	if (tip)
	{
		gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, tip);
		
		g_free (tip);
	}
	
}

static void
menu_item_deselect_cb (GtkMenuItem *proxy, gpointer userdata)
{
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);	
}

static void 
ui_connect_proxy_cb (GtkUIManager *manager,
								 GtkAction *action,
								 GtkWidget *proxy,
								 gpointer userdata)
{
	GSQL_TRACE_FUNC;
	
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_connect (proxy, "select",
						  G_CALLBACK (menu_item_select_cb), NULL);
		g_signal_connect (proxy, "deselect",
						  G_CALLBACK (menu_item_deselect_cb), NULL);	
	}
	
}

static void 
ui_disconnect_proxy_cb (GtkUIManager *manager,
								 GtkAction *action,
								 GtkWidget *proxy,
								 gpointer userdata)
{
	GSQL_TRACE_FUNC;
	
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_handlers_disconnect_by_func (proxy, 
											  G_CALLBACK (menu_item_select_cb), 
											  NULL);
		g_signal_handlers_disconnect_by_func (proxy, 
											  G_CALLBACK (menu_item_deselect_cb), 
											  NULL);
	}
	
}
