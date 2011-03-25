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



#include <libgsql/common.h>
#include <libgsql/session.h>
#include <libgsql/stock.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#define GSQL_PARAM_READWRITE G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS

static GHashTable *sessions;
static	GSQLSession *active_session = NULL;

struct _GSQLSessionPrivate
{
	const gchar *username;
	const gchar *password;
	const gchar *database;
	const gchar *hostname;
	const gchar *info;
	
	const gchar *stock;
	gchar		*name;
	gchar		*display_name;
	
	GtkWidget	*header;	// tab header widget
	GtkImage	*header_icon;
	GtkWidget	*close;		// close button
	
	GSQLWorkspace *workspace;
	GHashTable  *titles_hash;
	
	GMutex	  *lock;
	gboolean  busy;
};

enum {
	SIG_ON_SESSION_REOPEN,
	SIG_ON_SESSION_CLOSE,
	SIG_ON_SESSION_DUPLICATE,
	SIG_ON_SESSION_COMMIT,
	SIG_ON_SESSION_ROLLBACK,
	SIG_ON_SESSION_SWITCH,
	
	SIG_LAST
};

enum {
	PROP_0,
	PROP_USERNAME,
	PROP_PASSWORD,
	PROP_HOSTNAME,
	PROP_DATABASE,
	PROP_INFO
};

static void gsql_session_class_init (GSQLSessionClass *klass);
static void gsql_session_init (GSQLSession *obj);
static void gsql_session_destroy		(GtkObject	*object);
static void gsql_session_set_property	(GObject		*object,
							 guint			propid,
							 const GValue	*value,
							 GParamSpec		*pspec);
static void gsql_session_get_property	(GObject		*object,
							 guint			propid,
							 GValue			*value,
							 GParamSpec		*pspec);
static void gsql_session_set_session_name (GSQLSession *session);
static void gsql_session_size_request (GtkWidget *widget,
							GtkRequisition *requisition);
static void gsql_session_size_allocate (GtkWidget *widget, 
							GtkAllocation *allocation);
static void gsql_session_add (GtkContainer *container, GtkWidget *widget);
static void gsql_session_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data);

void static on_menu_session_reconnect (GtkMenuItem *mi, gpointer data);
void static on_menu_session_duplicate (GtkMenuItem *mi, gpointer data);
void static on_menu_session_close (GtkMenuItem *mi, gpointer data);
void static on_menu_session_close_all (GtkMenuItem *mi, gpointer data);
void static on_menu_session_commit (GtkMenuItem *mi, gpointer data);
void static on_menu_session_rollback (GtkMenuItem *mi, gpointer data);
static void on_unsaved_dialog_enabled_toggled (GtkCellRendererToggle *cell,
								   gchar *path_str,
								   GtkTreeView *tv);

static void untitled_hash_remove_key_notify (gpointer data);

static guint session_signals[SIG_LAST] = { 0 };
static GtkWidgetClass *parent_class;

static GtkActionGroup *session_actions = NULL;

static gchar session_ui[] =
"<ui>   "
"  <menubar name=\"MenuMain\">   "
"    <menu name=\"MenuSessions\" action=\"ActionMenuSessions\">   "
"	  <placeholder name=\"PHolderSessionDB\">    "
"      	<menuitem name=\"MenuReconnect\"  action=\"ActionMenuReconnect\"/>   "
"      	<menuitem name=\"MenuDuplicateSession\" action=\"ActionMenuDuplicateSession\"/>   "
"      	<menuitem name=\"MenuCloseSession\" action=\"ActionMenuCloseSession\"/>   "
"      	<menuitem name=\"MenuCloseAllSession\" action=\"ActionMenuCloseAllSessions\"/>   "
"      	<separator name=\"SeparatorA\" />   "
"      	<menuitem name=\"MenuCommit\" action=\"ActionMenuCommit\" />   "
"      	<menuitem name=\"MenueRollback\" action=\"ActionMenuRollback\" />   "
"      </placeholder>   "
"    </menu>   "
"  </menubar>   "
"  <toolbar name=\"ToolbarMain\">   "
"	<placeholder name=\"PHolderToolbarMain\">   "
"     	<toolitem name=\"CloseSession\" action=\"ActionMenuCloseSession\"/>   "
"      	<separator name=\"SeparatorA\" />   "
"      	<toolitem name=\"MenuCommit\" action=\"ActionMenuCommit\" />    "
"      	<toolitem name=\"MenueRollback\" action=\"ActionMenuRollback\" />   "
"	</placeholder>   "
"  </toolbar>   "
"</ui> ";


