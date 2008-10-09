/***************************************************************************
 *            cvariable.c
 *
 *  Fri Aug 22 00:54:13 2008
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

#include "cvariable.h"
#include "common.h"

struct  _GSQLVariablePrivate
{
	gpointer tmp;
};

static GObjectClass *parent_class;
static void gsql_variable_class_init (GSQLVariableClass *klass);
static void gsql_variable_init (GSQLVariable *obj);

enum {
	SIG_ON_FREE,
	SIG_LAST
};

static guint variable_signals[SIG_LAST] = { 0 };

GType
gsql_variable_get_type ()
{
	GSQL_TRACE_FUNC;

	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLVariableClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_variable_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLVariable),
			0,
			(GInstanceInitFunc) gsql_variable_init,
			NULL
		};
		obj_type = g_type_register_static (G_TYPE_OBJECT,
										   "GSQLVariable", &obj_info, 0);
		
	}
	return obj_type;	
}

GSQLVariable *
gsql_variable_new (void)
{
	GSQL_TRACE_FUNC;
	GSQLVariable *var;
	
	var = g_object_new (GSQL_VARIABLE_TYPE, NULL);
	
	return var;
}

/*  
 *  Static section:
 *
 *  gsql_variable_class_init
 *  gsql_variable_init
 *  gsql_variable_dispose
 *  gsql_variable_finalize
 *
 */



static void
gsql_variable_dispose (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLVariable *variable = GSQL_VARIABLE (obj);
	
	parent_class->dispose(obj);
	
	return;
}

static void
gsql_variable_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLVariable *variable = GSQL_VARIABLE (obj);

	g_free (variable->private);
	parent_class->finalize (obj);
	
	return;
}

static void
gsql_variable_class_init (GSQLVariableClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class;
	
	g_return_if_fail (klass != NULL);
	
	obj_class = (GObjectClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	variable_signals [SIG_ON_FREE] = 
		g_signal_new ("on_free", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLVariableClass,
									   on_free),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	obj_class->dispose = gsql_variable_dispose;
	obj_class->finalize = gsql_variable_finalize;
	
}

static void 
gsql_variable_init (GSQLVariable *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLVariablePrivate, 1);
	obj->is_array = FALSE;
	obj->raw_to_value = NULL;
	
}
