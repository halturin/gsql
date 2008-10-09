/***************************************************************************
 *            plugins.c
 *
 *  Sun Sep  9 02:37:23 2007
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


#include <config.h>
#include <libgsql/plugins.h>
#include <libgsql/common.h>
#include <gmodule.h>

#define GSQL_PLUGINS_MASK "libgsqlplugin"
#define GSQL_PLUGINS_EXT  ".so"

static GHashTable * hash_plugins = NULL;

static gboolean
gsql_plugin_load_symbols (GSQLPlugin *plugin);

static GSQLPlugin *
gsql_plugin_new (gchar * plugin_file);

void
gsql_plugins_lookup()
{
	GSQL_TRACE_FUNC;
		
	const gchar *file_name;
	GDir * dir;
	gchar * plugin_file;
	glong len;
	GSQLPlugin *plugin;
        
	dir = g_dir_open (PACKAGE_PLUGINS_DIR, 0, NULL);
	
	if (dir == NULL)
	{
		g_critical ("Couldn't open plugins directory: %s", PACKAGE_PLUGINS_DIR);
		return;
	}
	
	if (hash_plugins == NULL)
		hash_plugins = g_hash_table_new (g_str_hash, g_str_equal);
	
	do 
	{
		file_name = g_dir_read_name (dir);  
		
		if (file_name == NULL)
			break;
        		
        		// check the file extention
		len = g_utf8_strlen (file_name, 128) - g_utf8_strlen (GSQL_PLUGINS_EXT, 4);
		
		if (strncmp (file_name+len, GSQL_PLUGINS_EXT, g_utf8_strlen (GSQL_PLUGINS_EXT, 4)) )
			continue;
        		// is plugin file
		if (strncmp (file_name, GSQL_PLUGINS_MASK, sizeof(GSQL_PLUGINS_MASK)-1) == 0)
		{
			plugin_file = g_strconcat (PACKAGE_PLUGINS_DIR,"/", 
										file_name, NULL);
			
			GSQL_DEBUG ("Try to load the plugin: %s", plugin_file);
			
			if ((plugin = gsql_plugin_new (plugin_file)) == NULL)
			{
				g_critical ("Couldn't load plugin: %s", g_module_error() );
				g_free (plugin_file);
				
				continue;
			}
		}
		
	} while (file_name != NULL);

}

static GSQLPlugin *
gsql_plugin_new (gchar * plugin_file)
{
	GSQL_TRACE_FUNC;
		
	GSQLPlugin *plugin, *plugin1;
	GModule *module;

	module = g_module_open (plugin_file, G_MODULE_BIND_LOCAL);
	
	if (module == NULL)
		return NULL;

		// first step seems fine
	plugin = g_malloc0 (sizeof (GSQLPlugin));
	plugin->module = module;
	plugin->file = plugin_file;
		
	if (!gsql_plugin_load_symbols (plugin))
	{
		g_critical ("Plugin module error: the loading required symbols is failed");
		g_free (plugin);
		g_module_close (module);
		
		return NULL;
	}

	if (!plugin->plugin_load (plugin))
	{
		g_free (plugin);
		g_critical ("Plugin module error: [%s] internal initialization failed", plugin_file);
		g_module_close (module);
		
		return NULL;
	}

	if ((plugin1 = g_hash_table_lookup (hash_plugins, plugin->info.id)) == NULL)
	{
		g_hash_table_insert (hash_plugins, plugin->info.id, plugin);
	}
	else
	{
		g_critical ("Plugin module warning: the module with id \"%s\" is already loaded\n"
						 "            your file: %s\n"
				         "          module name: %s\n"
				         "          description: %s\n"
				         "              version: %s\n"
				         "               author: %s\n"
				         "     loaded file name: %s\n"
				         "          module name: %s\n"
				         "          description: %s\n"
				         "              version: %s\n"
				         "               author: %s",
				         plugin1->info.id,
				         plugin->file,
				         plugin->info.name,
				         plugin->info.desc,
				         plugin->info.version,
				         plugin->info.author,
				         plugin1->file,
				         plugin1->info.name,
				         plugin1->info.desc,
				         plugin1->info.version,
				         plugin1->info.author);
				         
				         plugin->plugin_unload (plugin);
				         g_free (plugin);
				         g_module_close (module);
		return NULL;
	}

	return plugin;
}


static gboolean
gsql_plugin_load_symbols (GSQLPlugin *plugin)
{
	GSQL_TRACE_FUNC;
		
// linking required symbols
	if ((!g_module_symbol (plugin->module, GSQL_PLUGIN_SYMBOL_LOAD, 
						      (gpointer) &plugin->plugin_load)) ||
		(!g_module_symbol (plugin->module, GSQL_PLUGIN_SYMBOL_UNLOAD,
						      (gpointer) &plugin->plugin_unload))
		) 
			return FALSE;

	return TRUE;		
}

guint
gsql_plugins_count()
{
	GSQL_TRACE_FUNC;
		
	g_return_val_if_fail (hash_plugins != NULL, 0);
	
	return g_hash_table_size (hash_plugins);
}

void
gsql_plugins_foreach (GHFunc func, gpointer userdata)
{
	GSQL_TRACE_FUNC;
		
	if (!gsql_plugins_count())
		return;
	
	g_hash_table_foreach (hash_plugins, 
							func,
							userdata);
}

static void
gsql_plugins_menu_update (gpointer key,
						gpointer value,
						gpointer userdata)
{
	GSQL_TRACE_FUNC;
		
	GSQLPlugin *plugin = value;
	
	if (plugin->menu_update_sensitive)
		plugin->menu_update_sensitive (userdata);

}


void
gsql_plugins_menu_disable_all()
{
	GSQL_TRACE_FUNC;
		
	gsql_plugins_foreach (gsql_plugins_menu_update, NULL);
}

void
gsql_plugins_menu_update_all (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC;
		
	gsql_plugins_foreach (gsql_plugins_menu_update, engine);
}
