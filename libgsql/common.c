/***************************************************************************
 *            common.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <libgsql/common.h>

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory (const gchar * directory)
{
	GSQL_TRACE_FUNC
		
  	pixmaps_directories = g_list_prepend (pixmaps_directories,
					g_strdup (directory));
}


static gchar *
find_pixmap_file (const gchar * filename)
{
	GSQL_TRACE_FUNC
	
	GList *elem;

  /* We step through each of the pixmaps directory to find it. */
	elem = pixmaps_directories;
	
	while (elem)
    {
		gchar *pathname = g_strdup_printf ("%s%s%s", (gchar *) elem->data,
					 G_DIR_SEPARATOR_S, filename);
		
		if (g_file_test (pathname, G_FILE_TEST_EXISTS))
			return pathname;
		
      g_free (pathname);
      elem = elem->next;
    }
	
	return NULL;
}


GtkWidget *
create_pixmap (const gchar * filename)
{
	GSQL_TRACE_FUNC
		
	gchar *pathname = NULL;
 	GtkWidget *pixmap;

	if (!filename || !filename[0])
		return gtk_image_new ();

	pathname = find_pixmap_file (filename);

	if (!pathname)
    {
		g_warning (_("Couldn't find pixmap file: %s"), filename);
		return gtk_image_new ();
    }

	pixmap = gtk_image_new_from_file (pathname);
	g_free (pathname);
	
	return pixmap;
}

GdkPixbuf *
create_pixbuf (const gchar * filename)
{
	GSQL_TRACE_FUNC
		
	gchar *pathname = NULL;
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	if (!filename || !filename[0])
		return NULL;

	pathname = find_pixmap_file (filename);

	if (!pathname)
    {
		g_warning (_("Couldn't find pixmap file: %s"), filename);
		return NULL;
    }

	pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
	
	if (!pixbuf)
    {
		fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
	       pathname, error->message);
		g_error_free (error);
    }
	
	g_free (pathname);
	
	return pixbuf;
}


void
gsql_marshal_VOID__STRING_STRING (GClosure  *closure,
								  GValue	*return_value,
								  guint		n_params,
								  const GValue  *params,
								  gpointer  *invocation,
								  gpointer  *marshal)
{
	GSQL_TRACE_FUNC;
	
	typedef void (*GSQLMarshalFunc_VOID__STRING_STRING) (gpointer   d1,
														const char  *a1,
														const char  *a2,
														 gpointer   d2);
	register GSQLMarshalFunc_VOID__STRING_STRING callback;
	register GCClosure *cc = (GCClosure *) closure;
	register gpointer d1, d2;
	
	g_return_if_fail (n_params == 3);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		d1 = closure->data;
		d2 = g_value_peek_pointer (params + 0);
		
	} else {
		
		d2 = closure->data;
		d1 = g_value_peek_pointer (params + 0);		
	};
	
	callback = (GSQLMarshalFunc_VOID__STRING_STRING) (marshal ? marshal : cc->callback);
	
	callback (d1, 
			  g_value_get_string(params +1), 
			  g_value_get_string(params +2),
			 d2);
	

}

gpointer
gsql_thread_join (GThread *thread)
{
	gpointer gsql_main_thread_orig;
	gpointer ret;
	
	gsql_main_thread_orig = gsql_main_thread;
	gsql_main_thread = (gpointer) thread;
	
	ret = g_thread_join (thread);
	
	gsql_main_thread = gsql_main_thread_orig;
	
	return ret;
}

