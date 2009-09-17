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


#include <libgsql/plugins.h>
#include <libgsql/stock.h>
#include <gtk/gtk.h>

#include <libssh/libssh.h>

#include <libgsql/common.h>
#include "plugin_tunnel.h"
#include "tunnel_conf.h"



#define PLUGIN_VERSION "0.1"
#define PLUGIN_ID    "plugin_tunnel"
#define PLUGIN_NAME  "Tunnel"
#define PLUGIN_DESC  "SSH tunneling"
#define PLUGIN_AUTHOR "Taras Halturin"
#define PLUGIN_HOMEPAGE "http://gsql.org"

/* list of ssh sessions */
static GList *ssh_sessions = NULL;

typedef struct _SSHSession		SSHSession;

struct _SSHSession {

	/* connect to */
	const gchar *hostname;
	const gchar *password;
	guint		port;

	SSH_SESSION *ssh;

	/* listen on */
	const gchar		*localname;
	guint			localport;

	/* forwaded from */
	const gchar		*fwdhost;
	guint			fwdport;

	/* list of CHANNELs */
	GList		*channels;

	gboolean	connected;
	GError		*err;
};

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_TUNNEL_STOCK_ICON, "tunnel.png" }
};

static gboolean
do_open_channel (gchar *username, gchar *password, gchar *remote,
                 guint remote_port, guint local_port);



gboolean 
plugin_load (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;
	
	plugin->info.author = PLUGIN_AUTHOR;
	plugin->info.id = PLUGIN_ID;
	plugin->info.name = PLUGIN_NAME;
	plugin->info.desc = PLUGIN_DESC;
	plugin->info.homepage = PLUGIN_HOMEPAGE;
	plugin->info.version = PLUGIN_VERSION;
	plugin->file_logo = "tunnel.png";
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));

	plugin->plugin_conf_dialog = plugin_tunnel_conf_dialog;

	

	return TRUE;
}

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
}

static gboolean
do_open_channel (gchar *username, gchar *password, gchar *remote,
                 guint remote_port, guint local_port)
{
	SSH_SESSION *ssh = ssh_new();
	SSH_OPTIONS *opts = ssh_options_new();
	CHANNEL *ch = NULL;

	ssh_options_set_port (opts, 22);
	ssh_options_set_host (opts, remote);
	
	ssh_set_options (ssh, opts);

	if (ssh_connect(ssh) != SSH_OK)
	{
		g_warning ("Error at connection :%s\n",ssh_get_error (ssh));
		return FALSE;
	}

	ssh_is_server_known(ssh);

	if (ssh_userauth_autopubkey(ssh) != SSH_AUTH_SUCCESS)
	{
		g_warning ("Authenticating with pubkey: %s\n",ssh_get_error(ssh));
		
		if (ssh_userauth_password (ssh, username, password) != SSH_AUTH_SUCCESS)
		{
			g_warning ("Authentication with password failed: %s\n",ssh_get_error (ssh));
			return FALSE;
		}
	}

	ch = channel_new (ssh);

	if (channel_open_forward (ch, "localhost", remote_port, "127.0.0.1", local_port) != SSH_OK)
	{
		g_warning ("Error when opening forward:%s\n", ssh_get_error (ssh));
		return FALSE;
	}

	// g_debug ("bytes: %d", channel_write(ch,"   ",3));
	

	g_debug ("Chanel is forwarded");

	return TRUE;
	//channel_close(channel);
	//channel_free(channel);
	//ssh_disconnect(ssh);
}

// http://dev.libssh.org/wiki/Tutorial
//  CHANNEL *channel_open_forward(SSH_SESSION *session, char *remotehost, int remoteport, char *sourcehost, int localport); 

