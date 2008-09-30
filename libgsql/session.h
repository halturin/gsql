/***************************************************************************
 *            session.h
 *
 *  Sat Sep  1 19:33:09 2007
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
 
#ifndef _SESSION_H
#define _SESSION_H


#include <glib.h>
#include <glib-object.h>

typedef struct _GSQLSession 		GSQLSession;
typedef struct _GSQLSessionClass 	GSQLSessionClass;
typedef struct _GSQLSessionPrivate	GSQLSessionPrivate;

#define GSQL_SESSION_TYPE 			(gsql_session_get_type ())
#define GSQL_SESSION(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_SESSION_TYPE, GSQLSession))
#define GSQL_SESSION_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_SESSION_TYPE, GSQLSessionClass))

#define GSQL_IS_SESSION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_SESSION_TYPE))
#define GSQL_IS_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_SESSION_TYPE))


#include <libgsql/engines.h>
#include <libgsql/workspace.h>


struct _GSQLSession
{
	GtkContainer parent;
	
	GSQLEngine	*engine;
	gpointer 	spec;
	
	GSQLSessionPrivate *private;
};


struct _GSQLSessionClass
{
	GtkContainerClass parent;
	/* signals */
	
	void (*on_session_open)		(GSQLSession *session);
	void (*on_session_close)	(GSQLSession *session);
	void (*on_session_reopen)	(GSQLSession *session);
	// ?
	void (*on_session_duplicate)	(GSQLSession *session);
	void (*on_session_commit)		(GSQLSession *session);
	void (*on_session_rollback)		(GSQLSession *session);	
};

G_BEGIN_DECLS

GType gsql_session_get_type (void);

GSQLSession * 
gsql_session_new (void);

GSQLSession *
gsql_session_new_with_attrs (gchar *attr_name,...);

GSQLSession *
gsql_session_get_active (void);

void
gsql_session_set_active (GSQLSession *session);

gchar *
gsql_session_get_name (GSQLSession *session);

GSQLWorkspace *
gsql_session_get_workspace (GSQLSession *session);

void 
gsql_session_set_workspace (GSQLSession *session, GSQLWorkspace *workspace);

void
gsql_session_set_attrs (GSQLSession *session, gchar *attr_name,...);

void
gsql_session_add_content (GSQLSession *session, GSQLContent *content);

gint
gsql_sessions_count (void);

gchar *
gsql_session_get_title (GSQLSession *session, gchar *title);

void
gsql_session_release_title (GSQLSession *session, gchar *name);

gboolean 
gsql_session_lock (GSQLSession *session);

void
gsql_session_unlock (GSQLSession *session);

gboolean
gsql_session_lock_state (GSQLSession *session);

G_END_DECLS

#endif /* _SESSION_H */
