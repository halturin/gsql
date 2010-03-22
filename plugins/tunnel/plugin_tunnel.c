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

#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <libgsql/common.h>
#include "plugin_tunnel.h"
#include "tunnel_conf.h"



#define PLUGIN_VERSION "0.1"
#define PLUGIN_ID    "plugin_tunnel"
#define PLUGIN_NAME  "Tunnel"
#define PLUGIN_DESC  "SSH tunneling"
#define PLUGIN_AUTHOR "Taras Halturin"
#define PLUGIN_HOMEPAGE "http://gsql.org"

static GObjectClass *parent_class;
static void gsqlp_tunnel_class_init (GSQLPTunnelClass *klass);
static void gsqlp_tunnel_init (GSQLPTunnel *obj);

static const gchar *common_keys[] = {
	".ssh/id_rsa",
	".ssh/id_dsa",
	".ssh/identity",
	NULL
};

struct _GSQLPTunnelPrivate {

	GSQLPTunnelState	state;
};

enum {
	SIG_STATE_CHANGED,
	SIG_LAST
};

static guint tunnel_signals[SIG_LAST] = { 0 };

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

	ssh_init ();
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));

	plugin->plugin_conf_dialog = plugin_tunnel_conf_dialog;

	plugin_tunnel_conf_load ();

	return TRUE;
}

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
}

GType
gsqlp_tunnel_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLPTunnelClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsqlp_tunnel_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLPTunnel),
			0,
			(GInstanceInitFunc) gsqlp_tunnel_init,
			NULL
		};
		obj_type = g_type_register_static (G_TYPE_OBJECT,
										   "GSQLPTunnel", &obj_info, 0);
		
	}
	
	return obj_type;	
}


GSQLPTunnelState
gsqlp_tunnel_get_state (GSQLPTunnel *tunnel)
{
//	GSQL_TRACE_FUNC;

	g_return_val_if_fail (GSQLP_IS_TUNNEL (tunnel), GSQLP_TUNNEL_STATE_ERROR);
	
	return tunnel->private->state;	
}

static void 
gsqlp_tunnel_set_state (GSQLPTunnel *tunnel, GSQLPTunnelState state)
{
	GSQL_TRACE_FUNC;

	GSQLP_TUNNEL_LOCK(tunnel);
	
	if (state == GSQLP_TUNNEL_STATE_CONNECTED)
	// clear error message
	    memset (tunnel->err, 0, GSQLP_TUNNEL_ERR_LEN);
	
	tunnel->private->state = state;
	GSQLP_TUNNEL_UNLOCK(tunnel)
	
	g_signal_emit_by_name (G_OBJECT (tunnel), "state-changed");

}

GSQLPTunnel *
gsqlp_tunnel_new (void)
{
	GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel;
	
	tunnel = g_object_new (GSQLP_TUNNEL_TYPE, NULL);	
	
	return tunnel;
}


static void
gsqlp_tunnel_dispose (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel = GSQLP_TUNNEL (obj);
	
	parent_class->dispose(obj);
	
}

static void
gsqlp_tunnel_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel = GSQLP_TUNNEL (obj);

	g_free (tunnel->private);

	pthread_mutex_destroy (&tunnel->mutex);

	parent_class->finalize (obj);
}

static void
gsqlp_tunnel_class_init (GSQLPTunnelClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class;
	
	g_return_if_fail (klass != NULL);
	obj_class = (GObjectClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	
	tunnel_signals [SIG_STATE_CHANGED] = 
		g_signal_new ("state_changed", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLPTunnelClass,
									   state_changed),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	obj_class->dispose = gsqlp_tunnel_dispose;
	obj_class->finalize = gsqlp_tunnel_finalize;
	
}

static void 
gsqlp_tunnel_init (GSQLPTunnel *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLPTunnelPrivate, 1);
	obj->private->state = GSQLP_TUNNEL_STATE_NONE;

	pthread_mutex_init (&obj->mutex, NULL);
	obj->channel_list = NULL;

}

static void
tunnel_channel_remove (GSQLPTunnel *tunnel, GList *rch)
{
	GSQL_TRACE_FUNC;

	GSQLPChannel *pch = rch->data;

	GSQLP_TUNNEL_LOCK(tunnel);

	channel_send_eof(pch->channel);

	channel_close (pch->channel);
	close (pch->sock);
	
	tunnel->channel_list = g_list_remove (tunnel->channel_list, pch);
	tunnel->channel_list = g_list_last (tunnel->channel_list);
	
	g_free (pch);

	GSQLP_TUNNEL_UNLOCK(tunnel);
}

