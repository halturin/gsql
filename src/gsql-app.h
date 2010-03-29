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

 
#ifndef _GSQLAPP_H
#define _GSQLAPP_H

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _GSQLApp			GSQLApp;
typedef struct _GSQLAppClass 	GSQLAppClass;
typedef struct _GSQLAppPrivate   GSQLAppPrivate;


#define GSQL_APP_TYPE 			(gsql_app_get_type ())
#define GSQL_APP(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_APP_TYPE, GSQLApp))
#define GSQL_APP_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_APP_TYPE, GSQLAppClass))

#define GSQL_IS_APP(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_APP_TYPE))
#define GSQL_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_APP_TYPE))


struct _GSQLApp
{
	GtkWindow parent;

	GSQLAppPrivate *private;
};

struct _GSQLAppClass
{
	GtkWindowClass   parent;
	
};


G_BEGIN_DECLS

GType gsql_app_get_type (void);

GtkWidget *
gsql_app_new (void);

G_END_DECLS

#endif /* _NAVIGATION_H */