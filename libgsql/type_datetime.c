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
 
#include "type_datetime.h"
#include "common.h"
#include <string.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <stddef.h>

#define DATETIME_MAX_LEN 128

static gchar *dt_format = NULL;
static gchar *dt_format_parsed = NULL;
static const gchar dt_format_default[] = GSQL_DATETIME_FORMAT_DEFAULT;
static size_t dt_offset[6];

static gpointer gsql_type_datetime_copy (GSQLTypeDateTime *dt);
static void gsql_type_datetime_free (GSQLTypeDateTime *dt);
static void gsql_datetime_format_property_set (gpointer pt);
static gchar * gsql_datetime_format_parse (void);

static void gsql_cell_renderer_datetime_class_init (GSQLCellRendererDateTimeClass *klass);
static void gsql_cell_renderer_datetime_init (GSQLCellRendererDateTime *obj);
static void gsql_cell_renderer_datetime_finalize   (GObject *object);

static void gsql_cell_renderer_datetime_property (GObject *object,
						 guint         prop_id,
						 GValue       *value,
						 GParamSpec   *spec);
static void gsql_cell_renderer_datetime_set_property (GObject *object,
						 guint         prop_id,
						 const GValue *value,
						 GParamSpec   *spec);

static GtkCellEditable * gsql_cell_renderer_datetime_start_editing (GtkCellRenderer     *cell,
							       GdkEvent            *event,
							       GtkWidget           *widget,
							       const gchar         *path,
							       GdkRectangle        *background_area,
							       GdkRectangle        *cell_area,
							       GtkCellRendererState flags);
static GtkCellRendererText *parent_class;

#define GSQL_CELL_RENDERER_DATETIME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
														GSQL_TYPE_CELL_RENDERER_DATETIME, \
														GSQLCellRendererDateTimePrivate))

enum {
	PROP_DATETIME = 1,
};

typedef union _dmyhms dmyhms;

struct _GSQLCellRendererDateTimePrivate
{
	gchar *test;

};

union  _dmyhms
{
	struct _datetime
	{
		guint D:1;
		guint M:1;
		guint Y:1;
		guint h:1;
		guint m:1;
		guint s:1;
		guint _align:2;
	} datetime;
	guchar b;
};


GType
gsql_type_datetime_get_type (void)
{
	static GType datetime_type = 0;
	
	if (!datetime_type)
	{
		datetime_type = g_boxed_type_register_static ("GSQLTypeDateTime",
									  (GBoxedCopyFunc) gsql_type_datetime_copy,
									  (GBoxedFreeFunc) gsql_type_datetime_free
									  );
		
	}
		
	return datetime_type;
	
}

GType
gsql_cell_renderer_datetime_get_type (void)
{		
	static GType cr_datetime = 0;
	
	if (!cr_datetime)
	{
		static const GTypeInfo info =
			{
  				sizeof (GSQLCellRendererDateTimeClass),
  				(GBaseInitFunc) NULL,
				(GBaseFinalizeFunc) NULL,
  				(GClassInitFunc) gsql_cell_renderer_datetime_class_init,
  				(GClassFinalizeFunc) NULL,
  				NULL,
  				sizeof (GSQLCellRendererDateTime),
  				0,
  				(GInstanceInitFunc) gsql_cell_renderer_datetime_init,
				NULL
			};
    

		cr_datetime = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT,
										  "GSQLCellRendererDateTime",
										  &info, 0);
	}
	
	return cr_datetime;
}

gchar *
gsql_type_datetime_to_gchar (GSQLTypeDateTime *dt, gchar *text, guint textlen)
{
	gchar *f_tmp;
	static GtkWidget *tmp;
	GSQLTypeDateTime dt1;
	
	if (!dt_format_parsed)
	{
		f_tmp = gsql_conf_value_get_string (GSQL_CONF_DATETIME_FORMAT);
		dt_format = f_tmp;
		gsql_datetime_format_property_set (NULL);

		// FIXME: just a fake widget. it is required for gsql_conf_nitify_add
		// 
		tmp = gtk_button_new ();
		gsql_conf_nitify_add (GSQL_CONF_DATETIME_FORMAT,
								(GSQLConfNotifyFunc) gsql_datetime_format_property_set,
								tmp);
	}
	
	if (!text)
	{
		f_tmp = g_strdup_printf (dt_format_parsed, 
							 *(guint *) ((void *) dt + dt_offset[0]), 
							 *(guint *) ((void *) dt + dt_offset[1]), 
							 *(guint *) ((void *) dt + dt_offset[2]), 
							 *(guint *) ((void *) dt + dt_offset[3]), 
							 *(guint *) ((void *) dt + dt_offset[4]), 
							 *(guint *) ((void *) dt + dt_offset[5]) );
		return f_tmp;
		
	} else {
		g_snprintf (text, textlen, dt_format_parsed,
					*(guint *) ((void *) dt + dt_offset[0]), 
					*(guint *) ((void *) dt + dt_offset[1]), 
					*(guint *) ((void *) dt + dt_offset[2]), 
					*(guint *) ((void *) dt + dt_offset[3]), 
					*(guint *) ((void *) dt + dt_offset[4]), 
					*(guint *) ((void *) dt + dt_offset[5]) );
	}
	
	return text;
}

