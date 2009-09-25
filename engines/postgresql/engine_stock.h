/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2009  Estêvão Samuel Procópio <tevaum@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

 
#ifndef _ENGINE_STOCK_H
#define _ENGINE_STOCK_H

#include <glib.h>

#define GSQLE_PGSQL_STOCK_PGSQL				"gsqle-pgsql-pgsql"
#define GSQLE_PGSQL_STOCK_PROCESS_LIST		"gsqle-pgsql-process-list"
#define GSQLE_PGSQL_STOCK_SESSION_VARIABLES "gsqle-pgsql-session-variables"
#define GSQLE_PGSQL_STOCK_GLOBAL_VARIABLES  "gsqle-pgsql-global-variables"

G_BEGIN_DECLS

void
engine_stock_init ();

G_END_DECLS

#endif /* _ENGINE_STOCK_H */

 
