/***************************************************************************
 *            plugin_exporter.c
 *
 *  Mon Feb 25 00:09:43 2008
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

#include <libgsql/plugins.h>
#include <libgsql/stock.h>
#include <libgsql/sqleditor.h>
#include <gtk/gtk.h>
#include <libgsql/common.h>
#include "plugin_exporter.h"
#include "exportercb.h"

#define PLUGIN_VERSION "0.1"
#define PLUGIN_ID    "plugin_exporter"
#define PLUGIN_NAME  "Exporter"
#define PLUGIN_DESC  "Export result set to CSV, TXT formats"
#define PLUGIN_AUTHOR "Taras Halturin"
#define PLUGIN_HOMEPAGE "http://gsql.org"

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_EXPORTER_STOCK_ICON,	"export_result.png" },
	{ GSQLP_EXPORTER_STOCK_LOGO,	"exporter.png" }
};


static GtkActionEntry exporter_action[] = 
{
	{ "ActionExportResultSet", GSQLP_EXPORTER_STOCK_ICON, N_("Export result set"), NULL, N_("Export result set"), G_CALLBACK(on_open_export_dialog_activate) }
};

static gchar plugin_ui[] = 
"<ui>	"
"  <toolbar name=\"SQLEditorToolbarFetch\">  "
"  		<placeholder name=\"PHolderSQLEditorFetch\" >  "
"			<toolitem name=\"SQLEditorExport\"  action=\"ActionExportResultSet\" />	"
"		</placeholder>	"
"	</toolbar>			"
"</ui>	";


static GtkActionGroup *action;

gboolean 
plugin_load (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	plugin->info.author = PLUGIN_AUTHOR;
	plugin->info.id = PLUGIN_ID;
	plugin->info.name = PLUGIN_NAME;
	plugin->info.desc = PLUGIN_DESC;
	plugin->info.homepage = PLUGIN_HOMEPAGE;
	plugin->info.version = PLUGIN_VERSION;
	plugin->file_logo = "exporter.png";
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));

	action = gtk_action_group_new ("ActionsPluginExporter");
	gtk_action_group_add_actions (action, exporter_action, 
									G_N_ELEMENTS (exporter_action), NULL);
	gsql_editor_merge_actions (plugin_ui, action);	

	plugin->plugin_conf_dialog = NULL;
	plugin->menu_update_sensitive = NULL;
	
	return TRUE;
};

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
};

