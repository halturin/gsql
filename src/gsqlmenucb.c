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


#include <config.h>

#ifdef WITH_GNOME
#include <libgnome/libgnome.h>
#endif

#include <libgsql/engines.h>
#include <libgsql/workspace.h>
#include <libgsql/session.h>
#include <libgsql/content.h>
#include <libgsql/utils.h>
#include <stdlib.h>

#include "gsqlconf.h"
#include "gsqlmenucb.h"
#include "gsql.h"
#include "gsqlfiles.h"
#include "gsqlcb.h"


#define _GNU_SOURCE
#include <stdio.h>

#define GSQL_GLADE_DIALOGS PACKAGE_GLADE_DIR"/gsql_dialogs.glade"

void
on_file_new_sql_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	
	session = gsql_session_get_active ();
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	gsql_files_open_file (session, NULL, NULL);

}

void
on_file_open_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GtkWidget *chooser;
	gint ret;
	gchar *file;
	GSQLSession *session;
	static gchar *folder = NULL;
	GtkWidget *hbox, *label;
	GtkWidget *encoding_list;
	gchar *encoding = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	
	session = gsql_session_get_active ();
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	chooser = gtk_file_chooser_dialog_new (N_("Open file..."),
					       NULL,
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					       NULL);
	
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (chooser), FALSE);
	
	encoding_list = gsql_enconding_list ();
	hbox = gtk_hbox_new (FALSE, 3);
	label = gtk_label_new (N_("Charset"));
	gtk_misc_set_padding (GTK_MISC (label), 3, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (hbox), encoding_list, FALSE, TRUE, 3);
	gtk_widget_show_all (hbox);
	
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (chooser), hbox);
	
	if (folder)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), 
											 folder);

	gtk_window_set_transient_for (GTK_WINDOW (chooser), GTK_WINDOW (gsql_window));
	
	ret = gtk_dialog_run (GTK_DIALOG (chooser));
	if (ret == GTK_RESPONSE_OK)
	{
		file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		
		if (folder)
			g_free (folder);
		
		folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (chooser));
		
		if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (encoding_list), &iter))
		{
			model = gtk_combo_box_get_model (GTK_COMBO_BOX (encoding_list));
			gtk_tree_model_get (model, &iter,
						1, &encoding, -1);
			GSQL_DEBUG ("Encoding selected: %s", encoding);
		}
		
		if (file != NULL)
			gsql_files_open_file (session, file, encoding);

	}
	
	gtk_widget_destroy (chooser); 

}

void
on_file_save_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GSQLContent *content = NULL;	
	
	content = gsql_workspace_get_current_content (NULL);
	if (!GSQL_IS_CONTENT (content))
		return;
	
	g_signal_emit_by_name (G_OBJECT(content), "save", FALSE);
	
}

void
on_file_save_as_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GSQLContent *content = NULL;	
	
	content = gsql_workspace_get_current_content (NULL);
	if (!GSQL_IS_CONTENT (content))
		return;
	
	g_signal_emit_by_name (G_OBJECT(content), "save", TRUE);

}

void
on_file_close_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GSQLContent *content = NULL;	
	
	content = gsql_workspace_get_current_content (NULL);
	if (!GSQL_IS_CONTENT (content))
		return;
	
	g_signal_emit_by_name (G_OBJECT(content), "close", FALSE);
	
}

