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


#include "session-manager.h"
#include "gsql-app.h"

#define GSQL_MIME_SESSION	"application/x-gsql"

static GSQLSession *current_session = NULL;


struct _GSQLSessionManagerPrivate
{
	GtkNotebook		*container;
	GList			*sessions; // list of GSQLSessionPack 

	GtkRecentManager	*recent_sessions;
};

void on_ssmn_new_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_clone_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_reconnect_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_commit_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_rollback_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_close_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_close_all_sessions (GtkMenuItem *mi, gpointer data);
void on_ssmn_next_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_prev_session (GtkMenuItem *mi, gpointer data);

static GtkActionEntry	session_entries[] = {
	{ "ActionNewSession", GTK_STOCK_CONNECT, N_("New session"), NULL,
			N_("Open a new session to the database"), G_CALLBACK (on_ssmn_new_session) },
	
	{ "ActionCloneSession", NULL, N_("Clone"), NULL,
			N_("Open a clone of the current session"), G_CALLBACK (on_ssmn_clone_session) },
	{ "ActionReconnectSession", NULL, N_("Reconnect"), NULL,
			N_("Recover the losted link to the database"), G_CALLBACK (on_ssmn_reconnect_session) },

	{ "ActionCommitSession", GTK_STOCK_YES, N_("Commit changes"), NULL,
			N_("Commit the changes to the database"), G_CALLBACK (on_ssmn_commit_session) },
	{ "ActionRollbackSession", GTK_STOCK_NO, N_("Rollback changes"), NULL,
			N_("Rollback changes"), G_CALLBACK (on_ssmn_rollback_session) },

	{ "ActionCloseSession", GTK_STOCK_DISCONNECT, N_("Close session"), NULL,
			N_("Close the current session to the database "), G_CALLBACK (on_ssmn_close_session) },
	{ "ActionCloseAllSessions", NULL, N_("Close all sessions"), NULL,
			N_("Close the all opened session to the databases "), G_CALLBACK (on_ssmn_close_all_sessions) },

	{ "ActionNextSession", NULL, N_("Next session"), NULL,
			N_("Switch to the next session"), G_CALLBACK (on_ssmn_next_session) },
	{ "ActionPrevSession", NULL, N_("Previous session"), NULL,
			N_("Switch to the previous session"), G_CALLBACK (on_ssmn_prev_session) }
	
};

G_DEFINE_TYPE (GSQLSessionManager, gsql_ssmn, GTK_TYPE_VBOX);

static void
gsql_ssmn_dispose (GObject *object)
{
	GSQL_TRACE_FUNC

//	GSQLSessionManager *ssmn = GSQL_SSMN (object);


}

static void
gsql_ssmn_finalize (GObject *object)
{
	GSQL_TRACE_FUNC



}

static void
gsql_ssmn_class_init (GSQLSessionManagerClass *ssmn_class)
{
	GSQL_TRACE_FUNC

	GObjectClass *object_class = G_OBJECT_CLASS (ssmn_class);

	object_class->dispose = gsql_ssmn_dispose;
	object_class->finalize = gsql_ssmn_finalize;
}

static void
gsql_ssmn_init (GSQLSessionManager *ssmn )
{
	GSQL_TRACE_FUNC

	ssmn->private = g_new0 (GSQLSessionManagerPrivate, 1);

}

static gboolean
on_ssmn_recent_activate (gpointer data)
{


	return TRUE;
}

