/***************************************************************************
 *            oracle_var.h
 *
 *  Thu Sep 20 22:05:37 2007
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
 
#ifndef _ORACLE_VAR_H
#define _ORACLE_VAR_H

#include <glib.h>
#include <libgsql/cursor.h>
#include <libgsql/cvariable.h>
#include "oracle.h"
#include "oracle_cursor.h"


typedef struct _GSQLEOracleVariable GSQLEOracleVariable;

struct _GSQLEOracleVariable
{
	OCIBind		*bind;
	OCIDefine	*define;
	OCIError 	*errhp; // errhp from spec_session. do not free.
	ub2			position;
	ub2			data_type;
	ub2			data_len;
	
	ub2			*actual_len;
	sb2			*indicator;
	ub2			*ret_code;
	GType		variable_type;
	gboolean	variable_len;

	gint (* init) (GSQLEOracleVariable *);
	gint (* free) (GSQLEOracleVariable *);
	
    void 		*data;
};

G_BEGIN_DECLS

gboolean 
oracle_variable_init(GSQLCursor *cursor, GSQLVariable *variable, 
					 OCIParam *param, gint pos);

void
oracle_variable_clear (GSQLCursor *cursor);

G_END_DECLS

#endif /* _ORACLE_VAR_H */

 
