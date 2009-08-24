/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2009  Taras Halturin  halturin@gmail.com
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


enum {
	PROP_0,
	PROP_ID,
	PROP_VERSION,
	PROP_CHILD_ID,
	PROP_STOCK_NAME,
	PROP_NAME
};

enum {
	SIG_ON_OBJ_OPEN,
	SIG_ON_OBJ_EXPAND,
	SIG_ON_OBJ_EVENT,
	SIG_ON_OBJ_POPUP,
	SIG_LAST
};

typedef struct {
	const gchar *sql;

	// list of QueryBindValue
	GList *binds;
} QuerySet;

typedef struct {
	const gchar *name;
	GValue	value;
} QueryBindValue;

typedef struct {
	GtkBuilder	*builder;
	GObject		*object;
	GValue		*values;

	const gchar *attr;
	const gchar *value;

	QuerySet	*qs;

	gboolean is_query;
	
} SubParserData;

static guint navtree_signals[SIG_LAST] = { 0 };

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
							gpointer		 *data);

static void gsql_navtree_buildable_add_child	(GtkBuildable	*buildable,
							GtkBuilder		*builder,
							GObject			*child,
							const gchar		*type);

static void on_navtree_obj_expand_default (GSQLNavTree *navtree);
static void on_navtree_obj_popup_default (GSQLNavTree *navtree);

G_DEFINE_TYPE_WITH_CODE (GSQLNavTree, gsql_navtree, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                gsql_navtree_buildable_interface_init))


