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


#include <config.h>
#include <libgsql/common.h>
#include <libgsql/engines.h>
#include <libgsql/menu.h>
#include <libgsql/stock.h>

#include "gsqlhelp.h"
#include "gsqlfiles.h"
#include "gsqlmenu.h"
#include "gsqlmenucb.h"


static GtkActionEntry gsqlmenu_actions[] = 
{
	{ "ActionMenuFile", NULL, N_("_File") },
	{ "ActionMenuEdit", NULL, N_("Edit") },
	{ "ActionMenuSearch", NULL, N_("Search") },
	{ "ActionMenuView", NULL, N_("_View") },
	{ "ActionMenuSessions", NULL, N_("_Session") },
	{ "ActionMenuTools", NULL, N_("_Tools") },
  
	{ "ActionExit", GTK_STOCK_QUIT, N_("Exit"), "<control>Q", N_("Exit"), G_CALLBACK(on_exit_activate) },
  
	{ "ActionNextSession", NULL, N_("Next Session"), "<control><alt>Page_Down", N_("Switch to the next session"), G_CALLBACK(on_next_session) },
	{ "ActionPrevSession", NULL, N_("Previous Session"), "<control><alt>Page_Up", N_("Switch to the next session"), G_CALLBACK(on_prev_session) },
	{ "ActionNextPage", NULL, N_("Next Page"), "<control>Page_Down", N_("Next Page"), G_CALLBACK(on_next_page) },
	{ "ActionPrevPage", NULL, N_("Previous Page"), "<control>Page_Up", N_("Previous Page"), G_CALLBACK(on_prev_page) },
	{ "ActionContentDetails", NULL, N_("Details"), "<alt>1", N_("Switch to the details page"), G_CALLBACK(on_content_details) },
	
	{ "ActionMenuPreferences", GTK_STOCK_PREFERENCES, "Preferences", NULL, "Preferences", G_CALLBACK(on_preferences_activate) },
 
	{ "ActionMenuNewSession", GTK_STOCK_CONNECT/*GSQL_STOCK_SESSION_NEW*/, N_("New Session"), NULL, N_("Open new session"), G_CALLBACK(on_new_session_activate) }  
};

static GtkToggleActionEntry gsqlmenu_toggle_actions[] =
{
	{ "ActionShowNavarea", NULL, N_("Navigator"), "F12", N_("Navigator"), G_CALLBACK(on_navarea_activate), TRUE },
	{ "ActionShowMessarea", NULL, N_("Messages"), "F11", N_("Messages"), G_CALLBACK(on_messarea_activate), TRUE }
};




GtkWidget *
gsql_main_menu_init()
{
	GSQL_TRACE_FUNC;

	GtkActionGroup *action;
	
	
	gsql_menu_init();
	
	gsql_files_menu_init();
	gsql_help_menu_init();
	
	action = gtk_action_group_new ("ActionsMenuMain");
	gtk_action_group_add_actions (action, gsqlmenu_actions, 
								  G_N_ELEMENTS (gsqlmenu_actions), NULL);
	gtk_action_group_add_toggle_actions(action, gsqlmenu_toggle_actions,
									G_N_ELEMENTS (gsqlmenu_toggle_actions), NULL);
	gsql_menu_merge (PACKAGE_UI_DIR "/gsql.ui", action);
		
	gsql_menu_merge_ui (PACKAGE_UI_DIR "/gsqltool.ui");
	
	return gsql_menu_get_widget("/MenuMain");	
};
