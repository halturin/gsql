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

#ifdef WITH_GNOME
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#include <libgsql/common.h>
#include <libgsql/conf.h>
#include <libgsql/engines.h>
#include <libgsql/plugins.h>
#include <libgsql/stock.h>
#include "gsql.h"


static gchar *sql_files = NULL;
static GList *files = NULL;

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
#ifdef WITH_GNOME
	GnomeProgram *program;
#endif
	GError *error = NULL;
	GOptionContext *context = g_option_context_new(_(" - Integrated Database Development Tool"));
	GtkWidget *dialog;
	g_thread_init (NULL);
	gdk_threads_init ();
	gdk_threads_enter ();
	
	gsql_opt_trace_enable = FALSE;
	gsql_opt_debug_enable = FALSE;
	
	g_option_context_add_main_entries(context, opts, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_set_ignore_unknown_options(context, TRUE);
	g_option_context_parse(context, &argc, &argv, &error);
	g_option_context_free(context);


#ifdef WITH_GNOME	
	program = gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
				argc, argv, 
				GNOME_PARAM_HUMAN_READABLE_NAME,
				N_("GSQL. Integrated Database Development Tool"),
				GNOME_PARAM_APP_DATADIR,
				PACKAGE_DATA_DIR, NULL);
#endif

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	gtk_set_locale ();
	
	
	gsql_main_thread = (gpointer) g_thread_self ();

	gtk_init (&argc, &argv);
	
	add_pixmap_directory (PACKAGE_PIXMAPS_DIR);
	add_pixmap_directory (PACKAGE_PIXMAPS_DIR "/plugins");
	gsql_stock_init ();
	
	gsql_conf_init ();
	gsql_window_create ();
	
	gsql_engines_lookup ();
	gsql_plugins_lookup ();
	
	gtk_widget_show (gsql_window);
	
	if (!gsql_engines_count())
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW (gsql_window),
							 GTK_DIALOG_MODAL,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_CLOSE,
							 N_("Engines not found at " PACKAGE_ENGINES_DIR) );
		g_signal_connect_swapped (dialog, "response",
						  G_CALLBACK (gtk_widget_destroy),
						  dialog);
		gtk_widget_show (dialog);
	}
	
	gtk_main ();

	gdk_threads_leave ();

	return 0;
}
