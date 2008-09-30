/***************************************************************************
 *            engine_confcb.c
 *
 *  Fri Sep  7 00:01:24 2007
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


void
on_conf_use_system_env_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data)
{
	GSQL_TRACE_FUNC

	GtkDialog * dialog = user_data;        
	gboolean status;
	GtkWidget *oracle_env_hbox;
        
	oracle_env_hbox = g_object_get_data (G_OBJECT (togglebutton), "oracle_env_hbox");        
	status = gtk_toggle_button_get_active (togglebutton);
    
	gtk_widget_set_sensitive ( oracle_env_hbox, !status);        
        
	gsql_conf_value_set_boolean (GSQLE_CONF_ORACLE_USE_SYS_ENV,
							status);    
    
	return;
};

void
on_conf_oracle_env_add_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC

	return;
};


void
on_conf_oracle_env_edit_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC

	return;
};


void
on_conf_oracle_env_del_button_activate (GtkButton *button,
											gpointer user_data)
{
	GSQL_TRACE_FUNC

	return;
};


void
on_conf_oracle_enable_trace_check_toggled (GtkToggleButton *togglebutton,
											gpointer user_data)
{
	GSQL_TRACE_FUNC

	gboolean status;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gsql_conf_value_set_boolean (GSQLE_CONF_ORACLE_ENABLE_TRACE,
							status);
	return;
};

