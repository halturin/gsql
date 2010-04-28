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

G_DEFINE_TYPE (GSQLSession, gsql_session, G_TYPE_OBJECT);

static void
gsql_session_dispose (GObject *obj)
{
	GSQLSession *session = GSQL_SESSION (obj);
	
	G_OBJECT_CLASS (gsql_session_parent_class)->dispose (obj);
}


static void 
gsql_session_finalize (GObject *obj)
{
	GSQLSession *session = GSQL_SESSION (obj);
	
	G_OBJECT_CLASS (gsql_session_parent_class)->finalize (obj);;
}

static void
gsql_session_class_init (GSQLSessionClass *class)
{
	GSQL_TRACE_FUNC

	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->dispose = gsql_session_dispose;
	object_class->finalize = gsql_session_finalize;
}

static void
gsql_session_init (GSQLSession *session)
{
	GSQL_TRACE_FUNC

	
}

gboolean
gsql_session_lock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_val_if_fail (GSQL_IS_SESSION (session), FALSE);
	
	return TRUE;	
}

void
gsql_session_unlock (GSQLSession *session)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	
	
}

