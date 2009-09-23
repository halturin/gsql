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
#include <netdb.h>
#include <string.h>
#include <errno.h>

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

	SSH_SESSION *ssh;

	/* listen on */
	const gchar		*localname;
	guint			localport;
	gint			sock;

	/* forwaded from */
	const gchar		*fwdhost;
	guint			fwdport;

	/* list of CHANNELs */
	GList		*channels;

	gboolean	connected;
	gchar		err[SSH_SESSION_ERR_LEN];
};

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_TUNNEL_STOCK_ICON, "tunnel.png" }
};

static gboolean 
do_open_session (SSHSession *session);

static gboolean
do_open_channel (SSHSession *session);

static gboolean
do_listen_fwd (SSHSession *session);



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

	SSHSession session;

	session.connected = FALSE;
	session.hostname = "myhost.mynet";
	session.port = 22;
	session.username = "megauser";
	session.password = "megapassword";
	session.localname = "*";
	session.localport = 10051;

	do_open_session (&session);
	

	return TRUE;
}

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
}

static gboolean 
do_open_session (SSHSession *session)
{

	SSH_SESSION *ssh = ssh_new();
	SSH_OPTIONS *opts = ssh_options_new();
	

	ssh_options_set_port (opts, session->port);
	ssh_options_set_host (opts, session->hostname);
	
	ssh_set_options (ssh, opts);

	if (ssh_connect(ssh) != SSH_OK)
	{
		//ssh_options_free (opts);
		ssh_disconnect (ssh);

		SSH_SESSION_SET_ERROR (session, "Error at connection :%s\n", ssh_get_error (ssh));
		
		return FALSE;
	}

	ssh_is_server_known(ssh);

	if (ssh_userauth_autopubkey(ssh) != SSH_AUTH_SUCCESS)
	{
		g_debug ("Authenticating with pubkey: %s\n",ssh_get_error(ssh));
		
		if (ssh_userauth_password (ssh, session->username, 
		                           session->password) != SSH_AUTH_SUCCESS)
		{
			SSH_SESSION_SET_ERROR (session, "Authentication with password failed: %s\n",
			                       ssh_get_error (ssh));

			//ssh_options_free (opts);
			ssh_disconnect (ssh);
			
			return FALSE;
		}
	}

	session->ssh = ssh;

	if (!do_listen_fwd (session))
	{
		//ssh_options_free (opts);
		ssh_disconnect (ssh);
		
		return FALSE;
	}

	return session->connected = TRUE;
}

static gboolean
do_open_channel (SSHSession *session)
{
	GSQL_TRACE_FUNC;

	g_return_val_if_fail (session != NULL, FALSE);

	CHANNEL *ch = NULL;
	
	ch = channel_new (session->ssh);

	if (channel_open_forward (ch, session->fwdhost, session->fwdport, 
	                          session->localname, session->localport) != SSH_OK)
	{
		SSH_SESSION_SET_ERROR (session, "Error when opening forward:%s\n", 
		                       ssh_get_error (session->ssh));
		
		return FALSE;
	}

	g_debug ("Chanel is forwarded");

	return TRUE;
}


static gboolean
do_listen_fwd (SSHSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (session != NULL, FALSE);
	
	int sock, ret, i, n;
	struct addrinfo hints, *lres;
	gboolean wildcard = FALSE, lstatus = FALSE;
	gchar ntop[NI_MAXHOST], strport[NI_MAXSERV];

	if (!session)
	{
		g_warning ("do_listen_fwd: 'session' is NULL");
		return lstatus;
	}

	if ((strcmp (session->localname, "0.0.0.0") == 0) ||
	    (strcmp (session->localname, "*") == 0) ||
	    (session->localname == NULL ? 1 :
		     (*session->localname == '\0' ? 1 : 0)) )
	{
		wildcard = TRUE;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
	hints.ai_flags = wildcard ? AI_PASSIVE : 0;
	hints.ai_socktype = SOCK_STREAM;

	snprintf(strport, sizeof strport, "%d", session->localport);
	
	if (ret = getaddrinfo (session->localname, strport, 
	                       &hints, &lres) != 0)
	{
		g_warning ("do_listen_fwd (getaddrinfo): %s", gai_strerror (ret));

		return lstatus;
	}


	if ((lres->ai_family != AF_INET && lres->ai_family != AF_INET6) ||
		(getnameinfo(lres->ai_addr, lres->ai_addrlen, ntop, sizeof(ntop),
		                strport, sizeof(strport), NI_NUMERICHOST|NI_NUMERICSERV) != 0))
	{
		g_warning ("do_listen_fwd (getnameinfo): failed");
		freeaddrinfo (lres);
		return lstatus;

	}

	sock = socket(lres->ai_family, lres->ai_socktype, lres->ai_protocol);

	if (sock < 0)
	{
		g_warning ("do_listen_fwd (socket): %s",  strerror(errno));
		
		freeaddrinfo (lres);
		return lstatus;
	}

	i = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	if (bind(sock, lres->ai_addr, lres->ai_addrlen) < 0)
	{
		close (sock);
		freeaddrinfo (lres);
		
		return lstatus;
	}

	if (listen(sock, 128) < 0)
	{
		g_warning ("do_listen_fwd (listen): %s",  strerror(errno));

		close (sock);
		freeaddrinfo (lres);
		
		return lstatus;
	}

	lstatus = TRUE;
	session->sock = sock;

	freeaddrinfo (lres);
	
	return lstatus;
}

