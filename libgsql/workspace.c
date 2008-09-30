/***************************************************************************
 *            workspace.c
 *
 *  Copyright  2007  Taras Halturin
 *  halturin@gmail.com
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

#include <libgsql/common.h>
#include <libgsql/workspace.h>
#include <libgsql/content.h>

#include "utils.h"
#include "stock.h"

struct _GSQLWorkspacePrivate
{
	GtkWidget *contents; // contents area. 
	GtkWidget *messages; // messages area.
	GtkWidget *messages_tview;
	GSQLNavigation *navigation; // navigation area. GSQLNavigation.
	
	GtkWidget *details_page; // default content's page. aka 'Details'
	GtkListStore *details_store; // default details page
	
	GtkWidget *root;	 // root widget;
	GtkWidget *box;		 // container for the 'contents', 'messages', 'navigation'
	
	gboolean navigate_show;
	gboolean messages_show;

	GList		*content_list;
};

static void gsql_workspace_class_init (GSQLWorkspaceClass *klass);
static void gsql_workspace_init (GSQLWorkspace *obj);
static void gsql_workspace_destroy (GtkObject *obj);
static void gsql_workspace_finalize (GObject *obj);
static void gsql_workspace_size_request (GtkWidget *widget, 
										 GtkRequisition *requisition);
static void gsql_workspace_size_allocate (GtkWidget *widget, 
										  GtkAllocation *allocation);
static void gsql_workspace_add (GtkContainer *container, GtkWidget *widget);
static void gsql_workspace_forall (GtkContainer *container,
									gboolean      include_internals,
									GtkCallback   callback,
									gpointer      callback_data);
static void
on_adjustment_changed (GtkAdjustment* adj, gpointer data);

static GtkWidgetClass *parent_class;


// callbacks

static gboolean gsql_messages_popup_menu_cb(GtkWidget *tv, void *user_data);

static gboolean gsql_messages_button_press_cb (GtkWidget *tv, 
											   GdkEventButton *event, 
											   gpointer data);
static gboolean gsql_messages_key_press_cb(GtkWidget *tv, GdkEventKey *event, 
										   gpointer user_data);
static void on_copy_text_messages_activate (GtkMenuItem * menuitem, 
											gpointer user_data);
static void on_select_all_messages_activate (GtkMenuItem * menuitem,
											 gpointer user_data);
static void on_save_to_file_messages_activate (GtkMenuItem * menuitem,
											   gpointer user_data);
static void on_clear_all_messages_activate (GtkMenuItem * menuitem,
											gpointer user_data);
static void gsql_message_save_selection_foreach (GtkTreeModel *model,
										GtkTreePath *path, GtkTreeIter *iter,
										gpointer data);


enum {
	SIG_ON_ACTIVATE,
	SIG_ON_CLOSE,
	SIG_LAST
};

static guint workspace_signals[SIG_LAST] = { 0 };

GType
gsql_workspace_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLWorkspaceClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_workspace_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLWorkspace),
			0,
			(GInstanceInitFunc) gsql_workspace_init,
			NULL
		};
		obj_type = g_type_register_static (GTK_TYPE_CONTAINER,
										   "GSQLWorkspace", &obj_info, 0);
		
	}
	
	return obj_type;	
}


static GType
gsql_workspace_child_type (GtkContainer   *container)
{
	return GTK_TYPE_WIDGET;
}

GSQLWorkspace *
gsql_workspace_new (GSQLSession *session)
{
	GSQL_TRACE_FUNC
	
	GSQLWorkspace 		*workspace;
	/* FIXME: layout choosing */
	gboolean			vertical_layout = FALSE;
	GtkTreeViewColumn 	*column;
	GtkCellRenderer 	*renderer;
	GtkTreeSelection 	*selection;
	GtkTreeIter			iter;
	
	GtkWidget			*contents_root;
	GtkWidget			*navigation_root;
	GtkWidget			*messages_root;
	GtkWidget			*messages_tview;
	GtkWidget			*root, *root_inside;
	
	GtkListStore		*lstore;
	GtkWidget			*scroll;
	GtkWidget			*header;
	GtkWidget			*menulbl;
	GtkAdjustment * adj;
	gint wnd_width;
	gint  wnd_height;
	
	g_return_val_if_fail (session != NULL, NULL);
	
	workspace = g_object_new (GSQL_WORKSPACE_TYPE, NULL);
	
	/*  CONTENTS AREA  */	
	
	workspace->private->contents = GTK_WIDGET (gtk_notebook_new ());
	
	contents_root = workspace->private->contents;
	
	gtk_notebook_popup_enable (GTK_NOTEBOOK (contents_root));
	
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (contents_root), TRUE);
	
	// 'Details' page
	
	workspace->private->details_page = GTK_WIDGET (gtk_tree_view_new ());
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (workspace->private->details_page));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (workspace->private->details_page), FALSE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (workspace->private->details_page), TRUE);
	gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (workspace->private->details_page), FALSE);
	
	//	'Details' page: icons column
	
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_pixbuf_new();
	
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "stock-id", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (workspace->private->details_page), 
								 column);
	
	//	'Details' page: attribute column
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes (_("Attribute"),
						renderer,
						NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	
	g_object_set(renderer,
					"wrap-mode", PANGO_WRAP_WORD,
				 	"wrap-width", 500,
					NULL);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", 1);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (workspace->private->details_page), 
								 column);
	
	//  'Details' page: value column
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes (_("Value"),
						renderer,
						NULL);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", 2);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (workspace->private->details_page), 
								 column);
	lstore = gtk_list_store_new (3,
									 G_TYPE_STRING,
									 G_TYPE_STRING,
									 G_TYPE_STRING);
	workspace->private->details_store = lstore;
	
	gtk_list_store_append(GTK_LIST_STORE(lstore), &iter);
	
	gtk_list_store_set(GTK_LIST_STORE(lstore), &iter,
							   0, GTK_STOCK_INFO,
							   1, N_(
		"In this area a detailed information about the selected object from"
		" the tree of database objects is displayed. In case when a class of"
		" objects is selected, e.g. Triggers, a list of objects will be displayed."
		" It is possible then to select individual objects from this list and"
		" perform actions with all the selected objects.\n\n"

		"Note that the actual behaviour of this area depends completely on" 
		" the implementation of pluggable DBMS engines.  Due to the intrinsic"
		" differences between DBMSs the consistensy of this object area"
		" look-and-feel can only be assured to a certain extent.\n\n"

		"Use the right mouse button to show context menu with actions"
		" applicable to the selected object."
					   			),
							  -1);
	gtk_tree_view_set_model (GTK_TREE_VIEW (workspace->private->details_page), 
							 GTK_TREE_MODEL (lstore));
	
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);	

	gtk_container_add (GTK_CONTAINER (scroll),
					   workspace->private->details_page);
	
	header = gsql_utils_header_new (NULL, _("Details"), NULL, FALSE, 0);	
	menulbl = gsql_utils_header_new (NULL, _("Details"), NULL, FALSE, 0);
	
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (contents_root),
							  GTK_WIDGET(scroll),
							  header, menulbl);

	/*  NAVIGATION AREA  */	
	
	workspace->private->navigation = gsql_navigation_new (session);
	navigation_root = GTK_WIDGET (workspace->private->navigation);

	/*  MESSAGES AREA */
	
	lstore = gtk_list_store_new(5, 
								G_TYPE_INT, // message type (notice, warning, error... etc
								G_TYPE_STRING, // image
								G_TYPE_STRING,  // time
								G_TYPE_STRING,  // message
								G_TYPE_POINTER); // reserved
	
	messages_tview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(lstore));
	
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (messages_tview), TRUE);	
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (messages_tview), FALSE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (messages_tview), FALSE);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (messages_tview));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
	
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_pixbuf_new();
	
	gtk_tree_view_column_pack_start (column, renderer, FALSE);							
	gtk_tree_view_column_add_attribute (column, renderer, "stock-id", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (messages_tview), column);

	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();

	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (messages_tview), column);
	
	column = gtk_tree_view_column_new ();	
	renderer = gtk_cell_renderer_text_new ();
	
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (messages_tview), column);	
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	messages_root = scroll;

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);
								
	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scroll));
								
	g_signal_connect(G_OBJECT(adj), "changed",
					 G_CALLBACK (on_adjustment_changed), NULL);
								
	gtk_widget_show (scroll);
	
	gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET(messages_tview));
								
	workspace->private->messages = messages_root;
	workspace->private->messages_tview = messages_tview;						
	workspace->private->messages_show = TRUE; //FIXME. use gconf
	
	g_signal_connect (G_OBJECT (messages_tview), "button-press-event", 
						G_CALLBACK(gsql_messages_button_press_cb), workspace);
								
	g_signal_connect (G_OBJECT (messages_tview), "key-press-event", 
						G_CALLBACK(gsql_messages_key_press_cb), workspace);
								
	g_signal_connect (G_OBJECT (messages_tview), "popup-menu", 
						G_CALLBACK(gsql_messages_popup_menu_cb), workspace);
								
	
	root = gtk_hpaned_new ();
	if (vertical_layout)
	{
		root_inside = gtk_hpaned_new ();
		
	} else {
		
		root_inside = gtk_vpaned_new ();
	}
								
	gtk_paned_pack2 (GTK_PANED (root), root_inside, TRUE, TRUE);
	gtk_paned_pack1 (GTK_PANED (root), navigation_root, TRUE, TRUE);
	gtk_paned_pack1 (GTK_PANED (root_inside), contents_root, TRUE, TRUE);
	gtk_paned_pack2 (GTK_PANED (root_inside), messages_root, TRUE, TRUE);
        
	gtk_window_get_size (GTK_WINDOW(gsql_window), &wnd_width, &wnd_height);
								
	gtk_paned_set_position (GTK_PANED(root), wnd_width*0.24);
	gtk_paned_set_position (GTK_PANED(root_inside), wnd_height*0.75);

	workspace->private->root = root;
								
	gtk_widget_freeze_child_notify (root);						
	gtk_widget_set_parent (root, GTK_WIDGET (workspace));						
	gtk_widget_thaw_child_notify (root);
								
	gtk_widget_show_all (GTK_WIDGET (workspace));
	
	gsql_session_set_workspace (session, workspace);
	
	return workspace;
}

