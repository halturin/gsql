/***************************************************************************
 *            gsqlconfcb.c
 *
 *  Tue Oct 17 20:17:59 2006
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

#include <gtk/gtk.h>
#include "gsqlconf.h"

#include <libgsql/editor.h>
#include <libgsql/workspace.h>
#include <libgsql/plugins.h>
#include <libgsql/type_datetime.h>
#include <libgsql/sqleditor.h>
#include <time.h>

void
on_gsql_prefs_show_dbnav_check_toggled (GtkToggleButton *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_WORKSPACE_SHOW_NAVIGATE,
								status);

};

void
on_gsql_prefs_show_message_check_toggled (GtkToggleButton *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_WORKSPACE_SHOW_MESSAGES,
							status);

}

void
on_gsql_prefs_restore_sizepos_check_toggled (GtkToggleButton  *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_UI_RESTORE_SIZE_POS,
							status);

}

void
on_gsql_prefs_widescreen_toggled (GtkToggleButton  *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_WORKSPACE_WIDESCREEN,
							status);

}

void 
on_fetch_limit_step_changed (GtkSpinButton *button, gpointer data)
{
	
	guint value;
	
	value = gtk_spin_button_get_value (button);
	gsql_conf_value_set_int (GSQL_CONF_SQL_FETCH_STEP,
							 value);
	
}

void 
on_fetch_limit_max_changed (GtkSpinButton *button, gpointer data)
{
	
	guint value;
	
	value = gtk_spin_button_get_value (button);
	gsql_conf_value_set_int (GSQL_CONF_SQL_FETCH_MAX,
							 value);
	
}

void
on_gsql_prefs_use_system_font_check_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GSQL_TRACE_FUNC;

	GtkWidget * font_button;
	gboolean status;
    
    status = gtk_toggle_button_get_active (togglebutton);
	
	font_button = (GtkWidget *) g_object_get_data (G_OBJECT(togglebutton), "font_button");
	
	gtk_widget_set_sensitive (font_button, !status);
    
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_USE_SYSTEM_FONT,
							status);
    
	return;
};

void
on_gsql_prefs_insert_space_check_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_USE_SPACE,
							status);
	return;
};

void
on_gsql_prefs_tab_width_spin_value_changed (GtkSpinButton *spin_button,
										gpointer user_data)

{
	GSQL_TRACE_FUNC;

	gint value;    
  
	value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_button));

	gsql_conf_value_set_int  (GSQL_CONF_EDITOR_TAB_WIDTH,
							value);
    
        return;
};

void
on_gsql_prefs_display_line_num_check_toggled (GtkToggleButton *togglebutton,
                                        gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_SHOW_LINE_NUM,
							status);
	return;
};

void
on_gsql_prefs_highlight_line_check_toggled (GtkToggleButton *togglebutton,
                                        gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_HIGHLIGHT_LINE,
							status);
	return;
};

void
on_gsql_prefs_enable_text_wrap_check_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_WRAPPING,
							status);
	return;
};

void
on_gsql_prefs_enable_auto_indent_check_toggled (GtkToggleButton *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQL_CONF_EDITOR_AUTO_INDENT,
							status);
	return;
};



void 
on_gsql_prefs_font_button_font_set (GtkFontButton * font_button,
                                    gpointer       user_data)
{
	GSQL_TRACE_FUNC;

	const gchar * font; 

	font = gtk_font_button_get_font_name (font_button);
	if (font != NULL)
	{
		gsql_conf_value_set_string (GSQL_CONF_EDITOR_FONT_NAME,
									(gchar *) font);            
	};
        
	return;
};

void
on_pref_plugins_configure_button_clicked (GtkButton *conf_button,
										  gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkTreeView *tv = user_data;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GSQLPlugin *plugin;
	
	model = gtk_tree_view_get_model (tv);
	gtk_tree_view_get_cursor (tv, &path, &column);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (model), (GtkTreeIter *) & iter,
						2, &plugin, -1);
	
	if (plugin->plugin_conf_dialog)
		plugin->plugin_conf_dialog();
	
	return;
}

void
on_prefs_plugins_tree_view_cursor_changed (GtkTreeView *tv,
										   gpointer     user_data)
{
	GSQL_TRACE_FUNC;

	GtkWidget *button = user_data;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GSQLPlugin *plugin;	

	model = gtk_tree_view_get_model (tv);
	gtk_tree_view_get_cursor (tv, &path, &column);
	
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (model), (GtkTreeIter *) & iter,
						2, &plugin, -1);
	
	if (plugin->plugin_conf_dialog != NULL)
		gtk_widget_set_sensitive (GTK_WIDGET(button), TRUE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET(button), FALSE);
		
	return;
}

void
on_pref_datetime_preview (GtkButton *conf_button,
										  gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkEntry *preview = user_data;
	
	gchar *text_preview;
	GSQLTypeDateTime test;
	time_t now;
	struct tm *t;
	time(&now);
	
	t = localtime(&now);

	test.year = t->tm_year+1900;
	test.mon = t->tm_mon+1;
	test.day = t->tm_mday;
	test.hour = t->tm_hour;
	test.min = t->tm_min;
	test.sec = t->tm_sec;
	
	text_preview = gsql_type_datetime_to_gchar (&test, NULL, 0);
	gtk_entry_set_text (preview, text_preview);
	return;
}

gboolean 
on_pref_datetime_entry_focus_out (GtkWidget *widget, GdkEventFocus *event,
							 gpointer user_data)
{
	GSQL_TRACE_FUNC;
		
	GtkEntry *entry = GTK_ENTRY(widget);
	const gchar *format;
	
	format = gtk_entry_get_text (entry);
	gsql_conf_value_set_string (GSQL_CONF_DATETIME_FORMAT, 
									(gchar *) format);
	
	return FALSE;
}
