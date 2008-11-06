/***************************************************************************
 *            gsqlcb.h
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

#ifndef _GSQLCB_H
#define _GSQLCB_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

void
on_dialog_logon_engine_name_changed (GtkComboBox *combobox,
										gpointer user_data);

void 
on_dialog_close_session_select_all_button_activate (GtkButton *button, 
													gpointer user_data);
void 
on_dialog_close_session_toggle_activate (GtkCellRendererToggle *renderer, 
											gchar *path_str, 
											gpointer data);
void 
on_gsql_window_destroy(GtkWidget * widget, gpointer data);

gint 
on_gsql_window_delete(GtkWidget * wd, GdkEvent * event, gpointer data);

void
on_sessions_notebook_change_current_page (GtkNotebook     *notebook,
											GtkNotebookPage *page,
											guint            page_num,
											gpointer         user_data);

G_END_DECLS
		
#endif /* _GSQLCB_H */