GSQLContent *
gsql_workspace_get_current_content (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC
	
	GtkNotebook *contents;
	GSQLContent *content;
	GtkWidget	*child;
	gint		page;
	GSQLSession *session;
	
	if (workspace == NULL)
	{
		session = gsql_session_get_active ();
		if (session == NULL)
			return NULL;
		
		workspace = gsql_session_get_workspace (session);
	}
	
	contents = GTK_NOTEBOOK (workspace->private->contents);
	page = gtk_notebook_get_current_page (contents);
	child = gtk_notebook_get_nth_page (contents, page);
	
	if (GSQL_IS_CONTENT (child))
	{
		GSQL_DEBUG ("Is GSQLContent page");
		return (GSQL_CONTENT (child));
	}
	
	GSQL_DEBUG ("Isn't GSQLContent page. Seems to be 'Details'");
	
	return NULL;
}

GSQLNavigation *
gsql_workspace_get_navigation (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), NULL);
	
	return workspace->private->navigation;
}
	

void
gsql_workspace_add_content (GSQLWorkspace *workspace, GSQLContent *content)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	GtkWidget *label;
	GtkWidget *menu_label;
	gint p;
	
	GtkNotebook *contents = GTK_NOTEBOOK (workspace->private->contents);
	
	label = gsql_content_get_header_label (content);
	menu_label = gsql_content_get_menu_label (content);

	p = gtk_notebook_append_page_menu (contents, GTK_WIDGET (content), label, menu_label);
	
	gtk_widget_show_all (GTK_WIDGET (content));
	
	gtk_notebook_set_current_page (contents, p);
	
};

