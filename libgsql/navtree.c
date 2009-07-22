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

enum {
	PROP_0,
	PROP_CHILD_ID,
	PROP_STOCK_NAME,
	PROP_NAME
};

static void gsql_navtree_class_init (GSQLNavTreeClass *klass);

static void gsql_navtree_get_property (GObject	*object,
                                       guint	 prop_id,
                                       GValue	*value,
                                       GParamSpec	*pspec);

static void gsql_navtree_set_property (GObject	*object,
                                       guint	 prop_id,
                                       const GValue	*value,
                                       GParamSpec	*pspec);


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

static void gsql_navtree_buildable_add_child	(GtkBuildable	*buildable,
							GtkBuilder		*builder,
							GObject			*child,
							const gchar		*type);

static GtkBuildableIface *buildable_parent_iface = NULL;

static GObjectClass *parent_class;

G_DEFINE_TYPE_WITH_CODE (GSQLNavTree, gsql_navtree, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                gsql_navtree_buildable_interface_init))


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

	obj_class->get_property = gsql_navtree_get_property;
	obj_class->set_property = gsql_navtree_set_property;
	obj_class->finalize = gsql_navtree_finalize;

	g_object_class_install_property (obj_class,
	                                 PROP_CHILD_ID,
	                                 g_param_spec_uint ("child-id",
	                                                    "Child ID",
	                                                    "Set child id for NavTree item",
	                                                    1, 65535, 1,
	                                                    G_PARAM_READWRITE));

	g_object_class_install_property (obj_class,
	                                 PROP_STOCK_NAME,
	                                 g_param_spec_string ("stock-name",
	                                                    "Stock name",
	                                                    "Set iconset for NavTree item",
	                                                    NULL,
	                                                    G_PARAM_READWRITE));

	g_object_class_install_property (obj_class,
	                                 PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                    "Name",
	                                                    "Set name for NavTree item",
	                                                    NULL,
	                                                    G_PARAM_READWRITE));
	
}

static void 
gsql_navtree_get_property (GObject	*object,
                                       guint	 prop_id,
                                       GValue	*value,
                                       GParamSpec	*pspec)
{
	GSQLNavTree *nt;

	nt = GSQL_NAVTREE (object);
	
	g_debug ("navtree get property");

	switch (prop_id)
	{
		case PROP_CHILD_ID:
			g_debug ("get prop child-id");
			break;

		case PROP_STOCK_NAME:
			g_debug ("get prop stock-name");
			break;

		case PROP_NAME:
			g_debug ("get prop name");
			break;

		default:
			g_debug ("unknown property type: %d", prop_id);

	}

}

static void 
gsql_navtree_set_property (GObject	*object,
                                       guint	 prop_id,
                                       const GValue	*value,
                                       GParamSpec	*pspec)
{
	GSQLNavTree *nt;
	
	nt = GSQL_NAVTREE (object);
	
	g_debug ("navtree set property");

	switch (prop_id)
	{
		case PROP_CHILD_ID:
			g_debug ("set prop child-id");
			break;

		case PROP_STOCK_NAME:
			g_debug ("set prop stock-name");
			break;

		case PROP_NAME:
			g_debug ("set prop name");
			break;

		default:
			g_debug ("unknown property type: %d", prop_id);

	}

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
	iface->add_child = gsql_navtree_buildable_add_child;
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

	
	
//	if (strcmp (tagname, "navitem") == 0)
//	{
		g_debug ("start parse custom tag: %s", tagname);
		
		parser_data = g_slice_new0 (SubParserData);
		parser_data->builder = builder;
		parser_data->object = G_OBJECT(buildable);

		*parser = navtree_data_parser;
		*data = parser_data;

		return TRUE;

//	} else	
//		g_warning ("Unknown custom tag: %s", tagname);
		


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

static void 
gsql_navtree_buildable_add_child	(GtkBuildable	*buildable,
							GtkBuilder		*builder,
							GObject			*child,
							const gchar		*type)
{

	g_debug ("add child (type: %s)", type);

}