/***************************************************************************
 *            engines.h
 *
 *  Mon Nov  6 21:56:26 2006
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
 
#ifndef _ENGINES_H
#define _ENGINES_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gmodule.h>


#define GSQL_ENGINE_SYMBOL_LOAD   			"engine_load"
#define GSQL_ENGINE_SYMBOL_UNLOAD 			"engine_unload"

#define GNOME_SYSTEM_FONT       "/desktop/gnome/interface/monospace_font_name"

typedef struct _GSQLEngineInfo GSQLEngineInfo;
typedef struct _GSQLEngine GSQLEngine;

#include <libgsql/cursor.h>

struct _GSQLEngineInfo
{
	gint major_version;
	gint minor_version;
	gchar * id;
	gchar * name;
	gchar * version;
	gchar * desc;
	gchar * author;
	gchar * homepage;
};


struct _GSQLEngine
{
	GSQLEngineInfo info;
	gchar   *file_logo; /* icon file */
	gchar   *stock_logo; /* stock name of logo */
	
	GModule *module;
	gchar   *file;
	
	GtkActionGroup *action;
	guint menu_id;

	gboolean	multi_statement; /* multistatement support. default value FALSE */
	gboolean	in_use;

	gboolean	(*load) (GSQLEngine * engine);
	gboolean	(*unload) (GSQLEngine * engine);

	gpointer	(*session_open) (GtkWidget *logon_widget, gchar *buffer);
	
	/* logon widget */
	GtkWidget  *(*logon_widget_new) ();
	void 		(*logon_widget_free) (GtkWidget * prefs_widget);
	
	/* prefs widget */
	GtkWidget  *(*conf_widget_new) ();
	void		(*conf_widget_free) (GtkWidget * prefs_widget);
	
	/* cursor routines */
	GSQLCursorState		(*cursor_open_with_bind)  (GSQLCursor *cursor, GList *args);
	GSQLCursorState		(*cursor_open_with_bind_by_name)  (GSQLCursor *cursor, GList *args);
	GSQLCursorState		(*cursor_open) (GSQLCursor *cursor);
	GSQLCursorState		(*cursor_stop) (GSQLCursor *cursor);
	gint				(*cursor_fetch) (GSQLCursor *cursor, gint rows);
	
};


G_BEGIN_DECLS

void
gsql_engines_lookup ();

guint
gsql_engines_count();

void
gsql_engines_foreach (GHFunc func, gpointer userdata);

void
gsql_engine_menu_set_status (GSQLEngine *engine,
								gboolean status);

G_END_DECLS

#endif /* _ENGINES_H */

 