static GtkActionEntry session_acts[] = 
{
	{ "ActionMenuReconnect", NULL, N_("Reconnect"), NULL, N_("Reconnect current session"), G_CALLBACK(on_menu_session_reconnect) },
	{ "ActionMenuDuplicateSession", NULL, N_("Duplicate"), NULL, N_("Duplicate current session"), G_CALLBACK(on_menu_session_duplicate) },
	{ "ActionMenuCloseSession", GTK_STOCK_DISCONNECT/*GSQL_STOCK_SESSION_CLOSE*/, N_("Close Session"), NULL, N_("Close current session"), G_CALLBACK(on_menu_session_close) },
	{ "ActionMenuCloseAllSessions", NULL, N_("Close All Session"), NULL, N_("Close all session"), G_CALLBACK(on_menu_session_close_all) },
	{ "ActionMenuCommit", GSQL_STOCK_SESSION_COMMIT, N_("Commit"), NULL, N_("Commit"), G_CALLBACK(on_menu_session_commit) },
	{ "ActionMenuRollback", GSQL_STOCK_SESSION_ROLLBACK, N_("Rollback"), NULL, N_("Rollback"), G_CALLBACK(on_menu_session_rollback) }
};

GType
gsql_session_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLSessionClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_session_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLSession),
			0,
			(GInstanceInitFunc) gsql_session_init,
			NULL
		};
		obj_type = g_type_register_static (GTK_TYPE_CONTAINER,
										   "GSQLSession", &obj_info, 0);
		
	}
	return obj_type;	
}

GSQLSession *
gsql_session_new (void)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	session = g_object_new (GSQL_SESSION_TYPE, NULL);
	
	if (!session_actions)
	{
		session_actions = gtk_action_group_new ("session_actions");
		gtk_action_group_add_actions (session_actions, session_acts, 
								  G_N_ELEMENTS (session_acts), NULL);
		gsql_menu_merge_from_string (session_ui, session_actions);
	}
	
	gtk_action_group_set_visible (session_actions, TRUE);
	
	return session;
}

GSQLSession *
gsql_session_new_with_attrs (gchar *attr_name,...)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	va_list vl;
	gchar *attr;
	gchar *value;
	GValue	*gval;
	
	session = gsql_session_new ();

	if (attr_name == NULL)
		return session;
	
	gval = g_malloc0 (sizeof(GValue));
	
	g_value_init (gval, G_TYPE_STRING);
	
	va_start (vl, attr_name);
	attr = attr_name;
	
	do
	{
		value = va_arg(vl, char *);
		
		if (value == NULL)
			break;
		
		g_value_set_string (gval, (const gchar *) value);
		
		g_object_set_property (G_OBJECT (session),  attr, gval);
		
		attr = va_arg(vl, char *);
		
		if (attr == NULL)
			break;
	}
	while (1);

	va_end (vl);
	
	gsql_session_set_session_name (session);
	gtk_action_group_set_visible (session_actions, TRUE);
	
	return session;	
}


void
gsql_session_set_attrs (GSQLSession *session, gchar *attr_name,...)
{
	GSQL_TRACE_FUNC;

	va_list vl;
	gchar *attr;
	gchar *value;
	GValue	gval = {0,};

	g_return_if_fail (session != NULL);

	if (attr_name == NULL)
		return;
	
	g_value_init (&gval, G_TYPE_STRING);
	
	va_start (vl, attr_name);
	attr = attr_name;
	
	do
	{
		value = va_arg(vl, char *);
		
		if (value == NULL)
			break;
		
		GSQL_DEBUG ("GSQLSession. set prop: [\"%s\"] = [\"%s\"]", attr, value);
		
		g_value_set_string (&gval, (const gchar *) value);
		
		g_object_set_property (G_OBJECT (session),
							   attr, &gval);
		
		attr = va_arg(vl, char *);
		
		if (attr == NULL)
			break;
	}
	while (1);
	
	va_end (vl);
	
	gsql_session_set_session_name (session);
	
	return;	
}

GtkAction *
gsql_session_get_action (GSQLSession *session, gchar *name)
{
	GSQL_TRACE_FUNC;
	
	GtkAction *act;
	
	act = gtk_action_group_get_action (session_actions, name);
	
	return act;
}


