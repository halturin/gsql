/***************************************************************************
 *            nav_tree__tables_cb.c
 *
 *  Mon Nov  5 22:59:45 2007
 *  Copyright  2006  Halturin Taras
 *  halturin@gmail.com
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

#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/session.h>
#include <libgsql/content.h>
#include <libgsql/stock.h>
#include <libgsql/common.h>
#include <libgsql/editor.h>
#include <libgsql/workspace.h>
#include <libgsql/navigation.h>

void
on_nav_tree_popup_tables_create (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
};

void
on_nav_tree_popup_tables_drop (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
};

void
on_nav_tree_popup_tables_alter (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
};

void
on_nav_tree_popup_tables_browse (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GtkTreeView *tv;
	gchar *realname;
	gchar *owner;
	
	
	session = gsql_session_get_active ();
	g_return_if_fail (session != NULL);
	
	/*workspace = (GSQLWorkspace *) session->workspace;
	tv = workspace->navigation_tv;
	
	model = gtk_tree_view_get_model (tv);
	selection = gtk_tree_view_get_selection (tv);
	
	if (!gtk_tree_selection_get_selected (selection,
										  &model,
										  &iter))
		// have no selection
		return;
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);
	nav_tree_tables_browse (realname, owner, FALSE);
	return;*/
};

void
on_nav_tree_popup_tables_browse_update (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GSQLSession *session;
	GSQLWorkspace *workspace;
	GtkTreeView *tv;
	gchar *realname;
	gchar *owner;
	
	
/*	session = gsql_session_get_current ();
	g_return_if_fail (session != NULL);
	
	workspace = (GSQLWorkspace *) session->workspace;
	tv = workspace->navigation_tv;
	
	model = gtk_tree_view_get_model (tv);
	selection = gtk_tree_view_get_selection (tv);
	
	if (!gtk_tree_selection_get_selected (selection,
										  &model,
										  &iter))
		// have no selection
		return;
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_REALNAME, 
						&realname, -1);
	gtk_tree_model_get (model, &iter,
						GSQL_NAV_TREE_OWNER, 
						&owner, -1);
	nav_tree_tables_browse (realname, owner, TRUE);
	return;
	 */
};

void
on_nav_tree_popup_tables_analyze (GtkMenuItem * menuitem, 
								 gpointer user_data)
{
	GSQL_TRACE_FUNC;

	return;
};

