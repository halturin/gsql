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
static GHashTable *tunnels = NULL;


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
on_connection_name_editing_started (GtkCellRendererText *renderer,
    							GtkCellEditable *editable,
    							gchar *path,
							  gpointer  user_data);

static void
do_set_image_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data);
static void
do_set_name_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data);
static void
do_set_connect_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data);

static void
tunnels_hash_remove_key_notify (gpointer user_data);

static void 
tunnels_hash_remove_value_notify (gpointer user_data);

static void
do_set_treeview_links (gpointer key, gpointer value, 
    					gpointer user_data);

static void on_entry_cshostname_changed (GtkEditable *editable, 
											gpointer user_data);
static gboolean on_entry_cshostname_unfocused (GtkWidget *widget,
												GdkEventFocus *event,
												gpointer user_data);
static void on_entry_csport_changed (GtkSpinButton *spinbutton,
									 gpointer user_data);
static void on_entry_csusername_changed (GtkEditable *editable, 
											gpointer user_data);
static void on_entry_cspassword_changed (GtkEditable *editable, 
											gpointer user_data);
static void on_entry_fslocaladr_changed (GtkEditable *editable, 
											gpointer user_data);
static void on_entry_fslocalport_changed (GtkSpinButton *spinbutton, 
											gpointer user_data);
static void on_entry_fsremoteadr_changed (GtkEditable *editable, 
											gpointer user_data);
static void on_entry_fsremoteport_changed (GtkSpinButton *spinbutton, 
											gpointer user_data);
static void on_rb_authpass_changed (GtkToggleButton *action,
											gpointer user_data);
static void on_rb_authpub_changed (GtkToggleButton *action,
											gpointer user_data);