GList *
gsql_workspace_get_content_list (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC

	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), NULL);
	
	return g_list_copy (workspace->private->content_list);
	
}

void
gsql_workspace_set_navigate_visible (GSQLWorkspace *workspace, gboolean visible)
{
	GSQL_TRACE_FUNC
	
	workspace->private->navigate_show = visible;
	
	if (visible)
		gtk_widget_show (GTK_WIDGET (workspace->private->navigation));
	else
		gtk_widget_hide (GTK_WIDGET (workspace->private->navigation));

}


void
gsql_workspace_set_messages_visible (GSQLWorkspace *workspace, gboolean visible)
{
	GSQL_TRACE_FUNC

	workspace->private->messages_show = visible;
	
	if (visible)
		gtk_widget_show (GTK_WIDGET (workspace->private->messages));
	else
		gtk_widget_hide (GTK_WIDGET (workspace->private->messages));

}


gboolean
gsql_workspace_get_navigate_visible (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC

	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), FALSE);
	
	return workspace->private->navigate_show;
}


gboolean
gsql_workspace_get_messages_visible (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC

	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), FALSE);
	
	return workspace->private->messages_show;
}

GSQLSession *
gsql_workspace_get_session (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC
	
	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), NULL);
		
	return GSQL_SESSION (GTK_WIDGET (workspace)->parent);
}

