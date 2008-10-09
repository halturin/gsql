/***************************************************************************
 *            menu.c
 *
 *  Sun Sep  2 23:23:18 2007
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

#include <libgsql/menu.h>
#include <libgsql/common.h>

static GSQLMenu  *gsql_menu;
extern GtkWidget *gsql_window;

void
gsql_menu_init()
{
	GSQL_TRACE_FUNC;
		
	gsql_menu = (GSQLMenu *) g_malloc0 (sizeof( GSQLMenu));
	gsql_menu->ui = gtk_ui_manager_new ();
	gsql_menu->accel= gtk_ui_manager_get_accel_group (gsql_menu->ui);
	gtk_window_add_accel_group (GTK_WINDOW(gsql_window), gsql_menu->accel);

}


guint
gsql_menu_merge (gchar *ui_file, GtkActionGroup *action)
{
	GSQL_TRACE_FUNC;
		
	GError **error;
	guint ret;

	gtk_ui_manager_insert_action_group (gsql_menu->ui, action, 0);
	ret = gtk_ui_manager_add_ui_from_file (gsql_menu->ui, ui_file, error);
	
	if (ret)
	{
		gtk_ui_manager_ensure_update (gsql_menu->ui);
		return ret;
		
	} else	{
		gtk_ui_manager_remove_action_group (gsql_menu->ui, action);
	}
	
	return 0;
}

guint
gsql_menu_merge_ui (gchar *ui_file)
{
	GError **error;
	guint ret;
	
	ret = gtk_ui_manager_add_ui_from_file (gsql_menu->ui, ui_file, error);
	
	if (ret)
		gtk_ui_manager_ensure_update (gsql_menu->ui);
	
	return ret;
}


guint
gsql_menu_merge_from_string (const gchar *str, GtkActionGroup *action)
{
	GSQL_TRACE_FUNC;
		
	GError **error;
	guint ret;

	gtk_ui_manager_insert_action_group (gsql_menu->ui, action, 0);
	ret = gtk_ui_manager_add_ui_from_string (gsql_menu->ui, str, -1, error);
	
	if (ret)
	{
		gtk_ui_manager_ensure_update (gsql_menu->ui);
		return ret;
		
	} else	{
		gtk_ui_manager_remove_action_group (gsql_menu->ui, action);
	}
	
	return 0;
}

void
gsql_menu_merge_action (GtkActionGroup *action)
{
	GSQL_TRACE_FUNC;
		
	gtk_ui_manager_insert_action_group (gsql_menu->ui, action, 0);
	gtk_ui_manager_ensure_update (gsql_menu->ui);

}

GtkWidget *
gsql_menu_get_widget (gchar *name)
{
	GSQL_TRACE_FUNC;
		
	return gtk_ui_manager_get_widget (gsql_menu->ui, name);
}
