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

const gchar *env_types[] = 
{
	"ORACLE_HOME",
	"TNS_ADMIN",
	"ORACLE_BASE",
	"ORACLE_SID",
	"NLS_LANG",
	"TMP",
	"TMPDIR",
	"SHELL",
	"EDITOR"
};

static GtkListStore *ls_types;


GtkWidget *
engine_conf_widget_create ()
{
	GSQL_TRACE_FUNC;
	
	GtkWidget *oracle_page_vbox = NULL;
	GtkWidget *oracle_env_frame;
	GtkWidget *alignment8;
	GtkWidget *oracle_env_vbox;
	GtkWidget *use_system_env_check;
	GtkWidget *oracle_env_hbox;
	GtkWidget *oracle_env_scroll;
	GtkWidget *oracle_env_treeview;
	GtkListStore *liststore;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;
	GtkWidget *oracle_buttons_vbox;
	GtkWidget *oracle_env_add_button;
	GtkWidget *oracle_env_edit_button;
	GtkWidget *oracle_env_del_button;
	GtkWidget *oracle_env_label;
	GtkWidget *oracle_options_frame;
	GtkWidget *alignment9;
	GtkWidget *oracle_options_vbox;
	GtkWidget *oracle_enable_trace_check;
	GtkWidget *oracle_options_label;
	GtkWidget *lbl;
	gchar	  **env_list, *env_all = NULL;
	gchar *env_name, *env_value;
	guint i;
	
	gboolean gconf_bool_value;
	
	oracle_page_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (oracle_page_vbox);
        
	oracle_env_frame = gtk_frame_new (NULL);
	gtk_widget_show (oracle_env_frame);
	gtk_box_pack_start (GTK_BOX (oracle_page_vbox), oracle_env_frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (oracle_env_frame), 2);
	gtk_frame_set_shadow_type (GTK_FRAME (oracle_env_frame), GTK_SHADOW_NONE);
        
	alignment8 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment8);
	gtk_container_add (GTK_CONTAINER (oracle_env_frame), alignment8);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment8), 0, 0, 12, 0);
        
	oracle_env_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (oracle_env_vbox);
	gtk_container_add (GTK_CONTAINER (alignment8), oracle_env_vbox);
        
	use_system_env_check = gtk_check_button_new_with_mnemonic (_("Use the system environment"));
	gtk_widget_show (use_system_env_check);
	gtk_box_pack_start (GTK_BOX (oracle_env_vbox), use_system_env_check, FALSE, FALSE, 0);

	lbl = gtk_label_new (N_("<small> Warning: the changes will affect after restart GSQL</small>"));
	gtk_label_set_use_markup (lbl, TRUE);
	gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
	gtk_widget_show (lbl);
	gtk_box_pack_start (GTK_BOX (oracle_env_vbox), lbl, FALSE, FALSE, 0);
	
	oracle_env_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (oracle_env_hbox);
	gtk_box_pack_start (GTK_BOX (oracle_env_vbox), oracle_env_hbox, TRUE, TRUE, 0);
	HOOKUP_OBJECT (use_system_env_check, oracle_env_hbox, "oracle_env_hbox");
        
	oracle_env_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (oracle_env_scroll);
	gtk_box_pack_start (GTK_BOX (oracle_env_hbox), oracle_env_scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (oracle_env_scroll), 
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (oracle_env_scroll), 
											GTK_SHADOW_IN);
        
        
	liststore = gtk_list_store_new (2,
									G_TYPE_STRING,  // env name
									G_TYPE_STRING);  // env value

	if (!ls_types)
	{
		ls_types = gtk_list_store_new (1, G_TYPE_STRING);
		for (i = 0; i < G_N_ELEMENTS (env_types); i++)
		{
			gtk_list_store_append (ls_types, &iter);
			gtk_list_store_set (ls_types, &iter,
							0, env_types[i],
							-1);		
		}
	}
	
	oracle_env_treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore));
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	renderer = gtk_cell_renderer_combo_new ();

	g_object_set(renderer, "model", ls_types, 
						   "text-column", 0,
						   "editable", TRUE, 
						   "has-entry", TRUE,
							NULL);
	
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, 
										renderer,
										"text", 
										0);
	gtk_tree_view_column_set_title (column, _("Name"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(oracle_env_treeview), 
									column);
	g_signal_connect (G_OBJECT(renderer), "edited", 
					  G_CALLBACK(env_type_renderer_edited_cb), liststore);
	
	g_signal_connect (G_OBJECT(renderer), "editing-canceled",
					  G_CALLBACK(env_type_renderer_canceled_cb), 
					  oracle_env_treeview);
	
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "editable", TRUE, NULL);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, 
										renderer,
										"text", 
										1);
	gtk_tree_view_column_set_title (column, _("Value"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(oracle_env_treeview), 
									column);
	
	g_signal_connect (G_OBJECT(renderer), "edited", 
					  G_CALLBACK(env_value_renderer_edited_cb), liststore);

	gtk_widget_show (oracle_env_treeview);
	gtk_container_add (GTK_CONTAINER (oracle_env_scroll), oracle_env_treeview);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (oracle_env_treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (oracle_env_treeview), FALSE);
        
	oracle_buttons_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (oracle_buttons_vbox);
	gtk_box_pack_start (GTK_BOX (oracle_env_hbox), oracle_buttons_vbox, FALSE, TRUE, 4);

	oracle_env_add_button = gtk_button_new_with_mnemonic (_("Add"));
	gtk_widget_show (oracle_env_add_button);
	gtk_box_pack_start (GTK_BOX (oracle_buttons_vbox), oracle_env_add_button, FALSE, FALSE, 0);

	oracle_env_del_button = gtk_button_new_with_mnemonic (_("Delete"));
	gtk_widget_show (oracle_env_del_button);
	gtk_box_pack_start (GTK_BOX (oracle_buttons_vbox), oracle_env_del_button, FALSE, FALSE, 0);
        
	oracle_env_label = gtk_label_new (_("<b>Environment</b>"));
	gtk_widget_show (oracle_env_label);
	gtk_frame_set_label_widget (GTK_FRAME (oracle_env_frame), oracle_env_label);
	gtk_label_set_use_markup (GTK_LABEL (oracle_env_label), TRUE);
        
	oracle_options_frame = gtk_frame_new (NULL);
	gtk_widget_show (oracle_options_frame);
	gtk_box_pack_start (GTK_BOX (oracle_page_vbox), oracle_options_frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (oracle_options_frame), 2);
	gtk_frame_set_shadow_type (GTK_FRAME (oracle_options_frame), GTK_SHADOW_NONE);
      
	
	GSQL_FIXME;
	/*alignment9 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment9);
	gtk_container_add (GTK_CONTAINER (oracle_options_frame), alignment9);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment9), 0, 0, 12, 0);
        
	oracle_options_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (oracle_options_vbox);
	gtk_container_add (GTK_CONTAINER (alignment9), oracle_options_vbox);
        
	oracle_enable_trace_check = gtk_check_button_new_with_mnemonic (_("Enable trace (may affect perfomance)"));
	gtk_widget_show (oracle_enable_trace_check);
	gtk_box_pack_start (GTK_BOX (oracle_options_vbox), oracle_enable_trace_check, FALSE, FALSE, 0);
        
	oracle_options_label = gtk_label_new (_("<b>Options</b>"));
	gtk_widget_show (oracle_options_label);
	gtk_frame_set_label_widget (GTK_FRAME (oracle_options_frame), oracle_options_label);
	gtk_label_set_use_markup (GTK_LABEL (oracle_options_label), TRUE);
	*/
	g_signal_connect ((gpointer) use_system_env_check, "toggled",
						G_CALLBACK (on_conf_use_system_env_check_toggled),
						NULL);
	
	g_signal_connect ((gpointer) oracle_env_add_button, "clicked",
						G_CALLBACK (on_conf_oracle_env_add_button_activate),
						oracle_env_treeview);

	g_signal_connect ((gpointer) oracle_env_del_button, "clicked",
						G_CALLBACK (on_conf_oracle_env_del_button_activate),
						oracle_env_treeview);
	/*
	g_signal_connect ((gpointer) oracle_enable_trace_check, "toggled",
						G_CALLBACK (on_conf_oracle_enable_trace_check_toggled),
						NULL);
	*/
	gconf_bool_value = gsql_conf_value_get_boolean (GSQLE_CONF_ORACLE_USE_SYS_ENV);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_system_env_check), gconf_bool_value);
    
	env_all = gsql_conf_value_get_string (GSQLE_CONF_ORACLE_ENV);
	env_list = g_strsplit (env_all, ",", 100);
	
	for (i = 0; env_list[i]; i ++)
	{
		env_name = env_list[i++];
		env_value = env_list[i];

		gtk_list_store_append (liststore, &iter);
		gtk_list_store_set (liststore, &iter,
							0, env_name,
							1, env_value,
							-1);
		
	}
	
	g_strfreev (env_list);
	/*
	gconf_bool_value = gsql_conf_value_get_boolean (GSQLE_CONF_ORACLE_ENABLE_TRACE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (oracle_enable_trace_check), gconf_bool_value);
	*/
	
	return oracle_page_vbox;
};

void
engine_conf_widget_free (GtkWidget *conf_widget)
{
	GSQL_TRACE_FUNC;

	return;
};

void
engine_conf_environment_load (GtkListStore *env, gboolean *use_system)
{
	GSQL_TRACE_FUNC;

	if (use_system)
	{
		return;
	};
	return;
}

void
engine_conf_environment_save (GtkListStore *env)
{
	GSQL_TRACE_FUNC;

	return;
}
