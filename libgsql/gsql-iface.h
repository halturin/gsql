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

#ifndef _GSQL_IFACE_H
#define _GSQL_IFACE_H

#include <libgsql/gsql-iface.h>
#include <libgsql/gsql-appui.h>
#include <libgsql/session.h>



G_BEGIN_DECLS

typedef struct _GSQLIface		GSQLIface;
typedef struct _GSQLInterface 	GSQLInterface;

#define GSQL_TYPE_IFACE (gsql_iface_get_type ())
#define GSQL_IFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_TYPE_IFACE, GSQLIface))
#define GSQL_IS_IFACE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_TYPE_IFACE))
#define GSQL_IFACE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), \
										GSQL_TYPE_IFACE, GSQLInterface))

typedef enum
{
	GSQL_IFACE_PLACEMENT_NONE = 0,
	GSQL_IFACE_PLACEMENT_TOP,
	GSQL_IFACE_PLACEMENT_BOTTOM,
	GSQL_IFACE_PLACEMENT_RIGHT,
	GSQL_IFACE_PLACEMENT_LEFT,
	GSQL_IFACE_PLACEMENT_CENTER,
	GSQL_IFACE_PLACEMENT_FLOATING
} GSQLIfacePlacement;


struct _GSQLInterface 
{
	GTypeInterface parent;

	GSQLAppUI *	(*get_ui) (GSQLIface *iface, GError **error);

	//GSQLAppPreferences * (*get_preferences) (GSQLIface *iface, GError **error);

	GObject* (*get_object) (GSQLIface *iface, const gchar iface_name,
	    					GError **error);
	
	void (*add_widget) (GSQLIface *iface, GSQLSession *session,
    					GtkWidget 	*widget,
    					const gchar	*name,
    					const gchar *title,
    					const gchar *stock_id,
    					GSQLIfacePlacement placement,
    					gboolean	locked,
    					GError 		**error);
};
 
GType gsql_iface_get_type (void);

GObject *
gsql_iface_get_object (GSQLIface *iface, const gchar iface_name,
	    					GError **error);

GSQLAppUI *
gsql_iface_get_ui (GSQLIface *iface, GError **error);

void
gsql_iface_add_widget (GSQLIface *iface, GSQLSession *session,
    					GtkWidget 	*widget,
    					const gchar	*name,
    					const gchar *title,
    					const gchar *stock_id,
    					GSQLIfacePlacement placement,
    					gboolean	locked,
    					GError 		**error);

G_END_DECLS

#endif