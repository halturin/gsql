/***************************************************************************
 *            common.h
 *
 *  Sun Oct  1 11:47:52 2006
 *  Copyright  2006  Halturin Taras
 *  halturin@gmail.com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 

#ifndef _COMMON_H
#define _COMMON_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef ENABLE_NLS

#include <glib/gi18n.h>

#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define Q_(String) g_strip_context ((String), (String))
#  define N_(String) (String)
#endif

#define HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)
  
#define GSQL_GLADE_DIALOGS PACKAGE_GLADE_DIR"/gsql_dialogs.glade"

extern GtkWidget   *gsql_window;
extern gpointer gsql_main_thread;

G_BEGIN_DECLS

/* Use this function to set the directory containing installed pixmaps. */
void 
add_pixmap_directory (const gchar * directory);


/* This is used to create the pixmaps used in the interface. */
GtkWidget *
create_pixmap (const gchar * filename);

/* This is used to create the pixbufs used in the interface. */
GdkPixbuf *
create_pixbuf (const gchar * filename);

/* marshallers */
void
gsql_marshal_VOID__STRING_STRING (GClosure  *closure,
								  GValue	*return_value,
								  guint		n_params,
								  const GValue  *params,
								  gpointer  *invocation,
								  gpointer  *marshal);

/* debug routines */

extern gboolean gsql_opt_trace_enable;
extern gboolean gsql_opt_debug_enable;

#ifdef HAVE_ENABLE_TRACE

#define GSQL_TRACE_FUNC \
	if (gsql_opt_trace_enable) \
		g_print("trace: [%p] %s [%s:%d]\n", (gpointer) g_thread_self(), __func__, __FILE__, __LINE__); \

#else

#define GSQL_TRACE_FUNC

#endif

#ifdef HAVE_ENABLE_DEBUG

#define GSQL_DEBUG(params...) \
	if (gsql_opt_debug_enable) \
		g_debug (params);\

#else

#define GSQL_DEBUG(params...)

#endif
	
#define GSQL_THREAD_ENTER \
	if (gsql_main_thread !=(gpointer)  g_thread_self()) \
		gdk_threads_enter (); \


#define GSQL_THREAD_LEAVE \
	if (gsql_main_thread != (gpointer)  g_thread_self()) \
	{ \
		gdk_flush (); \
		gdk_threads_leave (); \
	}; \

gpointer
gsql_thread_join (GThread *thread);

G_END_DECLS

#endif /* _COMMON_H */
