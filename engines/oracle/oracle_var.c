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
#include <libgsql/session.h>
#include <libgsql/type_datetime.h>
#include <libgsql/utils.h>

#include <string.h>
#include "engine_session.h"
#include "oracle_var.h"
#include "oracle_cursor.h"
#include "oracle.h"
#include <stdlib.h>

static void
oracle_raw_to_value (GSQLVariable *variable);

static void
oracle_variable_clear_foreach (gpointer item, gpointer user_data)
{
	GSQLVariable *var = item;
	GSQLEOracleVariable *var_spec;
	
	var_spec = var->spec;

	if (var_spec->data)
		memset (var_spec->data, 0, var->value_length);
}
	

void
oracle_variable_clear (GSQLCursor *cursor)
{
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));
	
	g_list_foreach (cursor->var_list, oracle_variable_clear_foreach, NULL);
	
}

gboolean
oracle_variable_init(GSQLCursor *cursor, GSQLVariable *variable, 
					 OCIParam *param, gint pos)
{
	GSQL_TRACE_FUNC;

	GSQLEOracleVariable *spec_var;
	GSQLEOracleSession  *spec_session;
	GSQLEOracleCursor   *spec_cursor;
	sword ret;
	unsigned char op[2000];
	sb2 precision;
	sb1 scale;
	dvoid* column_name;
	ub4		*column_name_len;
  
	spec_var = g_malloc0 (sizeof (GSQLEOracleVariable));
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = (GSQLEOracleCursor *) cursor->spec;
	
	ret = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &(spec_var->data_type), 0,
							OCI_ATTR_DATA_TYPE, spec_session->errhp);
	    	
    if (oracle_check_error (cursor, ret))
	{
		g_free(spec_var);
		return FALSE;
	}
	
	switch (spec_var->data_type)
	{
		case SQLT_VCS:
		case SQLT_CHR:
			GSQL_DEBUG ("Variable = SQLT_VCS");
			spec_var->variable_len = TRUE;
			variable->value_length = 4000;
			variable->value_type = G_TYPE_STRING;
			variable->value = spec_var->data = g_malloc0 (variable->value_length + 1);
			break;
                
		case SQLT_AFC:
			GSQL_DEBUG ("Variable = SQLT_AFC");
			spec_var->variable_len = FALSE;
			variable->value_length = 2000;
			variable->value_type = G_TYPE_STRING;
			variable->value = spec_var->data = g_malloc0 (variable->value_length + 1);
			break;
		
		case SQLT_NUM:
		case SQLT_VNU:
		case SQLT_INT:
			GSQL_DEBUG ("Variable = SQLT_NUM [%d]", spec_var->data_type);
			spec_var->data_type = SQLT_VNU;
			scale = precision = 0;
			spec_var->variable_len = FALSE;
			
			ret = OCIAttrGet (param, OCI_HTYPE_DESCRIBE, (dvoid*) &scale, 0,                                                  
								OCI_ATTR_SCALE, spec_session->errhp);
			
			ret = OCIAttrGet (param, OCI_HTYPE_DESCRIBE, (dvoid*) &precision, 0,                                              
								OCI_ATTR_PRECISION, spec_session->errhp);
			
			if (scale == 0 && precision > 0 && precision < 10) 
			{
				variable->value_type = G_TYPE_INT;
				variable->value = g_malloc0 (sizeof (gint));
				
			} else 
				if (scale == 0 || (scale == -127 && precision == 0))
				{
					variable->value_type = G_TYPE_INT64;
					variable->value = g_malloc0 (sizeof (gint64));

				} else {
					variable->value_type = G_TYPE_DOUBLE;
					variable->value = g_malloc0 (sizeof (gdouble));
				}

			GSQL_DEBUG ("SQLT_VNU: [scale = %d] [precision: %d]", scale, precision);
			
			
			variable->raw_to_value = oracle_raw_to_value;
			
			spec_var->data = g_malloc0 (sizeof(OCINumber));
			variable->value_length = sizeof(OCINumber);
			
			break;
		
		case SQLT_DATE:
        case SQLT_TIMESTAMP:
        case SQLT_TIMESTAMP_TZ:
        case SQLT_TIMESTAMP_LTZ:
			GSQL_FIXME;
			/* this types needs own handler */
			
		case SQLT_DAT:
		case SQLT_ODT:
			GSQL_DEBUG ("Variable = SQLT_DAT");
			spec_var->data_type = SQLT_ODT;
			
			variable->value_type = GSQL_TYPE_DATETIME;
			variable->value = g_malloc0 (sizeof (GSQLTypeDateTime));
			variable->raw_to_value = oracle_raw_to_value;
			
			spec_var->data = g_malloc0 (sizeof(OCIDate));
			spec_var->variable_len = FALSE;
			variable->value_length = sizeof(OCIDate);
			
			break;
		
		case SQLT_RDD:
			GSQL_DEBUG ("Variable = SQLT_RDD");
			spec_var->variable_len = FALSE;
			spec_var->data_type = SQLT_CHR;
			variable->value_type = G_TYPE_STRING;
			variable->value_length = 18;
			variable->value = spec_var->data = g_malloc0 (variable->value_length + 1);
			
			break;
		
		default:
			GSQL_DEBUG ("Variable = DEFAULT (type=[%d])", spec_var->data_type);
			spec_var->variable_len = FALSE;
			variable->value_type = GSQL_TYPE_UNSUPPORTED;
			variable->value_length = 4000;
			variable->value = spec_var->data = g_malloc0 (variable->value_length + 1);
	}

	if (spec_var->variable_len)
	{
		ret = OCIAttrGet (param, OCI_HTYPE_DESCRIBE,
							(dvoid*) &(spec_var->data_len), 0, OCI_ATTR_DATA_SIZE,
							spec_session->errhp);
		
		if (oracle_check_error (cursor, ret))
		{
			g_free (spec_var);
			return FALSE;
		}
            	    
		spec_var->ret_code = g_malloc0 (sizeof (ub2));
		
	} else {
		
		spec_var->data_len = variable->value_length;
	}
        
	
        
	ret = OCIAttrGet (param, OCI_HTYPE_DESCRIBE, 
						(dvoid *) &variable->field_name, 
						(ub4 *) &column_name_len, OCI_ATTR_NAME, 
						spec_session->errhp);
	
	

	
	spec_var->position = pos;
	spec_var->actual_len = g_malloc0 (sizeof(ub2));
	spec_var->indicator = g_malloc0 (sizeof(sb2));
	*(spec_var->indicator) = OCI_IND_NULL;
	variable->spec = spec_var;
	spec_var->errhp = spec_session->errhp;
	
	ret = OCIDefineByPos (spec_cursor->statement, &(spec_var->define),
							spec_session->errhp, spec_var->position, (dvoid *) spec_var->data,
							variable->value_length+1, spec_var->data_type, spec_var->indicator,
							spec_var->actual_len, spec_var->ret_code, OCI_DEFAULT);
	
	return TRUE;
}

