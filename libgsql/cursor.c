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
#include <libgsql/cursor.h>
#include <libgsql/variable.h>
#include <libgsql/session.h>
#include <string.h>



struct _GSQLCursorPrivate 
{
	GSQLCursorState state;
};

static void gsql_cursor_class_init (GSQLCursorClass *klass);
static void gsql_cursor_init (GSQLCursor *obj);
static void gsql_cursor_finalize (GObject *obj);
static void gsql_cursor_dispose (GObject *obj);
static GObjectClass *parent_class;
static gpointer gsql_cursor_open_bg (gpointer cur);
static void gsql_cursor_set_state (GSQLCursor *cursor, GSQLCursorState state);
static GSQLCursorState gsql_cursor_open_with_vbind (GSQLCursor *cursor, 
													GSQLCursorBindType btype, 
													GList *args);
static void gsql_cursor_bind_args_list_free (GList *list);


enum {
	SIG_CLOSE,
	SIG_STATE_CHANGED,
	SIG_LAST
};

static guint cursor_signals[SIG_LAST] = { 0 };

struct CursorOpenBGStruct
{
	gpointer cursor;
	gboolean use_bind;
	GSQLCursorBindType btype;
	GList *args;
};

GType
gsql_cursor_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLCursorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_cursor_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLCursor),
			0,
			(GInstanceInitFunc) gsql_cursor_init,
			NULL
		};
		obj_type = g_type_register_static (G_TYPE_OBJECT,
										   "GSQLCursor", &obj_info, 0);
		
	}
	
	return obj_type;	
}



GSQLCursorState
gsql_cursor_get_state (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	return cursor->private->state;	
}

/**
 * gsql_cursor_new:
 * @session: A #GSQLSession
 * @sql: SQL command
 * 
 * Creates a new #GSQLCursor object
 * 
 * Returns: the new #GSQLCursor 
 **/

GSQLCursor *
gsql_cursor_new (GSQLSession *session, gchar *sql)
{
	GSQL_TRACE_FUNC;

	GSQLCursor *cursor;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), NULL);
	g_return_val_if_fail (sql != NULL, NULL);
	
	cursor = g_object_new (GSQL_CURSOR_TYPE, NULL);	
	cursor->session = session;
	cursor->sql = g_strdup (sql);
	
	return cursor;
}



/**
 * gsql_cursor_open_with_bind:
 * @cursor: A #GSQLCursor
 * @background: bring to background
 * @btype: A #GSQLCursorBindType (by position or by name)
 * @Varargs: pairs of type (GType) and value, terminated with -1
 *
 * Run the cursor statement with binds. Pay attention to the args in example usage:
 * |[
 *	   gsql_cursor_open_with_bind (cursor, 
 *								   FALSE, 
 *								   GSQL_CURSOR_BIND_BY_POS,
 *								   G_TYPE_STRING, owner,
 *								   G_TYPE_INT, 23,
 *								   -1)
 *
 *	   gsql_cursor_open_with_bind (cursor, 
 *								   FALSE, 
 *								   GSQL_CURSOR_BIND_BY_NAME,
 *								   G_TYPE_STRING, "owner"
 *								   G_TYPE_STRING, owner,
 *								   G_TYPE_STRING, "type",
 *								   G_TYPE_INT, 23,
 *								   -1)
 *
 * ]|
 *
 * Returns: the #GSQLCursorState
 * 
 **/

