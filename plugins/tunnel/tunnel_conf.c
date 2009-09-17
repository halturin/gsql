/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2009  Taras Halturin  halturin@gmail.com
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
#include <libgsql/conf.h>
#include <libgsql/common.h>
#include <tunnel_conf.h>

#define GSQLP_TUNNEL_GLADE_DIALOG PACKAGE_GLADE_DIR"/plugins/tunnel_config.xml"

void
plugin_tunnel_conf_dialog ()
{
	GSQL_TRACE_FUNC;

	GtkBuilder *bld;
	GtkDialog *dialog;
	GError *err = NULL;

	bld = gtk_builder_new();

	gtk_builder_add_from_file (bld, GSQLP_TUNNEL_GLADE_DIALOG, &err);

	dialog = GTK_DIALOG (gtk_builder_get_object (bld, "tunnel_config_dialog"));

	if (!dialog) 
		return;

	gtk_dialog_run (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));
}


