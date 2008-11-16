/***************************************************************************
 *            nav_objects.c
 *
 *  Mon Sep 17 00:41:07 2007
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

#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/session.h>
#include <libgsql/navigation.h>
#include <libgsql/type_datetime.h>
#include "oracle_var.h"


void
oracle_navigation_fill_details (GSQLCursor *cursor, GtkListStore *store)
{
	GSQL_TRACE_FUNC;
	
	gint n = 0;
	GSQLVariable *var = NULL;
	GSQLEOracleVariable *spec;
	gchar attr_name[1024];
	gchar attr_value[1024];
	GtkTreeIter iter;
	GList *lst;
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));
	g_return_if_fail (GTK_IS_LIST_STORE (store));
	
	gtk_list_store_clear (store);
	lst = g_list_first (cursor->var_list);
	
	while (lst)
	{
		var = lst->data;
		spec = var->spec;
		
		g_snprintf (attr_name, 1024, "%s", var->field_name);
		if (!var->value)
		{
			GSQL_DEBUG ("Skip variable[%d]", n);
			n++; lst = g_list_next (lst);
			continue;
		} else {
		
			if (var->raw_to_value)
							var->raw_to_value (var);
			
			switch (var->value_type)
			{
				case G_TYPE_STRING:
					g_snprintf (attr_value, 1024, "%s", 
								(gchar *) var->value );
					break;
				case G_TYPE_INT64:
					g_snprintf (attr_value, 1024, "%llu", 
								*((gint64*) var->value));
					break;
				case G_TYPE_INT:
					g_snprintf (attr_value, 1024, "%d", 
								*((gint*) var->value));
					break;
				case G_TYPE_DOUBLE:
					g_snprintf (attr_value, 1024, "%f", 
								*((gdouble*) var->value));
					break;
				
				default:
					if (var->value_type == GSQL_TYPE_DATETIME)
					{
						
					
						gsql_type_datetime_to_gchar (var->value, attr_value, 1024);
					
						break;
					}
				
					g_snprintf (attr_value, 1024, N_("GSQL: Unsupported type"));
				
			}
		}
		
		n++; 
		lst = g_list_next (lst);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
							GSQL_NAV_DETAILS_STOCK, NULL, //FIXME
							GSQL_NAV_DETAILS_NAME, attr_name,
							GSQL_NAV_DETAILS_VALUE, attr_value,
							  -1);
	}

	
}

