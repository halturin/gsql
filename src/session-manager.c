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


#include "session-manager.h"
#include <libgsql/gsql-appui.h>
#include <libgsql/session.h>

struct _GSQLSessionManagerPrivate
{
	GdlDockLayout	*layout;
	GtkWidget		*dock;
	GList			*sessions; // list of GSQLSession objects

};

G_DEFINE_TYPE (GSQLSessionManager, gsql_ssmn, GTK_TYPE_WIDGET);

static void
gsql_ssmn_dispose (GObject *object)
{
	GSQL_TRACE_FUNC

	GSQLSessionManager *ssmn = GSQL_SSMN (object);

	g_object_unref (ssmn->private->layout);

}

static void
gsql_ssmn_finalize (GObject *object)
{
	GSQL_TRACE_FUNC



}

static void
gsql_ssmn_class_init (GSQLSessionManagerClass *ssmn_class)
{
	GSQL_TRACE_FUNC

	GObjectClass *object_class = G_OBJECT_CLASS (ssmn_class);

	object_class->dispose = gsql_ssmn_dispose;
	object_class->finalize = gsql_ssmn_finalize;
}

static void
gsql_ssmn_init (GSQLSessionManager *ssmn )
{
	GSQL_TRACE_FUNC

	ssmn->private = g_new0 (GSQLSessionManagerPrivate, 1);

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (ssmn->private), GTK_NO_WINDOW);
	
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (ssmn->private), FALSE);

}

GtkWidget *
gsql_ssmn_new ()
{
	GSQL_TRACE_FUNC

	GSQLSessionManager *ssmn;

	ssmn = g_object_new (GSQL_SSMN_TYPE, NULL);
	ssmn->private->dock = gdl_dock_new();
	
	ssmn->private->layout = gdl_dock_layout_new (GDL_DOCK (ssmn->private->dock));

	return GTK_WIDGET (ssmn);
}

