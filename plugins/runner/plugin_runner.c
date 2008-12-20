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


#include <libgsql/plugins.h>
#include <libgsql/stock.h>
#include <libgsql/sqleditor.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libgsql/common.h>
#include "plugin_runner.h"
#include "thread_run.h"



#define PLUGIN_VERSION "0.1"
#define PLUGIN_ID    "plugin_runner"
#define PLUGIN_NAME  "Runner"
#define PLUGIN_DESC  "Periodical SQL execution"
#define PLUGIN_AUTHOR "Taras Halturin"
#define PLUGIN_HOMEPAGE "http://gsql.org"

static void on_runner (GtkToggleAction *action,  gpointer user_data);

static GSQLStockIcon stock_icons[] = 
{
	{ GSQLP_RUNNER_STOCK_ICON,	"runner.png" },
	{ GSQLP_RUNNER_STOCK_LOGO,	"runner_logo.png" }
};


static GtkToggleActionEntry runner_action[] = 
{
	{ "ActionRunner", GSQLP_RUNNER_STOCK_ICON, N_("Periodical execution"), NULL, 
		N_("Periodical run"), G_CALLBACK (on_runner), FALSE}
};

static gchar plugin_ui[] = 
"<ui>	"
"  <toolbar name=\"SQLEditorToolbarRun\">  "
"  		<placeholder name=\"PHolderSQLEditorRun\" >  "
"			<toolitem name=\"SQLEditorRunner\"  action=\"ActionRunner\" />	"
"		</placeholder>	"
"	</toolbar>			"
"</ui>	";

enum {
	PREDEF_1MIN = 0,
	PREDEF_2MIN,
	PREDEF_3MIN,
	PREDEF_5MIN,
	PREDEF_10MIN
};

static GtkActionGroup *
on_f_action()
{
	GtkActionGroup *action;
	
	action = gtk_action_group_new ("ActionsRunner");
	gtk_action_group_add_toggle_actions (action, runner_action, 
									G_N_ELEMENTS (runner_action), NULL);
	return action;
}

gboolean 
plugin_load (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;
	
	GtkWidget *button;
	GtkMenu		*menu;
	GtkMenuItem *item;
	GtkWidget *widget;
	GSList    *group = NULL;

	plugin->info.author = PLUGIN_AUTHOR;
	plugin->info.id = PLUGIN_ID;
	plugin->info.name = PLUGIN_NAME;
	plugin->info.desc = PLUGIN_DESC;
	plugin->info.homepage = PLUGIN_HOMEPAGE;
	plugin->info.version = PLUGIN_VERSION;
	plugin->file_logo = "runner_logo.png";
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));

	
	gsql_editor_merge_f_actions (plugin_ui, on_f_action);

	plugin->plugin_conf_dialog = NULL;
	plugin->menu_update_sensitive = NULL;
	
	return TRUE;
}

gboolean 
plugin_unload (GSQLPlugin * plugin)
{
	GSQL_TRACE_FUNC;

	return TRUE;
}

static void
on_real_close_content (GSQLContent *content, gboolean force)
{
	GSQL_TRACE_FUNC;
	
	GSQLPRunnerExecOptions *eopt;
	
	eopt = g_object_get_data (G_OBJECT (content), "eopt");
	
	if (!eopt)
		return;
	
	eopt->run_state = FALSE;
	
	gsql_thread_join (eopt->thread);
	
	g_free (eopt);
		
}


static void
on_widget_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkWidget *w = user_data;
	
	gtk_widget_set_sensitive (w, gtk_toggle_button_get_active (togglebutton));
	
}

