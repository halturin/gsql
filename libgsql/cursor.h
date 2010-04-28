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

 
#ifndef _CURSOR_H
#define _CURSOR_H

typedef struct _GSQLCursor GSQLCursor;
typedef struct _GSQLCursorClass GSQLCursorClass;
typedef struct _GSQLCursorPrivate GSQLCursorPrivate;

#include <libgsql/session.h>

typedef enum {
	GSQL_CURSOR_STATE_ERROR,
	GSQL_CURSOR_STATE_NONE,
	
	GSQL_CURSOR_STATE_RUN,
	GSQL_CURSOR_STATE_OPEN,
	GSQL_CURSOR_STATE_STOP,
	GSQL_CURSOR_STATE_FETCH,
	GSQL_CURSOR_STATE_FETCHED
} GSQLCursorState;

typedef enum {
	GSQL_CURSOR_BIND_BY_NAME,
	GSQL_CURSOR_BIND_BY_POS
} GSQLCursorBindType;

typedef enum {

	GSQL_CURSOR_STMT_SELECT,
	GSQL_CURSOR_STMT_EXEC,
	GSQL_CURSOR_STMT_INSERT,
	GSQL_CURSOR_STMT_UPDATE,
	GSQL_CURSOR_STMT_DELETE,
	GSQL_CURSOR_STMT_DML, /* Common stmt type for Ins, Upd, Del */
	GSQL_CURSOR_STMT_CREATE,
	GSQL_CURSOR_STMT_DROP,
	GSQL_CURSOR_STMT_ALTER,
	GSQL_CURSOR_STMT_DDL /* commont stmt type for create, drop, alter */
} GSQLCursorStmtType;

#define GSQL_CURSOR_TYPE 			(gsql_cursor_get_type ())
#define GSQL_CURSOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_CURSOR_TYPE, GSQLCursor))
#define GSQL_CURSOR_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_CURSOR_TYPE, GSQLCursorClass))

#define GSQL_IS_CURSOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_CURSOR_TYPE))
#define GSQL_IS_CURSOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_CURSOR_TYPE))


struct _GSQLCursor
{
	GObject parent;
	
	GSQLSession *session;
	gchar		*sql;
	GThread		*thread;
	GtkWidget   *linked_widget;
	GList	    *var_list;
	
	gboolean	scrollable;
	
	GSQLCursorStmtType stmt_type;
	guint64		stmt_affected_rows;
	
	GSQLCursorPrivate *private;
};

struct _GSQLCursorClass
{
	GObjectClass parent;
	
	/* Signals */
	void (*close) (GSQLCursor *cursor);
	void (*state_changed) (GSQLCursor *cursor);
};


G_BEGIN_DECLS


GType gsql_cursor_get_type (void);

GSQLCursor *
gsql_cursor_new (GSQLSession *session, gchar *sql);

GSQLCursorState
gsql_cursor_get_state (GSQLCursor *cursor);

GSQLCursorState
gsql_cursor_open_with_bind (GSQLCursor *cursor, gboolean background, GSQLCursorBindType btype, ...);

GSQLCursorState
gsql_cursor_open (GSQLCursor *cursor, gboolean background);

void
gsql_cursor_stop (GSQLCursor *cursor);

gint
gsql_cursor_fetch (GSQLCursor *cursor, gint rows);

void
gsql_cursor_close (GSQLCursor *cursor);


G_END_DECLS

#endif /* _CURSOR_H */

 
