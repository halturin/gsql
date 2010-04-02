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

G_BEGIN_DECLS

typedef struct _GSQLIface GSQLIface; 
typedef struct _GSQLInterface GSQLInterface;

#define GSQL_TYPE_IFACE	(gsql_iface_get_type ())
#define GSQL_IFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_TYPE_IFACE, GSQLIface))
#define GSQL_IS_IFACE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_TYPE_IFACE))

struct _GSQLInterface 
{
	GTypeInterface g_iface;

	GSQLAppUI *	(*get_ui) (GSQLIface *iface, GError **error);
	GSQLAppPreferences * (*get_preferences) (GSQLIface *iface, GError **error);

	GObject* (*get_object) (GSQLIface *iface, const gchar iface_name,
	    					GError **error);
}
 
GType gsql_iface_get_type (void);

GObject *
gsql_iface_get_object (GSQLInterface *iface, const gchar iface_name,
	    					GError **error);

G_END_DECLS

#endif