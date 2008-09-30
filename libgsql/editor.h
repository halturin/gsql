/***************************************************************************
 *            gsqleditor.h
 *
 *  Thu Nov  9 21:51:30 2006
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

#ifndef _GSQLEDITOR_H
#define _GSQLEDITOR_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <libgsql/conf.h>

#define GSQL_CONF_EDITOR_USE_SYSTEM_FONT	GSQL_CONF_ROOT_KEY "/editor/use_system_font"
#define GSQL_CONF_EDITOR_FONT_NAME			GSQL_CONF_ROOT_KEY "/editor/font_name"
#define GSQL_CONF_EDITOR_USE_SPACE			GSQL_CONF_ROOT_KEY "/editor/use_space_instead_tab"
#define GSQL_CONF_EDITOR_TAB_WIDTH			GSQL_CONF_ROOT_KEY "/editor/tab_width"
#define GSQL_CONF_EDITOR_SHOW_LINE_NUM		GSQL_CONF_ROOT_KEY "/editor/show_line_number"
#define GSQL_CONF_EDITOR_HIGHLIGHT_LINE		GSQL_CONF_ROOT_KEY "/editor/highlight_line"
#define GSQL_CONF_EDITOR_WRAPPING			GSQL_CONF_ROOT_KEY "/editor/enable_text_wrapping"
#define GSQL_CONF_EDITOR_AUTO_INDENT		GSQL_CONF_ROOT_KEY "/editor/enable_auto_indent"

#define GNOME_SYSTEM_FONT       "/desktop/gnome/interface/monospace_font_name"


// default fetch rows limit 
#define SQL_EDITOR_CUSTOM_FETCH_LIMIT	100

#define GSQL_EDITOR_MARKER_COMPLETE "complete"
#define GSQL_EDITOR_MARKER_FAILED "error"
GtkWidget *
gsql_source_editor_new(gchar * buffer_body);

void 
gsql_source_editor_marker_set (GtkTextIter *iter, gchar *marker_type);

#endif /* _GSQLEDITOR_H */