GSQLCursorState
gsql_cursor_open_with_bind (GSQLCursor *cursor, gboolean background, GSQLCursorBindType btype, ...)
{
	GSQL_TRACE_FUNC;;
	
	struct CursorOpenBGStruct *bg_struct;
	GError *err;
	va_list args;
	GSQLCursorState state;
	gchar *tmp;
	GType value_type;
	GList *l_args = NULL;
	gchar *l_value_char = NULL;
	gint args_stop = 1;
	gpointer l_value_num;
	gfloat  *l_value_float;

	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	va_start (args, btype);
	
	do
	{
			value_type = va_arg(args, GType);

			switch ((gchar) value_type)
			{
				case -1:
					GSQL_DEBUG ("bind -1. last argument");
					args_stop = 0;
						
					break;
					
				case G_TYPE_STRING:
				case G_TYPE_POINTER:
					GSQL_DEBUG ("bind: TYPE_POINTER|TYPE_STRING");
					l_args = g_list_append (l_args, (gpointer) value_type);
					l_args = g_list_append (l_args, va_arg(args, void *));
					break;
				
				case G_TYPE_UINT:
				case G_TYPE_INT:
					GSQL_DEBUG ("bind: INT|UINT");
					l_args = g_list_append (l_args, (gpointer) value_type);
					/* x86_64: 
						cursor.c:206: warning: cast to pointer from integer of different size
					 */
					l_args = g_list_append (l_args, (gpointer) va_arg(args, gint));
					break;
					
				case G_TYPE_UINT64:
				case G_TYPE_INT64:
					l_args = g_list_append (l_args, (gpointer) value_type);
					GSQL_DEBUG ("bind: INT64|UINT64");
					l_value_num = g_malloc0(sizeof(gint64));
					*(gint64 *)l_value_num = va_arg(args, gint64);
					l_args = g_list_append (l_args, l_value_num);
					break;

				case G_TYPE_FLOAT:
					//‘gfloat’ is promoted to ‘double’ when passed through ‘...’
					//(so you should pass ‘double’ not ‘gfloat’ to ‘va_arg’)
				case G_TYPE_DOUBLE:
					l_args = g_list_append (l_args, (gpointer) G_TYPE_DOUBLE);
					GSQL_DEBUG ("bind: DBL");
					l_value_num = g_malloc0(sizeof(gdouble));
					*(gdouble *)l_value_num = va_arg(args, gdouble);
					l_args = g_list_append (l_args, l_value_num);
					break;
					
				case G_TYPE_INVALID:
					GSQL_DEBUG ("Cursor bind. Invalid argument");
					args_stop = -1;
					break;
					
				default:
					
					GSQL_DEBUG ("Cursor bind. Unhandled type. %d ", value_type);
					args_stop = -1;
			}
		
	} while (args_stop == 1);
		
	va_end (args);
	
	if (args_stop == -1)
	{
		gsql_cursor_bind_args_list_free (l_args);
		gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
		
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	l_args = g_list_first (l_args);

	if (background)
	{
		bg_struct = g_new0(struct CursorOpenBGStruct, 1);
		bg_struct->cursor = cursor;
		bg_struct->use_bind = TRUE;
		bg_struct->btype = btype;
		bg_struct->args = l_args;
		
		GSQL_DEBUG ("Run sql in background");
		cursor->thread = g_thread_create (gsql_cursor_open_bg,
							  (gpointer) bg_struct,
							  TRUE,
							  &err);
		
		
		
		if (!cursor->thread)
		{
			g_warning ("Couldn't create thread");
			gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
			gsql_cursor_bind_args_list_free (l_args);
			g_free (bg_struct);
			
			return GSQL_CURSOR_STATE_ERROR;
		}
		
		gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_RUN);		
		return GSQL_CURSOR_STATE_RUN;
	}
	
	state = gsql_cursor_open_with_vbind (cursor, btype, l_args);
	
	return state;
}

/**
 * gsql_cursor_open:
 * @cursor: A #GSQLCursor
 * @background: bring to background
 *
 * Run the cursor statement.
 *
 * Returns: the #GSQLCursorState
 * 
 **/

