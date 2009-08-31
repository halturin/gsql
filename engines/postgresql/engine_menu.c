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


#include <libgsql/common.h>
#include <libgsql/menu.h>
#include <libgsql/engines.h>
#include <libgsql/stock.h>

#include "engine_menu.h"
#include "engine_menucb.h"
#include "engine_conf.h"

static GtkActionEntry enginemenu_action[] = 
{
	{ "PGSQLActionMenu", NULL, N_("PostgreSQL") },

	{ "ActionMenuPGSQLCharsetList", NULL,  N_("Character Set"), NULL,  NULL, NULL }
};

static GtkActionGroup *action;


static GtkListStore *ListStorePredefinedCharset = NULL;

static PGSQLPredefinedCharset ArrPredefinedCharset[] = {
  { "LATIN1", "Latin 1" },
  { "UTF8", "UTF-8 Unicode" }
};

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
	GtkTreeIter iter;
	guint i;
	gchar desc_full[255];
	
	action = gtk_action_group_new ("ActionsMenuPGSQL");
	
	gtk_action_group_add_actions (action, enginemenu_action, 
				      G_N_ELEMENTS (enginemenu_action), NULL);
	printf(PACKAGE_UI_DIR "/postgresql/engine_pgsql.ui");

	engine->menu_id = gsql_menu_merge (PACKAGE_UI_DIR "/postgresql/engine_pgsql.ui", action);
	engine->action = action;
	
	widget = gsql_menu_get_widget ("/MenuMain/PHolderEngines/MenuPGSQL/MenuPGSQLCharsetList");
	
	submenu = gtk_menu_new ();
	
	if (!ListStorePredefinedCharset)
	{
		ListStorePredefinedCharset = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
		
		for (i=0; i < G_N_ELEMENTS (ArrPredefinedCharset); i++)
		{
			g_snprintf(desc_full, 255,"%s [%s]", 
					   ArrPredefinedCharset[i].name,
					   ArrPredefinedCharset[i].desc);
			
			gtk_list_store_append (ListStorePredefinedCharset,
								   &iter);
			gtk_list_store_set (ListStorePredefinedCharset, 
								&iter, 
								0, desc_full,
								1, ArrPredefinedCharset[i].name, -1);
			item = gtk_menu_item_new_with_label (desc_full);
			
			g_signal_connect (item, "activate", 
							  G_CALLBACK (on_character_set_activate),
							  ArrPredefinedCharset[i].name);
			
			gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
		}
		
	}

	
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), submenu);
	
	gtk_widget_show_all (submenu);
		
	g_object_set(G_OBJECT(action), "visible", FALSE, NULL);
	
}


GtkListStore *
engine_menu_get_charset_store ()
{
	return ListStorePredefinedCharset;	
}
