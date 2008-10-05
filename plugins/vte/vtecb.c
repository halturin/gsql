/***************************************************************************
 *            vtecb.c
 *
 *  Sun Sep  9 02:42:10 2007
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

#include <libgsql/session.h>
#include <libgsql/content.h>
#include <libgsql/conf.h>
#include <libgsql/stock.h>
#include <libgsql/common.h>
#include <vte/vte.h>
#include "vtecb.h"
#include "plugin_vte.h"
#include <string.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <signal.h>

static void
vte_exited_cb (GtkWidget *widget, gboolean force);
static void
vte_child_exited_cb (GtkWidget *widget, gpointer user_data);


static gchar **
vte_parse_command (GSQLSession *session, gchar *command)
{
	GSQL_TRACE_FUNC

	gchar **str, **ptr, *t;
	gchar *dest;
	GValue hostname = {0, };
	GValue database = {0, };
	GValue username = {0, };
	GValue password = {0, };
	
	g_return_val_if_fail (session != NULL, NULL);
	g_return_val_if_fail (command != NULL, NULL);
	
	g_value_init (&hostname, G_TYPE_STRING);
	g_value_init (&database, G_TYPE_STRING);
	g_value_init (&username, G_TYPE_STRING);
	g_value_init (&password, G_TYPE_STRING);
	g_object_get_property (G_OBJECT (session), "session-database", &database);
	g_object_get_property (G_OBJECT (session), "session-hostname", &hostname);
	g_object_get_property (G_OBJECT (session), "session-username", &username);
	g_object_get_property (G_OBJECT (session), "session-password", &password);
	
	str = g_strsplit (command, "%", 32);
	ptr = str;
	while (*ptr)
	{
		switch (**ptr)
		{
			case 'u':
				t = *ptr;
				*ptr = g_strdup_printf ("%s%s", g_value_get_string (&username), t+1);
				g_free (t);
				break;
			case 'd':
				t = *ptr;
				*ptr = g_strdup_printf ("%s%s", g_value_get_string (&database), t+1);
				g_free (t);
				break;
			case 'p':
				t = *ptr;
				*ptr = g_strdup_printf ("%s%s", g_value_get_string (&password), t+1);
				g_free (t);
				break;
			case 'h':
				t = *ptr;
				*ptr = g_strdup_printf ("%s%s", g_value_get_string (&hostname), t+1);
				g_free (t);
				break;
		};
		
		ptr++;
	};
	dest = g_strjoinv (NULL, str);
	g_strfreev (str);
	
	str = g_strsplit (dest, " ", 32);
	return str;
	
};

void
on_open_terminal_activate (GtkMenuItem * mi, gpointer data)
{
	GSQL_TRACE_FUNC

	GSQLSession *session;
	GSQLContent *content;
	
	gchar tmp[256];
	gchar *command;
	gchar **cmd;
	gchar *arg0, **p;
	gchar **args;
	
	extern char **environ;
	
	VteTerminal *term;
	GtkWidget *scroll;
	GtkWidget *hbox;
	gint pid;
	
	session = gsql_session_get_active ();
	if (session == NULL)
		return;
	
	g_snprintf(tmp, 256 ,"%s/vte/%s", GSQL_CONF_PLUGINS_ROOT_KEY, 
			   session->engine->info.id);
	command = gsql_conf_value_get_string (tmp);
	cmd = vte_parse_command (session, command);
	
	if (cmd == NULL)
		return;

	g_free (command);
	content = gsql_content_new (session, GSQLP_VTE_STOCK_TERMINAL);
	
	term = (VteTerminal *) vte_terminal_new ();
	g_signal_connect (G_OBJECT (term), "child-exited",
					  G_CALLBACK (vte_child_exited_cb), content);
	g_signal_connect (G_OBJECT (content), "close",
					  G_CALLBACK (vte_exited_cb), NULL);

	vte_terminal_set_scroll_on_output (term, TRUE);
	
	scroll = gtk_vscrollbar_new (GTK_ADJUSTMENT (term->adjustment));
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (term), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), scroll, FALSE, TRUE, 0);
	
	
	gsql_content_set_child (content, hbox);
	
	gsql_content_set_name_full(content, g_strdup_printf ("Terminal session: %s", 
														 gsql_session_get_name (session)),
							   "Terminal");
	pid = vte_terminal_fork_command (term, cmd[0], cmd, environ, NULL, 
							   FALSE, FALSE, FALSE);
	g_strfreev (cmd);
	
	gsql_session_add_content (session, content);
	
	return;
};



static void
vte_exited_cb (GtkWidget *widget, gboolean force)
{
	GSQL_TRACE_FUNC;
	if (force)
		GSQL_DEBUG ("Forced closing");
	else 
		GSQL_DEBUG ("Regular closing");
	
	gtk_widget_destroy (GTK_WIDGET (widget));
	
	return;
};

static void
vte_child_exited_cb (GtkWidget *widget, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLContent *content = user_data;	
	gtk_widget_destroy (GTK_WIDGET (content));
	
	return;
}
