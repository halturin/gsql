/***************************************************************************
 *            gsqlmenucb.c
 *
 *  Tue Jul 25 10:04:16 2006
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

#include <libgnome/libgnome.h>
#include <config.h>
#include <libgsql/engines.h>
#include <libgsql/workspace.h>
#include <libgsql/session.h>
#include <libgsql/content.h>
#include <libgsql/utils.h>
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
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	g_signal_emit_by_name (G_OBJECT(content), "save", FALSE);
	
}

void
on_file_save_as_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GSQLContent *content = NULL;	
	
	content = gsql_workspace_get_current_content (NULL);
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	g_signal_emit_by_name (G_OBJECT(content), "save", TRUE);

}

void
on_file_close_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;
		
	GSQLContent *content = NULL;	
	
	content = gsql_workspace_get_current_content (NULL);
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	g_signal_emit_by_name (G_OBJECT(content), "close", FALSE);
	
}

void
on_file_close_all_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	return;
};

void
on_file_reload_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	GSQLContent *content = NULL;
	
	content = gsql_workspace_get_current_content (NULL);
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	g_signal_emit_by_name (G_OBJECT(content), "revert");

}

void
on_exit_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	gsql_window_clean_exit();

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
	GtkWidget	*close;
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
														logon_message);
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
									   TRUE, (gint) 1);
	close = (GtkWidget *) g_object_get_data (G_OBJECT(header), "close_button");

	g_signal_connect (G_OBJECT (close), "clicked",
						G_CALLBACK (on_session_close_header_button_activate),
						session);
	gtk_widget_show (GTK_WIDGET (session));
	
	ret = gtk_notebook_append_page (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (session), 
							  header);

	gtk_notebook_set_current_page (GTK_NOTEBOOK(sessions), ret);
	gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(sessions),
							  GTK_WIDGET (session), TRUE);

}

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

void
on_about_activate (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC;

	static GtkWidget *about = NULL;
	GdkPixbuf *logo;
	FILE *fauth;
	size_t len = 0;
#define GSQL_HELP_MAX_CREDITS_LINES 512
	gchar *authors[GSQL_HELP_MAX_CREDITS_LINES];
	gchar *buffer;
	gint n = 0;

	fauth = fopen(PACKAGE_DOC_DIR "/AUTHORS", "r");
	g_return_if_fail (fauth != NULL);
	
	while ((getline(&buffer, &len, fauth) != -1) && 
		   (n < GSQL_HELP_MAX_CREDITS_LINES-1) )
	{
		authors[n++] = g_strdup(buffer);
		authors[n-1] = g_strdelimit(authors[n-1],"\n",'\0');
	}
	
	authors[n] = NULL;
	
	if (buffer)
		free (buffer);

	fclose (fauth);
	
	const gchar *copyright =
		"Copyright \xc2\xa9 2006-2008 Taras Halturin\n";
	const gchar *comments =
		N_("GSQL is an integrated database development tool. This application developing for the GNOME Desktop");


	if (about)
	{
		gtk_window_set_transient_for (GTK_WINDOW (about),
					      GTK_WINDOW (gsql_window));
		gtk_window_present (GTK_WINDOW (about));
		
		return;
	}

	logo = create_pixbuf ("logo.png");
	gtk_about_dialog_set_url_hook (on_about_activate_url, NULL, NULL);
	
	about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
			      "name", PROJECT_NAME,
			      "version", VERSION,
			      "copyright", copyright,
			      "comments", comments,
			      "website", PROJECT_URL,
			      "authors", authors,
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
	
	while (n > -1)
		g_free(authors[n--]);

}

