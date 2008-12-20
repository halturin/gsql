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

 
#include <libnotify/notify.h>
#include <session.h>
#include <common.h>
#include <stock.h>
#include <config.h>


static NotifyNotification *notify;

void
gsql_notify_send (GSQLSession *session, gchar *subject, gchar *message)
{
	GSQL_TRACE_FUNC;

	gchar subj[256];
	gchar *session_name;

	g_return_if_fail (GSQL_IS_SESSION (session));
	
	session_name = gsql_session_get_name (session);
	
	snprintf (subj, 256, "%s:\n%s", session_name, subject);

	if (!notify_is_initted())
		notify_init (PROJECT_NAME);
	
	notify = notify_notification_new (subj, message, NULL, NULL);
	
	notify_notification_set_icon_from_pixbuf(notify, create_pixbuf ("gsql.png"));
	notify_notification_set_timeout (notify, NOTIFY_EXPIRES_DEFAULT);
	notify_notification_show (notify, NULL);
	
}
