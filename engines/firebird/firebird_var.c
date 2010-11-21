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
#include <libgsql/cvariable.h>
#include <libgsql/type_datetime.h>
#include <libgsql/utils.h>
#include <math.h>

#include "engine_session.h"
#include "firebird_var.h"

static void firebird_blob_to_string (GSQLVariable *variable);
static void firebird_ints_to_ints (GSQLVariable *variable);
static void firebird_varchar_to_string (GSQLVariable *variable);
static void firebird_char_without_spaces (GSQLVariable *variable);
static void firebird_times_to_datetime (GSQLVariable *variable);
static void on_variable_free (GSQLVariable *var, gpointer user_data);

gboolean
firebird_variable_init (GSQLVariable *variable, Firebird_FIELD *field) {
	
	GSQL_TRACE_FUNC;
	
	GSQLEFirebirdVariable *mvar = NULL;
	
	g_return_val_if_fail (GSQL_IS_VARIABLE (variable), FALSE);
	
	mvar = g_new (GSQLEFirebirdVariable, 1);
	mvar->field = field;
	variable->spec = mvar;
	gboolean ret = TRUE;

	switch (field->type & ~1)
	{
		case SQL_FLOAT:
		case SQL_DOUBLE:
			GSQL_DEBUG ("Cast to G_TYPE_DOUBLE");
			variable->value_type = G_TYPE_DOUBLE;
			variable->value_length = sizeof (gdouble);
			variable->value = g_malloc0 (variable->value_length);
		break;
		case SQL_SHORT:
			if (mvar->field->scale == 0)
			{
				GSQL_DEBUG ("Cast to G_TYPE_INT");
				variable->value_type = G_TYPE_INT;
				variable->value_length = sizeof(gshort);
				variable->value = g_malloc0 (variable->value_length);
			}
			else
			{
				GSQL_DEBUG ("Cast to G_TYPE_STRING");
				variable->value_type = G_TYPE_STRING;
				variable->value_length = 64; //FIXME
				variable->value = g_malloc0 (variable->value_length + 1);
				variable->raw_to_value = firebird_ints_to_ints;
			}
			break;				
		case SQL_LONG:
			if (mvar->field->scale == 0)
			{
				GSQL_DEBUG ("Cast to G_TYPE_INT");
				variable->value_type = G_TYPE_INT;
				variable->value_length = sizeof(gint);
				variable->value = g_malloc0 (variable->value_length);
			}
			else
			{
				GSQL_DEBUG ("Cast to G_TYPE_STRING");
				variable->value_type = G_TYPE_STRING;
				variable->value_length = 64; //FIXME
				variable->value = g_malloc0 (variable->value_length + 1);
				variable->raw_to_value = firebird_ints_to_ints;
			}
			break;
		case SQL_INT64:
			if (mvar->field->scale == 0)
			{	
				GSQL_DEBUG ("Cast to G_TYPE_INT64");
				variable->value_type = G_TYPE_INT64;
				variable->value_length = sizeof(gint64);
				variable->value = g_malloc0 (variable->value_length + 1);
			}
			else
			{
				GSQL_DEBUG ("Cast to G_TYPE_DOUBLE");
				variable->value_type = G_TYPE_STRING;
				variable->value_length = 32; //FIXME
				variable->value = g_malloc0 (variable->value_length + 1);
				variable->raw_to_value = firebird_ints_to_ints;
			}
			break;
		case SQL_TEXT:
			GSQL_DEBUG ("Cast to G_TYPE_STRING");
			variable->value_type = G_TYPE_STRING;
			variable->value_length = field->size; //FIXME
			variable->value = g_malloc0 (variable->value_length + 1);
			variable->raw_to_value = firebird_char_without_spaces;
			break;
		case SQL_VARYING:
			GSQL_DEBUG ("Cast to G_TYPE_STRING");
			variable->value_type = G_TYPE_STRING;
			variable->value_length = field->size; //FIXME
			variable->value = g_malloc0 (variable->value_length + 1);
			variable->raw_to_value = firebird_varchar_to_string;
			break;
		case SQL_TIMESTAMP:
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
			variable->value_type = GSQL_TYPE_DATETIME;
			variable->value_length = sizeof(GSQLTypeDateTime);
			variable->value = g_malloc0 (variable->value_length);			
			variable->raw_to_value = firebird_times_to_datetime;
			break;
		case SQL_BLOB:
			GSQL_DEBUG ("Cast to G_TYPE_STRING");
			variable->value_type = G_TYPE_STRING;
			variable->value_length = 4096; //FIXME
			variable->value = g_malloc0 (variable->value_length + 1);
			variable->raw_to_value = firebird_blob_to_string;
			break;
		default:
			GSQL_DEBUG ("Firebird: Unsupported type [%d]", field->type);
			GSQL_DEBUG ("Cast Unhandled type to G_TYPE_STRING");
			variable->value_type = GSQL_TYPE_UNSUPPORTED;
			variable->value_length = 4096; //FIXME
			variable->value = g_malloc0 (variable->value_length + 1);
			break;
		}
	
	GSQL_DEBUG ("Field [%s] has Type [%d]", field->name, 
		    variable->value_type);
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
	ISC_INT64 tens;
	g_return_if_fail (GSQL_IS_VARIABLE (var));
	GSQLEFirebirdVariable *spec = var->spec;

	if (var->field_name)	g_free(var->field_name);
	if (spec->field) 		g_free (spec->field);
	spec->field = NULL;
	if (var->spec)			g_free (var->spec);
	/*if (var->value)			//g_free(<not string value>) errors 
		g_free (var->value);*/  
}

