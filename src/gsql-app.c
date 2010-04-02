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
#include "gsql-app.h"

static gpointer parent_class = NULL;

struct _GSQLAppPrivate {

	gint temp;

};


static void gsql_app_class_init (GSQLAppClass *class);
static void gsql_app_init (GSQLApp *app);

G_DEFINE_TYPE (GSQLApp, gsql_app, GTK_TYPE_WINDOW)

GtkWidget *
gsql_app_new (void)
{
	/*gint w,h,x,y;
	gboolean restore_xywh;
	gboolean maxz_state;*/
	GtkWindow *app;

	app = g_object_new (GSQL_APP_TYPE, NULL);

	gtk_window_set_title (app, "GSQL");

	/*restore_xywh = gsql_conf_value_get_boolean (GSQL_CONF_UI_RESTORE_SIZE_POS);

	if (restore_xywh)
	{
		w = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_X);
		h = gsql_conf_value_get_int (GSQL_CONF_UI_SIZE_Y);
	
		x = gsql_conf_value_get_int (GSQL_CONF_UI_POS_X);
		y = gsql_conf_value_get_int (GSQL_CONF_UI_POS_Y);

		GSQL_DEBUG ("x[%d] y[%d] w[%d] h[%d]", x, y, w, h);
		
		gtk_window_move (GTK_WINDOW (gsql_window), x, y);
		gtk_window_set_default_size (GTK_WINDOW (gsql_window), w, h);
		
		maxz_state = gsql_conf_value_get_boolean (GSQL_CONF_UI_MAXIMIZED);
		
		if (maxz_state)
			gtk_window_maximize (GTK_WINDOW (gsql_window));
		
	} else {
	
		gtk_window_set_default_size (GTK_WINDOW (gsql_window), 800, 700);
		gtk_window_set_position(GTK_WINDOW (gsql_window), GTK_WIN_POS_CENTER);
	}
	
	gsql_window_icon_pixbuf = create_pixbuf ("gsql.png");


	*/
















	/*add_pixmap_directory (PACKAGE_PIXMAPS_DIR);
	add_pixmap_directory (PACKAGE_PIXMAPS_DIR "/plugins");
	gsql_stock_init ();
	
	gsql_conf_init ();
	gsql_window_create ();
	
	gsql_engines_lookup ();
	gsql_plugins_lookup ();

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
	}*/

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

	// free app->{something}
	
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


}

