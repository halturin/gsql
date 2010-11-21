/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2010  Smyatkin Maxim <smyatkinmaxim@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */


#include <libgsql/common.h>
#include <libgsql/conf.h>
#include "engine_conf.h"


void
on_use_custom_charset_check_toggled (GtkToggleButton *togglebutton,
				     gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkWidget *charset_list = user_data;        
	gboolean status;

	status = gtk_toggle_button_get_active (togglebutton);
    
	gtk_widget_set_sensitive ( charset_list, status);        
        
	gsql_conf_value_set_boolean (GSQLE_CONF_Firebird_USE_CUSTOM_CHARSET,
				     status);    
    
	return;
};
