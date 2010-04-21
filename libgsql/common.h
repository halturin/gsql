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

 

#ifndef _COMMON_H
#define _COMMON_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libgsql/gsql-iface.h>


#define GSQL_GLADE_DIALOGS PACKAGE_GLADE_DIR"/gsql_dialogs.glade"

#define GSQL_TRACE_FUNC \
	if (gsql_opt_trace_enable) \
		g_print("trace: [%p] %s [%s:%d]\n", (gpointer) g_thread_self(), __func__, __FILE__, __LINE__);

#define GSQL_DEBUG(params...) \
	if (gsql_opt_debug_enable) \
		g_debug (params)

#define GSQL_FIXME \
	g_message ("FIXME: file [%s] line [%d]", __FILE__, __LINE__)


#define GSQL_THREAD_ENTER \
	if (gsql_main_thread !=(gpointer)  g_thread_self()) \
		gdk_threads_enter (); \


#define GSQL_THREAD_LEAVE \
	if (gsql_main_thread != (gpointer)  g_thread_self()) \
	{ \
		gdk_flush (); \
		gdk_threads_leave (); \
	} \



extern GSQLIface	*gsqlapp;
extern gpointer gsql_main_thread;
extern gboolean gsql_opt_trace_enable;
extern gboolean gsql_opt_debug_enable;

G_BEGIN_DECLS


void 
gsql_add_pixmap_directory (const gchar * directory);

GtkWidget *
gsql_create_pixmap (const gchar * filename);

GdkPixbuf *
gsql_create_pixbuf (const gchar * filename);

gpointer
gsql_thread_join (GThread *thread);


G_END_DECLS

#endif /* _COMMON_H */
