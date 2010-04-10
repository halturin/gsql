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
#include <libgsql/gsql-appui.h>
#include <config.h>

#include "gsql-app.h"
#include "gsql-actions.h"

static gpointer parent_class = NULL;

struct _GSQLAppPrivate {
	GSQLAppUI *appui;

	GtkWidget *mainvbox;
	GtkWidget *mainmenu;
	GtkWidget *maintoolbar;
	GtkWidget *sessionbar;
	GtkWidget *statusbar;
	GtkWidget *sysmessages;

	gint	uim_id;
	
};

static GSQLStockIcon common_stock_icons[] = 
{
	{ GSQL_STOCK_ALL_SCHEMAS,			"all.schemas.png" },
	{ GSQL_STOCK_ARGUMENTS,				"arguments.png" },
	{ GSQL_STOCK_CLOSE,					"close.png" },
	{ GSQL_STOCK_COLUMNS,				"columns.png" },
	{ GSQL_STOCK_CONSTRAINT,			"constraints.png" },
	{ GSQL_STOCK_CONSTRAINT_P,			"constraints.primary.png" },
	{ GSQL_STOCK_CONSTRAINT_F,			"constraints.foreign.png" },
	{ GSQL_STOCK_CONSTRAINT_U,			"constraints.unique.png" },
	{ GSQL_STOCK_CONSTRAINT_C,			"constraints.check.png" },
	{ GSQL_STOCK_FIND,					"find.png" },
	{ GSQL_STOCK_FUNCTIONS,				"functions.png" },
	{ GSQL_STOCK_GSQL,					"gsql.png" },
	{ GSQL_STOCK_INDEXES,				"indexes.png" },
	{ GSQL_STOCK_LOGO,					"logo.png" },
	{ GSQL_STOCK_MOUTPUT,				"msg_dboutput.png" },
	{ GSQL_STOCK_MERROR,				"msg_error.png" },
	{ GSQL_STOCK_MNORMAL,				"msg_normal.png" },
	{ GSQL_STOCK_MNOTICE,				"msg_notice.png" },
	{ GSQL_STOCK_MWARNING,				"msg_warning.png" },
	{ GSQL_STOCK_MY_SCHEMA,				"my.schema.png" },
	{ GSQL_STOCK_OBJ_CLONE,				"object_clone.png" },
	{ GSQL_STOCK_OBJ_CODE,				"object_code.png" },
	{ GSQL_STOCK_OBJ_NEW,				"object_new.png" },
	{ GSQL_STOCK_OBJ_REVERT,			"object_revert.png" },
	{ GSQL_STOCK_OBJ_SAVE,				"object_save.png" },
	{ GSQL_STOCK_OBJ_SQL,				"object_sql.png" },
	{ GSQL_STOCK_PROCEDURES,			"procedures.png" },
	{ GSQL_STOCK_PRIVILEGES,			"privileges.png" },
	{ GSQL_STOCK_SEQUENCES,				"sequences.png" },
	{ GSQL_STOCK_SESSION_CLOSE,			"session_close.png" },
	{ GSQL_STOCK_SESSION_COMMIT,		"session_commit.png" },
	{ GSQL_STOCK_SESSION_NEW,			"session_new.png" },
	{ GSQL_STOCK_SESSION_ROLLBACK,		"session_rollback.png" },
	{ GSQL_STOCK_SQL_FETCH_ALL,			"sql_fetch_all.png" },
	{ GSQL_STOCK_SQL_FETCH_NEXT,		"sql_fetch_next.png" },
	{ GSQL_STOCK_SQL_RUN_AT_CURSOR,		"sql_run_at_cursor.png" },
	{ GSQL_STOCK_SQL_RUN,				"sql_run.png" },
	{ GSQL_STOCK_SQL_RUN_STEP,			"sql_run_step.png" },
	{ GSQL_STOCK_SQL_SHOW_HIDE,			"sql_showhide_result.png" },
	{ GSQL_STOCK_SQL_STOP_ON_ERR,		"sql_stop_onerror.png" },
	{ GSQL_STOCK_SQL_STOP,				"sql_stop.png" },
	{ GSQL_STOCK_SQL_UPDATEABLE,		"sql_updateable.png" },
	{ GSQL_STOCK_TABLES,				"tables.png" },
	{ GSQL_STOCK_TRIGGERS,				"triggers.png" },
	{ GSQL_STOCK_UNKNOWN,				"unknown.png" },
	{ GSQL_STOCK_USERS,					"users.png" },
	{ GSQL_STOCK_VIEWS,					"views.png" }
};


static void gsql_app_class_init (GSQLAppClass *class);
static void gsql_app_init (GSQLApp *app);

G_DEFINE_TYPE (GSQLApp, gsql_app, GTK_TYPE_WINDOW)

