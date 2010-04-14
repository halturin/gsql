/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2010  Taras Halturin  halturin@gmail.com
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


#ifndef _COLOR_HINTER_H
#define _COLOR_HINTER_H

typedef struct _GSQLColorHinter			GSQLColorHinter;
typedef struct _GSQLColorHinterPrivate	GSQLColorHinterPrivate;
typedef struct _GSQLColorHinterClass	GSQLColorHinterClass;


#define GSQL_COLORHINTER_TYPE 			(gsql_colorhinter_get_type ())
#define GSQL_COLORHINTER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
										GSQL_COLORHINTER_TYPE, GSQLColorHinter))

#define GSQL_COLORHINTER_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), \
										GSQL_COLORHINTER_TYPE, GSQLColorHinterClass))

#define GSQL_IS_COLORHINTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
										GSQL_COLORHINTER_TYPE))

#define GSQL_IS_COLORHINTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
										GSQL_COLORHINTER_TYPE))

#include <libgsql/common.h>

struct _GSQLColorHinter
{
	GtkToolItem		parent;

	GSQLColorHinterPrivate *private;

};

struct _GSQLColorHinterClass
{
	GtkToolItemClass	parent;


};

GSQLColorHinter * gsql_colorhinter_new ();

void gsql_colorhinter_save_color (GSQLColorHinter *ch, const gchar *name);

void gsql_colorhinter_load_color (GSQLColorHinter *ch, const gchar *name);

void gsql_colorhinter_clear_color (GSQLColorHinter *ch);

#endif