static void
gsql_ssmn_recent_manager_update (GSQLSessionManager *ssmn, gchar *uri,
    								gboolean add)
{
	GSQL_TRACE_FUNC

	GtkRecentManager *rm = ssmn->private->recent_sessions;

	if (add)
	{
		GtkRecentData *recent;

		recent = g_slice_new0 (GtkRecentData);

		recent->display_name = "test1111";
		recent->description = "test desc";
		recent->mime_type = GSQL_MIME_SESSION;
		recent->app_name = (gchar *) g_get_application_name ();
		recent->app_exec = g_strjoin (" ", "gsql", "%u", NULL);
		recent->groups = NULL;
		recent->is_private = TRUE;

		gtk_recent_manager_add_full (rm, uri, recent);

		g_free (recent->app_exec);
		g_slice_free (GtkRecentData, recent);
		
	} else {
		
		gtk_recent_manager_remove_item (rm, uri, NULL);
	}

}


static void
gsql_ssmn_update_actions_status (GSQLSessionManager *ssmn)
{
	GSQL_TRACE_FUNC

	GSQLAppUI *appui = NULL;
	guint i;

	appui = gsql_iface_get_ui (gsqlapp, NULL);

	g_return_if_fail (GSQL_IS_APPUI (appui));
	
	i = g_list_length (ssmn->private->sessions);

	if (i)
	{
		gsql_appui_set_actions_sensitivity (appui, "ActionGroupSession",
	    					"ActionActiveSessions", TRUE,
	    					"ActionCloneSession", TRUE,
	    					"ActionReconnectSession", TRUE,
	    					"ActionCloseSession", TRUE,
	    					"ActionNextSession", TRUE,
		    				"ActionPrevSession", TRUE,
	    								NULL);

		/*
		 if has commit and rollback
				    		"ActionCommitSession", FALSE,
	    					"ActionRollbackSession", FALSE,

		 */

		if (i > 1)
			gsql_appui_set_actions_sensitivity (appui, "ActionGroupSession",
							"ActionCloseAllSessions", TRUE,
			    			NULL);
		else 
			gsql_appui_set_actions_sensitivity (appui, "ActionGroupSession",
							"ActionCloseAllSessions", FALSE,
			    			NULL);

		gsql_appui_set_actions_sensitivity (appui, "ActionGroupObject",
		    				"ActionMenuObject", TRUE,
		    				NULL);
		

	} else {
		
		gsql_appui_set_actions_sensitivity (appui, "ActionGroupSession",
	    					"ActionActiveSessions", FALSE,
	    					"ActionCloneSession", FALSE,
	    					"ActionReconnectSession", FALSE,
	    					"ActionCommitSession", FALSE,
	    					"ActionRollbackSession", FALSE,
	    					"ActionCloseSession", FALSE,
	    					"ActionCloseAllSessions", FALSE,

		    				"ActionNextSession", FALSE,
		    				"ActionPrevSession", FALSE,
	    								NULL);

		gsql_appui_set_actions_sensitivity (appui, "ActionGroupObject",
		    				"ActionMenuObject", FALSE,
		    							NULL);
		
	}
}