void
plugin_tunnel_conf_load ()
{
	GSQL_TRACE_FUNC;

	GSList *lst, *flst;
	gchar	path[512];
	gchar	*str;
	guint	d;
	gboolean b;
	GSQLPTunnel	*link;
	
	static gboolean is_loaded = FALSE;

	if (is_loaded)
	{
		g_debug ("Plugin Tunnel: the config is already loaded");
		return;
	}

	flst = lst = gsql_conf_dir_list (GSQL_CONF_PLUGINS_ROOT_KEY "/tunnel/sessions");

	if (!lst)
		return;

	if (!tunnels)
		tunnels = g_hash_table_new_full (g_str_hash,
		    								g_str_equal,
		    								tunnels_hash_remove_key_notify,
		    								tunnels_hash_remove_value_notify);

	while (lst)
	{
		is_loaded = TRUE;

		link = gsqlp_tunnel_new ();
		
		str = g_path_get_basename (lst->data);
		g_snprintf (link->confname, 32, "%s", str);

		g_debug ("PARSE listing: [%s]", (gchar *) lst->data);
		
		g_hash_table_insert (tunnels, str, link);

		// remote host settings
		
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "name");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->name, 128, "%s", str);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "hostname");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->hostname, 128, "%s", str);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "username");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->username, 128, "%s", str);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "password");	
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->password, 128, "%s", str);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "auth_type");
		if (d = gsql_conf_value_get_int (path))
		{
			link->auth_type = d;
			
		} else {
			
			link->auth_type = GSQLP_TUNNEL_AUTH_PASS;
		}

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "port");	
		if (d = gsql_conf_value_get_int (path))
		{
			link->port = d;
			
		} else {

			link->port = 22;
		}

		// localhost settings

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "localname");	
		if (str = gsql_conf_value_get_string (path))
		{
			g_snprintf (link->localname, 128, "%s", str);
		} else {
			g_snprintf (link->localname, 128, "localhost");
			gsql_conf_value_set_string (path, link->localname);
		}
		
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "localport");	
		if (d = gsql_conf_value_get_int (path))
		{
			link->localport = d;
		} else {

			link->localport = 1025;
			gsql_conf_value_set_int (path, link->localport);
		}
		
		// forwarded host settings

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "fwdhost");
		if (str = gsql_conf_value_get_string (path))
			g_snprintf (link->fwdhost, 128, "%s", str);

		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "fwdport");
		g_debug ("path = [%s]", path);
		if (d = gsql_conf_value_get_int (path))
		{
			g_debug ("==================== %d", d);
			link->fwdport = d;
		} else {
			g_debug ("++++++++++++++++++++ %d", d);
			link->fwdport = 22;
			gsql_conf_value_set_int (path, link->fwdport);
		}
		// ----
		g_snprintf (path, 512, "%s/%s", (gchar *) lst->data, "autoconnect");
		b = gsql_conf_value_get_boolean (path);
		link->autoconnect = b;

		if (b) // autoconnect
		{
			g_debug ("Do autoconnect... %s", link->name);

			// call threaded func

			gsqlp_tunnel_do_connect (link);
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
	GtkCellRendererPixbuf *pxbuf;
	GtkTreeViewColumn *column;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkToggleButton *tb;

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
	// connect column
	rndt = (GtkCellRendererToggle *) gtk_builder_get_object (bld, "cellrenderer_connect");

	g_signal_connect (rndt, "toggled",
					  G_CALLBACK (on_connect_toggled), tv);
	column = (GtkTreeViewColumn *) gtk_builder_get_object (bld, "tvcolumn_connect");
	gtk_tree_view_column_set_cell_data_func (column, (GtkCellRenderer *) rndt, 
	    									do_set_connect_status,
	    									tv, NULL);

	// name column
	rnd = (GtkCellRendererText *) gtk_builder_get_object (bld, "cellrenderer_name");	

	g_signal_connect (rnd, "edited",
					  G_CALLBACK (on_connection_name_edited), tv);
	g_signal_connect (rnd, "editing-started",
					  G_CALLBACK (on_connection_name_editing_started), tv);

	column = (GtkTreeViewColumn *) gtk_builder_get_object (bld, "tvcolumn_name");
	gtk_tree_view_column_set_cell_data_func (column, (GtkCellRenderer *) rnd, 
	    									do_set_name_status,
	    									tv, NULL);

	// status column
	column = (GtkTreeViewColumn *) gtk_builder_get_object (bld, "tvcolumn_status");
	pxbuf = (GtkCellRendererPixbuf *) gtk_builder_get_object (bld, "cellrenderer_status");
	
	gtk_tree_view_column_set_cell_data_func (column, (GtkCellRenderer *) pxbuf, 
	    									do_set_image_status,
	    									tv, NULL);
	
	// callbacks for connection details
	entry = (GtkEntry *) gtk_builder_get_object (bld, "cshostname");
	g_signal_connect (entry, "changed",
	    			  G_CALLBACK (on_entry_cshostname_changed), tv);
	g_signal_connect (GTK_WIDGET (entry), "focus-out-event", 
	    			  G_CALLBACK (on_entry_cshostname_unfocused), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "hostname");

	entry = (GtkEntry *) gtk_builder_get_object (bld, "csport");
	g_signal_connect (entry, "value-changed",
	    			  G_CALLBACK (on_entry_csport_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "port");
	
	entry = (GtkEntry *) gtk_builder_get_object (bld, "csusername");
	g_signal_connect (entry, "changed",
	    			  G_CALLBACK (on_entry_csusername_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "username");

	entry = (GtkEntry *) gtk_builder_get_object (bld, "cspassword");
	g_signal_connect (entry, "changed",
	    			  G_CALLBACK (on_entry_cspassword_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "password");

	entry = (GtkEntry *) gtk_builder_get_object (bld, "fslocaladr");
	g_signal_connect (entry, "changed",
	    			  G_CALLBACK (on_entry_fslocaladr_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "localname");

	entry = (GtkEntry *) gtk_builder_get_object (bld, "fslocalport");
	g_signal_connect (entry, "value-changed",
	    			  G_CALLBACK (on_entry_fslocalport_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "localport");
	
	entry = (GtkEntry *) gtk_builder_get_object (bld, "fsremoteadr");
	g_signal_connect (entry, "changed",
	    			  G_CALLBACK (on_entry_fsremoteadr_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "remotename");

	entry = (GtkEntry *) gtk_builder_get_object (bld, "fsremoteport");
	g_signal_connect (entry, "value-changed",
	    			  G_CALLBACK (on_entry_fsremoteport_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (entry), "remoteport");

	tb = (GtkToggleButton *) gtk_builder_get_object (bld, "rb_auth_pass");
	g_signal_connect (tb, "toggled",
	    			  G_CALLBACK (on_rb_authpass_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (tb), "rb_auth_pass");

	tb = (GtkToggleButton *) gtk_builder_get_object (bld, "rb_auth_pub");
	g_signal_connect (tb, "toggled",
	    			  G_CALLBACK (on_rb_authpub_changed), tv);
	HOOKUP_OBJECT (G_OBJECT (tv), GTK_WIDGET (tb), "rb_auth_pub");

	g_hash_table_foreach (tunnels, do_set_treeview_links, tv);
	
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

	GSQLPTunnel *link;
	
	
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

	link = gsqlp_tunnel_new();

	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						2, link,
						-1);

	g_snprintf (link->name, 128, "%s", N_("enter name here"));
	g_snprintf (link->confname, 32, "link%d", i);
	
	g_hash_table_insert (tunnels, g_strdup (link->confname),
		    		link
		    		);

	path = gtk_tree_model_get_path (model, &iter);
	
	col = gtk_tree_view_get_column (tv, 1);
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
	GSQLPTunnel *tunnel;
	
	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);
	
	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	g_snprintf (tmp, 256, "%s/tunnel/sessions/%s", GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);
	gsql_conf_value_unset (tmp, TRUE);

	g_hash_table_remove (tunnels, tunnel->confname);
	
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
	GSQLPTunnel *tunnel;

	gchar str[128];
	
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_string (c_path);
	model = gtk_tree_view_get_model (tv);
	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_path_free (path);

	gtk_tree_model_get (model, &iter, 2, &tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));
	
	GSQL_DEBUG ("new:[%s]", new_text);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					   1, new_text,
					   -1);

	g_snprintf (str, 128, "%s/tunnel/sessions/%s/name", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, new_text);
	g_snprintf (tunnel->name, 128, "%s", new_text); 
	
}

static void
on_connection_name_editing_started (GtkCellRendererText *renderer,
    							GtkCellEditable *editable,
    							gchar *c_path,
							  gpointer  user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeView *tv = user_data;
	GSQLPTunnel *tunnel;

	gchar str[128];
	
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_string (c_path);
	model = gtk_tree_view_get_model (tv);
	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_path_free (path);

	gtk_tree_model_get (model, &iter, 2, &tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	if (GTK_IS_ENTRY (editable)) {
		GtkEntry *entry = GTK_ENTRY (editable);

		gtk_entry_set_text(entry, tunnel->name);
	}

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
	GSQLPTunnel *tunnel;
	GSQLPTunnelState state;
	gboolean bvalue;
	gpointer p = NULL;
	guint n;
	gchar *cname;
	gchar str[128];
	
	path = gtk_tree_path_new_from_string (path_str);
	model = gtk_tree_view_get_model (tv);
	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_path_free (path);
	
	gtk_tree_model_get (model, &iter,  
						0, 
						&bvalue, -1);

	gtk_tree_model_get (model, &iter, 2, &tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	state = gsqlp_tunnel_get_state (tunnel);
	
	g_snprintf (str, 128, "%s/tunnel/sessions/%s/autoconnect", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	if (!bvalue) 
	{
		gsqlp_tunnel_do_connect (tunnel);

	} else {

		if ((g_list_length (tunnel->channel_list) > 0) || 
			(state == GSQLP_TUNNEL_STATE_CONNECTING))
		{
			// do not allow dissconnect with active sessions
			// or connection in progress

			g_debug ("U can't do that. still have active sessions via this tunnel");
		
			return;
		}
		else
				gsqlp_tunnel_do_disconnect (tunnel);
	}
	
	gsql_conf_value_set_boolean (str, !bvalue);
	
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
	GtkEntry *entry;
	GSQLPTunnel *tunnel;
	GtkToggleButton *tb;
	
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

	if (!selected)
		return;
	
	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));
	
	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "hostname");
	gtk_entry_set_text (entry, tunnel->hostname);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "username");
	gtk_entry_set_text (entry, tunnel->username);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "password");
	gtk_entry_set_text (entry, tunnel->password);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "port");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (entry), tunnel->port);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "localname");
	gtk_entry_set_text (entry, tunnel->localname);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "localport");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (entry), tunnel->localport);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "remotename");
	if (!*tunnel->fwdhost)
	{
		// if the forwarded hostname is empty set the hostname of the connection 
		g_snprintf (tunnel->fwdhost, 128, "%s", tunnel->hostname);
	}
	gtk_entry_set_text (entry, tunnel->fwdhost);

	entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "remoteport");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (entry), tunnel->fwdport);

	switch (tunnel->auth_type) {

		case GSQLP_TUNNEL_AUTH_PUB:
			tb = (GtkToggleButton *) g_object_get_data (G_OBJECT (tv), "rb_auth_pub");
			gtk_toggle_button_set_active (tb, TRUE);
			break;

		case GSQLP_TUNNEL_AUTH_PASS:
		default:
			tb = (GtkToggleButton *) g_object_get_data (G_OBJECT (tv), "rb_auth_pass");
			gtk_toggle_button_set_active (tb, TRUE);


	}

}