void 
gsql_session_set_workspace (GSQLSession *session, GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC;
	
	GtkWidget *child;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	g_return_if_fail (GSQL_IS_WORKSPACE (workspace));

	if (session->private->workspace)
		g_object_unref (session->private->workspace);
	
	gtk_widget_freeze_child_notify (GTK_WIDGET (workspace));
	gtk_widget_set_parent (GTK_WIDGET (workspace), GTK_WIDGET (session));
	gtk_widget_thaw_child_notify (GTK_WIDGET (workspace));
	
	session->private->workspace = workspace;

}

GSQLWorkspace *
gsql_session_get_workspace (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	if (GSQL_IS_SESSION (session))
		
		return session->private->workspace;
	
	else
		if (GSQL_IS_SESSION (active_session))
			return active_session->private->workspace;
		else 
			return NULL;
}


gchar *
gsql_session_get_name (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);

	return session->private->name;
}

const gchar *
gsql_session_get_username (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);

	return session->private->username;
}

const gchar *
gsql_session_get_password (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);

	return session->private->password;
}

const gchar *
gsql_session_get_database_name (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);

	return session->private->database;
}

const gchar *
gsql_session_get_hostname (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);

	return session->private->hostname;
}

gint
gsql_sessions_count (void)
{
	GSQL_TRACE_FUNC;

	return g_hash_table_size (sessions);
}

GSQLSession *
gsql_session_get_active (void)
{
	GSQL_TRACE_FUNC;

	return active_session;
}

void
gsql_session_set_active (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_SESSION (session) != NULL);
	
	active_session = session;
	
}


void
gsql_session_add_content (GSQLSession *session, GSQLContent *content)
{
	GSQL_TRACE_FUNC;
	
	gsql_workspace_add_content (session->private->workspace,
								content);
}

gchar *
gsql_session_get_title (GSQLSession *session, gchar *title)
{
	GSQL_TRACE_FUNC;
	gchar untitled_base[] = N_("Untitled");
	gchar variants[128];
	gchar *ret, *base;
	gint i = 1;
	
	if (!title)
		base = untitled_base;
	else
		base = title;
	
	memset (variants, 0, 128);
	
	g_snprintf (variants, 127,"%s", base);
	
	while (g_hash_table_lookup (session->private->titles_hash, variants))
	{
		g_snprintf (variants, 127,  "%s[%d]", base, i++);
	}
	
	ret = g_strdup(variants);
	
	g_hash_table_insert (session->private->titles_hash, ret, ret);
	
	return ret;
}

void
gsql_session_release_title (GSQLSession *session, gchar *name)
{
	GSQL_TRACE_FUNC;
	
	if (g_hash_table_remove (session->private->titles_hash, name))
		GSQL_DEBUG ("Untitled hash: entry found. removed.");
	else 
		GSQL_DEBUG ("Untitled hash: entry not found [%s].", name);
	

}

gboolean
gsql_session_lock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), FALSE);
	
	if (!g_mutex_trylock (session->private->lock))
	{
		
		workspace = gsql_session_get_workspace (session);
		gsql_message_add (workspace, 
						  GSQL_MESSAGE_WARNING, 
						  N_("The current session already in use. Please, try later "
							 "or stop the running statement."));
		return FALSE;
	} 
	
	session->private->busy = TRUE;
	
	return TRUE;
	
}

void
gsql_session_unlock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	g_mutex_unlock (session->private->lock);
	
	session->private->busy = FALSE;
	
}

gboolean
gsql_session_lock_state (GSQLSession *session)
{
	g_return_val_if_fail (GSQL_IS_SESSION (session), FALSE);
	
	return session->private->busy;
}

void
gsql_session_close (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace = NULL;
	GSQLContent *content = NULL;
	GList *clist;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);
	
	if (workspace)
	{
		clist = gsql_workspace_get_content_list (workspace);
		clist = g_list_first (clist);

		while (clist)
		{
			content = GSQL_CONTENT (clist->data);
		
			clist = g_list_next (clist);
		
			g_signal_emit_by_name (content, "close", TRUE);	
		}
	
		g_list_free (clist);
	}
	
	gtk_widget_destroy (GTK_WIDGET (session));

}

