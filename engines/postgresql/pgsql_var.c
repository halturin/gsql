/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2008  Taras Halturin  halturin@gmail.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

 
#include <libgsql/common.h>
#include <libgsql/cvariable.h>
#include <libgsql/type_datetime.h>
#include <libgsql/utils.h>

#include "engine_session.h"
#include "pgsql_var.h"


static void on_variable_free (GSQLVariable *var, gpointer user_data);

gboolean
pgsql_variable_init (GSQLVariable *variable, PGSQL_FIELD *field /*, PGSQL_BIND *bind*/)
{
	GSQL_TRACE_FUNC;
	
	GSQLEPGSQLVariable *mvar = NULL;
	
	g_return_val_if_fail (GSQL_IS_VARIABLE (variable), FALSE);
	
	mvar = g_new (GSQLEPGSQLVariable, 1);
	mvar->field = field;
	variable->spec = mvar;
	gboolean ret = TRUE;
	
	//GSQL_DEBUG ("field->max_length = [%d]", field->max_length);

	variable->value_type = G_TYPE_STRING;
	variable->value_length = 10240;
	
	/* switch (field->type) */
	/* { */
	/* 	case FLOAT4OID:   */
	/* 	  //case FLOAT8OID:	 */
	/* 		GSQL_DEBUG ("Cast to G_TYPE_DOUBLE (type: [%d])", field->type); */
	/* 		variable->value_type = G_TYPE_DOUBLE; */
	/* 		variable->value_length = sizeof (gdouble); */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = variable->value; *\/ */
	/* 		/\* bind->buffer_length = variable->value_length; *\/ */
	/* 		break; */
		
	/* 	case INT2OID: */
	/* 	case INT4OID: */
	/* 		GSQL_DEBUG ("Cast to G_TYPE_INT"); */
	/* 		variable->value_type = G_TYPE_INT; */
	/* 		variable->value_length = sizeof(gint); */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = variable->value; *\/ */
	/* 		/\* bind->buffer_length = variable->value_length; *\/ */
	/* 		break; */
		
	/* 	case INT8OID: */
	/* 		GSQL_DEBUG ("Cast to G_TYPE_INT64"); */
	/* 		variable->value_type = G_TYPE_INT64; */
	/* 		variable->value_length = sizeof(gint64); */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = variable->value; *\/ */
	/* 		/\* bind->buffer_length = variable->value_length; *\/ */
	/* 		break; */
		
	/* 	case VARCHAROID: */
	/* 	case BPCHAROID: */
	/* 	case CHAROID: */
	/* 	case NAMEOID: */
	/* 		GSQL_DEBUG ("Cast to G_TYPE_STRING"); */
	/* 		variable->value_type = G_TYPE_STRING; */
	/* 		variable->value_length = 4096; //FIXME */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = variable->value; *\/ */
	/* 		/\* bind->buffer_length = variable->value_length; *\/ */
	/* 		break; */
			
	/* 	case DATEOID: */
	/* 	case TIMEOID: */
	/* 	case TIMETZOID: */
	/* 	case TIMESTAMPOID: */
	/* 	case TIMESTAMPTZOID: */
	/* 		GSQL_DEBUG ("Cast to GSQL_TYPE_DATETIME"); */
	/* 		variable->value_type = GSQL_TYPE_DATETIME; */
	/* 		variable->value_length = sizeof(GSQLTypeDateTime); */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = g_malloc0 (sizeof(PGSQL_TIME)); *\/ */
	/* 		/\* bind->buffer_length = sizeof(PGSQL_TIME); *\/ */
			
	/* 		/\* variable->raw_to_value = pgsql_time_to_datetime; *\/ */
	/* 		break; */
		
	/* 	default: */
	/* 		GSQL_DEBUG ("PGSQL: Unsupported type [%d]", field->type); */
	/* 		GSQL_DEBUG ("Cast Unhandled type to G_TYPE_STRING"); */
	/* 		variable->value_type = GSQL_TYPE_UNSUPPORTED; */
	/* 		variable->value_length = 4096; //FIXME */
	/* 		/\* variable->value = g_malloc0 (variable->value_length); *\/ */
	/* 		/\* bind->buffer = variable->value; *\/ */
	/* 		/\* bind->buffer_length = variable->value_length; *\/ */
			
	/* } */
	GSQL_DEBUG ("Field [%s] has Type [%d]", field->name, variable->value_type);
	variable->value = field->value;
	variable->field_name = g_strdup (field->name);
	g_signal_connect (G_OBJECT (variable), "on-free", G_CALLBACK (on_variable_free), NULL);
	// can I free the field here?
	
	GSQL_DEBUG ("variable creation complete [0x%x] size [%lu]", variable->value, variable->value_length);
	
	return ret;
}


static void
on_variable_free (GSQLVariable *var, gpointer user_data) {
  GSQL_TRACE_FUNC;
	
  g_return_if_fail (GSQL_IS_VARIABLE (var));
  GSQLEPGSQLVariable *spec = var->spec;
	
  g_free (var->spec);
}

/* static void  */
/* pgsql_time_to_datetime (GSQLVariable *variable) */
/* { */
/* 	GSQL_TRACE_FUNC; */
	
/* 	g_return_if_fail (GSQL_IS_VARIABLE (variable)); */
/* 	g_return_if_fail (variable->value_type == GSQL_TYPE_DATETIME); */
	
/* 	PGSQL_TIME *time; */
/* 	GSQLTypeDateTime *gsql_time; */
/* 	GSQLEPGSQLVariable *spec = variable->spec; */
	
/* 	time = (PGSQL_TIME *) spec->bind->buffer; */
/* 	gsql_time = (GSQLTypeDateTime *) variable->value; */
/* 	gsql_time->year = time->year; */
/* 	gsql_time->mon = time->month; */
/* 	gsql_time->day = time->day; */
/* 	gsql_time->hour = time->hour; */
/* 	gsql_time->min = time->minute; */
/* 	gsql_time->sec = time->second; */
	
/* } */