GtkTreeView *
gsql_workspace_get_details (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC;
	GtkTreeView *ret;
	
	g_return_val_if_fail (GSQL_IS_WORKSPACE (workspace), NULL);
	
	ret = GTK_TREE_VIEW (workspace->private->details_page);
	
	return ret;
}

void
gsql_workspace_set_details (GSQLWorkspace *workspace,
							GtkListStore *details_store)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	
	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (workspace->private->details_page));
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (workspace->private->details_page), 1);
	
	if (!GTK_IS_LIST_STORE (details_store))
	{
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (workspace->private->details_page), FALSE);
		gtk_tree_view_column_queue_resize (column);
		
		model = (GtkTreeModel *) workspace->private->details_store;
	}
	else
	{
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (workspace->private->details_page), TRUE);
		gtk_tree_view_column_queue_resize (column);
		
		model = (GtkTreeModel *) details_store;
	}
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (workspace->private->details_page),
							 model);

}

void 
gsql_message_add(GSQLWorkspace *workspace, gint type, gchar * msg)
{    
	GSQL_TRACE_FUNC

	GtkWidget *vpaned;
	GtkTreeModel *liststore = NULL;
	gchar *stock = NULL;
	GtkTreeIter t_iter;
	GtkAdjustment * adj;
	GtkWidget * scroll;
	gchar tmp[GSQL_MESSAGE_LEN], tmp1[GSQL_MESSAGE_LEN];
	gchar ctime[16];
	time_t ttime = time(NULL);
	
    liststore = gtk_tree_view_get_model (GTK_TREE_VIEW (workspace->private->messages_tview));

	GSQL_THREAD_ENTER
	
	gtk_list_store_append(GTK_LIST_STORE(liststore), &t_iter);
	switch (type)
	{
		case GSQL_MESSAGE_NORMAL:
			g_snprintf(tmp, GSQL_MESSAGE_LEN,"<span>%s</span>", msg);
			break;
		
		case GSQL_MESSAGE_NOTICE:
			stock = GTK_STOCK_INFO;
			g_snprintf(tmp, GSQL_MESSAGE_LEN,"<span color='darkgreen'>%s</span>", msg);
			break;
		
		case GSQL_MESSAGE_WARNING:
			stock = GTK_STOCK_DIALOG_WARNING;
			g_snprintf(tmp, GSQL_MESSAGE_LEN,"<span color='darkorange'>%s</span>", msg);
			break;
		
		case GSQL_MESSAGE_ERROR:
			stock = GTK_STOCK_DIALOG_ERROR;
			g_snprintf(tmp, GSQL_MESSAGE_LEN,"<span color='red'>%s</span>", msg);
			break;

		case GSQL_MESSAGE_OUTPUT:
			g_snprintf(tmp, GSQL_MESSAGE_LEN,"<span color='darkblue'>%s</span>", msg);
			stock = GSQL_STOCK_MOUTPUT;
			break;
			
	}

	// in future... make the choose for 12/24 time format
	// strftime(ctime, 16, "%r", localtime(&ttime);
	strftime(ctime, 16, "%T", localtime(&ttime));
	
	g_snprintf(tmp1, GSQL_MESSAGE_LEN,"<span color='darkgray'>%s</span>", ctime);
	
	gtk_list_store_set(GTK_LIST_STORE(liststore), &t_iter,
							   0, type,
							   1, stock,
							   2, tmp1,
							   3, tmp,
							  -1);
	GSQL_THREAD_LEAVE

}

