/***************************************************************************
 *            gsqlcb.c
 *
 *  Mon Jul 24 17:05:28 2006
 *  Copyright  2006  Halturin Taras
 *  halturin@gmail.com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "gsqlcb.h"
#include "gsql.h"
#include <libgsql/common.h>
#include <libgsql/engines.h>
#include <libgsql/plugins.h>
#include <libgsql/session.h>
#include <libgsql/workspace.h>
#include <libgsql/menu.h>
#include "gsqlmenu.h"
#include "gsql.h"


void
on_dialog_logon_engine_name_changed (GtkComboBox *combobox,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkWidget * vbox = GTK_WIDGET(user_data);
	GtkWidget * engine_name;
	GtkWidget * notebook;
	gint page_num;
		
	engine_name = g_object_get_data (G_OBJECT (vbox), "engine_name");
	notebook = g_object_get_data (G_OBJECT (vbox), "notebook");
	page_num = gtk_combo_box_get_active (GTK_COMBO_BOX(engine_name));
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);
		
}


void
on_dialog_close_session_select_all_button_activate (GtkButton * button,
						    gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	return;
}

void
on_dialog_close_session_toggle_activate (GtkCellRendererToggle * renderer,
					 gchar * path_str, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GtkTreeIter iter, parent;
	GtkTreeModel *model = (GtkTreeModel *) data;
	gboolean will_close;


	gtk_tree_model_get_iter_from_string (model, &iter, path_str);
	gtk_tree_model_get (model, &iter, 1, &will_close, -1);
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 1, 
                            !will_close, -1);
	
	if (gtk_tree_model_iter_parent (model, &parent, &iter))
	{
		if (!will_close)
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 4,
								"<span weight='bold'>save</span>", -1);
		else
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 4, "", -1);
		
	} else {
		
		if (!will_close)
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
					    4,
					    "<span weight='bold'>close</span>",
					    -1);
		else
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 4, "", -1);
		
	}

}


void
on_sessions_notebook_change_current_page (GtkNotebook     *notebook,
											GtkNotebookPage *page,
											guint            page_num,
											gpointer         user_data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	gchar gsql_window_header[128];
	GtkWidget *mi;

	if (GSQL_IS_SESSION (page))
		GSQL_DEBUG ("is my object - GSQLSession");

	session = gsql_session_get_active();
	
	if (!session)
	{
		GSQL_DEBUG ("session = NULL");
		session = GSQL_SESSION (gtk_notebook_get_nth_page (notebook, page_num));
		
		if (session)
		{
			g_snprintf(gsql_window_header, 128, "%s: %s", "GSQL", 
						gsql_session_get_name (session));
			
			gsql_engine_menu_set_status (session->engine,
								TRUE);
			
		} else
			g_snprintf(gsql_window_header, 128, "%s", "GSQL");
		
	} else {
		GSQL_DEBUG ("session != NULL");
		gsql_engine_menu_set_status (session->engine,
								FALSE);
		
		session = GSQL_SESSION (gtk_notebook_get_nth_page (notebook, page_num));
		
		g_snprintf(gsql_window_header, 128, "%s: %s", "GSQL", 
				   gsql_session_get_name (session));
		
		gsql_engine_menu_set_status (session->engine, TRUE);
	}
	
	gsql_session_set_active (session);
	gtk_window_set_title (GTK_WINDOW (gsql_window), gsql_window_header);
	
	
	//update status menuitems 
	workspace = gsql_session_get_workspace (session);
	
	mi = gsql_menu_get_widget ("/MenuMain/MenuView/MenuShowNavarea");
	g_return_if_fail (mi != NULL);
	
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mi), 
									gsql_workspace_get_navigate_visible (workspace));
	
	mi = gsql_menu_get_widget ("/MenuMain/MenuView/MenuShowMessarea");
	g_return_if_fail (mi != NULL);
	
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mi), 
									gsql_workspace_get_messages_visible (workspace));
	
}

void
on_sessions_notebook_page_removed (GtkNotebook     *notebook,
											GtkNotebookPage *page,
											guint            page_num,
											gpointer         user_data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	
	session = gsql_session_get_active ();
	
/*	if ( session == NULL)
	{
		gsql_engines_menu_hide_all ();
		gsql_plugins_menu_update_all (NULL);
		
		return;
	}
	
	gsql_plugins_menu_update_all (session->engine);
*/
}

void 
on_gsql_window_destroy(GtkWidget * widget, gpointer data)
{
	GSQL_TRACE_FUNC;
	return;
}

gint 
on_gsql_window_delete(GtkWidget * wd, GdkEvent * event, gpointer data)
{
	GSQL_TRACE_FUNC;
        // here is checking for opened DB and not saved data
	gsql_window_clean_exit();
	return TRUE;
}


void
on_session_close_header_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;

	session = GSQL_SESSION (user_data);

	g_signal_emit_by_name (G_OBJECT (session), "close");

}