static void 
firebird_blob_to_string (GSQLVariable *variable) 
{
	GSQL_TRACE_FUNC;	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	g_return_if_fail (variable->value_type == G_TYPE_STRING);
	
	GSQLEFirebirdVariable *spec = variable->spec;

	ISC_QUAD                    blob_id;
	isc_blob_handle             blob_handle = NULL;
	ISC_STATUS					status_vector[20];
	short                       blob_seg_len;

	char blob_items[] = { isc_info_blob_max_segment };
	char res_buffer[20], *p, item;
	short length;
	long max_size = 0L, blob_stat = 0L;

	if (spec->field->subtype == 1)
	{
		isc_db_handle  dbhandle = spec->dbhandle;
  		isc_tr_handle  trhandle = spec->trhandle;
	
		blob_id = *(ISC_QUAD *) variable->value;
		
		if (isc_open_blob(status_vector, &dbhandle,
		                  &trhandle, &blob_handle, &blob_id))
					firebird_cursor_show_error (spec->workspace, status_vector, FALSE);

		isc_blob_info ( status_vector, &blob_handle, sizeof (blob_items),
				               blob_items, sizeof (res_buffer), res_buffer);
		for (p = res_buffer; *p != isc_info_end; )
		{
			item = *p++;
			length = (short) isc_vax_integer (p, 2);
			p += 2;
			switch (item)
			{
				case isc_info_blob_max_segment:
					max_size = isc_vax_integer (p, length);
					break;
				default:
					break;
			}
			p += length;
		}
		char blob_segment[max_size];

		blob_stat = isc_get_segment(status_vector, &blob_handle,
                                  (unsigned short *) &blob_seg_len,
                                  sizeof(blob_segment), blob_segment);
		
		gchar* temp_str = g_malloc0 (1024);
		gchar* blob_data = g_malloc0 (4096);
		while (blob_stat == 0 || status_vector[1] == isc_segment)
        {					
			sprintf(temp_str, "%*.*s", blob_seg_len, blob_seg_len, blob_segment);					
			strcat(blob_data, temp_str);
			blob_stat = isc_get_segment(status_vector, &blob_handle,
                                      (unsigned short *)&blob_seg_len,
                                      sizeof(blob_segment), blob_segment);
		}
		g_free (temp_str);

		if (status_vector[1] == isc_segstr_eof)
        {
        	if (isc_close_blob(status_vector, &blob_handle))
        	{
				firebird_cursor_show_error (spec->workspace, status_vector, FALSE);
        	}
        }
        else
        	firebird_cursor_show_error (spec->workspace, status_vector, FALSE);
		variable->value = strdup (blob_data);
		g_free (blob_data);
	}
	else
	{
		sprintf((char *)variable->value, "Subtype is %d", spec->field->subtype);
	}
}

