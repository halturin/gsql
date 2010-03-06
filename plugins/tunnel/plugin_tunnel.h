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

 
#ifndef _PLUGIN_TUNNEL_H
#define _PLUGIN_TUNNEL_H

#include <glib.h>
#include <libgsql/sqleditor.h>
#include <libgsql/session.h>

#include <libssh/libssh.h>

#define GSQLP_TUNNEL_STOCK_ICON "gsql-plugin-tunnel-icon"

#define SSH_SESSION_ERR_LEN	512

//#define SSH_SESSION_SET_ERROR(session, params...) \
	//	memset (session->err, 0, 512); \
		//g_snprintf (session->err, 512, params)

typedef struct _SSHLink		SSHLink;
typedef struct _SSHChannel	SSHChannel;
typedef enum {
	GSQLP_TUNNEL_STATE_NOT_CONNECTED,
	GSQLP_TUNNEL_STATE_FAILED,
	GSQLP_TUNNEL_STATE_CONNECTED,
	GSQLP_TUNNEL_STATE_CONNECTION
} GSQLPTunnelState;

struct _SSHLink {
	
	gchar name[128];

	/* connect to */
	gchar hostname[128];
	gchar username[128];
	gchar password[64];
	guint		port;

	ssh_session ssh;

	/* listen on */
	gchar		localname[128];
	guint			localport;

	int			sock;

	/* forwarded from */
	gchar		fwdhost[128];
	guint			fwdport;

	/* list of SSHChannel */
	GList		*channel_list;

	gboolean	autoconnect;
	GSQLPTunnelState	state;
	
	gboolean	has_changed;
	
	gchar		err[SSH_SESSION_ERR_LEN];
};

struct _SSHChannel {

	ssh_channel	*channel;

	guint64		rx; // we count this 
	guint64		tx; // for the future 
	
	GSQLSession *session; // is it possible to get it right there? check it!

};


G_BEGIN_DECLS


G_END_DECLS

#endif /* _PLUGIN_TUNNEL_H */

 
