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

 
#include <glib.h>
#include <gtk/gtk.h>
#include <glib-object.h>


#define ENGINE_MAJOR_VER 0
#define ENGINE_MINOR_VER 1
#define ENGINE_VERSION "0.1"
#define ENGINE_ID    "engine_pgsql"
#define ENGINE_NAME  "PostgreSQL"
#define ENGINE_DESC  "GSQL module for PostgreSQL support"
#define ENGINE_AUTHOR "Taras Halturin"
#define ENGINE_HOMEPAGE "http://gsql.org"

#include <config.h>
#include <libgsql/engines.h>
#include <libgsql/common.h>
#include <libgsql/workspace.h>

gboolean
engine_load (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC

	engine->info.author = ENGINE_AUTHOR;
	engine->info.id = ENGINE_ID;
	engine->info.name = ENGINE_NAME;
	engine->info.desc = ENGINE_DESC;
	engine->info.homepage = ENGINE_HOMEPAGE;
	engine->info.version = ENGINE_VERSION;
	engine->info.major_version = ENGINE_MAJOR_VER;
	engine->info.minor_version = ENGINE_MINOR_VER;

	engine->file_logo = "pgsql/pgsql.png";
	
//	engine_menu_init (engine);
	
//	engine->engine_session_open = engine_session_open;
//	engine->engine_session_close = engine_session_close;
//	engine->engine_session_reopen = engine_session_reopen;
	/* menu show/hide */
//	engine->engine_menu_set_visible = engine_menu_set_visible;
	/* logon widget */
//	engine->engine_logon_widget_create = NULL;
//	engine->engine_logon_widget_free = NULL;
	/* prefs widget */
//	engine->engine_conf_widget_create = engine_conf_widget_create;
//	engine->engine_conf_widget_free = engine_conf_widget_free;
	
	add_pixmap_directory (PACKAGE_PIXMAPS_DIR "/pgsql");
//	engine_stock_init();
		
    return TRUE;
};

gboolean
engine_unload (GSQLEngine * engine)
{
	GSQL_TRACE_FUNC

	if (engine->in_use)
	{
		fprintf (stdout, "Engine unload failed. Still in use.\n");
		return FALSE;
	};

	return TRUE;
};


void
engine_workspace_init (GSQLWorkspace *workspace)
{
	GSQL_TRACE_FUNC

	return;
};