GtkCellRenderer *
gsql_cell_renderer_datetime_new (void)
{
  return g_object_new (GSQL_TYPE_CELL_RENDERER_DATETIME, NULL);
}

/* 
 *  Static section:
 *
 *  gsql_type_datetime_copy
 *  gsql_type_datetime_free
 *  gsql_datetime_format_property_set
 *  gsql_cell_renderer_datetime_start_editing
 *  gsql_cell_renderer_datetime_class_init
 *  gsql_cell_renderer_datetime_init
 *  gsql_cell_renderer_datetime_finalize
 *	gsql_cell_renderer_datetime_get_property
 *  gsql_cell_renderer_datetime_set_property
 *  
 */

static GtkCellEditable * 
gsql_cell_renderer_datetime_start_editing (GtkCellRenderer     *cell,
							       GdkEvent            *event,
							       GtkWidget           *widget,
							       const gchar         *path,
							       GdkRectangle        *background_area,
							       GdkRectangle        *cell_area,
							       GtkCellRendererState flags)
{
	GSQL_TRACE_FUNC;
	
		//FIXME
	GSQL_DEBUG ("Not implemented");
	
	return NULL;
}


static void
gsql_cell_renderer_datetime_get_property (GObject      *object,
				     guint         prop_id,
				     GValue *value,
				     GParamSpec   *pspec)
{
	GSQLCellRendererDateTime *renderer;
	GSQLCellRendererDateTimePrivate *priv;
	gchar *text;

	renderer = GSQL_CELL_RENDERER_DATETIME (object);
	priv = GSQL_CELL_RENDERER_DATETIME_GET_PRIVATE (renderer);

	switch (prop_id)
    {
		case PROP_DATETIME:
			text = renderer->parent.text;
			g_value_set_boxed (value, text);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
    }
}


static void
gsql_cell_renderer_datetime_set_property (GObject      *object,
				     guint         prop_id,
				     const GValue *value,
				     GParamSpec   *pspec)
{
	GSQLCellRendererDateTime *renderer;
	GSQLCellRendererDateTimePrivate *priv;
	GSQLTypeDateTime *obj;
	gchar *text;

	renderer = GSQL_CELL_RENDERER_DATETIME (object);
	priv = GSQL_CELL_RENDERER_DATETIME_GET_PRIVATE (renderer);

	switch (prop_id)
    {
		case PROP_DATETIME:
			obj = g_value_get_boxed (value);
			if (obj)
			{
				if (renderer->parent.text)
				{
					memset (renderer->parent.text, 0, DATETIME_MAX_LEN);
					
				} else {
					
					renderer->parent.text = g_malloc0 (DATETIME_MAX_LEN);
					memset (renderer->parent.text, 0, DATETIME_MAX_LEN);
				}
				
				gsql_type_datetime_to_gchar (obj, renderer->parent.text, DATETIME_MAX_LEN);		
			};
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
    }
}


static void
gsql_cell_renderer_datetime_class_init (GSQLCellRendererDateTimeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

	object_class->finalize     = gsql_cell_renderer_datetime_finalize;
	object_class->get_property = gsql_cell_renderer_datetime_get_property;
	object_class->set_property = gsql_cell_renderer_datetime_set_property;

	cell_class->start_editing  = gsql_cell_renderer_datetime_start_editing;
	parent_class = g_type_class_peek_parent (klass);

	g_object_class_install_property (object_class,
							PROP_DATETIME,
							g_param_spec_boxed ("datetime",
							N_("DateTime value"),
							N_("DateTime value"),
							GSQL_TYPE_DATETIME,
							G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GSQLCellRendererDateTimePrivate));
}

static void
gsql_cell_renderer_datetime_init (GSQLCellRendererDateTime *obj)
{
  GSQLCellRendererDateTimePrivate *priv;

  priv = GSQL_CELL_RENDERER_DATETIME_GET_PRIVATE (obj);

  priv->test = NULL;
}

