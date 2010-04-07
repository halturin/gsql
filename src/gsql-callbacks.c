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

#include <libgsql/common.h>

void
on_gsql_quit (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_preferences (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_user_manual_activate (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_home_page_activate (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_api_activate (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_bug_report_activate (GtkMenuItem *mi, gpointer data)
{


}

void
on_gsql_mailgroup_activate (GtkMenuItem *mi, gpointer data)
{


}

void
on_about_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	static GtkWidget *about = NULL;
	GdkPixbuf *logo;
	GError *err = NULL;
	GIOChannel *ioc = NULL;
	GIOStatus status;
	gsize bytes_read;

	
#define CREDITS_BUFFER 8196
	gchar buffer[CREDITS_BUFFER];
	gchar **credits;

	if (about)
	{
//		gtk_window_set_transient_for (GTK_WINDOW (about),
//					      GTK_WINDOW (gsql_window));
		
		gtk_window_present (GTK_WINDOW (about));
		
		return;
	}
	
	ioc = g_io_channel_new_file (PACKAGE_DOC_DIR "/AUTHORS", "r", &err);

	g_return_if_fail (ioc != NULL);
	
	status = g_io_channel_read_chars (ioc, buffer,
									  CREDITS_BUFFER, &bytes_read,
									  &err);
	switch (status)
	{
		case G_IO_STATUS_EOF:
			GSQL_DEBUG ("Opening file: G_IO_STATUS_EOF");

			break;
			
		case G_IO_STATUS_NORMAL:
			GSQL_DEBUG ("Opening file: G_IO_STATUS_NORMAL");
			
			if (bytes_read == 0)
				GSQL_DEBUG ("AUTHORS file has zero length");
			
			break;
			
		case G_IO_STATUS_AGAIN:
		case G_IO_STATUS_ERROR:
		default:
			GSQL_DEBUG ("Unable to open 'AUTORS' file: %s", err->message);
			g_io_channel_unref (ioc);
			
			return;
	}
	
	g_io_channel_unref (ioc);
	
	credits = g_strsplit (buffer, "\n", 1024);
	
	const gchar *copyright =
		"Copyright \xc2\xa9 2006-2010 Taras Halturin\n";

	const gchar *comments =
		N_("GSQL is an integrated database development tool. "
		   "This application developing for the GNOME Desktop");


	

	logo = gsql_create_pixbuf ("logo.png");

	about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
			      "name", PROJECT_NAME,
			      "version", VERSION,
			      "copyright", copyright,
			      "comments", comments,
			      "website", PROJECT_URL,
			      "authors", credits,
			      "logo", logo, NULL);

	gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);
	
	g_signal_connect (about, "response", G_CALLBACK (gtk_widget_destroy),
			  NULL);
	g_signal_connect (about, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &about);
	
//	gtk_window_set_transient_for (GTK_WINDOW (about),
//				      GTK_WINDOW (gsql_window));

	gtk_window_present (GTK_WINDOW (about));

	if (logo)
		g_object_unref (logo);
	
	g_strfreev(credits);

}
