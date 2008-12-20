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

 
#ifndef _PLUGIN_EXPORTER_H
#define _PLUGIN_EXPORTER_H

#include <glib.h>

#define GSQLP_EXPORTER_STOCK_ICON "gsql-plugin-exporter-icon"
#define GSQLP_EXPORTER_STOCK_LOGO "gsql-plugin-exporter-logo"

#define GSQLP_EXPORTER_GLADE_DIALOG PACKAGE_GLADE_DIR"/plugins/exporter_dialog.glade"

G_BEGIN_DECLS

void
exporter_export_to_plain_text (GSQLContent *content, gchar *file, 
							   gchar *encoding, gboolean headers);

void
exporter_export_to_csv (GSQLContent *content, gchar *file, 
						gchar *encoding, gboolean headers);

void
exporter_export_to_html (GSQLContent *content, gchar *file, 
						 gchar *encoding, gboolean headers);

G_END_DECLS

#endif /* _PLUGIN_EXPORTER_H */

 
