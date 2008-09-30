/***************************************************************************
 *            nav_tree__tables_cb.h
 *
 *  Mon Nov  5 22:59:59 2007
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
 
#ifndef _NAV_TREE__TABLES_CB_H
#define _NAV_TREE__TABLES_CB_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

void
on_nav_tree_popup_tables_create (GtkMenuItem * menuitem, 
								 gpointer user_data);

void
on_nav_tree_popup_tables_drop (GtkMenuItem * menuitem, 
								 gpointer user_data);

void
on_nav_tree_popup_tables_alter (GtkMenuItem * menuitem, 
								 gpointer user_data);

void
on_nav_tree_popup_tables_browse (GtkMenuItem * menuitem, 
								 gpointer user_data);

void
on_nav_tree_popup_tables_browse_update (GtkMenuItem * menuitem, 
								 gpointer user_data);

void
on_nav_tree_popup_tables_analyze (GtkMenuItem * menuitem, 
								 gpointer user_data);

G_END_DECLS

#endif /* _NAV_TREE__TABLES_CB_H */

 