GSQLCursorState
gsql_cursor_open (GSQLCursor *cursor, gboolean background)
{
	GSQL_TRACE_FUNC;

	GError *err;
	struct CursorOpenBGStruct *bg_struct;
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_ERROR);
	
	GSQLCursorState state = GSQL_CURSOR_STATE_RUN;
	
	if (background)
	{
		GSQL_DEBUG ("Run sql in background");
		bg_struct = g_new0(struct CursorOpenBGStruct, 1);
		bg_struct->cursor = cursor;
		bg_struct->use_bind = FALSE;
		cursor->thread = g_thread_create (gsql_cursor_open_bg,
							  (gpointer) bg_struct,
							  TRUE,
							  &err);
		if (!cursor->thread)
		{
			g_warning ("Couldn't create thread");
			gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
			state = GSQL_CURSOR_STATE_ERROR;
		}
		
	} else {
		
		GSQL_DEBUG ("Run sql in foreground");
		
	}
	
	return state;
}

/**
 * gsql_cursor_get_variables:
 * @cursor: A #GSQLCursor
 *
 * Returns: list of #GSQLVariable. Warning! Do not free.
 * 
 **/

GList *
gsql_cursor_get_variables (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (cursor != NULL, NULL);
	g_return_val_if_fail (gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_OPEN,
							NULL);

	return cursor->var_list;
}

/**
 * gsql_cursor_stop:
 * @cursor: A #GSQLCursor
 *
 * Stop the statement execution.
 *
 **/

void
gsql_cursor_stop (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;

	GSQLCursorState state;
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));

	gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_RUN);

}

/**
 * gsql_cursor_fetch:
 * @cursor: A #GSQLCursor
 * @rows:  ***just reserved
 *
 * Fetch the result.
 *
 * Returns: the number fetched rows.
 *
 **/

gint
gsql_cursor_fetch (GSQLCursor *cursor, gint rows)
{
	GSQL_TRACE_FUNC;
	
	gint ret = 0;
	GSQLCursorState state;
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), -1);
	
	state = gsql_cursor_get_state (cursor);
	
	g_return_val_if_fail (state == GSQL_CURSOR_STATE_OPEN, -1);
	
	
		
	gsql_session_unlock (cursor->session);
	
	return ret;
	
}


/**
 * gsql_cursor_close:
 * @cursor: A #GSQLCursor
 *
 * Close the cursor.
 *
 **/

void
gsql_cursor_close (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;;
	
	GList *vlist;
	GSQLVariable *variable;
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));
	
	vlist = cursor->var_list;
	
	while (vlist)
	{
		variable = (GSQLVariable *) vlist->data;
		g_signal_emit_by_name (variable, "on-free", NULL);
		g_object_unref (variable);
		vlist = g_list_next (vlist);
	}
	
	g_signal_emit (cursor, cursor_signals[SIG_CLOSE], 0);
	g_object_unref (cursor);
}



/*
 *  Static section:
 *
 *  gsql_cursor_init
 *  gsql_cursor_class_init
 *  gsql_cursor_finalize
 *  gsql_cursor_dispose
 *  gsql_cursor_open_internal
 *  gsql_cursor_set_state
 *  gsql_cursor_open_with_vbind
 *  gsql_cursor_bind_args_list_free
 *
 */

static void
gsql_cursor_set_state (GSQLCursor *cursor, GSQLCursorState state)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (cursor != NULL);
	cursor->private->state = state;

	g_signal_emit_by_name (G_OBJECT (cursor), "state-changed");
	
}


static gpointer
gsql_cursor_open_bg (gpointer params)
{
	GSQL_TRACE_FUNC;

	struct CursorOpenBGStruct *bg_struct = params;
	
	GSQLCursor *cursor = GSQL_CURSOR (bg_struct->cursor);

	gchar *tmp;
		
	if (bg_struct->use_bind)
	{
		
		GSQL_DEBUG ("... in BG: gsql_cursor_open_with_vbind");
		gsql_cursor_open_with_vbind (cursor, bg_struct->btype, bg_struct->args);
		
	} else
	{
		GSQL_DEBUG ("... in BG: gsql_cursor_open");
		gsql_cursor_open (cursor, FALSE);
	}

	g_free (bg_struct);
	
	return NULL;
}

