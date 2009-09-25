/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2009  Estêvão Samuel Procópio <tevaum@gmail.com>
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
#include <libgsql/cvariable.h>
#include <libgsql/type_datetime.h>
#include <libgsql/utils.h>

#include "engine_session.h"
#include "pgsql_var.h"


static void on_variable_free (GSQLVariable *var, gpointer user_data);

gboolean
pgsql_variable_init (GSQLVariable *variable, PGSQL_FIELD *field) {
	GSQL_TRACE_FUNC;
	
	GSQLEPGSQLVariable *mvar = NULL;
	
	g_return_val_if_fail (GSQL_IS_VARIABLE (variable), FALSE);
	
	mvar = g_new (GSQLEPGSQLVariable, 1);
	mvar->field = field;
	variable->spec = mvar;
	gboolean ret = TRUE;
	
	variable->value_type = G_TYPE_STRING;
	variable->value_length = 20480;
	
	GSQL_DEBUG ("Field [%s] has Type [%d]", field->name, 
		    variable->value_type);
	variable->value = field->value;
	variable->field_name = g_strdup (field->name);
	g_signal_connect (G_OBJECT (variable), "on-free", 
			  G_CALLBACK (on_variable_free), NULL);
	
	GSQL_DEBUG ("variable creation complete [0x%x] size [%lu]", 
		    variable->value, variable->value_length);
	
	return ret;
}


static void
on_variable_free (GSQLVariable *var, gpointer user_data) {
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_VARIABLE (var));
	GSQLEPGSQLVariable *spec = var->spec;
	
	//TODO: check if all this frees are OK

	g_free (spec->field);
	spec->field = NULL;

	g_free (spec);
	var->spec = NULL;
	
	g_free(var->field_name);
}