static gboolean
tunnel_channel_add (GSQLPTunnel *tunnel, ssh_channel channel, gint sock)
{
	GSQL_TRACE_FUNC;

	gint flags;
	GSQLPChannel *pch = NULL;

	GSQLP_TUNNEL_LOCK(tunnel);

	pch = g_new0 (GSQLPChannel, 1);

	if (!pch)
	{
		g_debug ("Couldn't allocate GSQLPChannel structure");

		GSQLP_TUNNEL_UNLOCK(tunnel);
		
		return FALSE;
	}

	pch->channel = channel;
	pch->rx = pch->tx = 0;
	pch->sock = sock;

	flags = fcntl (sock, F_GETFL, 0);
	fcntl (sock, F_SETFL, flags | O_NONBLOCK);
	
	tunnel->channel_list = g_list_append (tunnel->channel_list, pch);
	tunnel->channel_list = g_list_last (tunnel->channel_list);

	GSQLP_TUNNEL_UNLOCK(tunnel);

	return TRUE;
}

static gpointer
tunnel_processing_thread (gpointer p)
{
	GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel = p;
	GSQLPChannel *pch;
	GList	*lst, *rem;
	struct timeval tv;
	struct timespec ts;
	gint i, lenr, lenw, fdmax;
	fd_set fds;
	gboolean broken;

#define CHANNEL_BUFF 32768
	gchar buff[CHANNEL_BUFF]; /* 32K seems to be enough */

	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; /* 0.1sec */
	
	while (1)
	{
		// the channel_list are pointer to the last item.
		GSQLP_TUNNEL_LOCK(tunnel);
		lst = tunnel->channel_list;
		GSQLP_TUNNEL_UNLOCK(tunnel);

		if (!tunnel->ssh)
		{
			// has been closed
			break;
		}

		if (!lst)
		{
			g_debug ("sleeping");
			nanosleep (&ts, NULL);	

			continue;
		}
		FD_ZERO (&fds);
		fdmax = 0;
		
		/* reading from the channels and writing to the sockets */
		do
		{
			broken = FALSE;
			pch = (GSQLPChannel *) lst->data;

			lenr = 0;
			lenr = channel_poll (pch->channel, FALSE);

			
			if ((lenr == SSH_EOF) || (lenr == SSH_ERROR))
			{
				g_debug ("channel_pool return SSH_EOF or SSH_ERROR. remove it.");

				rem = lst;
				lst = g_list_previous (lst);
				
				tunnel_channel_remove (tunnel, rem);

				continue;
				
			}

			FD_SET (pch->sock, &fds);
			fdmax = (fdmax > pch->sock) ? fdmax : pch->sock;
			
			if (lenr == 0)
			{
				lst = g_list_previous (lst);

				continue;
			}

			while (lenr > 0)
			{
				memset (buff, 0, CHANNEL_BUFF);
				i = channel_read_nonblocking (pch->channel, buff, CHANNEL_BUFF, FALSE);

				if ( ((i == 0) && (channel_is_eof (pch->channel))) || (i == SSH_ERROR))
				{
					g_debug ("channel_read_nonblocking return SSH_EOF or SSH_ERROR. remove it.");

					FD_CLR (pch->sock, &fds);
					
					broken = TRUE;
					
					break;
				}

				lenw = write (pch->sock, buff, i);
			
				if ((lenw == -1) && (errno == EAGAIN))
				{
					//g_debug ("error write. EAGAIN");
					continue;
				}

				if (lenw == -1)
				{
					g_debug ("write (to socket) return -1. remove it.");

					FD_CLR (pch->sock, &fds);

					broken = TRUE;

					break;
				}

				pch->rx +=lenw;

				lenr -= i;

			} // while (lenr > 0) 

			if (!broken)
			{
				lst = g_list_previous (lst);

			} else {

				rem = lst;
				lst = g_list_previous (lst);
				
				tunnel_channel_remove (tunnel, rem);
			}
			
		} while (lst);

		/* reading from the sockets and writing to the channels */

		fdmax++;
		tv.tv_sec = 0;
		tv.tv_usec = 20000;
		
		lenr = select (fdmax, &fds, NULL, NULL, &tv);

		if (lenr == -1)
		// seems to be closed. break this loop.
			break;

		if (lenr == 0)
		// nothing to be read
			continue;

		GSQLP_TUNNEL_LOCK(tunnel);
		lst = tunnel->channel_list;
		GSQLP_TUNNEL_UNLOCK(tunnel);
		
		do
		{
			broken = FALSE;
			pch = lst->data;
			
			if (FD_ISSET (pch->sock, &fds))
			{
				memset (buff, 0, CHANNEL_BUFF);

				while (1)
				{
					lenr = read (pch->sock, buff, CHANNEL_BUFF);
					
					if ((lenr == -1) && (errno == EAGAIN))
						break;
					
					if (lenr == -1)
					{
						g_debug ("error. removing....");
						broken = TRUE;

						break;
					}

					
					lenw = channel_write (pch->channel, buff, lenr);
					//g_debug ("lenr = %d", lenr);

					if (lenw == SSH_ERROR)
					{
						g_debug ("channel_write return SSH_ERROR. remove it.");
						broken = TRUE;
						
						break;
					}

					pch->tx += lenw;
				}
			}

			if (!broken)
			{
				lst = g_list_previous (lst);

			} else {

				rem = lst;
				lst = g_list_previous (lst);
				
				tunnel_channel_remove (tunnel, rem);
				
			}

		} while (lst);		
	}

	g_debug ("++++++ out from tunnel_processing_thread");

	lst = tunnel->channel_list;

	while (lst)
	{
		rem = lst;

		lst = g_list_previous (lst);
				
		tunnel_channel_remove (tunnel, rem);
	}

	tunnel->channel_list = NULL;

	g_debug ("------ out from tunnel_processing_thread");
	
}