GSQLNavTree *
gsql_navtree_new ()
{

	GSQLNavTree *navtree = NULL;
	
	
	navtree = g_object_new (GSQL_NAVTREE_TYPE, NULL);	

	if (!navtree)
	{
		g_warning ("Can not allocate new oject GSQLNavTree");

		return NULL;
	}

	g_signal_connect (G_OBJECT (navtree),
	                  "on-expand",
	                  G_CALLBACK (on_navtree_obj_expand_default),
	                  NULL);

	g_signal_connect (G_OBJECT (navtree),
	                  "on-popup",
	                  G_CALLBACK (on_navtree_obj_popup_default),
	                  NULL);
	
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

	g_hash_table_unref (navtree->queries);

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
	                                 PROP_ID,
	                                 g_param_spec_string ("id",
	                                                    "Object ID",
	                                                    "Set object id for NavTree item",
	                                                    NULL,
	                                                    G_PARAM_READWRITE));
	g_object_class_install_property (obj_class,
	                                 PROP_VERSION,
	                                 g_param_spec_string ("version",
	                                                    "Version",
	                                                    "Able to use for DB version",
	                                                    NULL,
	                                                    G_PARAM_READWRITE));

	g_object_class_install_property (obj_class,
	                                 PROP_CHILD_ID,
	                                 g_param_spec_string ("child-id",
	                                                    "Child ID",
	                                                    "Set child id for NavTree item",
	                                                    NULL,
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
	
	navtree_signals[SIG_ON_OBJ_OPEN] = 
		g_signal_new ("on-open",
		              G_TYPE_FROM_CLASS (obj_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (GSQLNavTreeClass, on_object_open),
		              NULL, // GSignalAccumulator
		              NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	navtree_signals[SIG_ON_OBJ_EXPAND] = 
		g_signal_new ("on-expand",
		              G_TYPE_FROM_CLASS (obj_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (GSQLNavTreeClass, on_object_expand),
		              NULL, // GSignalAccumulator
		              NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	navtree_signals[SIG_ON_OBJ_EVENT] = 
		g_signal_new ("on-event",
		              G_TYPE_FROM_CLASS (obj_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (GSQLNavTreeClass, on_object_event),
		              NULL, // GSignalAccumulator
		              NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	navtree_signals[SIG_ON_OBJ_POPUP] = 
		g_signal_new ("on-popup",
		              G_TYPE_FROM_CLASS (obj_class),
		              G_SIGNAL_RUN_FIRST,
		              G_STRUCT_OFFSET (GSQLNavTreeClass, on_object_popup),
		              NULL, // GSignalAccumulator
		              NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
		              
	
}

static void 
gsql_navtree_get_property (GObject	*object,
                                       guint	 prop_id,
                                       GValue	*value,
                                       GParamSpec	*pspec)
{
	GSQLNavTree *nt;

	nt = GSQL_NAVTREE (object);

	switch (prop_id)
	{
		case PROP_ID:
			g_debug ("get prop id");
			break;

		case PROP_VERSION:
			g_debug ("get prop version");
			break;
			
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
	guint id = 0;
	const gchar *str = NULL;

	switch (prop_id)
	{
		case PROP_CHILD_ID:
		case PROP_ID:

			str = g_value_get_string (value);
			id = GSQL_NAVTREE_GET_ID_BY_NAME (str);

			if (id == 0)
			{
				id = GSQL_NAVTREE_REGISTER_ID (str);
				g_debug ("Have found new object id. registering... [%s:%d]", str, id);
			}

			if (prop_id == PROP_ID)
				nt->id = id;
			else
				nt->child_id = id;
			
			break;

		case PROP_VERSION:

			if (nt->version)
				g_free ((gchar *) nt->version);

			nt->version = g_value_dup_string (value);
			
			break;

		case PROP_STOCK_NAME:
			
			if (nt->stock_name)
				g_free ((gchar *) nt->stock_name);

			nt->stock_name = g_value_dup_string (value);
			
			break;

		case PROP_NAME:

			if (nt->name)
				g_free ((gchar *) nt->name);

			nt->name = g_value_dup_string (value);
			
			break;

		default:
			g_debug ("unknown property type: %d", prop_id);

	}

}

static void
on_query_hashtable_notify (gpointer p)
{

	if (p)
		g_free (p);

}

static void 
gsql_navtree_init (GSQLNavTree *obj)
{
	g_return_if_fail (obj != NULL);

	obj->version = NULL;
	obj->stock_name = NULL;
	obj->name = NULL;
	obj->childs = NULL;

	obj->queries = g_hash_table_new_full (g_str_hash,
	                                      g_str_equal,
	                                      on_query_hashtable_notify,
	                                      on_query_hashtable_notify);

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
	QuerySet *qs;
	QueryBindValue *qbv;

	data->attr = NULL;
	data->value = NULL;

	g_debug ("element \"%s\" started", element_name);

	if (strcmp (element_name, "query") == 0)
	{
		for (i = 0; names[i]; i++)
		{
			if (strcmp (names[i], "version") == 0)
			{

				g_debug ("value: %s", values[i]);
				data->attr = g_strndup (values[i], 15);

			} else 			
				g_warning ("Unsupported tag for GSQLNavTree.query: %s", names[i]);

			data->is_query = TRUE;
			data->qs = g_malloc0 (sizeof(QuerySet));
			
		
		}
	} else if (strcmp (element_name, "bind") == 0) {

		if (!data->is_query)
		{
			g_warning ("Parent element isn't the 'query'. Skipping 'bind' tag");

			return;
		}
		
		for (i = 0; names[i]; i++)
		{
			qbv = g_malloc0 (sizeof(QueryBindValue));
			
			if (strcmp (names[i], "name") == 0)
			{
				g_debug ("bind name: %s", values[i]);
				qbv->name = g_strndup (values[i], 32);

			} else if (strcmp (names[i], "type") == 0) {

				qbv->value = g_malloc0 (sizeof(GValue));

				if (strcmp (values[i], "gint") == 0)
				{
					//= (gint) atol (values[i]);
					qbv->value = g_value_init (qbv->value, G_TYPE_LONG);					

				} else if (strcmp (values[i], "gint64") == 0) {
					//= (gint64) atoll (values[i]);
					qbv->value = g_value_init (qbv->value, G_TYPE_INT64);

				} else if (strcmp (values[i], "gdouble") == 0) {
					//= (gdouble) atof (values[i]);
					qbv->value = g_value_init (qbv->value, G_TYPE_DOUBLE);

				} else {
					// gchar *
					qbv->value = g_value_init (qbv->value, G_TYPE_STRING);
				}

				data->qs->binds = g_list_append (data->qs->binds, qbv);

			} else {
				
				g_warning ("Unsupported tag for GSQLNavTree.query.bind: %s", names[i]);
				g_free (qbv);
			}

		}
	}
		g_warning ("Unsupported element for GSQLNavTree: %s", names[i]);
}

static void
navtree_parse_data_end (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	SubParserData *data = (SubParserData*)user_data;
	GSQLNavTree *nt = GSQL_NAVTREE (data->object);

	if (strcmp (element_name, "query") == 0)

	{
		if (data->value == NULL)
		{
			g_warning ("query tag cannot be empty");

			return;
		}

		// for the default SQL set key="*"
		
		if (data->attr == NULL)
			data->attr = g_strndup ("*", 2);

		
		//g_hash_table_insert (nt->queries,
	      //               	 (gpointer) data->attr, 
		     //                (gpointer) data->value);		

		data->is_query = FALSE;
		data->qs = NULL;
		
	} else if (strcmp (element_name, "bind") == 0) {

		if ((data->value == NULL) || (data->attr == NULL))
		{
			g_warning ("bind name and value cannot be empty");

			return;
		}

		
	} else {

		data->is_query = FALSE;
		data->qs = NULL;

	}

	g_debug ("element \"%s\" ended (queries: %d)", element_name, g_hash_table_size (nt->queries));

}


static void
navtree_parse_data_text (GMarkupParseContext *context,
                         const gchar         *text,
                         gsize                text_len,
                         gpointer             user_data,
                         GError             **error)
{
	SubParserData *data = (SubParserData*)user_data;
	QueryBindValue *qbv;
	
	g_debug ("DATA TEXT");

	if (data->is_query)
	{


	} else {

		data->value = g_strndup (text, text_len);
	}
	g_debug ("element value: %s", text);

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
	
	if (strcmp (tagname, "query") == 0)
	{
		
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
							gpointer		 *data)
{
	
	g_debug ("end parse custom tag: %s", tagname);
}

static void 
gsql_navtree_buildable_add_child	(GtkBuildable	*buildable,
							GtkBuilder		*builder,
							GObject			*child,
							const gchar		*type)
{
	GSQLNavTree *nt = GSQL_NAVTREE(buildable);

	if (GSQL_IS_NAVTREE (child) && GSQL_IS_NAVTREE (buildable))
	{
		nt->childs = g_list_append (nt->childs, child);

	} else
		g_warning ("Buildable types mismatch");

}



static void 
on_navtree_obj_expand_default (GSQLNavTree *navtree)
{
	g_debug ("GSQLNavTree signal: on-expand");
}

static void 
on_navtree_obj_popup_default (GSQLNavTree *navtree)
{
	g_debug ("GSQLNavTree signal: on-popup");

}
                                          