static void
tunnels_hash_remove_key_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	g_free (user_data);
}

static void 
tunnels_hash_remove_value_notify (gpointer user_data)
{
	GSQL_TRACE_FUNC;

	g_object_unref (user_data);
}

static void
do_set_treeview_links (gpointer key, gpointer value, 
    					gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeModel	*model;
	GtkTreeIter		iter;
	GSQLPTunnel		*link = value;
	GSQLPTunnelState	state;
	gboolean b = FALSE;

	g_return_if_fail (GTK_IS_TREE_VIEW (user_data));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (user_data));

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	// 'connect' status

	state = gsqlp_tunnel_get_state (link);
	
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						0, (((state == GSQLP_TUNNEL_STATE_CONNECTED) || 
						     (state == GSQLP_TUNNEL_STATE_CONNECTING)) ? TRUE : FALSE),
						-1);
	// connection name
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						1, link->name,
						-1);

	// stored link%d name
	gtk_list_store_set(GTK_LIST_STORE (model), &iter,
						2, link,
						-1);

}

static void
do_set_image_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data)
{
	//GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel;
	GSQLPTunnelState	state;
	GtkTreeIter *it;
	gchar *stock;

	gtk_tree_model_get (model, iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	state = gsqlp_tunnel_get_state (tunnel);

	switch (state)
	{
		case GSQLP_TUNNEL_STATE_CONNECTED:
				stock = "gtk-yes";
				break;
		
		case GSQLP_TUNNEL_STATE_ERROR:
				stock = "gtk-no";
				break;

		case GSQLP_TUNNEL_STATE_NONE:
		default:
				stock = "gtk-disconnect";

	}

	g_object_set (G_OBJECT (rndr), "stock-id", stock, NULL);

}

static void
do_set_name_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data)
{
	//GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel;
	GSQLPTunnelState	state;
	GtkTreeIter *it;
	gchar str[GSQLP_TUNNEL_ERR_LEN];
	guint d = 0;
	GList *lst;
	
	gtk_tree_model_get (model, iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	state = gsqlp_tunnel_get_state (tunnel);

	switch (state)
	{
		case GSQLP_TUNNEL_STATE_CONNECTED:
			lst = g_list_first (tunnel->channel_list);
			d = g_list_length (lst);
			g_snprintf (str, GSQLP_TUNNEL_ERR_LEN, 
			    "%s\n<small><span color='darkgreen'>Connected.</span> Sessions: %d</small>",
			    tunnel->name,
			    d);

			break;

		case GSQLP_TUNNEL_STATE_CONNECTING:
			g_snprintf (str, GSQLP_TUNNEL_ERR_LEN, 
			    "%s\n<small><i>Connecting...</i></small>", 
			    tunnel->name,
			    tunnel->err);
			break;
		
		case GSQLP_TUNNEL_STATE_ERROR:
			g_snprintf (str, GSQLP_TUNNEL_ERR_LEN, 
			    "%s\n<small><span color='red'>Error: %s</span></small>", 
			    tunnel->name,
			    tunnel->err);

			break;

		case GSQLP_TUNNEL_STATE_NONE:
		default:
			g_snprintf (str, GSQLP_TUNNEL_ERR_LEN, "%s\n<small>Not connected</small>",
				    tunnel->name);

	}

	g_object_set (G_OBJECT (rndr), "markup", str, NULL);

}

static void
do_set_connect_status (GtkTreeViewColumn *column, GtkCellRenderer *rndr,
    					GtkTreeModel *model,
    					GtkTreeIter  *iter,
    					gpointer user_data)
{
	//GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel;
	GtkTreeIter *it;
	GSQLPTunnelState	state;
	gchar str[GSQLP_TUNNEL_ERR_LEN];
	
	gtk_tree_model_get (model, iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	state = gsqlp_tunnel_get_state (tunnel);

	if ( (g_list_length (tunnel->channel_list) > 0) || 
	     (state == GSQLP_TUNNEL_STATE_CONNECTING) )
	{
		// do not allow disconnect with active session
		// or connection in progress
		gtk_cell_renderer_set_sensitive (rndr, FALSE);
		
	} else {
		// have active sessions
		gtk_cell_renderer_set_sensitive (rndr, TRUE);
	}

}


static void 
on_entry_cshostname_changed (GtkEditable *editable, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	GtkEntry *entry;
	gboolean	bvalue;
	gchar *svalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	svalue = (gchar *) gtk_entry_get_text (GTK_ENTRY (editable));

	g_debug ("set new hostname: %s", svalue);

	g_snprintf (tunnel->hostname, 128, "%s", svalue);
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/hostname", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, tunnel->hostname);
	
}

static gboolean 
on_entry_cshostname_unfocused (GtkWidget *widget,
												GdkEventFocus *event,
												gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GtkEntry		*entry;

	g_return_val_if_fail (GTK_IS_TREE_VIEW (tv), FALSE);
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return FALSE;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_val_if_fail (GSQLP_IS_TUNNEL (tunnel), FALSE);

	if (!*tunnel->fwdhost)
	{
		entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "remotename");
			// if the forwarded hostname is empty set the hostname of the connection 
		g_snprintf (tunnel->fwdhost, 128, "%s", tunnel->hostname);

		gtk_entry_set_text (entry, tunnel->fwdhost);
	}

	if (!*tunnel->localname)
	{
		entry = (GtkEntry *) g_object_get_data (G_OBJECT (tv), "localname");
			// if the local hostname is empty set "localhost"
		g_snprintf (tunnel->localname, 128, "%s", "localhost");

		gtk_entry_set_text (entry, tunnel->localname);
	}
	

	return FALSE;

}

static void 
on_entry_csport_changed (GtkSpinButton *spinbutton,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	guint ivalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	ivalue = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

	g_debug ("set new port: %d", ivalue);

	tunnel->port = ivalue;
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/port", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_int (str, tunnel->port);

}

static void 
on_entry_csusername_changed (GtkEditable *editable, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	gchar *svalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	svalue = (gchar *) gtk_entry_get_text (GTK_ENTRY (editable));

	g_debug ("set new username: %s", svalue);

	g_snprintf (tunnel->username, 128, "%s", svalue);
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/username", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, tunnel->username);

}

static void 
on_entry_cspassword_changed (GtkEditable *editable, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	gchar *svalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	svalue = (gchar *) gtk_entry_get_text (GTK_ENTRY (editable));

	g_debug ("set new password: %s", svalue);

	g_snprintf (tunnel->password, 64, "%s", svalue);
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/password", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, tunnel->password);

}

static void 
on_entry_fslocaladr_changed (GtkEditable *editable, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	gchar *svalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	svalue = (gchar *) gtk_entry_get_text (GTK_ENTRY (editable));

	g_debug ("set new local name: %s", svalue);

	g_snprintf (tunnel->localname, 64, "%s", svalue);
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/localname", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, tunnel->localname);

}

static void 
on_entry_fslocalport_changed (GtkSpinButton *spinbutton, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	guint ivalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	ivalue = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

	g_debug ("set new local port: %d", ivalue);

	tunnel->localport = ivalue;
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/localport", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_int (str, tunnel->localport);

}

static void 
on_entry_fsremoteadr_changed (GtkEditable *editable, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	gchar *svalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	svalue = (gchar *) gtk_entry_get_text (GTK_ENTRY (editable));

	g_debug ("set new remote host: %s", svalue);

	g_snprintf (tunnel->fwdhost, 64, "%s", svalue);
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/fwdhost", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_string (str, tunnel->fwdhost);

}

static void 
on_entry_fsremoteport_changed (GtkSpinButton *spinbutton, 
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel;
	GSQLPTunnelState	state;
	gboolean	bvalue;
	guint ivalue;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));
	model = gtk_tree_view_get_model (tv);

	sel = gtk_tree_view_get_selection (tv);


	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	ivalue = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

	g_debug ("set new fwd port: %d", ivalue);

	tunnel->fwdport = ivalue;
	
	g_snprintf (str, 256, "%s/tunnel/sessions/%s/fwdport", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);

	gsql_conf_value_set_int (str, tunnel->fwdport);

}