GSQLSessionManager *
gsql_ssmn_new ()
{
	GSQL_TRACE_FUNC

	static GSQLSessionManager 	*ssmn;
	GtkAction			*recsess_action;
	GtkAction			*action;
	GtkWidget			*widget, *recent_menu;
	GtkRecentFilter *filter;
	GtkRecentManager *recent;

	GSQLAppUI *appui;

	if (ssmn)
		return ssmn;
	
	ssmn = g_object_new (GSQL_SSMN_TYPE, NULL);
	
	ssmn->private->container = GTK_NOTEBOOK (gtk_notebook_new ());
	gtk_box_pack_start(GTK_BOX (ssmn), GTK_WIDGET (ssmn->private->container), 
	    				TRUE, TRUE, 0);
	
	gtk_widget_show_all (GTK_WIDGET (ssmn));

	appui = gsql_iface_get_ui (GSQL_IFACE (gsqlapp), NULL);

	gsql_appui_add_actions (appui, "ActionGroupSession",
	    					session_entries,
	    					G_N_ELEMENTS (session_entries), 
	    					NULL);
	
	recsess_action = gtk_recent_action_new ("ActionRecentSessions", N_("Recent sessions"),
	    								N_("Open a recent session"), GTK_STOCK_CONNECT);
	
	gsql_appui_add_action (appui, "ActionGroupSession", GTK_ACTION (recsess_action), NULL);

	action = gtk_recent_action_new ("ActionActiveSessions", N_("Active sessions"),
	    								N_("Switch to the session"), NULL);
	
	gsql_appui_add_action (appui, "ActionGroupSession", GTK_ACTION (action), NULL);	

	recent = gtk_recent_manager_get_default ();
	recent_menu = gtk_recent_chooser_menu_new_for_manager (recent);

	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu), GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (recent_menu), 20);

	filter = gtk_recent_filter_new ();

	// FIXME
	//gtk_recent_filter_add_application (filter, g_get_application_name ());

	gtk_recent_filter_add_mime_type (filter, GSQL_MIME_SESSION);
	
	gtk_recent_chooser_set_filter (GTK_RECENT_CHOOSER (recent_menu), filter);
	
	g_signal_connect (recent_menu, "item-activated",
	    				G_CALLBACK (on_ssmn_recent_activate), NULL);

	ssmn->private->recent_sessions = recent;
	gsql_ssmn_recent_manager_update (ssmn, "gsql://asdfasfdasdfasdf", TRUE);

	gsql_ssmn_update_actions_status (ssmn);

	gsql_appui_merge (appui, PACKAGE_UI_DIR "/gsql-sessions.ui");

	widget = gsql_appui_get_widget (appui, "/ToolbarMain/PHolderSessionToolbar/ToolitemNewSession");

	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (widget), recent_menu);
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (widget), N_("New session"));
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (widget), 
	    			N_("Open a new session to the database"));
	gtk_menu_tool_button_set_arrow_tooltip_text (GTK_MENU_TOOL_BUTTON (widget), 
	    										N_("Open a recent session"));

	action = gsql_appui_get_action (appui, "ActionGroupSession", "ActionNewSession");
	g_object_set (G_OBJECT (action), "is-important", TRUE, NULL);
	gtk_action_connect_proxy (action, widget);
	    
	return ssmn;
}


static void
ssmn_session_set_active (GSQLSession *session)
{
	GSQL_TRACE_FUNC

	
}

void 
on_ssmn_new_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC

	GSQLSessionManager 	*ssmn = NULL;
	GSQLSession *session = NULL;
	gint i;
	ssmn = gsql_app_get_ssmn (GSQL_APP (gsqlapp));

	session = g_object_new (GSQL_SESSION_TYPE, NULL);
		gtk_widget_show_all (GTK_WIDGET (session));	
	
	if (session)
	{
		ssmn->private->sessions = g_list_append (ssmn->private->sessions, session);

		i = gtk_notebook_append_page (ssmn->private->container, GTK_WIDGET (session), NULL);
		gtk_notebook_set_current_page (ssmn->private->container, i);
	}
	
	gsql_ssmn_update_actions_status (ssmn);
}

void on_ssmn_clone_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC

//	gsql_ssmn_update_actions_status (ssmn);
}

void on_ssmn_reconnect_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC


}

void on_ssmn_commit_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC


}

void on_ssmn_rollback_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC


}

void on_ssmn_close_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC

//	gsql_ssmn_update_actions_status (ssmn);
}

void on_ssmn_close_all_sessions (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC

//	gsql_ssmn_update_actions_status (ssmn);
}

void on_ssmn_prev_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC
	
	GSQLSessionManager 	*ssmn = NULL;

	ssmn = gsql_app_get_ssmn (GSQL_APP (gsqlapp));

	gtk_notebook_prev_page (ssmn->private->container);

}

void on_ssmn_next_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC


		GSQLSessionManager 	*ssmn = NULL;

	ssmn = gsql_app_get_ssmn (GSQL_APP (gsqlapp));

	gtk_notebook_next_page (ssmn->private->container);

}
