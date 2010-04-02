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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <libgsql/common.h>

/* debug routines */
gpointer gsql_main_thread;
gboolean gsql_opt_trace_enable;
gboolean gsql_opt_debug_enable;

static GList *pixmaps_directories = NULL;

static gchar *
find_pixmap_file (const gchar * filename)
{	
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


/**
 * add_pixmap_directory:
 *
 * @directory: the directory name containing pixmaps.
 *
 * Use this function to set the directory containing installed pixmaps.
 */
void
gsql_add_pixmap_directory (const gchar * directory)
{
	GSQL_TRACE_FUNC;
		
  	pixmaps_directories = g_list_prepend (pixmaps_directories,
					g_strdup (directory));
}

/**
 * create_pixmap:
 *
 * @filename: the image filename
 *
 * Return value: a #GtkWidget object using by gtk_image_new_from_file (..)
 *
 * This is used to create the pixmaps used in the interface.
 */
GtkWidget *
gsql_create_pixmap (const gchar * filename)
{		
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


/**
 * create_pixbuf:
 *
 * @filename: the image filename
 *
 * Return value: a #GdkPixbuf object using by gdk_pixbuf_new_from_file (..)
 *
 * This is used to create the pixbufs used in the interface.
 */
GdkPixbuf *
gsql_create_pixbuf (const gchar * filename)
{		
	gchar *pathname = NULL;
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	if (!filename || !filename[0])
		return NULL;

	pathname = find_pixmap_file (filename);

	g_return_val_if_fail (pathname != NULL, NULL);
	
	pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
	
	if (!pixbuf)
    {
		GSQL_DEBUG ("Failed to load pixbuf file: %s: %s\n",
	       pathname, error->message);
		g_error_free (error);
    }
	
	g_free (pathname);
	
	return pixbuf;
}

/**
 * gsql_thread_join:
 *
 * @thread: a #GThread object
 *
 * Return value: the return value of the thread
 *
 * desc. FIXME
 */
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


