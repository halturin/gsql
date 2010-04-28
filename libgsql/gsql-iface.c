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

/**
 * SECTION:gsql-iface
 * @short_description: GSQL application interface 
 * @see_also: #GSQLAppUI
 *
 * #GSQLIface is subclass of #GTypeInterface
 *
 * <note>
 * 		<para>
 *			FIXME desc
 *		</para>
 * </note>
 */

#include <libgsql/common.h>
#include <libgsql/gsql-iface.h>
#include <libgsql/gsql-appui.h>

static void
gsql_iface_init ()
{
	static gboolean initdone = FALSE;

	if (!initdone)
	{
		// register signals

		initdone = TRUE;
	}

}

GType
gsql_iface_get_type ()
{
	GSQL_TRACE_FUNC
	
	static GType type = 0;

	if (!type)
	{
		static const GTypeInfo info = {
			sizeof (GSQLInterface),
			gsql_iface_init,
			NULL,
			NULL,
			NULL,
			NULL,
			0,
			0,
			NULL
		};

		type = g_type_register_static (G_TYPE_INTERFACE, "GSQLIface",
		    							&info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}


/**
 * gsql_iface_get_ui:
 *
 * @iface: a #GSQLIface object
 * @error: #GError object 
 *
 * Return value: a #GSQLAppUI object or NULL if no UI was found
 *
 * FIXME desc
 */
GSQLAppUI *
gsql_iface_get_ui (GSQLIface *iface, GError **error)
{
	g_return_val_if_fail (GSQL_IS_IFACE (iface), NULL);

	return GSQL_IFACE_GET_INTERFACE (iface)->get_ui (iface, error);
}

/**
 * gsql_iface_get_object:
 *
 * @iface: a #GSQLIface object
 * @iface_name: 
 * @error: #GError object 
 *
 * Return value: a #GObject or NULL if no UI was found
 *
 * FIXME desc
 */
GObject *
gsql_iface_get_object (GSQLIface *iface, const gchar iface_name,
	    					GError **error)
{
	g_return_val_if_fail (GSQL_IS_IFACE (iface), NULL);

	return GSQL_IFACE_GET_INTERFACE (iface)->get_object (iface, iface_name, error);
}


/**
 * gsql_iface_add_widget:
 *
 * @iface: a #GSQLIface object
 * @session: a #GSQLSession object
 * @widget: a #GtkWidget you would like to add
 * @name:	short name
 * @title:	title for the widget 
 * @stock_id: stock name of the icon
 * @placement: dock position #GSQLIfacePlacement
 * @locked:	lock status
 * @error: #GError object 
 *
 * FIXME desc
 */
void
gsql_iface_add_widget (GSQLIface *iface, GSQLSession *session,
    					GtkWidget 	*widget,
    					const gchar	*name,
    					const gchar *title,
    					const gchar *stock_id,
    					GSQLIfacePlacement placement,
    					gboolean	locked,
    					GError 		**error)
{
	g_return_if_fail (GSQL_IS_IFACE (iface));
	//g_return_if_fail (GSQL_IS_SESSION (session));
	g_return_if_fail (GTK_WIDGET (widget));

	GSQL_IFACE_GET_INTERFACE (iface)->add_widget (iface, session, 
	    widget,
	    						name, title, stock_id, placement, locked,
	    						error);
}