GtkDialog *
gsql_session_unsaved_dialog (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	GSQLWorkspace *workspace;
	GtkTreeStore *ts;
	GtkWidget  *tv = NULL;
	GtkContainer *content_area;
	GtkWidget *dialog;
	GtkWidget	*scroll;
	GtkWidget *label;
	GtkTreeViewColumn 	*column;
	GtkCellRenderer 	*renderer;
	GtkTreeIter iter, child;
	GList *clist = NULL;
	GList *slist = NULL;
	/* GSQLContent *content; */
	guint unsaved_count = 0;
	/* gchar *session_name; */
	gboolean content_state, have_changes;
	
	if (!sessions)
		return NULL;
	
	ts = gtk_tree_store_new (4,
							 G_TYPE_BOOLEAN,
							 G_TYPE_STRING,
							 G_TYPE_STRING,
							 G_TYPE_POINTER);
	
	if ((!session) && (g_hash_table_size (sessions) > 0))
	{
		slist = g_hash_table_get_values (sessions);
		slist = g_list_first (slist);
		
	} else {
		
		if (session)
			slist = g_list_append (slist, session);
	}
	
	GSQL_DEBUG ("Sessions hash len: [%d]", g_list_length (slist));
	
	while (slist)
	{
		session = GSQL_SESSION (slist->data);
		workspace = gsql_session_get_workspace (GSQL_SESSION (slist->data));
		
		clist = gsql_workspace_get_content_list (workspace);
	
		if (g_list_length (clist) == 0)
		{
			slist = g_list_next (slist);
			continue;
		}

		clist = g_list_first (clist);
	
		gtk_tree_store_append(ts, &iter, NULL);

		gtk_tree_store_set(ts, &iter,
						   0, TRUE,
						   1, session->engine->stock_logo,
						   2, session->private->name,
						   3, NULL,
						   -1);
		have_changes = FALSE;
		while (clist)
		{
			content_state = gsql_content_get_changed (GSQL_CONTENT (clist->data));
			
			if (content_state) 
			{
				unsaved_count++;
				have_changes = TRUE;
				gtk_tree_store_append(ts, &child, &iter);
				
				gtk_tree_store_set(ts, &child,
								   0, TRUE,
								   1, gsql_content_get_stock (GSQL_CONTENT (clist->data)),
								   2, gsql_content_get_display_name (GSQL_CONTENT (clist->data)),
								   3, clist->data,
								   -1);
				
			}
			
			clist = g_list_next (clist);
		}
		
		if (!have_changes)
			gtk_tree_store_remove(ts, &iter);

		slist = g_list_next (slist);
	}
	
	if (!unsaved_count)
	{
		gtk_tree_store_clear (ts);
		g_object_unref(ts);
		
		return NULL;
	}
	
	tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (ts));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tv), FALSE);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);
	
	renderer = gtk_cell_renderer_toggle_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
										"active", 0);
	g_signal_connect (renderer, "toggled",
					  G_CALLBACK (on_unsaved_dialog_enabled_toggled), tv);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);
	gtk_tree_view_column_set_title (column, N_("Unsaved Files"));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tv), FALSE);
	
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer,
										"stock-id", 1);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
										"text", 2);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);	
	
	gtk_container_add (GTK_CONTAINER (scroll),
					   GTK_WIDGET(tv));
	
	dialog = gtk_dialog_new_with_buttons (N_("Unsaved documents..."),
										  GTK_WINDOW (gsql_window),
										  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_STOCK_DISCARD,
										  GTK_RESPONSE_CLOSE,
										  GTK_STOCK_CANCEL,
										  GTK_RESPONSE_CANCEL,
										  GTK_STOCK_SAVE,
										  GTK_RESPONSE_OK,
										  NULL);
	
	gtk_window_set_default_size (GTK_WINDOW (dialog), 360, 280);
	
	label = gtk_label_new (N_("Select the files which you want to save. "
							  "Note that checking the session name will "
							  "result in selection of all the unsaved "
							  "files in this session."));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), scroll);
	g_object_set_data (G_OBJECT (dialog), "treeview", tv);
	
	gtk_tree_view_expand_all (GTK_TREE_VIEW (tv));
	
	
	gtk_widget_show_all (dialog);
	
	g_list_free (clist);
	g_list_free (slist);
	
	return GTK_DIALOG (dialog);
}

