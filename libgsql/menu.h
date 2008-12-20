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

 
#ifndef _MENU_H
#define _MENU_H

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _GSQLMenu GSQLMenu;

struct _GSQLMenu
{
	GtkAccelGroup  *accel;
	GtkUIManager   *ui;

	GtkWidget *gsqlmenu;
};

G_BEGIN_DECLS

void
gsql_menu_init ();

guint
gsql_menu_merge (gchar *ui_file, GtkActionGroup *action);

guint
gsql_menu_merge_from_string (const gchar *str, GtkActionGroup *action);

void
gsql_menu_merge_action (GtkActionGroup *action);

guint
gsql_menu_merge_ui (gchar *ui_file);

GtkWidget *
gsql_menu_get_widget (gchar *name);


G_END_DECLS

#endif /* _MENU_H */

 
