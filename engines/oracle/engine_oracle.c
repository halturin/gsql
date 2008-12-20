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
#include <stdlib.h>


#define ENGINE_MAJOR_VER 0
#define ENGINE_MINOR_VER 1
#define ENGINE_VERSION "0.1"
#define ENGINE_ID    "engine_oracle"
#define ENGINE_NAME  "Oracle"
#define ENGINE_DESC  "GSQL module for Oracle support"
#define ENGINE_AUTHOR "Taras Halturin"
#define ENGINE_HOMEPAGE "http://gsql.org"

#define LOGON_GLADE PACKAGE_GLADE_DIR"/oracle/oracle_logon_widget.glade"

#include <config.h>
#include <libgsql/engines.h>
#include <libgsql/common.h>
#include <libgsql/workspace.h>

#include "engine_menu.h"
#include "engine_conf.h"
#include "engine_stock.h"
#include "engine_session.h"
#include "oracle_cursor.h"

GtkWidget *
engine_logon_widget_create ();
void
engine_logon_widget_free (GtkWidget * logon_widget);
GtkWidget *
engine_prefs_widget_create ();
void
engine_prefs_widget_free (GtkWidget * prefs_widget);

static GtkListStore *
engine_oracle_get_tns_aliases ();

gboolean
engine_load (GSQLEngine *engine)
{
	GSQL_TRACE_FUNC;
	gchar *env = NULL;
	gboolean use_sys_env = TRUE;
	gchar *env_all, *env_name, *env_value;
	gchar **env_list;
	gint i;

	engine->info.author = ENGINE_AUTHOR;
	engine->info.id = ENGINE_ID;
	engine->info.name = ENGINE_NAME;
	engine->info.desc = ENGINE_DESC;
	engine->info.homepage = ENGINE_HOMEPAGE;
	engine->info.version = ENGINE_VERSION;
	engine->info.major_version = ENGINE_MAJOR_VER;
	engine->info.minor_version = ENGINE_MINOR_VER;

	engine->file_logo = "oracle/oracle.png";
	engine->stock_logo = GSQLE_ORACLE_STOCK_ORACLE;
	
	engine_menu_init (engine);
	
	engine->session_open = engine_session_open;
	
	/* logon widget */
	engine->logon_widget_new = engine_logon_widget_create;
	engine->logon_widget_free = engine_logon_widget_free;
	/* prefs widget */
	engine->conf_widget_new = engine_conf_widget_create;
	engine->conf_widget_free = engine_conf_widget_free;
	
	/* cursor */
	engine->cursor_open_with_bind = oracle_cursor_open_bind;
	engine->cursor_open_with_bind_by_name = oracle_cursor_open_bind_by_name;
	engine->cursor_open = oracle_cursor_open;
	engine->cursor_stop = oracle_cursor_stop;
	engine->cursor_fetch = oracle_cursor_fetch;
	
	engine->multi_statement = TRUE;
	
	add_pixmap_directory (PACKAGE_PIXMAPS_DIR "/oracle");
	engine_stock_init();
	
	// set default NLS
	putenv ("NLS_LANG=AMERICAN_AMERICA.UTF8");
	
	if (!gsql_conf_value_get_boolean (GSQLE_CONF_ORACLE_USE_SYS_ENV))
	{
		env_all = gsql_conf_value_get_string (GSQLE_CONF_ORACLE_ENV);
		
		if (!env_all)
			return TRUE;
		
		GSQL_DEBUG ("Set custom environment");
		
		env_list = g_strsplit (env_all, ",", 100);
	
		for (i = 0; env_list[i]; i ++)
		{
			env_name = env_list[i++];
			env_value = env_list[i];
			
			GSQL_DEBUG ("ENV: %s=%s", env_name, env_value);
			
			setenv (env_name, env_value, 1);
		}
	
		g_strfreev (env_list);
	
	}

    return TRUE;
}