gboolean
gsql_session_close_all ()
{
	GSQL_TRACE_FUNC;
	
	on_menu_session_close_all (NULL, NULL);
	
	if (g_hash_table_size (sessions) > 0)
		return FALSE;
	
	return TRUE;
}

/* Static section:
 *	gsql_session_init
 *	gsql_session_class_init
 *	gsql_session_finalize
 *	gsql_session_set_property
 *	gsql_session_get_property
 *	gsql_session_set_session_name
 *	gsql_session_size_request
 *	gsql_session_size_allocate
 *	gsql_session_for_all
 *  on_menu_session_reconnect
 *  on_menu_session_duplicate
 *  on_menu_session_close
 *  on_menu_session_close_all
 *  on_menu_session_commit
 *  on_menu_session_rollback
 *
 *  untitled_hash_remove_key_notify
 *  
 */
static void
gsql_session_destroy (GtkObject *obj)
{
	GSQL_TRACE_FUNC;

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
	
}

static void
gsql_session_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session = GSQL_SESSION (obj);
	
	g_hash_table_remove (sessions, session->private->name);
	
	if (g_hash_table_size (sessions) == 0)
	{
		gtk_action_group_set_visible (session_actions, FALSE);
		active_session = NULL;
	}
	
	if (session->private->titles_hash)
		g_hash_table_unref (session->private->titles_hash);

	if (session->private->username)
		g_free ((gchar *) session->private->username);
	if (session->private->password)
		g_free ((gchar *) session->private->password);
	if (session->private->database)
		g_free ((gchar *) session->private->database);
	if (session->private->hostname)
		g_free ((gchar *) session->private->hostname);
	if (session->private->info)
		g_free ((gchar *) session->private->info);
	
	g_free (session->private);

	(* G_OBJECT_CLASS (parent_class)->finalize) (obj);
	
}

static GType
gsql_session_child_type (GtkContainer   *container)
{
	return GTK_TYPE_WIDGET;
}

static void
gsql_session_class_init (GSQLSessionClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	
	obj_class->set_property = gsql_session_set_property;
	obj_class->get_property = gsql_session_get_property;
	obj_class->finalize = gsql_session_finalize;
	
	gtkobject_class->destroy = gsql_session_destroy;
	
	widget_class->size_request = gsql_session_size_request;
	widget_class->size_allocate = gsql_session_size_allocate;
	
	container_class->add = gsql_session_add;
	container_class->forall = gsql_session_forall;
	container_class->child_type = gsql_session_child_type;
	
	g_object_class_install_property (obj_class,
									 PROP_USERNAME,
									 g_param_spec_string ("session-username",
														  "Session's username",
														  "Session's username. FIXME (description)",
														  NULL,
														  GSQL_PARAM_READWRITE));
	g_object_class_install_property (obj_class,
									 PROP_PASSWORD,
									 g_param_spec_string ("session-password",
														  "Session's username",
														  "Session's username. FIXME (description)",
														  NULL,
														  GSQL_PARAM_READWRITE));								  
	g_object_class_install_property (obj_class,
									 PROP_HOSTNAME,
									 g_param_spec_string ("session-hostname",
														  "Session's username",
														  "Session's username. FIXME (description)",
														  NULL,
														  GSQL_PARAM_READWRITE));							 
	g_object_class_install_property (obj_class,
									 PROP_DATABASE,
									 g_param_spec_string ("session-database",
														  "Session's username",
														  "Session's username. FIXME (description)",
														  NULL,
														  GSQL_PARAM_READWRITE));
	g_object_class_install_property (obj_class,
									 PROP_INFO,
									 g_param_spec_string ("session-info",
														  "Session's username",
														  "Session's username. FIXME (description)",
														  NULL,
														  GSQL_PARAM_READWRITE));

	
	session_signals [SIG_ON_SESSION_CLOSE] = 
		g_signal_new ("close", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_close),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	session_signals [SIG_ON_SESSION_REOPEN] = 
		g_signal_new ("reopen", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_reopen),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	session_signals [SIG_ON_SESSION_DUPLICATE] = 
		g_signal_new ("duplicate", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_duplicate),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	session_signals [SIG_ON_SESSION_COMMIT] = 
		g_signal_new ("commit", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_commit),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	session_signals [SIG_ON_SESSION_ROLLBACK] = 
		g_signal_new ("rollback", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_rollback),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	session_signals [SIG_ON_SESSION_SWITCH] = 
		g_signal_new ("switch", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLSessionClass,
									   on_session_switch),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
}