void
gsql_workspace_next_content (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));
	
	gtk_notebook_next_page (GTK_NOTEBOOK (workspace->private->contents));
	
}


void
gsql_workspace_prev_content (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));
	
	gtk_notebook_prev_page (GTK_NOTEBOOK (workspace->private->contents));

}

void
gsql_workspace_set_content_page (GSQLWorkspace *workspace, GSQLContent *content)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));
	
	gint n = 0;
	
	if (content)
	{
		n = gtk_notebook_page_num (GTK_NOTEBOOK (workspace->private->contents), 
								   GTK_WIDGET (content));
		if (n < 0)
		{
			GSQL_DEBUG ("Content page not found at this Workspace. Fix your bug.");
			return;
		};
	};
	
	gtk_notebook_set_current_page (GTK_NOTEBOOK (workspace->private->contents), n);
	
}

/*
 *	Static section:
 *	gsql_workspace_size_request
 *	gsql_workspace_size_allocate
 *	gsql_workspace_forall
 *	gsql_workspace_add
 *	gsql_workspace_init
 *	gsql_workspace_class_init
 *	gsql_workspace_finalize
 *	gsql_workspace_destroy
 */

static void
gsql_workspace_destroy (GtkObject *obj)
{
	GSQL_TRACE_FUNC

	GSQLWorkspace *workspace = GSQL_WORKSPACE (obj);
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
	
}

static void
gsql_workspace_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC

	GSQLWorkspace *workspace = GSQL_WORKSPACE (obj);

	g_free (workspace->private);
	
	(* G_OBJECT_CLASS (parent_class)->finalize) (obj);
	
}

static void
gsql_workspace_class_init (GSQLWorkspaceClass *klass)
{
	GSQL_TRACE_FUNC

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	
	obj_class->finalize = gsql_workspace_finalize;
	
	gtkobject_class->destroy = gsql_workspace_destroy;
	
	widget_class->size_request = gsql_workspace_size_request;
	widget_class->size_allocate = gsql_workspace_size_allocate;
	
	container_class->add = gsql_workspace_add;
	container_class->forall = gsql_workspace_forall;
	container_class->child_type = gsql_workspace_child_type;
	
	workspace_signals [SIG_ON_ACTIVATE] = 
		g_signal_new ("on-activate", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLWorkspaceClass,
									   on_activate),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	workspace_signals [SIG_ON_CLOSE] = 
		g_signal_new ("on-close", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLWorkspaceClass,
									   on_close),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	gtkobject_class->destroy = gsql_workspace_destroy;

}

static void 
gsql_workspace_init (GSQLWorkspace *obj)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLWorkspacePrivate, 1);
	obj->private->navigate_show = TRUE;
	obj->private->messages_show = TRUE;
	
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (obj), GTK_NO_WINDOW);
	
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (obj), FALSE);
	
}

static void
gsql_workspace_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GSQLWorkspace *workspace = GSQL_WORKSPACE (widget);
	GtkWidget *child = GTK_WIDGET (workspace->private->root);
	
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
gsql_workspace_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GSQLWorkspace *workspace = GSQL_WORKSPACE (widget);
	GtkWidget *child = GTK_WIDGET (workspace->private->root);
	
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
gsql_workspace_add (GtkContainer *container, GtkWidget *widget)
{
	// stub
	return;
}


