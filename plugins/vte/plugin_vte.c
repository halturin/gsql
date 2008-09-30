/***************************************************************************
 *            plugin_vte.c
 *
 *  Mon Dec  3 23:16:09 2007
 *  Copyright  2007  Taras Halturin
 *  <halturin@gmail.com>
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
 

#include <gtk/gtk.h>
#include <libgsql/common.h>
#include <libgsql/menu.h>
#include <libgsql/session.h>
#include <libgsql/stock.h>
#include <libgsql/plugins.h>

#include "plugin_vte.h"
#include "vtecb.h"
#include "vte_conf.h"

#define PLUGIN_VERSION "0.1"
#define PLUGIN_ID    "plugin_vte"
#define PLUGIN_NAME  "Terminal session"
#define PLUGIN_DESC  "Allow to open terminal session to the database"
#define PLUGIN_AUTHOR "Taras Halturin"
#define PLUGIN_HOMEPAGE "http://gsql.org"

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_VTE_STOCK_TERMINAL,	"vte-terminal.png" }
};


static GtkActionEntry vtemenu_action[] = 
{
	{ "ActionTerminalOpen", GSQLP_VTE_STOCK_TERMINAL, N_("Open Terminal"), NULL, N_("Open terminal"), G_CALLBACK(on_open_terminal_activate) }
};

static GtkActionGroup *action;
static menu_id;

static void
plugin_menu_update_sensitive (GSQLEngine *engine);

gboolean 
plugin_load (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC

	plugin->info.author = PLUGIN_AUTHOR;
	plugin->info.id = PLUGIN_ID;
	plugin->info.name = PLUGIN_NAME;
	plugin->info.desc = PLUGIN_DESC;
	plugin->info.homepage = PLUGIN_HOMEPAGE;
	plugin->info.version = PLUGIN_VERSION;
	plugin->file_logo = "vte-terminal.png";
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));

	action = gtk_action_group_new ("ActionsPluginVte");
	gtk_action_group_add_actions (action, vtemenu_action, 
									G_N_ELEMENTS (vtemenu_action), NULL);
	menu_id = gsql_menu_merge (PACKAGE_UI_DIR "/plugins/vte.ui", action);	
	//g_object_set(G_OBJECT(action), "visible", TRUE, NULL);
	//g_object_set(G_OBJECT(action), "sensitive", FALSE, NULL);
	
	plugin->plugin_conf_dialog = plugin_vte_conf_dialog;
	//plugin->menu_update_sensitive = plugin_menu_update_sensitive;
	
	
	
	return TRUE;
};

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC

	return TRUE;
};

static void
plugin_menu_update_sensitive (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC

	// we can work with all engines
	if (engine)
		g_object_set(G_OBJECT(action), "sensitive", TRUE, NULL);
	else 
		g_object_set(G_OBJECT(action), "sensitive", FALSE, NULL);
	return;
};
