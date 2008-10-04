/***************************************************************************
 *            gsqlconfcb.h
 *
 *  Tue Oct 17 20:18:25 2006
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
 
#ifndef _GSQLCONFCB_H
#define _GSQLCONFCB_H

G_BEGIN_DECLS

void
on_gsql_prefs_show_dbnav_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_gsql_prefs_show_message_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_gsql_prefs_restore_sizepos_check_toggled (GtkToggleButton  *togglebutton,
											gpointer user_data);

void 
on_fetch_limit_step_changed (GtkSpinButton *button, gpointer data);

void 
on_fetch_limit_max_changed (GtkSpinButton *button, gpointer data);

void
on_gsql_prefs_use_system_font_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void 
on_gsql_prefs_font_button_font_set (GtkFontButton *widget,
											gpointer user_data);

void
on_gsql_prefs_insert_space_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_gsql_prefs_tab_width_spin_value_changed (GtkSpinButton *spin_button,
											gpointer user_data);

void
on_gsql_prefs_display_line_num_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_gsql_prefs_highlight_line_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_gsql_prefs_enable_text_wrap_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);
void
on_gsql_prefs_enable_auto_indent_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data);

void
on_pref_plugins_configure_button_clicked (GtkButton *conf_button,
										  gpointer user_data);
void
on_pref_plugins_row_activated (GtkTreeView * tv, GtkTreePath * path,
							   GtkTreeViewColumn * col, gpointer data);

void
on_prefs_plugins_tree_view_cursor_changed (GtkTreeView *tree_view,
										   gpointer     user_data);
										   
void
on_pref_datetime_preview (GtkButton *conf_button,
										  gpointer user_data);

gboolean 
on_pref_datetime_entry_focus_out (GtkWidget *widget, GdkEventFocus *event,
							 gpointer user_data);

G_END_DECLS

#endif /* _GSQLCONFCB_H */