static void
gsql_workspace_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data)
{
	GSQLWorkspace *workspace = GSQL_WORKSPACE (container);
	
	GtkWidget *child = GTK_WIDGET (workspace->private->root);
	
	g_return_if_fail (callback != NULL);
	
	if (child)
		(* callback) (child, callback_data);

}


static void
on_adjustment_changed (GtkAdjustment* adj, gpointer data)
{
	GSQL_THREAD_ENTER
		
	gtk_adjustment_set_value (adj, adj->upper - adj->page_size);
	
	GSQL_THREAD_LEAVE
	
}

static void
gsql_message_save_selection_foreach (GtkTreeModel *model,
										GtkTreePath *path,
										GtkTreeIter *iter,
										gpointer data)
{
	GSQL_TRACE_FUNC

	gchar * type_message_str;
	gint type_message;
	#define TYPE_MESSAGE_FIELD 0
	gchar * time_message;
	#define TIME_MESSAGE_FIELD 2
	gchar * body_message;
	#define BODY_MESSAGE_FIELD 3
		
	gchar * tmp;	
	FILE * fout = data;
	
	gtk_tree_model_get (model,	iter,
			    TYPE_MESSAGE_FIELD, &type_message, -1);
	gtk_tree_model_get (model,	iter,
			    TIME_MESSAGE_FIELD, &time_message, -1);
	gtk_tree_model_get (model,	iter,
			    BODY_MESSAGE_FIELD, &body_message, -1);
	
	switch (type_message)
	{
		case GSQL_MESSAGE_NORMAL:
			type_message_str = "Normal";
			break;
			
		case GSQL_MESSAGE_NOTICE:
			type_message_str = "Notice";
			break;
			
		case GSQL_MESSAGE_WARNING:
			type_message_str = "Warning";
			break;
			
		case GSQL_MESSAGE_ERROR:
			type_message_str = "Error";
			break;
			
		case GSQL_MESSAGE_OUTPUT:
			type_message_str = "Output";
	}
	
	pango_parse_markup (time_message, -1, 0, NULL, &tmp, NULL, NULL);
	time_message = tmp;
	
	pango_parse_markup (body_message, -1, 0, NULL, &tmp, NULL, NULL);
	body_message = tmp;
	
	fprintf (fout, "%s: [%s] %s\n", type_message_str, time_message, body_message);
	
	g_free (time_message);
	g_free (body_message);

}


static void 
on_copy_text_messages_activate (GtkMenuItem * menuitem, 
                                gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkTreeView *treeview = user_data;
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GdkDisplay *disp;
	GtkClipboard *clip;
	GtkTreeIter iter;
	gchar *body_message;
	gchar *tmp;
	
	
	sel = gtk_tree_view_get_selection(treeview);
	
	model = gtk_tree_view_get_model (treeview);
	
	gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
	
	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		// have no one selected. 
		return;
	
	gtk_tree_model_get (model,	&iter,
			    3, &body_message, -1);
	
	gtk_tree_selection_set_mode (sel, GTK_SELECTION_MULTIPLE);
	
	pango_parse_markup (body_message, -1, 0, NULL, &tmp, NULL, NULL);
	body_message = tmp;
	
	disp = gdk_display_get_default();
	clip = gtk_clipboard_get_for_display (disp, GDK_SELECTION_CLIPBOARD);
		
	gtk_clipboard_set_text (clip, body_message, GSQL_MESSAGE_LEN);

}

static void 
on_select_all_messages_activate (GtkMenuItem * menuitem, 
                                 gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkTreeView * treeview = user_data;
	GtkTreeSelection * sel;
	
	sel = gtk_tree_view_get_selection(treeview);
	
	gtk_tree_selection_select_all(sel);

}

