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

#ifndef _SESSION_MANAGER_H
#define _SESSION_MANAGER_H

#include <libgsql/common.h>
#include <libgsql/session.h>
#include <libgsql/gsql-appui.h>


typedef struct _GSQLSessionManager GSQLSessionManager;
typedef struct _GSQLSessionManagerClass GSQLSessionManagerClass;
typedef struct _GSQLSessionManagerPrivate GSQLSessionManagerPrivate;


#define GSQL_SSMN_TYPE 			(gsql_ssmn_get_type ())
#define GSQL_SSMN(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
									GSQL_SSMN_TYPE, GSQLSessionManager))

#define GSQL_SSMN_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), \
									GSQL_SSMN_TYPE, GSQLSessionManagerClass))

#define GSQL_IS_SSMN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_SSMN_TYPE))
#define GSQL_IS_SSMN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_SSMN_TYPE))



struct _GSQLSessionManager
{
	GtkVBox		parent;

	GSQLSessionManagerPrivate *private;

};

struct _GSQLSessionManagerClass
{
	GtkVBoxClass	parent;

};


GType gsql_session_manager_get_type (void);

GSQLSessionManager* gsql_ssmn_new ();

#endif