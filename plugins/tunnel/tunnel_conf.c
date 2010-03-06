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


#include <config.h>
#include <libgsql/conf.h>
#include <libgsql/common.h>
#include <tunnel_conf.h>
#include <plugin_tunnel.h>

#define GSQLP_TUNNEL_GLADE_DIALOG PACKAGE_GLADE_DIR"/plugins/tunnel_config.xml"

/* list of ssh sessions */
static GHashTable *ssh_links = NULL;


static void
on_conf_button_new_activate (GtkButton *button,
											gpointer user_data);

static void
on_conf_button_remove_activate (GtkButton *button,
											gpointer user_data);

static void
on_connect_toggled (GtkCellRendererToggle *cell,
								   gchar *path_str,
								   GtkTreeView *tv);

static void
on_tv_cursor_changed (GtkTreeView *tv,
								   gpointer user_data);

static void
on_connection_name_edited (GtkCellRendererText *renderer,
							  gchar		*c_path,
							  gchar		*new_text,
							  gpointer  user_data);

static void
ssh_links_hash_remove_key_notify (gpointer user_data);

static void 
ssh_links_hash_remove_value_notify (gpointer user_data);

static void
do_set_treeview_links (gpointer key, gpointer value, 
    					gpointer user_data);

// gconf notifiers
static void tunnel_name_notify (gpointer user_data);
static void tunnel_hostname_notify (gpointer user_data);
static void tunnel_username_notify (gpointer user_data);
static void tunnel_password_notify (gpointer user_data);
static void tunnel_port_notify (gpointer user_data);
static void tunnel_localname_notify (gpointer user_data);
static void tunnel_localport_notify (gpointer user_data);
static void tunnel_fwdhost_notify (gpointer user_data);
static void tunnel_fwdport_notify (gpointer user_data);
static void tunnel_autoconnect_notify (gpointer user_data);


void
plugin_tunnel_conf_load ()
{
	GSQL_TRACE_FUNC;

	GSList *lst, *flst;
	gchar	path[512];
	gchar	*str;
	guint	d;
	gboolean b;
	SSHLink	*link;
	
	static gboolean is_loaded = FALSE;

	if (is_loaded)
	{
		g_debug ("Plugin Tunnel: the config is already loaded");
		return;
	}

	flst = lst = gsql_conf_dir_list (GSQL_CONF_PLUGINS_ROOT_KEY "/tunnel/sessions");

	if (!lst)
		return;

	if (!ssh_links)
		ssh_links = g_hash_table_new_full (g_str_hash,
		    								g_str_equal,
		    								ssh_links_hash_remove_key_notify,
		    								ssh_links_hash_remove_value_notify);

	while (lst)
	{
		is_loaded = TRUE;
		g_debug ("PARSE listing: [%s]", (gchar *) lst->data);

		// we dont call  we store it into hash table

		link = g_new0 (SSHLink, 1);
		
		str = g_path_get_basename (lst->data);
		g_hash_table_insert (ssh_links, str, link);

		link->ssh = ssh_new ();
		
		if (!link->ssh)
		{
			g_debug ("Couldn't allocate new SSH session");
			break;
		}

		// remote host settings
		
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "name");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->name, 128, "%s", str);

//		gsql_conf_nitify_add (path, tunnel_name_notify, link);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "hostname");	
		if (str = gsql_conf_value_get_string (path))
		{
			g_snprintf (link->hostname, 128, "%s", str);

			ssh_options_set (link->ssh, SSH_OPTIONS_HOST, link->hostname);
		}

//		gsql_conf_nitify_add (path, tunnel_hostname_notify, link);
		
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "username");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->username, 128, "%s", str);

//		gsql_conf_nitify_add (path, tunnel_username_notify, link);
		
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "password");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->password, 128, "%s", str);

//		gsql_conf_nitify_add (path, tunnel_password_notify, link);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "port");	
		if (d = gsql_conf_value_get_int (path))
		{
			link->port = d;

			ssh_options_set (link->ssh, SSH_OPTIONS_PORT, &link->port);
		}