void
on_file_close_all_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GtkDialog *dialog;
	GSQLSession *session;
	GSQLWorkspace *workspace;
	guint ret;
	GtkTreeView *tv;
	GtkTreeModel *model;
	GtkTreeIter iter, child;
	GSQLContent *content;
	gboolean  bvalue;
	guint n;
	GList *clist;
	
	session = gsql_session_get_active ();
	
	g_return_if_fail (GSQL_IS_SESSION (session));
	
	workspace = gsql_session_get_workspace (session);
	
	dialog = gsql_session_unsaved_dialog (session);
	
	if (dialog)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gsql_window));
		ret = gtk_dialog_run(dialog);
		
		switch (ret)
		{
			case GTK_RESPONSE_CLOSE:
			case GTK_RESPONSE_OK:
				tv = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (dialog), "treeview"));
				model = gtk_tree_view_get_model (tv);
				
				gtk_tree_model_get_iter_first (model, &iter);
				
				for (n=0; n < gtk_tree_model_iter_n_children (model, &iter); n++)
				{
					gtk_tree_model_iter_nth_child (model, &child, &iter, n);
					gtk_tree_model_get (model, &child,
										3, &content, -1);
					gtk_tree_model_get (model, &child,  
										0, &bvalue, -1);
					
					if (!bvalue)
						continue;
					
					if (GSQL_IS_CONTENT (content))
					{
						if (ret == GTK_RESPONSE_OK)
							g_signal_emit_by_name (content, "save");
						
						gtk_widget_destroy (GTK_WIDGET (content));
							
					} else {
						
						GSQL_DEBUG ("It is not GSQLContent");
					}
				}
			
				break;
				
			case GTK_RESPONSE_CANCEL:
				gtk_widget_destroy (GTK_WIDGET (dialog));
				return;

		}
		
		gtk_widget_destroy (GTK_WIDGET (dialog));	
		
	}
	
	clist = gsql_workspace_get_content_list (workspace);
	
	while (clist)
	{
		content = clist->data;
		
		gtk_widget_destroy (GTK_WIDGET (content));
		
		clist = g_list_next (clist);
	}
	
}

void
on_file_reload_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLContent *content = NULL;
	
	content = gsql_workspace_get_current_content (NULL);
	if (!GSQL_IS_CONTENT (content))
		return;
	
	g_signal_emit_by_name (G_OBJECT(content), "revert");

}

void
on_exit_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	gsql_window_clean_exit();

}


void
on_fullscreen_activate (GtkToggleAction *ta, gpointer data)
{
	GSQL_TRACE_FUNC;

	if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (ta)))
		gtk_window_fullscreen (GTK_WINDOW (gsql_window));
	else
		gtk_window_unfullscreen (GTK_WINDOW (gsql_window));
}

void
on_navarea_activate (GtkToggleAction *ta, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	gboolean status;
	
	session = gsql_session_get_active ();
	
	if (!session)
		return;
	
	workspace = gsql_session_get_workspace (session);
	status = gtk_toggle_action_get_active (ta);
	
	gsql_workspace_set_navigate_visible (workspace, status);
}

void
on_messarea_activate (GtkToggleAction *ta, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	gboolean status;
	
	session = gsql_session_get_active ();
	
	if (!session)
		return;
	
	workspace = gsql_session_get_workspace (session);
	status = gtk_toggle_action_get_active (ta);
	
	gsql_workspace_set_messages_visible (workspace, status);

}

void
on_next_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GtkNotebook *sessions;

	sessions = GTK_NOTEBOOK (g_object_get_data (G_OBJECT (gsql_window), 
												"sessions"));
	gtk_notebook_next_page (sessions);

}

void
on_prev_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GtkNotebook *sessions;

	sessions = GTK_NOTEBOOK (g_object_get_data (G_OBJECT (gsql_window), 
												"sessions"));
	gtk_notebook_prev_page (sessions);

}

void
on_next_page (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	
	gsql_workspace_next_content (workspace);

}

void
on_prev_page (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	
	gsql_workspace_prev_content (workspace);

}

void
on_content_details (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLSession *session;
	GSQLWorkspace *workspace;
	
	session = gsql_session_get_active ();
	workspace = gsql_session_get_workspace (session);
	
	gsql_workspace_set_content_page (workspace, NULL);  

}

void
on_preferences_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	gsql_conf_dialog();
}