static gpointer
do_connect_bg (gpointer p)
{
	GSQL_TRACE_FUNC;

	GSQLPTunnel *tunnel = p;
	gboolean wildcard = FALSE;
	struct addrinfo hints, *ai;
	int sock = 0, i;
	gchar ntop[NI_MAXHOST], strport[NI_MAXSERV];
	char *hexa;
	unsigned char *hash = NULL;
	GSQLPTunnelState state;

	static GThread *thread = NULL;
	static GError *error = NULL;

	ssh_channel channel = NULL;
	struct timespec ts;
	
	gint ret;

	memset (tunnel->err, 0, GSQLP_TUNNEL_ERR_LEN);
	
	if (tunnel->ssh) 
	{
		if ((tunnel->private->state == GSQLP_TUNNEL_STATE_NONE) || 
			 (tunnel->private->state == GSQLP_TUNNEL_STATE_ERROR))
		{
			ssh_free (tunnel->ssh);
			tunnel->ssh = ssh_new ();
		} else 
			return NULL;
	} else
		tunnel->ssh = ssh_new ();

	gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_CONNECTING);
	state = GSQLP_TUNNEL_STATE_CONNECTING;
	
	ssh_options_set (tunnel->ssh, SSH_OPTIONS_HOST, tunnel->hostname);
	ssh_options_set (tunnel->ssh, SSH_OPTIONS_USER, tunnel->username);
	ssh_options_set (tunnel->ssh, SSH_OPTIONS_PORT, &tunnel->port);
	i = 30;
	ssh_options_set (tunnel->ssh, SSH_OPTIONS_TIMEOUT, &i);

	if (ssh_connect(tunnel->ssh))
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", ssh_get_error (tunnel->ssh));

		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);
		tunnel->autoconnect = FALSE;

		ssh_disconnect (tunnel->ssh);
		
		return NULL;
	}
	
	ret = ssh_is_server_known (tunnel->ssh);
	i = ssh_get_pubkey_hash(tunnel->ssh, &hash);

	if (i > 0)
	{	
		switch (ret)
		{
			case SSH_SERVER_KNOWN_OK:
				break;

			case SSH_SERVER_KNOWN_CHANGED:
				GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", N_("Host key for server changed"));

				state = GSQLP_TUNNEL_STATE_ERROR;
				
				break;

			case SSH_SERVER_FOUND_OTHER:
				GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", N_("The host key not found"));

				state = GSQLP_TUNNEL_STATE_ERROR;

				break;

			case SSH_SERVER_FILE_NOT_FOUND:
			case SSH_SERVER_NOT_KNOWN:
				if (ssh_write_knownhost(tunnel->ssh) < 0)
				{
					GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", strerror(errno));

					state = GSQLP_TUNNEL_STATE_ERROR;
				}

				break;

			case SSH_SERVER_ERROR:
				GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", ssh_get_error (tunnel->ssh));
				
				state = GSQLP_TUNNEL_STATE_ERROR;

				break;
		}
		
	} else {

		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", N_("Incorrect public key hash"));
		
		state = GSQLP_TUNNEL_STATE_ERROR;
	}

	if (state == GSQLP_TUNNEL_STATE_ERROR)
	{
		tunnel->autoconnect = FALSE;
		gsqlp_tunnel_set_state (tunnel, state);
		
		if (hash)
			free (hash);

		ssh_disconnect (tunnel->ssh);

		return NULL;
	}

	if (hash)
		free (hash);

	switch (tunnel->auth_type) {

		case GSQLP_TUNNEL_AUTH_PUB:
			ret = ssh_userauth_autopubkey(tunnel->ssh, NULL);
			break;

		case GSQLP_TUNNEL_AUTH_PASS:
		default:
			ret = ssh_userauth_password (tunnel->ssh, tunnel->username, tunnel->password);
			break;
	}
	
	if (ret != SSH_AUTH_SUCCESS)
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", ssh_get_error (tunnel->ssh));

		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);
		tunnel->autoconnect = FALSE;

		ssh_disconnect (tunnel->ssh);

		return NULL;
	} 

	if ((strcmp (tunnel->localname, "0.0.0.0") == 0) ||
		(strcmp (tunnel->localname, "*") == 0) ||
		(tunnel->localname == NULL ? 1 :
			 (*tunnel->localname == '\0' ? 1 : 0)) )
	{
		wildcard = TRUE;
	}

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6 
	hints.ai_flags = wildcard ? AI_PASSIVE : 0;
	hints.ai_socktype = SOCK_STREAM;

	snprintf (strport, NI_MAXSERV, "%d", tunnel->localport);

	if (ret = getaddrinfo (tunnel->localname, strport, 
	                      &hints, &ai) != 0)
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", gai_strerror (ret));
		
		freeaddrinfo (ai);

		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);

		ssh_disconnect (tunnel->ssh);

		return NULL;
	} 
		 

	if ((ai->ai_family != AF_INET && ai->ai_family != AF_INET6) ||
		(getnameinfo(ai->ai_addr, ai->ai_addrlen, ntop, sizeof(ntop),
		            strport, sizeof(strport), NI_NUMERICHOST|NI_NUMERICSERV) != 0))
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", strerror(errno));

		freeaddrinfo (ai);
		
		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);
		
		ssh_disconnect (tunnel->ssh);
		
		return NULL;
	}

	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

	if (sock < 0)
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", strerror(errno));

		freeaddrinfo (ai);
		
		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);

		ssh_disconnect (tunnel->ssh);

		return NULL;
	}

	i = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	//  bind and listen...
	
	if ((bind(sock, ai->ai_addr, ai->ai_addrlen) < 0)
		|| (listen(sock, 128) < 0))
	{
		GSQLP_TUNNEL_SET_ERROR (tunnel, "%s", strerror(errno));
		
		close (sock);
		freeaddrinfo (ai);
		
		gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_ERROR);

		ssh_disconnect (tunnel->ssh);

		return NULL;
	}

	freeaddrinfo (ai);
	
	tunnel->sock = sock;
	gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_CONNECTED);
	
	g_debug ("CONNECTED!!! and waiting for the connection");

	tunnel->sftp = sftp_new (tunnel->ssh);
	sftp_init (tunnel->sftp);

	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; /* 0.1sec */

	i = fcntl (sock, F_GETFL, 0);
	fcntl (sock, F_SETFL, i | O_NONBLOCK);

	while (1)
	{
		i = accept (sock, NULL, NULL);

		if ((i == -1) && (errno == EAGAIN))
		{
			nanosleep (&ts, NULL);

			continue;
		}
		 

		if (i == -1)
			// seems to be closed
			break;

		channel = channel_new (tunnel->ssh);

		if (channel_open_forward (channel, tunnel->fwdhost, tunnel->fwdport,
		    					"127.0.0.1", 0) != SSH_OK)
		{
			g_snprintf (tunnel->err, GSQLP_TUNNEL_ERR_LEN,
		    		N_("Forwarding failed. Seems like administratively prohibited."));
			g_debug ("%s [%s:%d]", tunnel->err, tunnel->fwdhost, tunnel->fwdport);
			
			close (i);
			continue;
		}
		
		if (!tunnel_channel_add (tunnel, channel, i))
		{
			channel_close (channel);
			close (i);

			continue;

		}

		
		if (thread)
			continue;

		thread = g_thread_create (tunnel_processing_thread, tunnel, FALSE, &error);
		
	}

	sftp_free (tunnel->sftp);
	tunnel->sftp = NULL;
	
	ssh_disconnect (tunnel->ssh);
	ssh_free (tunnel->ssh);
	tunnel->ssh = NULL;
	
	gsqlp_tunnel_set_state (tunnel, GSQLP_TUNNEL_STATE_NONE);
	g_debug ("exit from accepting");
	
	return NULL;
}

void
gsqlp_tunnel_do_connect (GSQLPTunnel *tunnel)
{
	GSQL_TRACE_FUNC;

	GThread *thread = NULL;
	GError *error = NULL;

	g_return_if_fail (GSQLP_IS_TUNNEL (tunnel));

	thread = g_thread_create (do_connect_bg, tunnel, FALSE, &error);

	if (!thread)
		g_debug ("plugin tunnel: can not create a thread");

}

void
gsqlp_tunnel_do_disconnect (GSQLPTunnel *tunnel)
{
	GSQL_TRACE_FUNC;

	close (tunnel->sock);
	tunnel->sock = -1;

}