static GSQLCursorState
gsql_cursor_open_with_vbind (GSQLCursor *cursor, GSQLCursorBindType btype, GList *args)
{
	GSQL_TRACE_FUNC;
	
	GSQLCursorState state = GSQL_CURSOR_STATE_RUN;
	guint n;
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_NONE);
	g_return_val_if_fail (args != NULL, GSQL_CURSOR_STATE_NONE);
	
	if (!gsql_session_lock (cursor->session))
	{
		gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
		return GSQL_CURSOR_STATE_ERROR;
	}
	
	n = g_list_length (args);

	switch (btype)
	{
		case GSQL_CURSOR_BIND_BY_NAME:
			GSQL_DEBUG ("Bind by name");
	
			if ((n < 4) || (n % 4 ))
			{
				GSQL_DEBUG ("Wrong GSQL_CURSOR_BIND_BY_NAME usage. The number of arguments does not comply with conditions.");
				state = GSQL_CURSOR_STATE_NONE;
				break;
			}
			
			break;
		
		case GSQL_CURSOR_BIND_BY_POS:
			GSQL_DEBUG ("Bind by pos");
			
			if ((n < 2) || (n % 2))
			{
				GSQL_DEBUG ("Wrong GSQL_CURSOR_BIND_BY_POS usage. The number of arguments does not comply with conditions.");
				state = GSQL_CURSOR_STATE_NONE;
				break;
			}
			
			break;
			
		default:
			GSQL_DEBUG ("Unknown GSQLCursorBindType [%d]", (gint) btype);
			state = GSQL_CURSOR_STATE_NONE;
	}
	
	gsql_cursor_set_state (cursor, state);
	
	gsql_session_unlock (cursor->session);
	
	return state;
}

static void
gsql_cursor_dispose (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLCursor *cursor = GSQL_CURSOR (obj);
	
	parent_class->dispose(obj);
	
}

static void
gsql_cursor_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLCursor *cursor = GSQL_CURSOR (obj);

	g_free (cursor->private);

	if (cursor->sql)
		g_free (cursor->sql);

	g_list_free(cursor->var_list);
	parent_class->finalize (obj);

}

static void
gsql_cursor_class_init (GSQLCursorClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class;
	
	g_return_if_fail (klass != NULL);
	obj_class = (GObjectClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	
	cursor_signals [SIG_CLOSE] = 
		g_signal_new ("close", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLCursorClass,
									   close),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	cursor_signals [SIG_STATE_CHANGED] = 
		g_signal_new ("state_changed", 
					  G_TYPE_FROM_CLASS (obj_class),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET (GSQLCursorClass,
									   state_changed),
					  NULL, // GSignalAccumulator
					  NULL, g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0);
	
	obj_class->dispose = gsql_cursor_dispose;
	obj_class->finalize = gsql_cursor_finalize;
	
}

static void 
gsql_cursor_init (GSQLCursor *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLCursorPrivate, 1);
	obj->private->state = GSQL_CURSOR_STATE_NONE;
	obj->scrollable = FALSE;
	obj->stmt_affected_rows = 0;

}

static void
gsql_cursor_bind_args_list_free (GList *list)
{
	GList *l_item;
	GList *v_item;
	GType value_type;
	g_return_if_fail (list != NULL);
	gpointer value;
	
	l_item = g_list_first (list);
	
	while (l_item)
	{
		value_type = (GType) l_item->data;
		
		switch (value_type)
		{
			case G_TYPE_INT64:
			case G_TYPE_FLOAT:
			case G_TYPE_DOUBLE:
				l_item = g_list_next(l_item);
				value = l_item->data;
				g_free(value);
				break;
				
			default:
				l_item = g_list_next(l_item);
		
		}
	}
}
