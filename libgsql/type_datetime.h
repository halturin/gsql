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
 
#ifndef _TYPE_DATETIME_H
#define _TYPE_DATETIME_H

#include <glib.h>
#include <glib-object.h>
#include <libgsql/conf.h>

#define GSQL_CONF_DATETIME_FORMAT		GSQL_CONF_ROOT_KEY "/datetime_format"
#define GSQL_DATETIME_FORMAT_DEFAULT    "%D/%M/%Y %h:%m:%s"

#include <gtk/gtkcellrenderertext.h>

#define GSQL_TYPE_DATETIME (gsql_type_datetime_get_type ())

#define GSQL_TYPE_CELL_RENDERER_DATETIME  (gsql_cell_renderer_datetime_get_type ())
#define GSQL_CELL_RENDERER_DATETIME(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_TYPE_CELL_RENDERER_DATETIME, GSQLCellRendererDateTime))
#define GSQL_CELL_RENDERER_DATETIME_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_TYPE_CELL_RENDERER_DATETIME, GSQLCellRendererDateTimeClass))

#define GSQL_IS_CELL_RENDERER_DATETIME(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_TYPE_CELL_RENDERER_DATETIME))
#define GSQL_IS_CELL_RENDERER_DATETIME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_TYPE_CELL_RENDERER_DATETIME))


typedef struct _GSQLTypeDateTime GSQLTypeDateTime;

typedef struct _GSQLCellRendererDateTime GSQLCellRendererDateTime;
typedef struct _GSQLCellRendererDateTimeClass GSQLCellRendererDateTimeClass;
typedef struct _GSQLCellRendererDateTimePrivate GSQLCellRendererDateTimePrivate;


typedef enum {
	GSQLTypeDateTimeFULL = 0,
	GSQLTypeDateTimeDMY,
	GSQLTypeDateTimeHMS
} GSQLTypeDateTime_type;

struct _GSQLTypeDateTime
{
		guint  sec;
		guint  min;
		guint  hour;
		guint  day;
		guint  mon;
		guint  year;	
		GSQLTypeDateTime_type dt_type;
};

struct _GSQLCellRendererDateTime
{
	GtkCellRendererText parent;

};

struct _GSQLCellRendererDateTimeClass
{
	GtkCellRendererTextClass parent;

};



G_BEGIN_DECLS

GType
gsql_type_datetime_get_type (void) G_GNUC_CONST;

gchar *
gsql_type_datetime_to_gchar (GSQLTypeDateTime *dt, gchar *text, guint textlen);

GType 
gsql_cell_renderer_datetime_get_type (void) G_GNUC_CONST;

GtkCellRenderer *
gsql_cell_renderer_datetime_new (void);


G_END_DECLS

#endif /* _TYPE_DATETIME_H */