//		gsql_conf_nitify_add (path, tunnel_port_notify, link);
		
		// localhost settings

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "localname");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->localname, 128, "%s", str);

//		gsql_conf_nitify_add (path, tunnel_localname_notify, link);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "localport");	
		if (d = gsql_conf_value_get_int (path))
			link->localport = d;

//		gsql_conf_nitify_add (path, tunnel_localport_notify, link);
		
		// forwarded host settings

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "fwdhost");
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->fwdhost, 128, "%s", str);

//		gsql_conf_nitify_add (path, tunnel_fwdhost_notify, link);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "fwdport");	
		if (d = gsql_conf_value_get_int (path))
			link->fwdport = d;

//		gsql_conf_nitify_add (path, tunnel_fwdport_notify, link);

		// ----
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "autoconnect");
		b = gsql_conf_value_get_boolean (path);

//		gsql_conf_nitify_add (path, tunnel_autoconnect_notify, link);

		if (b) // autoconnect
		{
			g_debug ("Do autoconnect... %s", link->name);

			// call threaded func
		}
		
		g_free(lst->data);
		lst = lst->next;
		
	}

	g_slist_free (flst);

}

void
plugin_tunnel_conf_dialog ()
{
	GSQL_TRACE_FUNC;

	GtkBuilder *bld;
	GtkDialog *dialog;
	GError *err = NULL;
	GtkButton *button;
	GtkTreeView *tv;
	GtkCellRendererToggle *rndt;
	GtkCellRendererText *rnd;
	GtkWidget *widget;

	bld = gtk_builder_new();

	gtk_builder_add_from_file (bld, GSQLP_TUNNEL_GLADE_DIALOG, &err);

	dialog = GTK_DIALOG (gtk_builder_get_object (bld, "tunnel_config_dialog"));

	if (!dialog) 
		return;

	tv = GTK_TREE_VIEW (gtk_builder_get_object (bld, "treeview_sessions"));

	button = GTK_BUTTON (gtk_builder_get_object (bld, "button_new"));
	
	g_signal_connect ((gpointer) button, "clicked",
						G_CALLBACK (on_conf_button_new_activate),
						tv);

	button = GTK_BUTTON (gtk_builder_get_object (bld, "button_remove"));

	widget = GTK_WIDGET (gtk_builder_get_object (bld, "parameters_box"));

	g_signal_connect ((gpointer) tv, "cursor-changed",
						G_CALLBACK (on_tv_cursor_changed),
						widget);
	
	g_signal_connect ((gpointer) button, "clicked",
						G_CALLBACK (on_conf_button_remove_activate),
						tv);

	rndt = (GtkCellRendererToggle *) gtk_builder_get_object (bld, "cellrenderer_connect");
	
	g_signal_connect (rndt, "toggled",
					  G_CALLBACK (on_connect_toggled), tv);

	rnd = (GtkCellRendererText *) gtk_builder_get_object (bld, "cellrenderer_name");
	
	g_signal_connect (rnd, "edited",
					  G_CALLBACK (on_connection_name_edited), tv);

	g_hash_table_foreach (ssh_links, do_set_treeview_links, tv);
	
	gtk_dialog_run (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
on_conf_button_new_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GtkTreeIter iter;
	GtkTreeView *tv = user_data;
	GtkTreeModel *model;
	GtkTreePath  *path;
	GtkTreeViewColumn *col;
	gboolean bvalue = FALSE;
	gchar tmp[256];
	gint i;

	SSHLink *link;
	
	
	i = 1;
	while (i < 128)
	{
		g_snprintf (tmp, 256 ,"%s/tunnel/sessions/link%d", GSQL_CONF_PLUGINS_ROOT_KEY, i);
		
		if (!gsql_conf_dir_exist (tmp))
			break;

		i++;
	}

	g_return_if_fail (i<128);

	g_snprintf (tmp, 256, "%s/tunnel/sessions/link%d/name", GSQL_CONF_PLUGINS_ROOT_KEY, i);
	gsql_conf_value_set_string (tmp, N_("enter name here"));

	model = gtk_tree_view_get_model (tv);
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						0, bvalue,
						-1);

	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						1, N_("enter name here"),
						-1);

	g_snprintf (tmp, 12, "link%d", i);
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						2, tmp,
						-1);

	link = g_new0 (SSHLink, 1);
	g_snprintf (link->name, 128, "%s", N_("enter name here"));
	
	g_hash_table_insert (ssh_links, g_strdup (tmp),
		    		link
		    		);

	path = gtk_tree_model_get_path (model, &iter);
	
	col = gtk_tree_view_get_column (tv, 0);
	gtk_tree_view_set_cursor (tv, path, col, TRUE);

}

