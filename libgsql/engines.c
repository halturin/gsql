/***************************************************************************
 *            engines.c
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
 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <gmodule.h>
#include <libgsql/common.h>
#include <libgsql/engines.h>

#define GSQL_ENGINES_MASK "libgsqlengine"
#define GSQL_ENGINES_EXT  ".so"

static GHashTable * hash_engines = NULL;

static gboolean
gsql_engine_load_symbols (GSQLEngine * engine);

static GSQLEngine *
gsql_engine_new (gchar * engine_file);

/**
 * gsql_engines_lookup
 * 
 * Looking up available engines at PACKAGE_ENGINES_DIR 
 *
 * Returns:
 */
void
gsql_engines_lookup ()
{
	GSQL_TRACE_FUNC;
		
	const gchar *file_name;
	GDir * dir;
	gchar * engine_file;
	glong len;
	GSQLEngine * engine;
        
	dir = g_dir_open (PACKAGE_ENGINES_DIR, 0, NULL);
	
	if (dir == NULL)
	{
		g_critical ("Couldn't open engines directory: %s", PACKAGE_ENGINES_DIR);
		return;
	}
	
	if (hash_engines == NULL)
		hash_engines = g_hash_table_new (g_str_hash, g_str_equal);
	
	do 
	{
		file_name = g_dir_read_name (dir);
		
		if (file_name == NULL)
			break;
        		
        		// check the file extention
		len = g_utf8_strlen (file_name, 128) - g_utf8_strlen (GSQL_ENGINES_EXT, 4);
		
		if (strncmp (file_name+len, GSQL_ENGINES_EXT, g_utf8_strlen (GSQL_ENGINES_EXT, 4)) )
			continue;
		
        		// is engine file
		if (strncmp (file_name, GSQL_ENGINES_MASK, sizeof(GSQL_ENGINES_MASK)-1) == 0)
		{
			engine_file = g_strconcat (PACKAGE_ENGINES_DIR,"/", 
										file_name, NULL);
			GSQL_DEBUG ("Try to load the engine: %s", engine_file);
			
			if ((engine = gsql_engine_new (engine_file)) == NULL)
			{
				g_critical ("Couldn't load engine: %s",
							g_module_error() );
				g_free (engine_file);
				continue;
			}
		}
        		
	} while (file_name != NULL);     
        
}

/**
 * gsql_engine_new
 * @engine_file: file name
 *
 * There is short description
 *
 * Returns: #GSQLEngine struct
 */

static GSQLEngine *
gsql_engine_new (gchar * engine_file)
{
	GSQL_TRACE_FUNC;
		
	GSQLEngine *engine, *engine1;
	GModule *module;
		
	module = g_module_open (engine_file, G_MODULE_BIND_LOCAL);
	
	if (module == NULL)
		return NULL;

		// first step seems fine
	engine = g_malloc0 (sizeof (GSQLEngine));
	engine->module = module;
	engine->file = engine_file;
	engine->in_use = FALSE;		
	engine->multi_statement = FALSE;
	
	if (!gsql_engine_load_symbols (engine))
	{
		g_critical ("Engine module error: the loading required symbols is failed");
		g_free (engine);
		g_module_close (module);
		
		return NULL;
	}
		
	if (!engine->load (engine))
	{
		g_free (engine);
		g_critical ("Engine module error: [%s] internal initialization failed", engine_file);
		g_module_close (module);
		
		return NULL;
	}
		
	if ((engine1 = g_hash_table_lookup (hash_engines, engine->info.id)) == NULL)
	{
		g_hash_table_insert (hash_engines, engine->info.id, engine);
	}
	else
	{
		g_critical ("Engine module warning: the module with id \"%s\" is already loaded\n"
						 "=========== Your file: %s\n"
				         "          module name: %s\n"
				         "          description: %s\n"
				         "              version: %s\n"
				         "               author: %s\n"
				         "==== Loaded file name: %s\n"
				         "          module name: %s\n"
				         "          description: %s\n"
				         "              version: %s\n"
				         "               author: %s",
				         engine1->info.id,
				         engine->file,
				         engine->info.name,
				         engine->info.desc,
				         engine->info.version,
				         engine->info.author,
				         engine1->file,
				         engine1->info.name,
				         engine1->info.desc,
				         engine1->info.version,
				         engine1->info.author);
				         
				         engine->unload (engine);
				         g_free (engine);
				         g_module_close (module);
		return NULL;
	}
		
	return engine;
}

static gboolean
gsql_engine_load_symbols (GSQLEngine * engine)
{
	GSQL_TRACE_FUNC;
		
// linking required symbols
	if ((!g_module_symbol (engine->module, GSQL_ENGINE_SYMBOL_LOAD, 
						      (gpointer) &engine->load)) ||
		(!g_module_symbol (engine->module, GSQL_ENGINE_SYMBOL_UNLOAD,
						      (gpointer) &engine->unload))
		) 
			return FALSE;

	return TRUE;		
}


guint
gsql_engines_count()
{
	GSQL_TRACE_FUNC;
		
	g_return_val_if_fail (hash_engines != NULL, 0);
	
	return g_hash_table_size (hash_engines);
}

void
gsql_engines_foreach (GHFunc func, gpointer userdata)
{
	GSQL_TRACE_FUNC;
		
	if (!gsql_engines_count())
		return;
	
	g_hash_table_foreach (hash_engines, 
							func,
							userdata);
}


void
gsql_engine_menu_set_status (GSQLEngine *engine,
								gboolean status)
{
	GSQL_TRACE_FUNC;
		
	g_object_set(G_OBJECT(engine->action), "visible", status, NULL);
}