GtkWidget *
gsql_app_new (void)
{
	GSQL_TRACE_FUNC
	
	gint w,h,x,y;
	gboolean restore_xywh;
	gboolean maxz_state;
	GSQLApp *app;
	GdkPixbuf *icon;

	app = g_object_new (GSQL_APP_TYPE, NULL);

	gtk_window_set_title (GTK_WINDOW (app), "GSQL");

	restore_xywh = gsql_conf_value_get_boolean (GSQL_CONF_UI_RESTORE_SIZE_POS);

	if (restore_xywh)
	{
		w = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_X);
		h = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_Y);
	
		x = gsql_conf_value_get_int (GSQL_CONF_UI_POS_X);
		y = gsql_conf_value_get_int (GSQL_CONF_UI_POS_Y);

		GSQL_DEBUG ("x[%d] y[%d] w[%d] h[%d]", x, y, w, h);
		
		gtk_window_move (GTK_WINDOW (app), x, y);
		gtk_window_set_default_size (GTK_WINDOW (app), w, h);
		
		maxz_state = gsql_conf_value_get_boolean (GSQL_CONF_UI_MAXIMIZED);
		
		if (maxz_state)
			gtk_window_maximize (GTK_WINDOW (app));
		
	} else {
	
		gtk_window_set_default_size (GTK_WINDOW (app), 800, 700);
		gtk_window_set_position(GTK_WINDOW (app), GTK_WIN_POS_CENTER);
	}
GSQL_DEBUG ("1");
	app->private->mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (app), app->private->mainvbox);

	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupFile",
	    									menu_entries_file,
	    									G_N_ELEMENTS (menu_entries_file),
	    									NULL);
	
	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupEdit",
	    									menu_entries_edit,
	    									G_N_ELEMENTS (menu_entries_edit),
	    									NULL);

	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupView",
	    									menu_entries_view,
	    									G_N_ELEMENTS (menu_entries_view),
	    									NULL);

	gsql_appui_add_toggle_actions (app->private->appui,
	    							"ToggleActionView",
	    							menu_toggle_entries_view,
	    							G_N_ELEMENTS (menu_toggle_entries_view),
	    							NULL);

	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupSearch",
	    									menu_entries_search,
	    									G_N_ELEMENTS (menu_entries_search),
	    									NULL);

	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupSession",
	    									menu_entries_session,
	    									G_N_ELEMENTS (menu_entries_session),
	    									NULL);
	
	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupTools",
	    									menu_entries_tools,
	    									G_N_ELEMENTS (menu_entries_tools),
	    									NULL);

	gsql_appui_add_actions (app->private->appui,
	    									"ActionGroupHelp",
	    									menu_entries_help,
	    									G_N_ELEMENTS (menu_entries_help),
	    									NULL);
	
	app->private->uim_id = gsql_appui_merge (app->private->appui, 
	    										PACKAGE_UI_DIR "/gsql.ui");


GSQL_DEBUG ("2");
	app->private->mainmenu = gsql_appui_get_widget (app->private->appui,
	    											"/MenuMain");
	gtk_box_pack_start (GTK_BOX (app->private->mainvbox), 
	    				app->private->mainmenu, FALSE, FALSE, 0);
GSQL_DEBUG ("3");
	app->private->maintoolbar = gsql_appui_get_widget (app->private->appui,
	    											"/ToolbarMain");
	gtk_box_pack_start (GTK_BOX (app->private->mainvbox), 
	    				app->private->maintoolbar, FALSE, FALSE, 0);
GSQL_DEBUG ("4");
	app->private->statusbar = gtk_statusbar_new ();
	gtk_box_pack_end (GTK_BOX (app->private->mainvbox), 
	    				app->private->statusbar, FALSE, FALSE, 0);
GSQL_DEBUG ("5");
	app->private->sessionbar = gtk_toolbar_new ();
	gtk_box_pack_end (GTK_BOX (app->private->mainvbox), 
	    				app->private->sessionbar, FALSE, FALSE, 0);
GSQL_DEBUG ("6");	
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (app->private->statusbar), FALSE);
GSQL_DEBUG ("7");	
	app->private->sysmessages = gtk_combo_box_entry_new();//gtk_combo_box_new_text ();
	gtk_box_pack_end (GTK_BOX (app->private->statusbar), 
	    			app->private->sysmessages, FALSE, FALSE, 0);
GSQL_DEBUG ("8");
	gtk_widget_set_size_request (app->private->sysmessages, 400, 26);
	
	gtk_widget_show_all (app->private->mainvbox);

	icon = gsql_create_pixbuf ("gsql.png");
	gtk_window_set_icon (GTK_WINDOW (app), icon);
	gdk_pixbuf_unref (icon);

	return GTK_WIDGET (app);
}

static void
gsql_app_dispose (GObject *object)
{
	GSQLApp *app;

	g_return_if_fail (GSQL_IS_APP (object));

	app = GSQL_APP (object);

	// free app->{something}
	
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gsql_app_finalize (GObject *object)
{
	GSQLApp *app;

	g_return_if_fail (GSQL_IS_APP (object));

	app = GSQL_APP (object);

	g_free (app->private);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
gsql_app_class_init (GSQLAppClass *class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	object_class->finalize = gsql_app_finalize;
	object_class->dispose = gsql_app_dispose;
}

static void 
gsql_app_init (GSQLApp *app)
{

	g_return_if_fail (GSQL_IS_APP (app));

	app->private = g_new0 (GSQLAppPrivate, 1);

	app->private->appui = gsql_appui_new ();

	gsql_add_pixmap_directory (PACKAGE_PIXMAPS_DIR);
	
	gsql_appui_factory_add (app->private->appui, common_stock_icons, 
	    						G_N_ELEMENTS (common_stock_icons));

	gsql_conf_init ();
}

