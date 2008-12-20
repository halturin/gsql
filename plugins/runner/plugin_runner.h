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

 
#ifndef _PLUGIN_RUNNER_H
#define _PLUGIN_RUNNER_H

#include <glib.h>
#include <libgsql/sqleditor.h>

#define GSQLP_RUNNER_STOCK_ICON "gsql-plugin-runner-icon"
#define GSQLP_RUNNER_STOCK_LOGO "gsql-plugin-runner-logo"

#define GSQLP_RUNNER_GLADE_DIALOG PACKAGE_GLADE_DIR"/plugins/runner_dialog.glade"

typedef struct _GSQLPRunnerExecOptions GSQLPRunnerExecOptions;

struct _GSQLPRunnerExecOptions
{
	GSQLEditor *editor;
	GSQLWorkspace *workspace;
	GtkToggleAction *action;
	
	gboolean custom;
	guint period;
	
	gboolean suspended;
	guint suspended_period;
	
	gboolean cycles_limit;
	guint cycles;
	
	gboolean run_state;
	GThread *thread;
	gint	thread_id;
};

G_BEGIN_DECLS


G_END_DECLS

#endif /* _PLUGIN_RUNNER_H */

 