static void 
on_save_to_file_messages_activate (GtkMenuItem * menuitem, 
                                   gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkWidget * dialog;
	static gchar * uri, *file_name;
	FILE *fout;
	GtkWidget * messages_treeview = user_data;
	GtkTreeSelection * selection;

	
	dialog = gtk_file_chooser_dialog_new (_("Save messages to file..."),
						GTK_WINDOW (gsql_window),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE,
						GTK_RESPONSE_OK,
						NULL);
	if (uri != NULL)
		gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (dialog), uri);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{	   
		uri = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER (dialog));
		file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		
		fout = fopen (file_name, "w+");
		
		if (fout == NULL)
		{
			g_critical ("Couldn't open file %s for writing\n", file_name);
			
			gtk_widget_destroy (GTK_WIDGET(dialog));
			
			return;
		};
		
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (messages_treeview));
		
		gtk_tree_selection_selected_foreach (selection,
											 gsql_message_save_selection_foreach,
											 fout);		
		fclose (fout);
	}
	
	gtk_widget_destroy (GTK_WIDGET(dialog));
	
}

static void 
on_clear_all_messages_activate (GtkMenuItem * menuitem, 
								gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkListStore * liststore;
	GtkTreeView * treeview = user_data;
	
	liststore = GTK_LIST_STORE (gtk_tree_view_get_model(treeview));

	gtk_list_store_clear(liststore);	

}

static void 
gsql_messages_menu_show(GtkWidget * treeview)
{
	GSQL_TRACE_FUNC

	GtkWidget *menu;
	GtkWidget *menuitem;
	GtkWidget *image;
  
	menu = gtk_menu_new();
	
	menuitem = gtk_image_menu_item_new_with_mnemonic (_("Copy text"));
	
	gtk_widget_show (menuitem);
	
	image = gtk_image_new_from_stock ("gtk-copy", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	
	gtk_container_add (GTK_CONTAINER (menu), menuitem);
	
	g_signal_connect ((gpointer) menuitem, "activate",
						G_CALLBACK (on_copy_text_messages_activate),
						treeview);
    
	menuitem = gtk_menu_item_new_with_mnemonic (_("Select all"));
	gtk_widget_show (menuitem);
	
	gtk_container_add (GTK_CONTAINER (menu), menuitem);
	
	g_signal_connect ((gpointer) menuitem, "activate",
						G_CALLBACK (on_select_all_messages_activate),
						treeview);
                    
	menuitem = gtk_image_menu_item_new_with_mnemonic (_("Save to file"));
	gtk_widget_show (menuitem);
	
	image = gtk_image_new_from_stock ("gtk-save-as", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	
	gtk_container_add (GTK_CONTAINER (menu), menuitem);
	
	g_signal_connect ((gpointer) menuitem, "activate",
						G_CALLBACK (on_save_to_file_messages_activate),
						treeview);

	menuitem = gtk_separator_menu_item_new ();
	gtk_widget_show (menuitem);
	
	gtk_container_add (GTK_CONTAINER (menu), menuitem);
	
	gtk_widget_set_sensitive (menuitem, FALSE);
	
	menuitem = gtk_image_menu_item_new_with_mnemonic (_("Clear all"));
	gtk_widget_show (menuitem);
	
	image = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	
	gtk_container_add (GTK_CONTAINER (menu), menuitem);
	
	g_signal_connect ((gpointer) menuitem, "activate",
						G_CALLBACK (on_clear_all_messages_activate),
						treeview);
                    
	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
	
}


static gboolean 
gsql_messages_popup_menu_cb (GtkWidget *tv, void *user_data)
{
	GSQL_TRACE_FUNC

	gsql_messages_menu_show (tv);
	
	return FALSE;
}

static gboolean 
gsql_messages_button_press_cb (GtkWidget *tv, 
								GdkEventButton *event,   
								gpointer data)
{
	GSQL_TRACE_FUNC

	if ((event->button == 3) && (event->type == GDK_BUTTON_PRESS)) 
	{
		gsql_messages_menu_show(tv);
		return TRUE;
	}
	
	return FALSE;      
}

static gboolean
gsql_messages_key_press_cb (GtkWidget *tv, 
                            GdkEventKey *event,                             
                            gpointer user_data)
{
	GSQL_TRACE_FUNC

	return TRUE;
}

