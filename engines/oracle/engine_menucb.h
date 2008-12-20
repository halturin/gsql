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

 
#ifndef _ENGINE_MENUCB_H
#define _ENGINE_MENUCB_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

void
on_server_output (GtkToggleAction *ta, gpointer data);
 
void
on_empty_recycle_activate (GtkMenuItem * mi, gpointer data);
 
void
on_job_manager_activate (GtkMenuItem * mi, gpointer data);
 
void
on_find_code_activate (GtkMenuItem * mi, gpointer data);
 
void
on_find_object_activate (GtkMenuItem * mi, gpointer data);


G_END_DECLS

#endif /* _ENGINE_MENUCB_H */

 