void
on_new_session_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GtkDialog *dialog;
	GtkDialog *msg;
	GtkWidget *dialog_vbox;		
	GtkWidget *notebook;
	GtkWidget *logon_widget;
	gint page_num;
	GSQLEngine *engine = NULL;
	GSQLSession *session = NULL;
	GtkWidget   *sessions;
	GtkWidget	*header;
	gchar 		*session_name;
	gint 		ret;

	gchar logon_message[256];

	dialog = (GtkDialog *) create_dialog_logon();
	
	if (!dialog)
		return;
	
	while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		dialog_vbox = GTK_DIALOG (dialog)->vbox;
		
		notebook = (GtkWidget *) g_object_get_data (G_OBJECT (dialog_vbox),
													"notebook");
		page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
		logon_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook),
														  page_num);
		
		engine = (GSQLEngine *) g_object_get_data (G_OBJECT (logon_widget), 
														  "engine");
		if (!engine->session_open)	
			continue;
		
		session = engine->session_open (logon_widget, logon_message);
		
		if (!session)
		{
			msg = (GtkDialog *) gtk_message_dialog_new (GTK_WINDOW (dialog), 
                                                        0,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_OK,
														"%s", logon_message);
			gtk_dialog_run (msg);
			gtk_widget_destroy ((GtkWidget *) msg);
			
			continue;
		}

		break;
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
	
	if (!GSQL_IS_SESSION (session))
		return;
	
	session->engine = engine;
	sessions = g_object_get_data(G_OBJECT(gsql_window), "sessions");
	session_name = gsql_session_get_name (session);

	header = gsql_utils_header_new (create_pixmap(session->engine->file_logo),
									   session_name, NULL,
									   FALSE, (gint) 1);
	
	gtk_widget_show (GTK_WIDGET (session));
	
	ret = gtk_notebook_append_page (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (session), 
							  header);

	gtk_notebook_set_current_page (GTK_NOTEBOOK(sessions), ret);
	gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (session), TRUE);

}
#ifdef WITH_GNOME
void
on_help_manual_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	gnome_help_display ("gsql.xml", NULL, NULL);
}

void
on_home_page_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GError *error = NULL;
	gnome_url_show (PROJECT_URL, &error);	
}


void
on_api_refs_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GError *error = NULL;
	gnome_url_show (PROJECT_URL "/development/references", &error);	
}


void
on_bug_report_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GError *error = NULL;
	gnome_url_show (PROJECT_BUG_REPORT_URL, &error);
}


void
on_mailgroup_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GError *error = NULL;
	gnome_url_show (PROJECT_GROUP_URL, &error);
}



void
on_about_activate_url (GtkAboutDialog *about,
						const gchar *link,
						gpointer data)
{
	GSQL_TRACE_FUNC;

	GError *error = NULL;
	gnome_url_show (PROJECT_URL, &error);
}

#endif

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
	//size_t len = 0;
	
#define CREDITS_BUFFER 8196
	gchar buffer[CREDITS_BUFFER];
	gchar **credits;

	if (about)
	{
		gtk_window_set_transient_for (GTK_WINDOW (about),
					      GTK_WINDOW (gsql_window));
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
			GSQL_DEBUG ("Opening file: G_IO_STATUS_AGAIN");
			g_io_channel_unref (ioc);
			
			return;
			
		case G_IO_STATUS_ERROR:
		default:
			GSQL_DEBUG ("Opening file: G_IO_STATUS_ERROR");
			g_io_channel_unref (ioc);
			
			return;
	}
	
	g_io_channel_unref (ioc);
	
	credits = g_strsplit (buffer, "\n", 1024);
	
	const gchar *copyright =
		"Copyright \xc2\xa9 2006-2010 Taras Halturin\n";
	const gchar *comments =
		N_("GSQL is an integrated database development tool. This application developing for the GNOME Desktop");


	

	logo = create_pixbuf ("logo.png");
#ifdef WITH_GNOME	
	gtk_about_dialog_set_url_hook (on_about_activate_url, NULL, NULL);
#endif
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
	
	gtk_window_set_transient_for (GTK_WINDOW (about),
				      GTK_WINDOW (gsql_window));
	gtk_window_present (GTK_WINDOW (about));

	if (logo)
		g_object_unref (logo);
	
	g_strfreev(credits);
	
}

