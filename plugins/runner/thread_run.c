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



#include <libgsql/common.h>
#include <libgsql/workspace.h>
#include "plugin_runner.h"
#include "thread_run.h"
#include <unistd.h>


static gpointer
periodical_thread (gpointer data)
{
		
	GSQLPRunnerExecOptions *eopt = data;
	guint period;
	gchar mess[GSQL_MESSAGE_LEN];
	guint c, i, p;
	
	if (eopt->suspended)
	{
		GSQL_DEBUG ("Suspended period: %d", eopt->suspended_period);
		period = eopt->suspended_period;
		while (period)
		{
			if (!eopt->run_state)
			{
				GSQL_DEBUG ("stop from suspend");
				g_snprintf (mess, GSQL_MESSAGE_LEN, "Periodical execution stopped on suspend state [thread %d]",
							eopt->thread_id);
				gsql_message_add (eopt->workspace, GSQL_MESSAGE_NOTICE, mess);
				g_thread_exit (NULL);
			}
			period--;
			sleep(1);
			
		}
		
	}
	
	c = eopt->cycles;
	i = 0;
	
	while (c)
	{
		
		gsql_editor_run_sql (eopt->editor);
		GSQL_DEBUG ("c = %d",c);
		i++;
		
		p = eopt->period;
		while (p)
		{
			if (!eopt->run_state)
			{
				GSQL_DEBUG ("stop from run");
				g_snprintf (mess, GSQL_MESSAGE_LEN, "Periodical execution stopped [thread %d]"
							"\n\truns total: %d",
							eopt->thread_id, i);
				gsql_message_add (eopt->workspace, GSQL_MESSAGE_NOTICE, mess);
				g_thread_exit (NULL);
			}
		
			sleep(1);
			p--;
		}
		if (eopt->cycles_limit)
				c--;
	}
	
	g_snprintf (mess, GSQL_MESSAGE_LEN, "Periodical execution completed [thread %d]"
				"\n\truns total: %d",
				eopt->thread_id, i);
	gsql_message_add (eopt->workspace, GSQL_MESSAGE_NOTICE, mess);
	
	eopt->run_state = FALSE;
	gtk_toggle_action_set_active (eopt->action, FALSE);
	
	g_thread_exit (NULL);
}

void 
start_periodical (GSQLPRunnerExecOptions *eopt)
{
	
	GThread *thread = NULL;
	GError *err;
	gchar mess[GSQL_MESSAGE_LEN];
	guint s, c;
	static gint id = 1;
	
	thread = g_thread_create (periodical_thread,
							  eopt,
							  TRUE,
							  &err);
	if (!thread)
	{
		GSQL_DEBUG ("Couldn't create thread");
		return;
	}
	
	eopt->thread = thread;
	eopt->thread_id = id++;
	
	s = (eopt->suspended) ? eopt->suspended_period : 0;
	c = (eopt->cycles_limit) ? eopt->cycles : 0;
	g_snprintf (mess, GSQL_MESSAGE_LEN, 
				"Periodical execution started [thread: %d]..."
				"\n\tinterval: %d second[s]. "
				"\n\tsuspended: %d second[s]"
				"\n\tcycles: %d",
				eopt->thread_id, eopt->period, s, c);
	
	gsql_message_add (eopt->workspace, GSQL_MESSAGE_NOTICE, mess);
	
}