static void
gsql_cell_renderer_datetime_finalize (GObject *obj)
{
  GSQLCellRendererDateTimePrivate *priv;

  priv = GSQL_CELL_RENDERER_DATETIME_GET_PRIVATE (obj);

  if (priv && priv->test)
    g_free (priv->test);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (* G_OBJECT_CLASS (parent_class)->finalize) (obj);

}

static gpointer
gsql_type_datetime_copy (GSQLTypeDateTime *dt)
{
	GSQLTypeDateTime *new_dt;
	
	g_return_val_if_fail (dt != NULL, NULL);
	new_dt = g_new0(GSQLTypeDateTime, 1);
	
	memcpy ((void *) new_dt, (void *) dt, sizeof (GSQLTypeDateTime));
	
	return new_dt;
}

static void
gsql_type_datetime_free (GSQLTypeDateTime *dt)
{
	g_return_if_fail (dt != NULL);
	g_free (dt);
	
	return;
}

static void
gsql_datetime_format_property_set (gpointer pt)
{
	GSQL_TRACE_FUNC;
	
	gchar *c_value;
	gint notstop = DATETIME_MAX_LEN;
	gchar *c;
	dmyhms v;
	gboolean zeropref;
	
	if (dt_format)
		g_free (dt_format);
	
	dt_format = gsql_conf_value_get_string (GSQL_CONF_DATETIME_FORMAT);
	
	if (!dt_format)
		dt_format = g_strdup (dt_format_default);

	
	v.b = 0;
	c = dt_format;
	
	while ((*c) && (notstop))
	{
		if (*c == '%')
		{
			c++;
			if (*c == '0')
					c++;
			
			if (!*c)
			{
				v.b = 0;
				break;
			}
					
			switch (*c)
			{
				case 'D':
					v.datetime.D = !v.datetime.D;
					break;
					
				case 'M':
					v.datetime.M = !v.datetime.M;
					break;
					
				case 'Y':
					v.datetime.Y = !v.datetime.Y;
					break;
					
				case 'h':
					v.datetime.h = !v.datetime.h;
					break;
					
				case 'm':
					v.datetime.m = !v.datetime.m;
					break;
					
				case 's':
					v.datetime.s = !v.datetime.s;
					break;
					
				case '%':
					break;
					
				case 0:
					c--;
					break;

				default:
					notstop = 1;
					v.b = 0;
			};
		};
		c++; notstop--;
	};
	
	if (v.b != 0x3f)
	{
		
		GSQL_DEBUG("format is incorrect");
		g_free (dt_format);
		dt_format = g_strdup(dt_format_default);
	}
	
	if (dt_format_parsed)
		g_free(dt_format_parsed);
	
	dt_format_parsed = gsql_datetime_format_parse();
	
	GSQL_DEBUG ("format: %s", dt_format);
	GSQL_DEBUG ("format parsed: %s", dt_format_parsed);

}

static gchar *
gsql_datetime_format_parse (void)
{
	GSQL_TRACE_FUNC;
	
	gint notstop = DATETIME_MAX_LEN;
	gchar *c = dt_format;
	gboolean zeropref;
	gchar parsed[DATETIME_MAX_LEN];
	gchar *d = parsed;
	gint i = 0;
	
	memset (parsed, 0, DATETIME_MAX_LEN);
	
	while ((*c) && (notstop))
	{
		if (*c == '%')
		{
			*d = *c;
			c++;d++;
			
			if (*c == '0')
			{
				*d = *c;
				c++;d++;
				*d = '2';
				d++;
			}
			
			switch (*c)
			{
				case 'D':
					dt_offset[i] = offsetof(GSQLTypeDateTime, day);
					break;
					
				case 'M':
					dt_offset[i] = offsetof(GSQLTypeDateTime, mon);
					break;
					
				case 'Y':
					dt_offset[i] = offsetof(GSQLTypeDateTime, year);
					break;
					
				case 'h':
					dt_offset[i] = offsetof(GSQLTypeDateTime, hour);
					break;
				case 'm':
					dt_offset[i] = offsetof(GSQLTypeDateTime, min);
					break;
					
				case 's':
					dt_offset[i] = offsetof(GSQLTypeDateTime, sec);
					break;
					
				
			}
			i++;
			
			if (*c =='%')
				*d = *c;
			else
				*d = 'd';
			d++; c++; notstop--;
			
		} else {
			*d = *c;
			c++; d++; notstop--;
		}
	}
	
	return g_strdup(parsed);
}




