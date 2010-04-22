/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2010  Taras Halturin  halturin@gmail.com
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

#include "gsql-callbacks.h"


static GtkActionEntry	menu_entries_file[] = {
	{ "ActionMenuFile", NULL, N_("_File") },
	{ "ActionExit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
			N_("Quit GSQL"), G_CALLBACK (on_gsql_quit) }
};

static GtkActionEntry	menu_entries_edit[] = {
	{ "ActionMenuEdit", NULL, N_("_Edit") }

};

static GtkActionEntry	menu_entries_view[] = {
	{ "ActionMenuView", NULL, N_("_View") },
	{ "ActionMenuPreferences", NULL, N_("Preferences"), NULL,
			N_("Customize the GSQL behavior"), G_CALLBACK (on_gsql_preferences) }

	
};

static GtkToggleActionEntry menu_toggle_entries_view[] = {
	{ "ActionFullScreen", GTK_STOCK_FULLSCREEN, N_("Full Screen"), "F11",
			N_("Switch to full screen mode"), G_CALLBACK (on_gsql_fullscreen) },
	{ "ActionShowToolbar", NULL, N_("Show Toolbar"), NULL,
			N_("Show toolbar"), G_CALLBACK (on_gsql_showtoolbar) },
	{ "ActionShowStatusbar", NULL, N_("Show Statusbar"), NULL,
			N_("Show statusbar"), G_CALLBACK (on_gsql_showstatusbar) }
	
};


static GtkActionEntry	menu_entries_search[] = {
	{ "ActionMenuSearch", NULL, N_("Search") }



};

static GtkActionEntry	menu_entries_session[] = {
	{ "ActionMenuSession", NULL, N_("_Session") }



};

static GtkActionEntry	menu_entries_object[] = {
	{ "ActionMenuObject", NULL, N_("_Object") }



};

static GtkActionEntry	menu_entries_tools[] = {
	{ "ActionMenuTools", NULL, N_("Tools") }



};

static GtkActionEntry	menu_entries_help[] = {
	{ "ActionMenuHelp", NULL, N_("_Help") },
	
	{ "ActionUsersManual", GTK_STOCK_HELP, N_("User's Manual"), "F1", 
		N_("User's Manual"), G_CALLBACK(on_gsql_user_manual_activate) },

	{ "ActionHomePage", GTK_STOCK_HOME, N_("Home Page"), NULL, 
		PROJECT_URL, G_CALLBACK(on_gsql_home_page_activate) },
	
	{ "ActionAPIReference", NULL, N_("API Reference"), NULL, 
		PROJECT_URL "/docs/reference/index.html", G_CALLBACK(on_gsql_api_activate) },

	{ "ActionBugReport", NULL, N_("Bug Report"), NULL, 
		PROJECT_BUG_REPORT_URL, G_CALLBACK(on_gsql_bug_report_activate) },

	{ "ActionRequestsQuestions", NULL, N_("Wish List"), NULL, 
		PROJECT_GROUP_URL, G_CALLBACK(on_gsql_mailgroup_activate) },

	{ "ActionAbout", GTK_STOCK_ABOUT, N_("About"), NULL, 
		N_("About GSQL"), G_CALLBACK(on_about_activate) }
};

