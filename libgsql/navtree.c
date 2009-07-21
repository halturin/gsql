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


#include <navtree.h>
#include <gtk/gtkbuildable.h>


struct _GSQLNavTreePrivate {
	gboolean reserved;
};

typedef struct {
	GtkBuilder	*builder;
	GObject		*object;
	GValue		*values;
} SubParserData;

static void gsql_navtree_class_init (GSQLNavTreeClass *klass);
static void gsql_navtree_init 		(GSQLNavTree *obj);
static void gsql_navtree_finalize 	(GObject *obj);
static void gsql_navtree_dispose 	(GObject *obj);

static void gsql_navtree_buildable_interface_init	(GtkBuildableIface *iface);

static gboolean gsql_navtree_buildable_custom_tag_start	(GtkBuildable     *buildable,
							GtkBuilder       *builder,
							GObject          *child,
							const gchar      *tagname,
							GMarkupParser    *parser,
							gpointer         *data);

static void gsql_navtree_buildable_custom_tag_end	(GtkBuildable     *buildable,
							GtkBuilder       *builder,
							GObject          *child,
							const gchar      *tagname,
							gpointer		  user_data);


static GtkBuildableIface *buildable_parent_iface = NULL;

static GObjectClass *parent_class;

G_DEFINE_TYPE_WITH_CODE (GSQLNavTree, gsql_navtree, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                gsql_navtree_buildable_interface_init))



/*GType
gsql_navtree_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLNavTreeClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_navtree_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLNavTree),
			0,
			(GInstanceInitFunc) gsql_navtree_init,
			NULL
		};
		obj_type = g_type_register_static (G_TYPE_OBJECT,
										   "GSQLNavTree", &obj_info, 0);
		
	}
	
	return obj_type;	
}*/



GSQLNavTree *
gsql_navtree_new ()
{

	GSQLNavTree *navtree;
	
	
	navtree = g_object_new (GSQL_NAVTREE_TYPE, NULL);	
	
	return navtree;
}



/*
 *  Static section:
 *
 *  gsql_navtree_init
 *  gsql_navtree_class_init
 *  gsql_navtree_finalize
 *	gsql_navtree_buildable_interface_init
 *
 *
 */

static void
gsql_navtree_finalize (GObject *obj)
{
	GSQLNavTree *navtree = GSQL_NAVTREE (obj);

	g_free (navtree->private);

	G_OBJECT_CLASS (gsql_navtree_parent_class)->finalize (obj);
}

static void
gsql_navtree_class_init (GSQLNavTreeClass *klass)
{
	GObjectClass *obj_class;
	
	g_return_if_fail (klass != NULL);
	obj_class = (GObjectClass *) klass;
	
	obj_class->finalize = gsql_navtree_finalize;
	
}

static void 
gsql_navtree_init (GSQLNavTree *obj)
{
	g_return_if_fail (obj != NULL);
	
	obj->private = g_new0 (GSQLNavTreePrivate, 1);
}

static void
navtree_parse_data_start (GMarkupParseContext *context,
                    const gchar         *element_name,
                    const gchar        **names,
                    const gchar        **values,
                    gpointer             user_data,
                    GError             **error)
{
	SubParserData *data = (SubParserData*)user_data;

	int i;

		g_debug ("element \"\"%s\"\" started", element_name);

		for (i = 0; names[i]; i++)
			g_debug ("\tattr: %s", names[i]);

}

static void
navtree_parse_data_end (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	SubParserData *data = (SubParserData*)user_data;

	g_debug ("element \"\"%s\"\" ended", element_name);
}


static void
navtree_parse_data_text (GMarkupParseContext *context,
                         const gchar         *text,
                         gsize                text_len,
                         gpointer             user_data,
                         GError             **error)
{
	SubParserData *data = (SubParserData*)user_data;

}

static const GMarkupParser navtree_data_parser = 
{
	navtree_parse_data_start,
	navtree_parse_data_end,
	navtree_parse_data_text
};

static void 
gsql_navtree_buildable_interface_init	(GtkBuildableIface *iface)
{
	iface->custom_tag_start = gsql_navtree_buildable_custom_tag_start;
	iface->custom_tag_end = gsql_navtree_buildable_custom_tag_end;
}

static gboolean 
gsql_navtree_buildable_custom_tag_start	(GtkBuildable     *buildable,
							GtkBuilder       *builder,
							GObject          *child,
							const gchar      *tagname,
							GMarkupParser    *parser,
							gpointer         *data)
{
	SubParserData *parser_data;

	
	
	if (strcmp (tagname, "navitem") == 0)
	{
		g_debug ("start parse custom tag NAVITEM");
		
		parser_data = g_slice_new0 (SubParserData);
		parser_data->builder = builder;
		parser_data->object = G_OBJECT(buildable);

		*parser = navtree_data_parser;
		*data = parser_data;

		return TRUE;

	} else	
		g_warning ("Unknown custom tag: %s", tagname);
		


	return FALSE;
}

static void 
gsql_navtree_buildable_custom_tag_end	(GtkBuildable     *buildable,
							GtkBuilder       *builder,
							GObject          *child,
							const gchar      *tagname,
							gpointer		  user_data)
{
	
	g_debug ("end parse custom tag: %s", tagname);
}