static void 
gsql_session_init (GSQLSession *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	
	pthread_mutexattr_t mutexattr;
	
	obj->private = g_new0 (GSQLSessionPrivate, 1);
	
	obj->private->titles_hash = g_hash_table_new_full (g_str_hash, 
											  g_str_equal,
											  untitled_hash_remove_key_notify,
											  NULL);
	
	gtk_widget_set_has_window (GTK_WIDGET (obj), FALSE);
	
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (obj), FALSE);
	
	obj->private->lock  = g_mutex_new ();
	
	obj->private->busy = FALSE;
	
}

static void 
gsql_session_get_property	(GObject		*object,
							 guint			propid,
							 GValue	*value,
							 GParamSpec		*pspec)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	g_return_if_fail (object != NULL);
	
	session = GSQL_SESSION (object);
	
	switch (propid)
	{
		case PROP_USERNAME:
			if (session->private->username)
				g_value_set_string (value, session->private->username);
			break;

		case PROP_PASSWORD:
			if (session->private->password)
				g_value_set_string (value, session->private->password);
			break;
		
		case PROP_HOSTNAME:
			if (session->private->hostname)
				g_value_set_string (value, session->private->hostname);
			break;
		
		case PROP_DATABASE:
			if (session->private->database)
				g_value_set_string (value, session->private->database);
			break;
		
		case PROP_INFO:
			if (session->private->info)
				g_value_set_string (value, session->private->info);
			break;
	}
	
}

static void 
gsql_session_set_property	(GObject		*object,
							 guint			propid,
							 const GValue	*value,
							 GParamSpec		*pspec)
{
	GSQL_TRACE_FUNC;
	
	GSQLSession *session;
	g_return_if_fail (object != NULL);
	
	session = GSQL_SESSION (object);
	
	switch (propid)
	{
		case PROP_USERNAME:
			if (session->private->username)
				g_free ((gchar *) session->private->username);
			session->private->username = g_strdup (g_value_get_string (value));
			break;

		case PROP_PASSWORD:
			if (session->private->password)
				g_free ((gchar *) session->private->password);
			session->private->password = g_strdup (g_value_get_string (value));
			break;
		
		case PROP_HOSTNAME:
			if (session->private->hostname)
				g_free ((gchar *) session->private->hostname);
			session->private->hostname = g_strdup (g_value_get_string (value));
			break;
		
		case PROP_DATABASE:
			if (session->private->database)
				g_free ((gchar *) session->private->database);
			session->private->database = g_strdup (g_value_get_string (value));
			break;
		
		case PROP_INFO:
			if (session->private->info)
				g_free ((gchar *) session->private->info);
			session->private->info = g_strdup (g_value_get_string (value));
			break;
	}

}

static void 
gsql_session_set_session_name (GSQLSession *session)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (session != NULL);

	gchar *name;
	gint i = 1;

	if (sessions == NULL)
		sessions = g_hash_table_new (g_str_hash, g_str_equal);
	else
		if (session->private->name)
			g_hash_table_remove (sessions, session->private->name);

	if (session->private->name)
		g_free (session->private->name);

	name = g_strdup_printf ("%s@%s (%s)", 
							g_utf8_strup(session->private->username,
										 g_utf8_strlen (session->private->username, 128)),
							g_utf8_strup(session->private->database,
										 g_utf8_strlen (session->private->database, 128)),
							session->private->info);
	
	while (g_hash_table_lookup (sessions, name))
	{
		g_free (name);

		name = g_strdup_printf ("%s@%s (%s)[%d] ", 
								g_utf8_strup(session->private->username,
											 g_utf8_strlen (session->private->username, 128)),
								g_utf8_strup(session->private->database,
											 g_utf8_strlen (session->private->database, 128)),
								session->private->info, i++);
	}

	session->private->name = name;
	
	g_hash_table_insert (sessions, name, session);

}

static void
gsql_session_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GSQLSession *session = GSQL_SESSION (widget);
	GtkWidget 	*child = GTK_WIDGET (session->private->workspace);
	
	GtkRequisition req, *wreq;
	
	gtk_widget_size_request (child, &req);

	gtk_widget_get_requisition (widget, wreq);
	wreq->width = 0;
	wreq->height = 0;
	
	wreq->width = MAX (wreq->width, req.width);
	wreq->height = MAX (wreq->height, req.height);
	
	wreq->width += gtk_container_get_border_width (GTK_CONTAINER (widget)) * 2;
	wreq->height += gtk_container_get_border_width (GTK_CONTAINER (widget)) * 2;

}

