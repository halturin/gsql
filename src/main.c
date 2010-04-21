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

#include <gtk/gtk.h>
#include <glib.h>
#include <libgsql/common.h>
#include "gsql-app.h"


static gchar *sql_files = NULL;

static GOptionEntry opts[] =
{
	{	"enable-trace", 0, 0, 
		G_OPTION_ARG_NONE, &gsql_opt_trace_enable, 
		N_("Enable tracing information (useful for bugs hunting)"), NULL },
	
	{ 	"enable-debug", 0, 0, 
		G_OPTION_ARG_NONE, &gsql_opt_debug_enable, 
		N_("Enable debugging information (useful for bugs hunting)"), NULL },
	
	{	G_OPTION_REMAINING, 0, 0, 
		G_OPTION_ARG_FILENAME_ARRAY, &sql_files, 
		NULL, NULL },
	
	{ NULL }
};

int
main (int argc, char *argv[])
{
	GError *error = NULL;

	GOptionContext *context = g_option_context_new(N_(" - Integrated Database Development Tool"));

	g_thread_init (NULL);
	gdk_threads_init ();
	gdk_threads_enter ();
	
	g_option_context_add_main_entries(context, opts, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_set_ignore_unknown_options(context, TRUE);
	g_option_context_parse(context, &argc, &argv, &error);
	g_option_context_free(context);


#ifdef ENABLE_NLS

#include <libintl.h>
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

#endif
	gtk_set_locale ();
	
	
	gsql_main_thread = (gpointer) g_thread_self ();

	gtk_init (&argc, &argv);

	gtk_widget_show (GTK_WIDGET (gsql_app_new ()));
	
	gtk_main ();

	gdk_threads_leave ();

	return 0;
}