static void
on_conf_button_remove_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	GtkTreeView *tv = user_data;
	GtkTreeModel *model;
	GtkTreeSelection *sel = NULL;
	GtkTreeIter iter;
	gchar tmp[256];
	gchar *link;
	
	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);
	
	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&link, -1);

	g_snprintf (tmp, 256, "%s/tunnel/sessions/%s", GSQL_CONF_PLUGINS_ROOT_KEY, link);
	gsql_conf_value_unset (link, TRUE);

	g_hash_table_remove (ssh_links, link);
	
	gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

	g_signal_emit_by_name (tv, "cursor-changed");

}

static void
on_connection_name_edited (GtkCellRendererText *renderer,
							  gchar		*c_path,
							  gchar		*new_text,
							  gpointer  user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeView *tv = user_data;
	
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_string (c_path);
	model = gtk_tree_view_get_model (tv);
	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_path_free (path);

	GSQL_DEBUG ("new:[%s]", new_text);

	//g_snprintf(tmp, 256 ,"%s/tunnel/link%s/%s", GSQL_CONF_PLUGINS_ROOT_KEY, new_text);
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					   1, new_text,
					   -1);
	
}

static void
on_connect_toggled (GtkCellRendererToggle *cell,
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

	gtk_tree_path_free (path);
	
	gtk_tree_model_get (model, &iter,  
						0, 
						&bvalue, -1);

	
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						0, !bvalue,
						-1);
}

static void
on_tv_cursor_changed (GtkTreeView *tv,
								   gpointer user_data)

{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean bvalue = FALSE;
	gboolean selected;
	
	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);

	selected = gtk_tree_selection_get_selected (sel, &model, &iter);

	if (selected)
	{
		gtk_tree_model_get (model, &iter,  
						0, 
						&bvalue, -1);

		
	}
	
	gtk_widget_set_sensitive (GTK_WIDGET (user_data),
							  !bvalue && selected);

}

static void
ssh_links_hash_remove_key_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	g_free (user_data);
}

static void 
ssh_links_hash_remove_value_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	// here we need to free all of SSHLink structure
	

}

static void
do_set_treeview_links (gpointer key, gpointer value, 
    					gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel	*model;
	GtkTreeIter		iter;
	SSHLink			*link = value;

	g_return_if_fail (GTK_IS_TREE_VIEW (user_data));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (user_data));

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	// 'connect' status

	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						0, (link->state == GSQLP_TUNNEL_STATE_CONNECTED ? TRUE : FALSE),
						-1);
	// connection name
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						1, link->name,
						-1);

	// stored link%d name
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						2, key,
						-1);

}

static void 
tunnel_name_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;
	gchar *key;
	gchar path[512];

	key = (gchar *) g_hash_table_lookup (ssh_links, link);

	g_snprintf (path, 512, "%s/tunnel/%s/name", GSQL_CONF_PLUGINS_ROOT_KEY, key);

	g_debug ("notify PATH: %s", path);

}


static void tunnel_hostname_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_username_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_password_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_port_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_localname_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_localport_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_fwdhost_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_fwdport_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


static void tunnel_autoconnect_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	SSHLink *link = user_data;

}