static void
gsql_session_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GSQLSession *session = GSQL_SESSION (widget);
	GtkWidget 	*child = GTK_WIDGET (session->private->workspace);
	GtkAllocation child_allocation;
	gint width, height;
	
	gtk_widget_set_allocation (widget, allocation);
	width = allocation->width - gtk_container_get_border_width (GTK_CONTAINER (widget))*2;
	height = allocation->height - gtk_container_get_border_width (GTK_CONTAINER (widget))*2;

	child_allocation.width = width;
	child_allocation.height = height;
	child_allocation.x = allocation->x;
	child_allocation.y = allocation->y;
	
	gtk_widget_size_allocate (child, &child_allocation);
	gtk_widget_show (child);
	
}


static void
gsql_session_add (GtkContainer *container, GtkWidget *widget)
{
	gsql_session_set_workspace (GSQL_SESSION (container), GSQL_WORKSPACE (widget));
}


static void
gsql_session_forall (GtkContainer *container,
					  gboolean      include_internals,
					  GtkCallback   callback,
					  gpointer      callback_data)
{
	GSQLSession *session = GSQL_SESSION (container);
	GtkWidget *child = GTK_WIDGET (session->private->workspace);
	
	g_return_if_fail (callback != NULL);
	
	if (child)
		(* callback) (child, callback_data);
	
}

static void 
on_menu_session_reconnect (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	session = gsql_session_get_active ();
	
	g_return_if_fail (session != NULL);
	
	g_signal_emit_by_name (G_OBJECT (session), "reopen");

}

static void 
on_menu_session_duplicate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	session = gsql_session_get_active ();
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	g_signal_emit_by_name (G_OBJECT (session), "duplicate");

}

static void 
on_menu_session_close (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	GtkDialog *dialog = NULL;
	guint ret;
	GtkTreeView *tv;
	GtkTreeModel *model;
	GtkTreeIter iter, child;
	GSQLContent *content;
	gboolean  bvalue;
	guint n;
	
	
	session = gsql_session_get_active ();
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	dialog = gsql_session_unsaved_dialog (session);
	
	if (dialog)
	{
		ret = gtk_dialog_run(dialog);
		
		switch (ret)
		{
			case GTK_RESPONSE_OK:
				tv = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (dialog), "treeview"));
				model = gtk_tree_view_get_model (tv);
				
				gtk_tree_model_get_iter_first (model, &iter);
				
				for (n=0; n < gtk_tree_model_iter_n_children (model, &iter); n++)
				{
					gtk_tree_model_iter_nth_child (model, &child, &iter, n);
					gtk_tree_model_get (model, &child,
										3, &content, -1);
					gtk_tree_model_get (model, &child,  
										0, &bvalue, -1);
					
					if (!bvalue)
						continue;
					
					if (GSQL_IS_CONTENT (content))
					{
						g_signal_emit_by_name (content, "save");	
							
					} else {
						
						GSQL_DEBUG ("It is not GSQLContent");
					}
				}
			
				break;
					
			case GTK_RESPONSE_CLOSE:
				break;
				
			default:
				gtk_widget_destroy (GTK_WIDGET (dialog));
				return;
		}
		
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
	
	gsql_engine_menu_set_status (session->engine, FALSE);
	
	g_signal_emit_by_name (G_OBJECT (session), "close");

}

