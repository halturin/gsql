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

static GtkToggleActionEntry enginemenu_toggle_actions[] =
{
	{ "MySQLActionServerOutput", NULL, N_("Server output"), NULL, N_("Exit"), G_CALLBACK(on_server_output), FALSE }
};

static GtkActionEntry enginemenu_action[] = 
{
	{ "MySQLActionMenu", NULL, N_("MySQL") },

	{ "MySQLActionEmptyRecycle", NULL, N_("Empty Recycle bin"), NULL, N_("Empty Recycle bin"), G_CALLBACK(on_empty_recycle_activate) },
	{ "MySQLActionJobManager", NULL, N_("Job manager"), NULL, N_("Job manager"), G_CALLBACK(on_job_manager_activate) },
	
	{ "MySQLActionFindCode", NULL, N_("Find code"), NULL, N_("Find code"), G_CALLBACK(on_find_code_activate) },
	{ "MySQLActionFindObject", NULL, N_("Find object"), NULL, N_("Find object"), G_CALLBACK(on_find_object_activate) },

};

static GtkActionGroup *action;


void
engine_menu_init (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC;

	guint id;
	GError * error; 
	
	action = gtk_action_group_new ("ActionsMenuMySQL");
	
	gtk_action_group_add_actions (action, enginemenu_action, 
									G_N_ELEMENTS (enginemenu_action), NULL);
	gtk_action_group_add_toggle_actions(action, enginemenu_toggle_actions,
									G_N_ELEMENTS (enginemenu_toggle_actions), NULL);
	engine->menu_id = gsql_menu_merge (PACKAGE_UI_DIR "/mysql/engine_mysql.ui", action);
	engine->action = action;
		
	g_object_set(G_OBJECT(action), "visible", FALSE, NULL);
	
	return;
	
};











