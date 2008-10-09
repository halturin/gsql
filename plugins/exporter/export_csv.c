/***************************************************************************
 *            export_csv.c
 *
 *  Mon Mar  3 01:08:53 2008
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
 
 
#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/common.h>
#include <libgsql/content.h>

void
exporter_export_to_csv (GSQLContent *content, gchar *file, gchar *encoding, gboolean headers)
{
	GSQL_TRACE_FUNC;
	
	return;
};