static void 
on_menu_session_close_all (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GtkDialog *dialog;
	guint ret;
	GtkTreeView *tv;
	GtkTreeModel *model;
	GtkTreeIter iter, child;
	GSQLContent *content;
	GSQLSession *session;
	gboolean  bvalue;
	guint n;
	gchar *session_name;
	GList *list;
	
	dialog = gsql_session_unsaved_dialog (NULL);
	
	if (dialog)
	{
		ret = gtk_dialog_run(dialog);
		
		tv = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (dialog), "treeview"));
		model = gtk_tree_view_get_model (tv);
				
		gtk_tree_model_get_iter_first (model, &iter);
		
		switch (ret)
		{
			case GTK_RESPONSE_OK:
				
		
				do
				{
					gtk_tree_model_get (model, &iter, 2, &session_name, -1);
					
					GSQL_DEBUG ("Session name: [%s]", session_name);
					
					for (n=0; n < gtk_tree_model_iter_n_children (model, &iter); n++)
					{
						gtk_tree_model_iter_nth_child (model, &child, &iter, n);
						gtk_tree_model_get (model, &child,
											3, &content, -1);
						gtk_tree_model_get (model, &child,  
											0, &bvalue, -1);
					
						if (!bvalue)
							continue;
					
						if (GSQL_IS_CONTENT (content))
						{
							g_signal_emit_by_name (content, "save");	
							
						} else {
						
							GSQL_DEBUG ("It is not GSQLContent");
						}
					}
					
					session = g_hash_table_lookup (sessions, session_name);
					
					if (!GSQL_IS_SESSION (session)) 
					{
						GSQL_DEBUG ("Session not found. Bug!");
					} else {
						
						g_signal_emit_by_name (G_OBJECT (session), "close");
					}
												   
				}
				while (gtk_tree_model_iter_next (model, &iter));
				
				gsql_engine_menu_set_status (session->engine, FALSE);
			
				break;
				
			case GTK_RESPONSE_CANCEL:
				
				break;
				
			case GTK_RESPONSE_CLOSE:
			
				do
				{
					gtk_tree_model_get (model, &iter, 2, &session_name, -1);
					
					GSQL_DEBUG ("Session name: [%s]", session_name);
					
					session = g_hash_table_lookup (sessions, session_name);
					
					if (!GSQL_IS_SESSION (session)) 
					{
						GSQL_DEBUG ("Session not found. Bug!");
					} else {
						
						g_signal_emit_by_name (G_OBJECT (session), "close");
					}
												   
				}
				while (gtk_tree_model_iter_next (model, &iter));
				
				gsql_engine_menu_set_status (session->engine, FALSE);
				
				break;
				
			default:
				GSQL_DEBUG ("Unknown response. Bug!");
		} 
		
		
	} else {
		
		list = g_hash_table_get_values (sessions);
		list = g_list_first (list);
		
		while (list)
		{
			session = GSQL_IS_SESSION (list->data) ? GSQL_SESSION (list->data) : NULL;
			GSQL_DEBUG ("Have no unsaved. Session list != NULL");
			if (!session)
			{
				GSQL_DEBUG ("Session list. Unknown data in list. Bug");
				
			} else {
				
				g_signal_emit_by_name (G_OBJECT (session), "close");
				
			}
			
			gsql_engine_menu_set_status (session->engine, FALSE);
			
			list = g_list_next (list);
		}
		
		GSQL_DEBUG ("Have no unsaved. Session list processed");
	}
	
	if (dialog)
		gtk_widget_destroy (GTK_WIDGET (dialog));
	
}

static void
on_menu_session_commit (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	
	session = gsql_session_get_active ();
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	g_signal_emit_by_name (G_OBJECT (session), "commit");

}

static void
on_menu_session_rollback (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
	GSQLSession *session = NULL;
	
	session = gsql_session_get_active ();
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	g_signal_emit_by_name (G_OBJECT (session), "rollback");
	
}

static void
untitled_hash_remove_key_notify (gpointer data)
{
	GSQL_TRACE_FUNC;

	g_free (data);
}

static void
on_unsaved_dialog_enabled_toggled (GtkCellRendererToggle *cell,
								   gchar *path_str,
								   GtkTreeView *tv)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeIter iter, child;
	GtkTreeModel *model;
	GtkTreePath *path;
	gboolean bvalue;
	gpointer p = NULL;
	guint n;
	
	path = gtk_tree_path_new_from_string (path_str);
	model = gtk_tree_view_get_model (tv);
	gtk_tree_model_get_iter (model, &iter, path);
	
	gtk_tree_model_get (model, &iter,  
						3, 
						&p, -1);
	gtk_tree_model_get (model, &iter,  
						0, 
						&bvalue, -1);
	if (!p)
	{		
		for (n=0; n < gtk_tree_model_iter_n_children (model, &iter); n++)
		{
			gtk_tree_model_iter_nth_child (model, &child, &iter, n);
			
			gtk_tree_store_set(GTK_TREE_STORE (model), &child,
						0, !bvalue,
						-1);
		}
	} 
		
	gtk_tree_store_set(GTK_TREE_STORE (model), &iter,
						0, !bvalue,
						-1);
}
