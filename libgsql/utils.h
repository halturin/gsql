/***************************************************************************
 *            utils.h
 *
 *  Thu Nov  9 21:52:12 2006
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
 
#ifndef _UTILS_H
#define _UTILS_H

#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS

  /* create a header pack for GtkNotebook */
GtkWidget*
gsql_utils_header_new (GtkWidget *icon, gchar *label, gchar *tooltip,
						 gboolean close_button, 
					     gint direction);

  /* use it for clear treeview */
void
gsql_tree_view_remove_column (gpointer column, 
								gpointer treeview);

gchar *
gsql_source_buffer_get_delim_block (GtkTextIter * iter,
									GtkTextIter ** block_start,
									GtkTextIter ** block_end);

 /* return the GtkComboBox widget with encoding list */
GtkWidget *
gsql_enconding_list (void);

gchar *
gsql_utils_escape_string (gchar *message);

G_END_DECLS

#endif /* _UTILS_H */