static void
oracle_raw_to_value (GSQLVariable *variable)
{
	GSQL_TRACE_FUNC;

	gchar * gchar_value;
#define MAX_LEN_STR_VALUE   1024
	ub4    gchar_value_size = MAX_LEN_STR_VALUE;
	gint *gint_value = NULL;
	gint64 *gint64_value = NULL;
	gdouble *gdouble_value = NULL;
	OCIError *errhp = NULL;
	GSQLEOracleVariable *spec_var;
	GSQLTypeDateTime *gsql_time;
	sword ret;
	uword valid;

	sb2           year;
	ub1           month, day, hour, min, sec;
	

	
	g_return_if_fail (GSQL_IS_VARIABLE (variable));
	
	spec_var = variable->spec;
	
	switch (spec_var->data_type)
	{
		case SQLT_CHR:
		case SQLT_VCS:                        
			gchar_value = g_utf8_normalize ((gchar *) spec_var->data, 
											strlen((gchar *) spec_var->data), 
											G_NORMALIZE_DEFAULT);
			return;
                
		case SQLT_DAT:
		case SQLT_ODT:
			GSQL_DEBUG ("GSQL_TYPE_DATETIME displaying");
			gsql_time = (GSQLTypeDateTime *) variable->value;
			
			OCIDateGetDate ((CONST OCIDate *) spec_var->data, &year, &month, &day);
			OCIDateGetTime ((CONST OCIDate *) spec_var->data, &hour, &min, &sec);
			
			gsql_time->year = year;
			gsql_time->mon = month;
			gsql_time->day = day;
			gsql_time->hour = hour;
			gsql_time->min = min;
			gsql_time->sec = sec;
			return;

		case SQLT_NUM:
		case SQLT_INT:
		case SQLT_VNU:                        
			if (variable->value_type == G_TYPE_INT)
			{
				GSQL_DEBUG ("G_TYPE_INT displaying");
				gint_value = (gint *) variable->value;
				
				OCINumberToInt ( spec_var->errhp,
								(CONST OCINumber *) spec_var->data,
								(uword) sizeof(gint),
								OCI_NUMBER_SIGNED,
 								(dvoid *) gint_value);
				
				return;
			}
			
			if (variable->value_type == G_TYPE_INT64)
			{
				/*
				 "Yes, the lack of support for 64bit integers is a bummer. 
				 I've been told Oracle is working on it, but it will take time 
				 because it's more complex that it appears. So not 11gR2, but 
				 perhaps 12g."
				 
				 http://forums.oracle.com/forums/message.jspa?messageID=2561648#2561698
				 */
				
				GSQL_DEBUG ("G_TYPE_INT64 displaying");
				gint64_value = (gint64 *) variable->value;
				
				gchar_value_size = 128;
				gchar_value = g_malloc0 (gchar_value_size);
				ret = OCINumberToText(spec_var->errhp,                                                             
								(CONST OCINumber *) spec_var->data, 
								(unsigned char*) "TM9", 3, NULL, 0,                                                        
								&gchar_value_size,
								(text *) gchar_value);
				
				if (ret != OCI_SUCCESS)
					GSQL_DEBUG ("Error [OCINumberToText]: %s", oracle_get_error_string (spec_var->errhp));
				
				*gint64_value = (gint64) atoll (gchar_value);
				
				g_free (gchar_value);		
				
				return;
			}
			
			GSQL_DEBUG ("G_TYPE_DOUBLE displaying");
			
			gdouble_value = (gdouble *) variable->value;
			
			ret = OCINumberToReal(spec_var->errhp,
							(CONST OCINumber *) spec_var->data, 
							sizeof(gdouble), (dvoid*) gdouble_value);
			
			if (ret != OCI_SUCCESS)
				GSQL_DEBUG ("OCINumberToReal failed with error: %s", oracle_get_error_string (spec_var->errhp));
			
			if (gdouble_value == NULL)
				GSQL_DEBUG ("gdouble_value is NULL");
			
			GSQL_DEBUG ("gdouble [value = %f] [size = %d]", *gdouble_value, sizeof (gdouble));
			
			return;
                
		default:
			GSQL_DEBUG ("Unknown data type: %d", spec_var->data_type);
			return;                    
	}

}

void 
oracle_variable_free(GSQLEOracleVariable *var)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (var != NULL);
	
	// need check type here for LOB objects;
	
	if (var->data)			g_free(var->data);
	if (var->indicator)		g_free(var->indicator);
	if (var->actual_len)	g_free(var->actual_len);
	if (var->ret_code)		g_free (var->ret_code);
	
	g_free (var);

}
