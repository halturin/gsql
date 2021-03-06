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

 
#ifndef _NAV_TREE__PROCEDURES_H
#define _NAV_TREE__PROCEDURES_H

#include <glib.h>
#include <libgsql/navigation.h>

G_BEGIN_DECLS

void
nav_tree_refresh_procedures (GSQLNavigation *navigation,
						 GtkTreeView *tv,
						 GtkTreeIter *iter);

G_END_DECLS

#endif /* _NAV_TREE__PROCEDURES_H */

 
