/***************************************************************************
 *            mysql_var.c
 *
 *  Copyright  2008  Taras Halturin
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
 
#include <libgsql/common.h>
#include <libgsql/cvariable.h>
#include <libgsql/type_datetime.h>

#include "mysql.h"
#include "mysql_var.h"


static void mysql_time_to_datetime (GSQLVariable *variable);
static void on_variable_free (GSQLVariable *var, gpointer user_data);

gboolean
mysql_variable_init (GSQLVariable *variable, MYSQL_FIELD *field, MYSQL_BIND *bind)
{
	GSQL_TRACE_FUNC;
	
	GSQLEMySQLVariable *mvar = NULL;
	
	g_return_val_if_fail (GSQL_IS_VARIABLE (variable), FALSE);
	
	mvar = g_new (GSQLEMySQLVariable, 1);
	mvar->field = field;
	mvar->bind = bind;
	bind->buffer_type = field->type;
	bind->error = &(mvar->err);
	bind->length = &(mvar->length);
	bind->is_null = &(mvar->is_null);
	variable->spec = mvar;
	gboolean ret = TRUE;
	
	GSQL_DEBUG ("field->max_length = [%d]", field->max_length);
	
	switch (field->type)
	{
		case MYSQL_TYPE_FLOAT:  
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_DECIMAL:
			GSQL_DEBUG ("Cast to G_TYPE_DOUBLE");
			variable->value_type = G_TYPE_DOUBLE;
			variable->value_length = 8;
			variable->value = g_malloc0 (variable->value_length);
			bind->buffer = variable->value;
			bind->buffer_length = variable->value_length;
			break;
		
		case MYSQL_TYPE_SHORT:  
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_TINY:
			GSQL_DEBUG ("Cast to G_TYPE_INT");
			variable->value_type = G_TYPE_INT;
			variable->value_length = 4;
			variable->value = g_malloc0 (variable->value_length);
			bind->buffer = variable->value;
			bind->buffer_length = variable->value_length;
			break;
		
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_INT24:
			GSQL_DEBUG ("Cast to G_TYPE_INT64");
			variable->value_type = G_TYPE_INT64;
			variable->value_length = 8;
			variable->value = g_malloc0 (variable->value_length);
			bind->buffer = variable->value;
			bind->buffer_length = variable->value_length;
			break;
		
		case MYSQL_TYPE_BLOB: //FIXME: LONGBLOB, LONGTEXT 	L + 4 bytes, where L <  2^32
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_VARCHAR:
			GSQL_DEBUG ("Cast to G_TYPE_STRING");
			variable->value_type = G_TYPE_STRING;
			variable->value_length = 4096; //FIXME
			variable->value = g_malloc0 (variable->value_length);
			bind->buffer = variable->value;
			bind->buffer_length = variable->value_length;
			break;
			
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATETIME:
			GSQL_DEBUG ("Cast to GSQL_TYPE_DATETIME");
			variable->value_type = GSQL_TYPE_DATETIME;
			variable->value_length = sizeof(GSQLTypeDateTime);
			variable->value = g_malloc0 (variable->value_length);
			
			bind->buffer = g_malloc0 (sizeof(MYSQL_TIME));
			bind->buffer_length = sizeof(MYSQL_TIME);
			
			variable->raw_to_value = mysql_time_to_datetime;
			break;
		
		default:
			GSQL_DEBUG ("MySQL: Unsupported type [%d]", field->type);
			ret = FALSE;
	}
	variable->field_name = g_strdup (field->name);
	g_signal_connect (G_OBJECT (variable), "on-free", G_CALLBACK (on_variable_free), NULL);
	
	GSQL_DEBUG ("variable creation complete [0x%x] size [%lu]", variable->value, variable->value_length);
	
	
	return ret;
}


static void
on_variable_free (GSQLVariable *var, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_VARIABLE (var));
	GSQLEMySQLVariable *spec = var->spec;
	
	if (var->value_type == GSQL_TYPE_DATETIME)
		g_free (spec->bind->buffer);
	
	if (var->field_name)
		g_free (var->field_name);
	
	g_free (var->spec);
	g_free (var->value);
	
	return;
}

/*
void *
mysql_variable_data_to_display_format (GSQLEMySQLVariable *var)
{
	GSQL_TRACE_FUNC;

	void *res = NULL;
	MYSQL_TIME *time;
	gchar * gchar_value;
	gint *gint_value = NULL;
	gint64 *gint64_value = NULL;
	gdouble *gdouble_value = NULL;

	g_return_val_if_fail (var != NULL, NULL);
	
	switch (var->variable_type)
	{
		case G_TYPE_STRING:
			if ((var->field->type == MYSQL_TYPE_DATE) ||
				(var->field->type == MYSQL_TYPE_TIME) ||
				(var->field->type == MYSQL_TYPE_DATETIME))
			{
				time = (MYSQL_TIME *) var->data;
				gchar_value = g_strdup_printf ("%02d-%02d-%04d %02d:%02d:%02d",
                     time->day, time->month, time->year, 
                     time->hour, time->minute, time->second);
			} else
			gchar_value = g_utf8_normalize ((gchar *) var->data, 
											var->data_length, 
											G_NORMALIZE_DEFAULT);
			return gchar_value;
		
		case G_TYPE_INT:
			gint_value = g_malloc0 (sizeof(gint));
			*gint_value = *((gint *)(var->data));
			return gint_value;
			
		case G_TYPE_INT64:
			gint64_value = g_malloc0 (sizeof(gint64));
			*gint64_value = *((gint64 *)(var->data));
			return gint64_value;
		
		case G_TYPE_DOUBLE:
			gdouble_value = g_malloc0 (sizeof(gdouble));
			*gdouble_value = *((gdouble *) (var->data));
			return gdouble_value;
		
		default:
			GSQL_DEBUG ("(details) Unsupported type: %d", var->variable_type);
			break;
	};
	
	return res;
};
*/

static void 
mysql_time_to_datetime (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	g_return_if_fail (variable->value_type == GSQL_TYPE_DATETIME);
	
	MYSQL_TIME *time;
	GSQLTypeDateTime *gsql_time;
	GSQLEMySQLVariable *spec = variable->spec;
	
	time = (MYSQL_TIME *) spec->bind->buffer;
	gsql_time = (GSQLTypeDateTime *) variable->value;
	gsql_time->year = time->year;
	gsql_time->mon = time->month;
	gsql_time->day = time->day;
	gsql_time->hour = time->hour;
	gsql_time->min = time->minute;
	gsql_time->sec = time->second;
	
}

