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

static GObjectClass *parent_class;
static void gsqlp_tunnel_class_init (GSQLPTunnelClass *klass);
static void gsqlp_tunnel_init (GSQLPTunnel *obj);



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
	GSQL_TRACE_FUNC;

	g_return_val_if_fail (GSQLP_IS_TUNNEL (tunnel), GSQLP_TUNNEL_STATE_ERROR);
	
	return tunnel->private->state;	
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

}





