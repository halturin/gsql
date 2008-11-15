/***************************************************************************
 *            engine_conf.h
 *
 *  Thu Sep  6 23:43:10 2007
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
 
#ifndef _ENGINE_CONF_H
#define _ENGINE_CONF_H

#include <glib.h>
#include <gtk/gtk.h>


#define GSQLE_CONF_ORACLE_ENABLE_TRACE	GSQL_CONF_ENGINES_ROOT_KEY "/oracle/enable_trace"
#define GSQLE_CONF_ORACLE_USE_SYS_ENV	GSQL_CONF_ENGINES_ROOT_KEY "/oracle/use_system_environment"
#define GSQLE_CONF_ORACLE_ENV			GSQL_CONF_ENGINES_ROOT_KEY "/oracle/environment"


G_BEGIN_DECLS


GtkWidget *
engine_conf_widget_create ();

void
engine_conf_widget_free (GtkWidget *conf_widget);


G_END_DECLS

#endif /* _ENGINE_CONF_H */

 
