/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2010  Taras Halturin  halturin@gmail.com
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

 
#ifndef _SESSION_H
#define _SESSION_H

typedef struct _GSQLSession 		GSQLSession;
typedef struct _GSQLSessionClass 	GSQLSessionClass;
typedef struct _GSQLSessionPrivate	GSQLSessionPrivate;

#define GSQL_SESSION_TYPE 			(gsql_session_get_type ())
#define GSQL_SESSION(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_SESSION_TYPE, GSQLSession))
#define GSQL_SESSION_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_SESSION_TYPE, GSQLSessionClass))

#define GSQL_IS_SESSION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_SESSION_TYPE))
#define GSQL_IS_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_SESSION_TYPE))

struct _GSQLSession
{
	GtkContainer parent;
	GHashTable	*subsessions;
	
	GSQLSessionPrivate *private;
};

#define GSQL_SESSION_SUB_MAIN "main-subsession"

struct _GSQLSessionClass
{
	GtkContainerClass parent;

	
	


	/* signals */
	
	void (*on_session_close)	(GSQLSession *session, gboolean reserved);

	void (*on_session_commit)		(GSQLSession *session);
	void (*on_session_rollback)		(GSQLSession *session);	

};

G_BEGIN_DECLS

GType gsql_session_get_type (void);



G_END_DECLS

#endif /* _SESSION_H */
