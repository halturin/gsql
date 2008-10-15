/***************************************************************************
 *            cursor.c
 *
 *  Sat Mar  8 19:14:28 2008
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

#include <string.h>

#include "session.h"
#include "cursor.h"
#include "common.h"

#include "cvariable.h"

struct _GSQLCursorPrivate 
{
	GSQLCursorState state;
	gboolean notify_on_finish;
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

void
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

/**
 * gsql_cursor_open_with_bind:
 * @cursor: A #GSQLCursor
 * @background: bring to background
 * @btype: A #GSQLCursorBindType (by position, by name)
 * @Varargs: pairs of type (GType) and value, terminated with -1
 *
 * Run the cursor statement and retrun a #GSQLCursorState.
 * 
 */

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
	
	va_start (args, btype);
	
	do
	{
			value_type = va_arg(args, GType);
			switch (value_type)
			{

				case -1: //last arg
					GSQL_DEBUG ("bind -1. last argument");
					args_stop = 0;
					break;
				
				case G_TYPE_CHAR:
				case G_TYPE_POINTER:
					GSQL_DEBUG ("bind: TYPE_POINTER");
					l_args = g_list_append (l_args, (gpointer) value_type);
					l_args = g_list_append (l_args, va_arg(args, void *));
					break;
				
				case G_TYPE_UINT:
				case G_TYPE_INT:
					GSQL_DEBUG ("bind: INT|UINT");
					l_args = g_list_append (l_args, (gpointer) value_type);
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
	
	gsql_cursor_set_state (cursor, state);
	
	return state;
}


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
		if (cursor->session->engine->cursor_open != NULL)
			{
				
				
				if (!gsql_session_lock (cursor->session))
					return GSQL_CURSOR_STATE_ERROR;
				
				gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_RUN);
				state = cursor->session->engine->cursor_open (cursor);
				gsql_cursor_set_state (cursor, state);
					
				gsql_session_unlock (cursor->session);
			}
			else 
			{
				GSQL_DEBUG("[%s] open not implemented",
						cursor->session->engine->info.name);
				gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
			}
	}
	
	return state;
}

void
gsql_cursor_notify_set (GSQLCursor *cursor, gboolean notify)
{
	GSQL_TRACE_FUNC;;
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));
	
	if (!GTK_IS_WIDGET (cursor->linked_widget))
		return;
	
	cursor->private->notify_on_finish = notify;

}

GList *
gsql_cursor_get_variables (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (cursor != NULL, NULL);
	g_return_val_if_fail (gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_OPEN,
							NULL);

	return cursor->var_list;
}

void
gsql_cursor_stop (GSQLCursor *cursor)
{
	GSQL_TRACE_FUNC;
		
	GSQLWorkspace *workspace;
	gchar msg[GSQL_MESSAGE_LEN];
	GSQLCursorState state;
	
	g_return_if_fail (GSQL_IS_CURSOR (cursor));

	if ((gsql_cursor_get_state (cursor) != GSQL_CURSOR_STATE_RUN) ||
		(gsql_cursor_get_state (cursor) != GSQL_CURSOR_STATE_FETCH))
		return;
	
	workspace = gsql_session_get_workspace (NULL);
	
	if (cursor->session->engine->cursor_stop == NULL)
	{
		GSQL_DEBUG("[%s] stop not implemented",
						cursor->session->engine->info.name);
		memset (msg, 0, GSQL_MESSAGE_LEN);
		
		g_sprintf (msg, N_("The %s engine does not support this feature"), 
				   cursor->session->engine->info.name);
		
		gsql_message_add (workspace, GSQL_MESSAGE_WARNING, msg);
		
		return;
	}
	
	state = cursor->session->engine->cursor_stop (cursor);
	
	gsql_cursor_set_state (cursor, state);

}