static void on_rb_authpass_changed (GtkToggleButton * button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel = NULL;
	gboolean	ret;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));

	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);

	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));
	
	ret = gtk_toggle_button_get_active (button);
	
	g_debug ("Auth by PASS: %s", ret ? "True": "False");
	if (ret)
	{
		tunnel->auth_type = GSQLP_TUNNEL_AUTH_PASS;

		g_snprintf (str, 256, "%s/tunnel/sessions/%s/auth_type", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);
		
		gsql_conf_value_set_int (str, tunnel->auth_type);
	}
	


}

static void on_rb_authpub_changed (GtkToggleButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeSelection *sel;
	GtkTreeView	*tv = user_data;
	GtkTreeModel *model;
	GtkTreeIter		iter;
	GSQLPTunnel		*tunnel = NULL;
	gboolean	ret;
	gchar str[256];

	g_return_if_fail (GTK_IS_TREE_VIEW (tv));

	model = gtk_tree_view_get_model (tv);
	sel = gtk_tree_view_get_selection (tv);

	if (!gtk_tree_selection_get_selected (sel, &model, &iter))
		return;

	gtk_tree_model_get (model, &iter,  
						2, 
						&tunnel, -1);

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));
	
	ret = gtk_toggle_button_get_active (button);
	
	g_debug ("Auth by PUB: %s", ret ? "True": "False");
	if (ret)
	{
		tunnel->auth_type = GSQLP_TUNNEL_AUTH_PUB;

		g_snprintf (str, 256, "%s/tunnel/sessions/%s/auth_type", 
	    					GSQL_CONF_PLUGINS_ROOT_KEY, tunnel->confname);
		
		gsql_conf_value_set_int (str, tunnel->auth_type);
	}


}

