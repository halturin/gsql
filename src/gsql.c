/***************************************************************************
 *            gsql.c
 *
 *  Tue Jul 25 08:42:58 2006
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

#include <libgnomeui/libgnomeui.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "gsqlcb.h"
#include "gsql.h"
#include "gsqlmenu.h"
#include <libgsql/common.h>
#include <libgsql/engines.h>
#include <libgsql/menu.h>
#include "gsqlconf.h"

extern GtkWidget *gsql_window;
extern GtkWidget *statusbar;

static void
set_non_homogeneus (GtkWidget *widget, gpointer data)
{
	gtk_tool_item_set_homogeneous (GTK_TOOL_ITEM (widget), FALSE);
}

void
gsql_window_create (void)
{
	GSQL_TRACE_FUNC;

	GdkPixbuf *gsql_window_icon_pixbuf;
	GtkWidget * mainvbox;
	GtkWidget * gsqlmenu;	
	GtkWidget * notebook;
	GtkWidget *toolbar;

	gint w,h,x,y;
	gboolean restore_xywh;
       
	gsql_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	HOOKUP_OBJECT_NO_REF (gsql_window, gsql_window, "gsql_window");
	
	gtk_window_set_title (GTK_WINDOW (gsql_window), "GSQL");
	restore_xywh = gsql_conf_value_get_boolean (GSQL_CONF_UI_RESTORE_SIZE_POS);
	
	if (restore_xywh)
	{
		w = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_X);
		h = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_Y);
	
		x = gsql_conf_value_get_int (GSQL_CONF_UI_POS_X);
		y = gsql_conf_value_get_int (GSQL_CONF_UI_POS_Y);

		GSQL_DEBUG ("x[%d] y[%d] w[%d] h[%d]", x, y, w, h);
		
		gtk_window_move (GTK_WINDOW (gsql_window), x, y);
		gtk_window_set_default_size (GTK_WINDOW (gsql_window), w, h);
		
	} else {
	
		gtk_window_set_default_size (GTK_WINDOW (gsql_window), 800, 700);
		gtk_window_set_position(GTK_WINDOW (gsql_window), GTK_WIN_POS_CENTER);
	}
	
	gsql_window_icon_pixbuf = create_pixbuf ("gsql.png");
	
	if (gsql_window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (gsql_window), gsql_window_icon_pixbuf);
		gdk_pixbuf_unref (gsql_window_icon_pixbuf);
	}
        
	mainvbox = gtk_vbox_new (FALSE, 0);
	HOOKUP_OBJECT_NO_REF (gsql_window, mainvbox, "mainvbox");
	
	gtk_widget_show (mainvbox);
	gtk_container_add (GTK_CONTAINER (gsql_window), mainvbox);
	
	gsqlmenu = gsql_main_menu_init();
	HOOKUP_OBJECT_NO_REF (gsql_window, gsqlmenu, "gsqlmenu");
	gtk_box_pack_start (GTK_BOX (mainvbox), gsqlmenu, FALSE, FALSE, 0);
	
	toolbar = gsql_menu_get_widget("/ToolbarMain");
	gtk_box_pack_start (GTK_BOX (mainvbox), toolbar, FALSE, FALSE, 0);
	
	gtk_container_foreach (GTK_CONTAINER (toolbar),
						   (GtkCallback) set_non_homogeneus, NULL);
               
	notebook = gtk_notebook_new ();
	
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), TRUE);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_RIGHT);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);  
	
	gtk_widget_show(notebook);  
	
	HOOKUP_OBJECT (gsql_window, notebook, "sessions");
	gtk_box_pack_start (GTK_BOX (mainvbox), notebook, TRUE, TRUE, 0);
	
	statusbar = gtk_statusbar_new ();
	
	// FIXME: statusbar routines
	gtk_box_pack_start (GTK_BOX (mainvbox), statusbar, FALSE, FALSE, 0);
	gtk_widget_show (statusbar);
	
	g_signal_connect_after ((gpointer) notebook, "switch-page",
						G_CALLBACK (on_sessions_notebook_change_current_page),
						NULL);
	
	g_signal_connect (G_OBJECT (gsql_window), "delete_event",
						G_CALLBACK (on_gsql_window_delete), 
					  NULL);
	g_signal_connect (G_OBJECT (gsql_window), "destroy_event",
						G_CALLBACK (on_gsql_window_destroy), 
					  NULL);

}

void
gsql_window_clean_exit()
{
	GSQL_TRACE_FUNC;
	
	gint w,h,x,y;
	gboolean restore_xywh;
	
	if (!gsql_session_close_all ())
		return;
	
    restore_xywh = gsql_conf_value_get_boolean (GSQL_CONF_UI_RESTORE_SIZE_POS);
	
	if (restore_xywh)
	{
		
		gtk_window_get_position (GTK_WINDOW (gsql_window), &x, &y);
		
		gsql_conf_value_set_int (GSQL_CONF_UI_POS_X, x);
		gsql_conf_value_set_int (GSQL_CONF_UI_POS_Y, y);
	
		gtk_window_get_size (GTK_WINDOW (gsql_window), &w, &h);
		
		gsql_conf_value_set_int (GSQL_CONF_UI_SIZE_X, w);
		gsql_conf_value_set_int (GSQL_CONF_UI_SIZE_Y, h);
		
	}
	
	gnome_accelerators_sync ();
	gtk_main_quit ();
	
}

void
gsql_dialog_logon_make_widget (gpointer key, 
								gpointer value,
								gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GSQLEngine * engine = (GSQLEngine *) value;
	GtkWidget * vbox = (GtkWidget *) user_data;
	GtkWidget * engine_name;
	GtkWidget * notebook;
	GtkWidget * logon_widget;
	GtkTreeIter engine_iter;
	GtkListStore *model;
		
	engine_name = g_object_get_data (G_OBJECT (vbox), "engine_name");
	notebook = g_object_get_data (G_OBJECT (vbox), "notebook");
	model = (GtkListStore *) g_object_get_data (G_OBJECT (vbox), "model");
	
	g_return_if_fail (engine->logon_widget_new != NULL);

	logon_widget = engine->logon_widget_new ();
	
	if (logon_widget == NULL)
	{
		fprintf (stderr, "Engine module error: [%s] failed to create logon widget\n",
					engine->info.id);
		return;
	}
		
	HOOKUP_OBJECT_NO_REF (G_OBJECT (logon_widget), engine, "engine");

	gtk_list_store_append(GTK_LIST_STORE(model), &engine_iter);
	gtk_list_store_set( GTK_LIST_STORE(model), &engine_iter,
							0,			create_pixbuf (engine->file_logo),
							1, 			engine->info.name,
							-1);
	
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), logon_widget, NULL);

}

GtkWidget*
create_dialog_logon (void)
{
	GSQL_TRACE_FUNC;

	GtkWidget *dialog;
	GtkWidget *dialog_logon;
	GtkWidget *dialog_vbox;
	GtkWidget *engine_name;
	GtkWidget *hbox1;
	GtkWidget *label;
	GtkWidget *notebook;
	GtkListStore *model;
	
	GtkCellRenderer *cell;
        
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
		
		return NULL;
	}
		
	dialog_logon = gtk_dialog_new_with_buttons (_("Database logon"),
												GTK_WINDOW (gsql_window),
												GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_STOCK_CANCEL,
												GTK_RESPONSE_CANCEL,
												GTK_STOCK_OK,
												GTK_RESPONSE_OK,
												NULL); 
	
	gtk_window_set_resizable (GTK_WINDOW (dialog_logon), FALSE);
        
	dialog_vbox = GTK_DIALOG (dialog_logon)->vbox;
	gtk_widget_show (dialog_vbox);
        
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox1, FALSE, TRUE, 0);

	label = gtk_label_new (_("Choose the engine:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label), 8, 0);

	engine_name = gtk_combo_box_new ();
	gtk_widget_show (engine_name);
	gtk_box_pack_start (GTK_BOX (hbox1), engine_name, TRUE, TRUE, 0);
	g_signal_connect ((gpointer) engine_name, "changed",
						G_CALLBACK (on_dialog_logon_engine_name_changed),
						dialog_vbox);
	
	HOOKUP_OBJECT (G_OBJECT (dialog_vbox), engine_name, "engine_name");

	cell = gtk_cell_renderer_pixbuf_new ();
	
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (engine_name),
                                    cell, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (engine_name),
                                        cell, "pixbuf", 0, NULL);	
	
    cell = gtk_cell_renderer_text_new ();
    
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (engine_name),
                                    cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (engine_name),
                                        cell, "markup", 1, NULL);
	model = gtk_list_store_new (2, 
								GDK_TYPE_PIXBUF,
								G_TYPE_STRING);
	
	gtk_combo_box_set_model (GTK_COMBO_BOX (engine_name), GTK_TREE_MODEL (model));
	HOOKUP_OBJECT_NO_REF (G_OBJECT (dialog_vbox), model, "model");
		
	notebook = gtk_notebook_new ();
	gtk_widget_show (notebook);
	
	gtk_box_pack_start (GTK_BOX (dialog_vbox), notebook, TRUE, TRUE, 0);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
	
	HOOKUP_OBJECT (G_OBJECT (dialog_vbox), notebook, "notebook");

	gsql_engines_foreach (gsql_dialog_logon_make_widget, dialog_vbox);
	
	gtk_combo_box_set_active (GTK_COMBO_BOX (engine_name), 0);        

	return dialog_logon;
}

GtkDialog*
create_dialog_close_session (void)
{
	GSQL_TRACE_FUNC;

	GtkDialog *dialog_close_session;
	GtkWidget *dialog_vbox2;
	GtkWidget *scrolledwindow;
	GtkWidget *tv_sessions;
	GtkWidget *dialog_action_area2;
	GtkWidget *cancel_button;
	GtkWidget *ok_button;
	GtkWidget *select_all_button;

	dialog_close_session = (GtkDialog *) gtk_dialog_new ();
	
	gtk_widget_set_size_request (GTK_WIDGET(dialog_close_session), 350, 250);
	gtk_window_set_title (GTK_WINDOW (dialog_close_session), _("Close session(s)"));
	gtk_window_set_position (GTK_WINDOW (dialog_close_session), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (dialog_close_session), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (dialog_close_session), FALSE);
	gtk_window_set_type_hint (GTK_WINDOW (dialog_close_session), GDK_WINDOW_TYPE_HINT_DIALOG);

	dialog_vbox2 = GTK_DIALOG (dialog_close_session)->vbox;
	gtk_widget_show (dialog_vbox2);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow);
	gtk_box_pack_start (GTK_BOX (dialog_vbox2), scrolledwindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	tv_sessions = gtk_tree_view_new ();
	gtk_widget_show (tv_sessions);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), tv_sessions);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tv_sessions), FALSE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tv_sessions), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_sessions), FALSE);

	dialog_action_area2 = GTK_DIALOG (dialog_close_session)->action_area;
	gtk_widget_show (dialog_action_area2);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

	select_all_button = gtk_button_new_with_mnemonic (_("Select all"));
	gtk_widget_show (select_all_button);  
	gtk_container_add(GTK_CONTAINER(dialog_close_session->action_area),select_all_button);
	GTK_WIDGET_SET_FLAGS (select_all_button, GTK_CAN_DEFAULT);
	g_signal_connect ((gpointer) select_all_button, "clicked",
						G_CALLBACK (on_dialog_close_session_select_all_button_activate),
						dialog_close_session);

	ok_button = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (ok_button);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog_close_session), ok_button, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);

	cancel_button = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancel_button);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog_close_session), cancel_button, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);        

	HOOKUP_OBJECT_NO_REF (dialog_close_session, dialog_close_session, "dialog_close_session");
	HOOKUP_OBJECT_NO_REF (dialog_close_session, dialog_vbox2, "dialog_vbox2");
	HOOKUP_OBJECT (dialog_close_session, scrolledwindow, "scrolledwindow");
	HOOKUP_OBJECT (dialog_close_session, tv_sessions, "tv_sessions");
	HOOKUP_OBJECT_NO_REF (dialog_close_session, dialog_action_area2, "dialog_action_area2");
	HOOKUP_OBJECT (dialog_close_session, cancel_button, "cancel_button");
	HOOKUP_OBJECT (dialog_close_session, ok_button, "ok_button");

	return dialog_close_session;
}