gint
gsql_cursor_fetch (GSQLCursor *cursor, gint rows)
{
	GSQL_TRACE_FUNC;
	
	gint ret = 0;
	GSQLCursorState state;
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), -1);
	
	state = gsql_cursor_get_state (cursor);
	
	g_return_val_if_fail (state == GSQL_CURSOR_STATE_OPEN, -1);
	
	
	if (cursor->session->engine->cursor_fetch == NULL)
	{
		GSQL_DEBUG("[%s] fetch not implemented",
						cursor->session->engine->info.name);
		return -1;
	}
	
	if (!gsql_session_lock (cursor->session))
		return -1;
	
	gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_FETCH);
	
	ret = cursor->session->engine->cursor_fetch (cursor, rows);
	
	switch (ret)
	{
		case -1:
			gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_ERROR);
			ret = 0;
			break;
			
		case 0:
			gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_FETCHED);
			break;
			
		default:
			gsql_cursor_set_state (cursor, GSQL_CURSOR_STATE_OPEN);
	}
		
	gsql_session_unlock (cursor->session);
	
	return ret;
	
}

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

	GSQLCursorState state;

	struct CursorOpenBGStruct *bg_struct = params;
	
	GSQLCursor *cursor = GSQL_CURSOR (bg_struct->cursor);

	gchar *tmp;
		
	if (bg_struct->use_bind)
	{
		
		GSQL_DEBUG ("... in BG: gsql_cursor_open_with_vbind");
		state = gsql_cursor_open_with_vbind (cursor, bg_struct->btype, bg_struct->args);
		
	} else
	{
		GSQL_DEBUG ("... in BG: gsql_cursor_open");
		state = gsql_cursor_open (cursor, FALSE);
	}

	g_free (bg_struct);
	
	return NULL;
}

static GSQLCursorState
gsql_cursor_open_with_vbind (GSQLCursor *cursor, GSQLCursorBindType btype, GList *args)
{
	GSQL_TRACE_FUNC;
	
	GSQLCursorState state = GSQL_CURSOR_STATE_OPEN;
	
	g_return_val_if_fail (GSQL_IS_CURSOR (cursor), GSQL_CURSOR_STATE_NONE);
	
	if (!gsql_session_lock (cursor->session))
					return GSQL_CURSOR_STATE_ERROR;

	switch (btype)
	{
		case GSQL_CURSOR_BIND_BY_NAME:
			GSQL_DEBUG ("Bind by name");
			if (cursor->session->engine->cursor_open_with_bind_by_name != NULL)
			{
				cursor->session->engine->cursor_open_with_bind_by_name (cursor, args);
			}
			else 
			{
				GSQL_DEBUG("[%s] bind by name not implemented",
						cursor->session->engine->info.name);
			}
			break;
		
		case GSQL_CURSOR_BIND_BY_POS:
			GSQL_DEBUG ("Bind by pos");
			if (cursor->session->engine->cursor_open_with_bind != NULL)
			{
				cursor->session->engine->cursor_open_with_bind (cursor, args);
			}
			else 
			{
				GSQL_DEBUG("[%s] bind by position not implemented",
						cursor->session->engine->info.name);
			}
			break;
			
		default:
			GSQL_DEBUG ("Unknown GSQLCursorBindType [%d]", (gint) btype);
	}
	
	gsql_session_unlock (cursor->session);
	
	if ((state == GSQL_CURSOR_STATE_OPEN) && (cursor->private->notify_on_finish))
	{
		GSQL_DEBUG ("widgets status: [content->widget = %d] [gsql_window = %d]",
		
		GTK_WIDGET_DRAWABLE (cursor->linked_widget), 
		gtk_window_is_active ( GTK_WINDOW (gsql_window)));
		
		if ((!GTK_WIDGET_DRAWABLE (cursor->linked_widget)) ||
			(!gtk_window_is_active ( GTK_WINDOW (gsql_window))))
		{
			//gtk_window_set_urgency_hint (GTK_WINDOW (gsql_window), TRUE);
			gsql_notify_send (cursor->session, "SQL execution complete", cursor->sql);	
		}
		
	}
	
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
	obj->private->notify_on_finish = FALSE;
	obj->scrollable = FALSE;
	obj->stmt_affected_rows = 0;

}

