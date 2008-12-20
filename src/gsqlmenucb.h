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


#ifndef _GSQLMENUCB_H
#define _GSQLMENUCB_H

#include <gtk/gtk.h>
#include <glib.h>

#include <config.h>

G_BEGIN_DECLS


void
on_file_new_sql_activate (GtkMenuItem *mi, gpointer data);

void
on_file_open_activate (GtkMenuItem *mi, gpointer data);

void
on_file_save_activate (GtkMenuItem *mi, gpointer data);

void
on_file_save_as_activate (GtkMenuItem *mi, gpointer data);

void
on_file_close_activate (GtkMenuItem *mi, gpointer data);

void
on_file_close_all_activate (GtkMenuItem *mi, gpointer data);

void
on_file_reload_activate (GtkMenuItem *mi, gpointer data);

void
on_exit_activate (GtkMenuItem *mi, gpointer data);

void
on_navarea_activate (GtkToggleAction *ta, gpointer data);

void
on_messarea_activate (GtkToggleAction *ta, gpointer data);

void
on_next_session (GtkMenuItem *mi, gpointer data);

void
on_prev_session (GtkMenuItem *mi, gpointer data);

void
on_next_page (GtkMenuItem *mi, gpointer data);

void
on_prev_page (GtkMenuItem *mi, gpointer data);

void
on_content_details (GtkMenuItem *mi, gpointer data);

void
on_preferences_activate (GtkMenuItem *mi, gpointer data);

void
on_new_session_activate (GtkMenuItem *mi, gpointer data);

#ifdef WITH_GNOME
void
on_help_manual_activate (GtkMenuItem *mi, gpointer data);

void
on_home_page_activate (GtkMenuItem *mi, gpointer data);

void
on_api_refs_activate (GtkMenuItem *mi, gpointer data);

void
on_bug_report_activate (GtkMenuItem *mi, gpointer data);

void
on_mailgroup_activate (GtkMenuItem *mi, gpointer data);
#endif

void
on_about_activate (GtkMenuItem *mi, gpointer data);

G_END_DECLS

#endif /* _GSQLMENUCB_H */

