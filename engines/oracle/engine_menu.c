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
#include <libgsql/session.h>
#include "engine_menu.h"
#include "engine_menucb.h"
#include "engine_session.h"

static GtkToggleActionEntry enginemenu_toggle_actions[] =
{
	{ "OracleActionServerOutput", NULL, N_("Server output"), NULL, N_("Exit"), G_CALLBACK(on_server_output), FALSE }
};

static GtkActionEntry enginemenu_action[] = 
{
	{ "OracleActionMenu", NULL, N_("Oracle") },

	{ "OracleActionEmptyRecycle", NULL, N_("Empty Recycle bin"), NULL, N_("Empty Recycle bin"), G_CALLBACK(on_empty_recycle_activate) },
	{ "OracleActionJobManager", NULL, N_("Job manager"), NULL, N_("Job manager"), G_CALLBACK(on_job_manager_activate) },
	
	{ "OracleActionFindCode", NULL, N_("Find code"), NULL, N_("Find code"), G_CALLBACK(on_find_code_activate) },
	{ "OracleActionFindObject", NULL, N_("Find object"), NULL, N_("Find object"), G_CALLBACK(on_find_object_activate) },

};

static GtkActionGroup *action;


void
engine_menu_init (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC

	guint id;
	GError * error;
	
	action = gtk_action_group_new ("OracleActionsMenu");
	
	gtk_action_group_add_actions (action, enginemenu_action, 
									G_N_ELEMENTS (enginemenu_action), NULL);
	gtk_action_group_add_toggle_actions(action, enginemenu_toggle_actions,
									G_N_ELEMENTS (enginemenu_toggle_actions), NULL);
	engine->menu_id = gsql_menu_merge (PACKAGE_UI_DIR "/oracle/engine_oracle.ui", action);
	
	
	g_object_set(G_OBJECT(action), "visible", FALSE, NULL);
	
	return;
	
};

void
engine_menu_set_visible (gboolean status)
{
	GSQL_TRACE_FUNC

	g_object_set(G_OBJECT(action), "visible", status, NULL);
	return;
};

void
engine_menu_update (gpointer session)
{
	GSQL_TRACE_FUNC
	
	GtkWidget *mi;
	GSQLEOracleSession *spec;
	GtkAction *act;
	
	GSQLSession *sess = session;
	spec = (GSQLEOracleSession *) sess->spec;
	mi = gsql_menu_get_widget ("/MenuMain/PHolderEngines/MenuOracle/OracleServerOutput");
	g_return_if_fail (mi != NULL);
	act = gtk_action_group_get_action (action, "OracleActionServerOutput");
	gtk_action_block_activate_from (act, mi);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mi), spec->dbms_output);
	gtk_action_unblock_activate_from (act, mi);
	
	return;
};