gboolean
engine_unload (GSQLEngine * engine)
{
	GSQL_TRACE_FUNC;

	if (engine->in_use)
	{
		g_critical ("Engine unload failed. Still in use.");
		return FALSE;
	}
	
	GSQL_FIXME;
	
	return FALSE;
}





GtkWidget *
engine_logon_widget_create ()
{
	GSQL_TRACE_FUNC;

	GtkWidget *table;
	GtkWidget *database_name;
	GtkWidget *database_name_entry;
	GtkWidget *username;
	GtkWidget *password;
	GtkWidget *label;
	GtkWidget *options_vbox;
	GtkWidget *expander;
	GtkWidget *oracle_option_hbox;
	GtkWidget *connect_as;
	GtkListStore *aliases;

	table = gtk_table_new (5, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 2);
	gtk_widget_show (table);
	
	aliases = engine_oracle_get_tns_aliases();
        
	database_name = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (aliases), 0);
//		gtk_combo_box_entry_new_text ();
	gtk_widget_show (database_name);
	gtk_table_attach (GTK_TABLE (table), database_name, 1, 2, 1, 2,
						(GtkAttachOptions) (GTK_FILL),
						(GtkAttachOptions) (GTK_FILL), 0, 0);
	database_name_entry = gtk_bin_get_child(GTK_BIN(database_name));
	gtk_entry_set_activates_default(GTK_ENTRY (database_name_entry), TRUE);
        
	label = gtk_label_new (_("Database name"));
	gtk_widget_show (label);
		
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 14, 0);
       
	username = gtk_entry_new ();
	gtk_widget_show (username);
	gtk_table_attach (GTK_TABLE (table), username, 1, 2, 2, 3,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_max_length (GTK_ENTRY (username), 32);
	gtk_entry_set_invisible_char (GTK_ENTRY (username), 9679);
	gtk_entry_set_activates_default(GTK_ENTRY (username), TRUE);
        
	label = gtk_label_new (_("Username"));
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 14, 0);
        
	password = gtk_entry_new ();
	gtk_widget_show (password);
	gtk_table_attach (GTK_TABLE (table), password, 1, 2, 3, 4,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_max_length (GTK_ENTRY (password), 32);
	gtk_entry_set_visibility (GTK_ENTRY (password), FALSE);
	gtk_entry_set_invisible_char (GTK_ENTRY (password), 9679);
	gtk_entry_set_activates_default(GTK_ENTRY (password), TRUE);
        
	label = gtk_label_new (_("Password"));
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 14, 0);
        
	expander = gtk_expander_new (NULL);
	gtk_widget_show (expander);        
	gtk_table_attach (GTK_TABLE (table), expander, 0, 2, 4, 5,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (0), 0, 0);
        
	options_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (options_vbox);
	gtk_container_add (GTK_CONTAINER (expander), options_vbox);
        
	oracle_option_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (oracle_option_hbox);
	gtk_box_pack_start (GTK_BOX (options_vbox), oracle_option_hbox, TRUE, TRUE, 0);
        
	label = gtk_label_new (_("Connect as "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (oracle_option_hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label), 14, 0);
        
	connect_as = gtk_combo_box_new_text ();
	gtk_widget_show (connect_as);
	gtk_box_pack_start (GTK_BOX (oracle_option_hbox), connect_as, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (connect_as, GTK_CAN_DEFAULT);
	gtk_combo_box_append_text (GTK_COMBO_BOX (connect_as), "Normal");
	gtk_combo_box_append_text (GTK_COMBO_BOX (connect_as), "SYSDBA");
	gtk_combo_box_append_text (GTK_COMBO_BOX (connect_as), "SYSOPER");
	gtk_combo_box_set_active (GTK_COMBO_BOX (connect_as), 0); 
        
	label = gtk_label_new (_("Options"));
	gtk_widget_show (label);
	gtk_expander_set_label_widget (GTK_EXPANDER (expander), label);        
        
	HOOKUP_OBJECT (table, database_name, "database");  
	HOOKUP_OBJECT (table, username, "username");  
	HOOKUP_OBJECT (table, password, "password");  
	HOOKUP_OBJECT (table, connect_as, "mode");        

	return table;	
}

void
engine_logon_widget_free (GtkWidget * logon_widget)
{
	GSQL_TRACE_FUNC;

}

static GtkListStore *
engine_oracle_get_tns_aliases ()
{
	GSQL_TRACE_FUNC;
	
	gchar *tnsdir = NULL;
	gchar *file;
	GIOChannel *ioc;
	GError *err = NULL;
	
	gchar buffer[8192];
	gchar alias[512];
	gint brakets = 0;
	gboolean reading = TRUE;
	GtkListStore *store;
	gchar *p, *c;
	
	store = gtk_list_store_new (1, G_TYPE_STRING, -1);
	
	tnsdir = getenv("TNS_ADMIN");
	
	if (!tnsdir)
	{
		tnsdir = getenv("ORACLE_HOME");
		
		if (!tnsdir)
			return store;
		
		GSQL_DEBUG ("Use ORALCE_HOME for tns aliases scanning");
		
		file = g_strdup_printf("%s/network/admin/tnsnames.ora", tnsdir);
		
		
	} else {
		
		file = g_strdup_printf("%s/tnsnames.ora", tnsdir);
		//free (tnsdir);
	}

	ioc = g_io_channel_new_file (file, "r", &err);
	if (!ioc)
	{
		GSQL_DEBUG ("failed to open file [%s]: %s", file, err->message);
		return store;
	}
	GSQL_DEBUG ("TNS file: %s", file);

	while (reading)
	{
		gsize bytes_read;
		GIOStatus status;
		guint i = 0;
		guint j, n;
		GtkTreeIter iter;
		
		memset (buffer, 0, 8192);

		status = g_io_channel_read_chars (ioc, buffer,
										  8192, &bytes_read,
										  &err);

		switch (status)
		{
			case G_IO_STATUS_EOF:
				GSQL_DEBUG ("TNS. Opening file: G_IO_STATUS_EOF");
				reading = FALSE;
				break;
				
			case G_IO_STATUS_NORMAL:
				GSQL_DEBUG ("TNS. Opening file: G_IO_STATUS_NORMAL");
				
				if (bytes_read == 0)
					continue;
				
				c = buffer;
				while (i++ < bytes_read)
				{
					if (*c == '#') // comment
						while ((*c++ != '\n') && (i++ < bytes_read));
						
					while((g_ascii_isspace(*c)) && (i++ < bytes_read)) c++;
					
					j = 0;
					memset (alias, 0, 256);
					
					while ((g_ascii_isalnum(*c) || (*c == '.') || (*c == '_')) && 
						   (i++ < bytes_read) &&  (j < 256))
					{
						alias[j++] = *c++;							
					}
					
					if (!j)
						break;
					
					GSQL_DEBUG ("Alias found: [%s]", alias);
					gtk_list_store_append (store, &iter);
					gtk_list_store_set (store, &iter,
										0, alias, -1);
					
					// looking for the first braket
					while (*c != '(' && i < bytes_read )
					{
						c++; i++;
					}
					
					n = 1; c++;
					// looking for closing braket for the first one
					while ( *c && n && i < bytes_read)
					{
						//GSQL_DEBUG ("I=[%d]", i);
						i++;
						if (*c == '(') n++;
						if (*c == ')') n--;
						c++;
					}

					c++;
				}
				
				break;
				
			case G_IO_STATUS_AGAIN:
				GSQL_DEBUG ("TNS. Opening file: G_IO_STATUS_AGAIN");
				continue;
				
			case G_IO_STATUS_ERROR:
			default:
				GSQL_DEBUG ("TNS.Opening file: G_IO_STATUS_ERROR");
				
				g_io_channel_unref (ioc);
				
				return store;
		}
	}
	
	g_io_channel_unref (ioc);
	
	g_free (file);
	
	return store;
}




