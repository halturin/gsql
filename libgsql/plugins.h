/***************************************************************************
 *            plugins.h
 *
 *  Sun Sep  9 02:37:35 2007
 *  Copyright  2007  Taras Halturin
 *  <halturin@gmail.com>
 ****************************************************************************/

/*
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
 
#ifndef _PLUGINS_H
#define _PLUGINS_H

#include <glib.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <libgsql/engines.h>

#define GSQL_PLUGIN_SYMBOL_LOAD   			"plugin_load"
#define GSQL_PLUGIN_SYMBOL_UNLOAD 			"plugin_unload"

typedef struct _GSQLPluginInfo GSQLPluginInfo;
typedef struct _GSQLPlugin GSQLPlugin;

struct _GSQLPluginInfo
{
	gchar * id;
	gchar * name;
	gchar * version;
	gchar * desc;
	gchar * author;
	gchar * homepage;
};


struct _GSQLPlugin
{
	struct _GSQLPluginInfo info;
	gchar   *file_logo; /* icon file */
	
	GModule *module;
	gchar   *file;
	
	GtkActionGroup *action;
	guint menu_id;

	gboolean	(*plugin_load) (GSQLPlugin * plugin);
	gboolean	(*plugin_unload) (GSQLPlugin * plugin);

	/* menu show/hide */
	void		(*menu_update_sensitive) (GSQLEngine *engine);
	/* prefs widget */
	void  		(*plugin_conf_dialog) ();
};

G_BEGIN_DECLS

void
gsql_plugins_lookup();

guint
gsql_plugins_count();

void
gsql_plugins_foreach (GHFunc func, gpointer userdata);

void
gsql_plugins_menu_update_all (GSQLEngine *engine);

G_END_DECLS

#endif /* _PLUGINS_H */

 