static void 
on_runner (GtkToggleAction *action,  gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GladeXML* gxml;
	GtkDialog *dialog;
	gint ret, i;
	guint p, p_hour, p_min, p_sec;
	gboolean state;
	GSQLWorkspace *workspace;
	GSQLPRunnerExecOptions *eopt;
	GSQLContent *content;
	GSQLEditor *editor;
	gchar mess[GSQL_MESSAGE_LEN];
	GtkWidget *period_predef, *period_custom, *suspend_start, 
			  *cycles_chbox, *cycles_limit, *period_predef_value, *custom_hbox;
	GtkWidget *custom_hour, *custom_min, *custom_sec, *suspend_hour, *suspend_min;
	GtkWidget *w2;
	GList      *l_childs;
	
	state = gtk_toggle_action_get_active (action);
	workspace = gsql_session_get_workspace (NULL);
	content = gsql_workspace_get_current_content (workspace);
	
	eopt = g_object_get_data (G_OBJECT (content), "eopt");
	
	if (!eopt)
	{
		l_childs = gtk_container_get_children (GTK_CONTAINER (content));
		
		g_return_if_fail (g_list_length (l_childs) == 1);
		
		editor = GSQL_EDITOR (l_childs->data);
		g_return_if_fail (GSQL_IS_EDITOR (editor));

		eopt = g_new0 (GSQLPRunnerExecOptions, 1);
		
		eopt->custom = FALSE;
		eopt->editor = editor;
		eopt->workspace = workspace;
		eopt->suspended = FALSE;
		eopt->cycles_limit = FALSE;
		eopt->run_state = FALSE;
		eopt->action = action;
		
		GSQL_DEBUG ("Set_data");
		
		g_object_set_data (G_OBJECT (content), "eopt", eopt);
		GSQL_DEBUG ("Connect callback");
		g_signal_connect (G_OBJECT (content), "destroy", G_CALLBACK (on_real_close_content),
						  NULL);
	}
		
	
	if (!state) {
		
		if (!eopt->run_state)
			return;
		
		GSQL_DEBUG ("Do stop thread");
		
		eopt->run_state = FALSE;
		
		return;
	}
		
	
		
	gxml = glade_xml_new (GSQLP_RUNNER_GLADE_DIALOG, "periodical_dialog", NULL);
	
	dialog = (GtkDialog *) glade_xml_get_widget (gxml, "periodical_dialog");
	
	period_predef = glade_xml_get_widget (gxml, "period_predef");
	period_predef_value = glade_xml_get_widget (gxml, "period_predef_value");
	g_signal_connect (G_OBJECT (period_predef), "toggled", 
					  G_CALLBACK (on_widget_toggled), 
					  period_predef_value);
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (period_predef),
								  !eopt->custom);

	GSQL_DEBUG ("a1");
	period_custom = glade_xml_get_widget (gxml, "period_custom");
	custom_hbox = glade_xml_get_widget (gxml, "custom_hbox");
	g_signal_connect (G_OBJECT (period_custom), "toggled", 
					  G_CALLBACK (on_widget_toggled), custom_hbox);
	GSQL_DEBUG ("a2");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (period_custom),
								  eopt->custom);
	
	custom_hour = glade_xml_get_widget (gxml, "custom_hour");
	custom_min = glade_xml_get_widget (gxml, "custom_min");
	custom_sec = glade_xml_get_widget (gxml, "custom_sec");
	
	if (!eopt->custom)
	{
		// predefined
		switch (eopt->period)
		{
			case 60: i = PREDEF_1MIN;
				break;
			
			case 120: i = PREDEF_2MIN;
				break;
			
			case 180: i = PREDEF_3MIN;
				break;
			
			case 300: i = PREDEF_5MIN;
				break;
			
			case 600:  i = PREDEF_10MIN;
				break;
			
			default:
				i = PREDEF_1MIN;
				eopt->period = 60;
				break;
		}
		gtk_combo_box_set_active (GTK_COMBO_BOX (period_predef_value),
								  i);
		
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (custom_min),
								   eopt->period / 60);
		
	} else {
		
		gtk_combo_box_set_active (GTK_COMBO_BOX (period_predef_value),
								  0);
		
		if (eopt->period == 0 ) 
			eopt->period = 60;
		
		p_hour = eopt->period / (60*60);
		p_min = (eopt->period - p_hour*60*60)/60;
		p_sec = eopt->period - (p_hour*60*60 + p_min*60);
		
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (custom_hour), p_hour);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (custom_min), p_min);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (custom_sec), p_sec);
	}

	suspend_start = glade_xml_get_widget (gxml, "suspend_start");
	w2 = glade_xml_get_widget (gxml, "suspend_hbox");
	g_signal_connect (G_OBJECT (suspend_start), "toggled", 
					  G_CALLBACK (on_widget_toggled), w2);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (suspend_start),
								  eopt->suspended);
	suspend_hour = glade_xml_get_widget (gxml, "suspend_hour");
	suspend_min = glade_xml_get_widget (gxml, "suspend_min");
	
	p_hour =  eopt->suspended_period / (60*60);
	p_min = (eopt->suspended_period - p_hour*60*60)/60;
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (suspend_hour), p_hour);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (suspend_min), p_min);
	
	cycles_chbox = glade_xml_get_widget (gxml, "cycles_chbox");
	cycles_limit = glade_xml_get_widget (gxml, "cycles_limit");
	g_signal_connect (G_OBJECT (cycles_chbox), "toggled", 
					  G_CALLBACK (on_widget_toggled), cycles_limit);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cycles_chbox),
								  eopt->cycles_limit);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (cycles_limit), eopt->cycles);
	
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gsql_window));
	ret = gtk_dialog_run (dialog);
	
	switch (ret)
	{
		case 0:
			GSQL_DEBUG ("State true. Do run thread");
			
			
			eopt->run_state = TRUE;
			eopt->custom = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (period_custom));
			
			if (!eopt->custom)
			{
				i = gtk_combo_box_get_active (GTK_COMBO_BOX (period_predef_value));
			
				switch (i) 
				{
					case PREDEF_1MIN:
						eopt->period = 60;
						break;
						
					case PREDEF_2MIN:
						eopt->period = 120;
						break;
					
					case PREDEF_3MIN:
						eopt->period = 180;
						break;
					
					case PREDEF_5MIN:
						eopt->period = 300;
						break;
					
					case PREDEF_10MIN:
						eopt->period = 600;
						break;
						
					default:
						eopt->period = 60;
						break;
				}
				
			} else {
				
				p_hour = gtk_spin_button_get_value (GTK_SPIN_BUTTON (custom_hour));
				p_min = gtk_spin_button_get_value (GTK_SPIN_BUTTON (custom_min));
				p_sec = gtk_spin_button_get_value (GTK_SPIN_BUTTON (custom_sec));
				
				eopt->period = p_hour*60*60 + p_min*60 + p_sec;
				
			}
			
			p_hour = gtk_spin_button_get_value (GTK_SPIN_BUTTON (suspend_hour));
			p_min = gtk_spin_button_get_value (GTK_SPIN_BUTTON (suspend_min));
			eopt->suspended_period = p_hour*60*60 + p_min*60;
			eopt->suspended = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (suspend_start));
			
			eopt->cycles_limit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cycles_chbox));
			eopt->cycles = gtk_spin_button_get_value (GTK_SPIN_BUTTON (cycles_limit));
			
			
			start_periodical (eopt);
			
			if (!eopt->thread)
			{
				g_object_set_data (G_OBJECT (content), "eopt", NULL);
				g_free (eopt);
			}
			
			break;
			
		default: 
			GSQL_DEBUG ("Cancel");
			
		// FIXME:
		//  block/unblock does not work here. I dont know why.
		//  double run this func ("toggled" callback action) is happen.
		//  Does any body can explain me this feature?
			
			g_signal_handlers_block_by_func (action, on_runner, NULL);
			
			gtk_toggle_action_set_active (action, FALSE);
		
			g_signal_handlers_unblock_by_func (action, on_runner, NULL);
			
			break;
	
	}
	
	gtk_widget_destroy ((GtkWidget *) dialog);
	g_object_unref(G_OBJECT(gxml));
	
}
