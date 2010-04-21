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
#include "color-hinter.h"

struct _GSQLSessionManagerPrivate
{
	GdlDockLayout	*layout;
	GtkWidget		*dock;
	GList			*sessions; // list of GSQLSession objects
	GtkWidget		*sessionbar;
	GtkWidget		*colorhint;

};

void on_ssmn_new_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_clone_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_reconnect_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_commit_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_rollback_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_close_session (GtkMenuItem *mi, gpointer data);
void on_ssmn_close_all_sessions (GtkMenuItem *mi, gpointer data);

static GtkActionEntry	session_entries[] = {
	{ "ActionNewSession", GTK_STOCK_CONNECT, N_("New session"), NULL,
			N_("Open a new session to the database"), G_CALLBACK (on_ssmn_new_session) },
	
	{ "ActionCloneSession", GTK_STOCK_CONNECT, N_("Clone"), NULL,
			N_("Open a clone of the current session"), G_CALLBACK (on_ssmn_clone_session) },
	{ "ActionReconnectSession", GTK_STOCK_CONNECT, N_("Reconnect"), NULL,
			N_("Recover the losted link to the database"), G_CALLBACK (on_ssmn_reconnect_session) },

	{ "ActionCommitSession", GTK_STOCK_CONNECT, N_("Commit changes"), NULL,
			N_("Commit the changes to the database"), G_CALLBACK (on_ssmn_commit_session) },
	{ "ActionRollbackSession", GTK_STOCK_CONNECT, N_("Rollback changes"), NULL,
			N_("Rollback changes"), G_CALLBACK (on_ssmn_rollback_session) },

	{ "ActionCloseSession", GTK_STOCK_CONNECT, N_("Close session"), NULL,
			N_("Close the current session to the database "), G_CALLBACK (on_ssmn_close_session) },
	{ "ActionCloseAllSessions", GTK_STOCK_CONNECT, N_("Close all sessions"), NULL,
			N_("Close the all opened session to the databases "), G_CALLBACK (on_ssmn_close_all_sessions) }
};

G_DEFINE_TYPE (GSQLSessionManager, gsql_ssmn, GTK_TYPE_VBOX);

static void
gsql_ssmn_dispose (GObject *object)
{
	GSQL_TRACE_FUNC

	GSQLSessionManager *ssmn = GSQL_SSMN (object);

	g_object_unref (ssmn->private->layout);

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
gsql_ssmn_recent_sessions_menu (GtkRecentChooser *recent)
{

	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent), TRUE);
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent), TRUE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER (recent), TRUE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent), GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (recent), 20);

	g_signal_connect (recent, "item-activated",
	    				G_CALLBACK (on_ssmn_recent_activate), NULL);
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
		
		gtk_widget_set_sensitive (GTK_WIDGET (ssmn->private->colorhint), TRUE);

	} else {
		
		gsql_appui_set_actions_sensitivity (appui, "ActionGroupSession",
	    					"ActionActiveSessions", FALSE,
	    					"ActionCloneSession", FALSE,
	    					"ActionReconnectSession", FALSE,
	    					"ActionCommitSession", FALSE,
	    					"ActionRollbackSession", FALSE,
	    					"ActionCloseSession", FALSE,
	    					"ActionCloseAllSessions", FALSE,
	    								NULL);

		gsql_appui_set_actions_sensitivity (appui, "ActionGroupObject",
		    				"ActionMenuObject", FALSE,
		    							NULL);
		gtk_widget_set_sensitive (GTK_WIDGET (ssmn->private->colorhint), FALSE);
	}
}

GSQLSessionManager *
gsql_ssmn_new ()
{
	GSQL_TRACE_FUNC

	GSQLSessionManager 	*ssmn;
	GtkToolItem			*dummy;
	GSQLColorHinter		*colorhint;
	GtkAction			*action;

	GSQLAppUI *appui;
	
	ssmn = g_object_new (GSQL_SSMN_TYPE, NULL);
	ssmn->private->dock = gdl_dock_new();
	
	ssmn->private->layout = gdl_dock_layout_new (GDL_DOCK (ssmn->private->dock));
	ssmn->private->sessionbar = gtk_toolbar_new ();

	gtk_box_pack_start(GTK_BOX (ssmn), ssmn->private->dock, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX (ssmn), ssmn->private->sessionbar, FALSE, FALSE, 0);

	dummy = gtk_tool_item_new ();
	gtk_tool_item_set_expand (dummy, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (ssmn->private->sessionbar), 
	    				dummy,
	    				-1);

	gtk_container_set_border_width (GTK_CONTAINER (ssmn->private->sessionbar), 1);
	
	colorhint = gsql_colorhinter_new ();
	ssmn->private->colorhint = GTK_WIDGET (colorhint);

	gtk_toolbar_insert (GTK_TOOLBAR (ssmn->private->sessionbar), 
	    				gtk_separator_tool_item_new (),	-1);
	
	gtk_toolbar_insert (GTK_TOOLBAR (ssmn->private->sessionbar), 
	    				GTK_TOOL_ITEM (colorhint), -1);
	
	gtk_widget_show_all (GTK_WIDGET (ssmn));

	appui = gsql_iface_get_ui (GSQL_IFACE (gsqlapp), NULL);

	gsql_appui_add_actions (appui, "ActionGroupSession",
	    					session_entries,
	    					G_N_ELEMENTS (session_entries), 
	    					NULL);
	
	action = gtk_recent_action_new ("ActionRecentSessions", N_("Recent sessions"),
	    								N_("Open a recent session"), NULL);
	
	gsql_appui_add_action (appui, "ActionGroupSession", GTK_ACTION (action), NULL);

	action = gtk_recent_action_new ("ActionActiveSessions", N_("Active sessions"),
	    								N_("Switch to the session"), NULL);
	
	gsql_appui_add_action (appui, "ActionGroupSession", GTK_ACTION (action), NULL);

	
	gsql_ssmn_recent_sessions_menu (GTK_RECENT_CHOOSER (action));
	
	gsql_ssmn_update_actions_status (ssmn);
	
	return ssmn;
}





void 
on_ssmn_new_session (GtkMenuItem *mi, gpointer data)
{
	GSQL_TRACE_FUNC

	GSQLSessionManager 	*ssmn = NULL;

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



