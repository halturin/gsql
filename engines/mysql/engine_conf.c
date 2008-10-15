/***************************************************************************
 *            engine_conf.c
 *
 *  Thu Sep  6 23:42:52 2007
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

#include <libgsql/conf.h>
#include <libgsql/common.h>
#include "engine_conf.h"
#include "engine_confcb.h"
#include "engine_menu.h"


static void
on_custom_charset_changed (GtkWidget *widget, gpointer user_data);

GtkWidget *
engine_conf_widget_new ()
{
	GSQL_TRACE_FUNC;
	
	GtkWidget *vbox, *vbox_charset;
	GtkWidget *check_charset;
	GtkWidget *charset_list;
	GtkWidget *label;
	GtkWidget *alignment;
	GtkWidget *frame;
	gboolean gconf_bool_value;
	gchar *gconf_char_value;
	GtkListStore *ls;
	GtkCellRenderer *cell;
	GtkTreeIter iter;
	gchar *ls_value;
	vbox = gtk_vbox_new (FALSE, 0);
	gboolean have_found = TRUE;
	
	
	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	
	label = gtk_label_new (_("<b>Character set</b>"));
	gtk_widget_show (label);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
        
	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);
        
	vbox_charset = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_charset);
	gtk_container_add (GTK_CONTAINER (alignment), vbox_charset);
        
	check_charset = gtk_check_button_new_with_mnemonic (_("Use custom character set"));
	gtk_widget_show (check_charset);
	gtk_box_pack_start (GTK_BOX (vbox_charset), check_charset, FALSE, FALSE, 0);
	
	ls = engine_menu_get_charset_store ();
	
	charset_list = gtk_combo_box_new_with_model (GTK_TREE_MODEL (ls));
	
	cell = gtk_cell_renderer_text_new ();
    
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (charset_list),
                                    cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (charset_list),
                                        cell, "markup", 0, NULL);
	
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ls), &iter);
	
	gconf_char_value =  gsql_conf_value_get_string (GSQLE_CONF_MYSQL_CUSTOM_CHARSET);
	
	do
	{
		if (!gconf_char_value)
			break;
		
		gtk_tree_model_get (GTK_TREE_MODEL (ls), 
							&iter,
							1, &ls_value, -1);
		
		
		if (!g_ascii_strncasecmp (ls_value, gconf_char_value, 32))
			break;
		
		
	} while (have_found = gtk_tree_model_iter_next (GTK_TREE_MODEL (ls), &iter));
	
	if (have_found)
		gtk_combo_box_set_active_iter (GTK_COMBO_BOX (charset_list), &iter);
	
	g_signal_connect (charset_list, "changed",
						G_CALLBACK (on_custom_charset_changed),
						ls);
	
	gtk_widget_show (charset_list);
	gtk_box_pack_start (GTK_BOX (vbox_charset), charset_list, FALSE, FALSE, 0);
	
	g_signal_connect ((gpointer) check_charset, "toggled",
						G_CALLBACK (on_use_custom_charset_check_toggled),
						charset_list);
	
	gconf_bool_value = gsql_conf_value_get_boolean (GSQLE_CONF_MYSQL_USE_CUSTOM_CHARSET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_charset), gconf_bool_value);
	gtk_widget_set_sensitive (charset_list, gconf_bool_value);
	
	gtk_widget_show_all (vbox);
	
	return vbox;
}

void
engine_conf_widget_free (GtkWidget *conf_widget)
{
	GSQL_TRACE_FUNC;

}


static void
on_custom_charset_changed (GtkWidget *widget, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkTreeIter iter;
	GtkTreeModel *model = user_data;

	gchar *ls_value;
	
	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
		return;
	
	gtk_tree_model_get (model, &iter, 1, &ls_value, -1);
	
	gsql_conf_value_set_string (GSQLE_CONF_MYSQL_CUSTOM_CHARSET, ls_value);
	
}

