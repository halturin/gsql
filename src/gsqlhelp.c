/***************************************************************************
 *            gsqlhelp.c
 *
 *  Mon Sep  3 00:27:56 2007
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

#include <config.h>
#include <libgsql/common.h>

#include <libgsql/menu.h>
#include "gsqlmenucb.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

static GtkActionEntry gsqlmenu_action[] = 
{
	{ "ActionMenuHelp", NULL, N_("_Help") },

	{ "ActionUsersManual", GTK_STOCK_HELP, N_("User's Manual"), "F1", N_("User's Manual"), G_CALLBACK(on_help_manual_activate) },

	{ "ActionHomePage", GTK_STOCK_HOME, N_("Home Page"), NULL, PROJECT_URL, G_CALLBACK(on_home_page_activate) },
	{ "ActionAPIReference", NULL, N_("API Reference"), NULL, PROJECT_URL "/docs/reference/index.html", G_CALLBACK(on_api_refs_activate) },
	{ "ActionBugReport", NULL, N_("Bug Report"), NULL, PROJECT_BUG_REPORT_URL, G_CALLBACK(on_bug_report_activate) },
	{ "ActionRequestsQuestions", NULL, N_("Wish List"), NULL, PROJECT_GROUP_URL, G_CALLBACK(on_mailgroup_activate) },
	{ "ActionAbout", GTK_STOCK_ABOUT, N_("About"), NULL, N_("About"), G_CALLBACK(on_about_activate) }
  
};

void
gsql_help_menu_init()
{
	GSQL_TRACE_FUNC;

	GtkActionGroup *action;

	action = gtk_action_group_new ("ActionsMenuHelp");
	gtk_action_group_add_actions (action, gsqlmenu_action, 
								  G_N_ELEMENTS (gsqlmenu_action), NULL);	

	gsql_menu_merge_action (action);
	
}



