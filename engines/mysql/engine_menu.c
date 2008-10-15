/***************************************************************************
 *            engine_menu.c
 *
 *  Wed Sep  5 01:09:48 2007
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

#include <libgsql/common.h>
#include <libgsql/menu.h>
#include <libgsql/engines.h>
#include <libgsql/stock.h>

#include "engine_menu.h"
#include "engine_menucb.h"

static GtkActionEntry enginemenu_action[] = 
{
	{ "MySQLActionMenu", NULL, N_("MySQL") },

	{ "ActionMenuMySQLCharsetList", NULL,  N_("Charter Set"), NULL,  NULL, NULL },
	{ "MySQLActionProcesses", NULL, N_("Show processes"), NULL, N_("Show processes"), G_CALLBACK(on_show_processes) }
};

static GtkActionGroup *action;


void
engine_menu_init (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC;

	guint id;
	GError * error; 
	GtkWidget *widget;
	GtkWidget *submenu;
	GtkWidget *item;
	GSList *group;
	
	action = gtk_action_group_new ("ActionsMenuMySQL");
	
	gtk_action_group_add_actions (action, enginemenu_action, 
									G_N_ELEMENTS (enginemenu_action), NULL);
	engine->menu_id = gsql_menu_merge (PACKAGE_UI_DIR "/mysql/engine_mysql.ui", action);
	engine->action = action;
	
	widget = gsql_menu_get_widget ("/MenuMain/PHolderEngines/MenuMySQL/MenuMySQLCharsetList");
	
	submenu = gtk_menu_new ();
	
	item = gtk_radio_menu_item_new_with_label (NULL, "latin1");
	group =  gtk_radio_menu_item_get_group (item);


	gtk_menu_shell_append (submenu, 
						   item);
	item = gtk_radio_menu_item_new_with_label (group, "utf8");


	gtk_menu_shell_append (submenu, 
						   item);
	/*
	 show variables
where Variable_name = 'character_set_client'
	 
	 */
	
	gtk_menu_item_set_submenu (widget, submenu);
	
	gtk_widget_show_all (submenu);
		
	g_object_set(G_OBJECT(action), "visible", FALSE, NULL);
	
	return;
	
};











