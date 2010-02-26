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

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

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
static GList *ssh_list = NULL;

typedef struct _SSHSession		SSHSession;

#define SSH_SESSION_ERR_LEN	512

#define SSH_SESSION_SET_ERROR(session, params...) \
		memset (session->err, 0, 512); \
		g_snprintf (session->err, 512, params)


struct _SSHSession {

	/* connect to */
	const gchar *hostname;
	const gchar *username;
	const gchar *password;
	guint		port;

	ssh_session *ssh;

	/* listen on */
	const gchar		*localname;
	guint			localport;

	int			sock;

	/* forwaded from */
	const gchar		*fwdhost;
	guint			fwdport;

	/* list of CHANNELs */
	GList		*channels;

	ssh_channel *ch;

	gboolean	connected;
	gchar		err[SSH_SESSION_ERR_LEN];
};

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_TUNNEL_STOCK_ICON, "tunnel.png" }
};


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

	SSHSession *session = g_new0 (SSHSession, 1);

	return TRUE;
}

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
}