static void 
firebird_ints_to_ints (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	
	GSQLEFirebirdVariable *spec = variable->spec;
	
	gshort dtype = spec->field->type & ~1;
	short dscale = spec->field->scale;
	if (dscale >= 0) return;
	ISC_INT64	value;
	gchar* value_str = g_malloc0 (33);//FIXME
    short field_width;
	switch (dtype)
	{
		case SQL_SHORT:
			value = (ISC_INT64) *(short *) (gchar *)variable->value;
			break;
		case SQL_LONG:
			value = (ISC_INT64) *(int *) (gchar *)variable->value;
			break;
		case SQL_INT64:
			value = (ISC_INT64) *(ISC_INT64 *) (gchar *)variable->value;
			break;
	}
	
	if (dscale < 0)
	{
	    ISC_INT64 tens;
	    short	i;
	    tens = 1;
		ISC_INT64 temp_val = value;
		short count = 0;

		for (i = 0; i > dscale; i--)
			tens *= 10;
		
		if (count <= 0) count = 1;
		if (value >= 0)
		sprintf (value_str,"%" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d",
		    (ISC_INT64) value / tens,
			-dscale, 
			(ISC_INT64) value % tens);
	    else if ((value / tens) != 0)
		sprintf (value_str, "%" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d",
			(ISC_INT64) (value / tens),
			-dscale, 
			(ISC_INT64) -(value % tens));
	    else
		sprintf (value_str, "%s.%0*" ISC_INT64_FORMAT "d",
			"-0",
			-dscale, 
			(ISC_INT64) -(value % tens));		
	}
	variable->value = strdup (value_str);
	g_free (value_str);
}


static void 
firebird_varchar_to_string (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;
	
	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	g_return_if_fail (variable->value_type == G_TYPE_STRING);
	
	GSQLEFirebirdVariable *spec = variable->spec;
	VARY2 *vary2 = NULL;
	vary2 = (VARY2 *) (char *) variable->value;
    vary2->vary_string[strlen(vary2->vary_string)] = '\0';
	
	variable->value = strdup(vary2->vary_string);		
}

static void 
firebird_char_without_spaces (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;
	
	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	g_return_if_fail (variable->value_type == G_TYPE_STRING);
	
	GSQLEFirebirdVariable *spec = variable->spec;
	gint strlength = 0; 
	gchar* tempvalstr = strdup ((gchar *) variable->value);
	strlength = strlen (tempvalstr);
	while (tempvalstr[strlength-1] == ' ')
		tempvalstr[--strlength] = 0;
	variable->value = strdup (tempvalstr);
	g_free (tempvalstr);
	gint asd = 0; 
}

static void 
firebird_times_to_datetime (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;
	struct tm   times;
	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	
	GSQLTypeDateTime *gsql_time = g_malloc0 (sizeof (GSQLTypeDateTime));
	GSQLEFirebirdVariable *spec = (GSQLEFirebirdVariable *)variable->spec;
	gshort type = spec->field->type & ~1;

	switch (type)
	{
		case SQL_TIMESTAMP:
			isc_decode_timestamp ((ISC_TIMESTAMP *)variable->value, &times);
			break;
		case SQL_TYPE_DATE:
			isc_decode_sql_date ((ISC_DATE *)variable->value, &times);
			break;
		case SQL_TYPE_TIME:
			isc_decode_sql_time ((ISC_TIME *)variable->value, &times);
			break;
	}
			gsql_time->year = times.tm_year + 1900;
			gsql_time->mon  = times.tm_mon + 1;
			gsql_time->day  = times.tm_mday;
			gsql_time->hour = times.tm_hour;
			gsql_time->min  = times.tm_min;
			gsql_time->sec  = times.tm_sec;
			variable->value = (gpointer ) (char *) gsql_time;
}

void firebird_cursor_show_error (GSQLWorkspace *workspace,
                            ISC_STATUS status[20],
                            gboolean is_sql_error)
{
	gchar error_str[2048];
	glong SQLCODE = 0;
	if (!is_sql_error)
	{
		isc_print_status (status);
		const ISC_STATUS *pvector = status;
		fb_interpret (error_str, sizeof (error_str), &pvector);
	}
	else
	{
		SQLCODE = isc_sqlcode (status);
		isc_print_sqlerror (SQLCODE, status);
		isc_sql_interprete (SQLCODE, error_str, sizeof (error_str));		
	}
	gsql_message_add (workspace, GSQL_MESSAGE_ERROR, error_str);
}