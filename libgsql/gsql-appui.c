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

/**
 * SECTION:gsql-appui
 * @short_description: GSQL's interface manager
 * @see_also: #GtkUIManager, #GtkActionEntry, #GtkActionGroup
 *
 * #GSQLAppUI is subclass of #GtkUIManager
 *
 * <note>
 * 		<para>
 *			You shouln'd use #GtkUIManager directly for additionals/removals.
 *			Try to use #GSQLAppUI methods whenever possible.
 *		</para>
 * </note>
 */


#include <libgsql/common.h>
#include <libgsql/gsql-appui.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

struct _GSQLAppUIPrivate
{
	GHashTable		*action_groups;
	GtkIconFactory	*icon_factory;
};

G_DEFINE_TYPE (GSQLAppUI, gsql_appui, GTK_TYPE_UI_MANAGER)

static void
gsql_appui_dispose (GObject *obj)
{
	GSQLAppUI *appui = GSQL_APPUI (obj);
	GList *keys, *k;
	
	k = keys = g_hash_table_get_keys (appui->private->action_groups);
	while (keys)
	{
		gsql_appui_remove_action_group_n (appui, keys->data);

		keys = g_list_next (keys);
	}

	g_list_free (k);
	
	G_OBJECT_CLASS (gsql_appui_parent_class)->dispose (obj);
}


static void 
gsql_appui_finalize (GObject *obj)
{
	GSQLAppUI *appui = GSQL_APPUI (obj);

	g_free (appui->private);
	
	G_OBJECT_CLASS (gsql_appui_parent_class)->finalize (obj);
}

static void
gsql_appui_class_init (GSQLAppUIClass *class)
{
	GSQL_TRACE_FUNC

	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->dispose = gsql_appui_dispose;
	object_class->finalize = gsql_appui_finalize;
}

static void
gsql_appui_init (GSQLAppUI *appui)
{
	GSQL_TRACE_FUNC

	appui->private = g_new0 (GSQLAppUIPrivate, 1);

	appui->private->action_groups = 
		g_hash_table_new_full (g_str_hash,
	    								g_str_equal,
		    							(GDestroyNotify) g_free,
		    							NULL);

	appui->private->icon_factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (appui->private->icon_factory);

}

static gboolean
on_action_group_remove (gpointer key, gpointer value, gpointer userdata)
{
	if (userdata == value)
		return TRUE;
	else 
		return FALSE;
}

/**
 * gsql_appui_new:
 *
 * Creates a new instance of #GSQLAppUI
 *
 * Return value: A #GSQLAppUI object
 */
GSQLAppUI *
gsql_appui_new (void)
{
	GSQL_TRACE_FUNC

	return g_object_new (GSQL_TYPE_APPUI, NULL);

}

/**
 * gsql_appui_add_action_group:
 *
 * @appui: A #GSQLAppUI object
 * @group_name: name of action group
 * @group: #GtkActionGroup object
 *
 * desc. FIXME.
 */
void
gsql_appui_add_action_group (GSQLAppUI *appui, const gchar *group_name,
    								GtkActionGroup *group)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	g_return_if_fail (group_name != NULL);
	g_return_if_fail (GTK_IS_ACTION_GROUP (group));

	gtk_ui_manager_insert_action_group (GTK_UI_MANAGER (appui), group, 0);

	g_hash_table_insert (appui->private->action_groups,
	    					g_strdup (group_name), group);
}


/**
 * gsql_appui_add_actions:
 *
 * @appui: A #GSQLAppUI object
 * @group_name: name of action group
 * @entries: array of #GtkActionEntry
 * @n_entries: number of @entries. (G_N_ELEMENTS (@group))
 * @user_data: 
 *
 * desc. FIXME.
 */
void
gsql_appui_add_actions (GSQLAppUI *appui,
    					const gchar *group_name,
    					GtkActionEntry *entries,
    					gint n_entries,
    					gpointer user_data)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	g_return_if_fail (group_name != NULL);

	GtkActionGroup *group;
	
	group = gtk_action_group_new (group_name);

	gtk_action_group_add_actions (group,
	    						  (GtkActionEntry *) entries,
	    						  n_entries, user_data);

	gsql_appui_add_action_group (appui, group_name, group);
}

/**
 * gsql_appui_add_toggle_actions:
 *
 * @appui: A #GSQLAppUI object
 * @group_name: name of action group
 * @entries: array of #GtkToggleActionEntry
 * @n_entries: number of @entries. (G_N_ELEMENTS (@group))
 * @user_data: 
 *
 * desc. FIXME.
 */
void
gsql_appui_add_toggle_actions (GSQLAppUI *appui,
    							const gchar *group_name,
    							GtkToggleActionEntry *entries,
    							gint n_entries,
    							gpointer user_data)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	g_return_if_fail (group_name != NULL);

	GtkActionGroup *group;
	
	group = gtk_action_group_new (group_name);

	gtk_action_group_add_toggle_actions (group,
	    						  (GtkToggleActionEntry *) entries,
	    						  n_entries, user_data);

	gsql_appui_add_action_group (appui, group_name, group);
}


/**
 * gsql_appui_remove_action_group:
 *
 * @appui: A #GSQLAppUI object
 * @group: #GtkActionGroup object
 *
 * desc. FIXME.
 */
void
gsql_appui_remove_action_group (GSQLAppUI *appui, 
    								GtkActionGroup *group)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	g_return_if_fail (GTK_IS_ACTION_GROUP (group));

	gtk_ui_manager_remove_action_group (GTK_UI_MANAGER (appui), group);

	g_hash_table_foreach_remove (appui->private->action_groups,
	    							on_action_group_remove, group);
}

/**
 * gsql_appui_remove_action_group_n:
 *
 * @appui: A #GSQLAppUI object
 * @group_name: name of action group
 *
 * Remove the action group by name. FIXME.
 */
void
gsql_appui_remove_action_group_n (GSQLAppUI *appui, 
    								const gchar *group_name)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	
	GtkActionGroup *group = 
			g_hash_table_lookup (appui->private->action_groups, group_name);

	g_return_if_fail (GTK_IS_ACTION_GROUP (group));

	gtk_ui_manager_remove_action_group (GTK_UI_MANAGER (appui), group);

	g_hash_table_remove (appui->private->action_groups, group_name);
}

/**
 * gsql_appui_get_action:
 *
 * @appui: a #GSQLAppUI object
 * @group_name: the name of group you would like find
 * @action_name: the action name in the group
 *
 * Return value: a #GtkAction object
 *
 * desc. FIXME.
 */
GtkAction *
gsql_appui_get_action (GSQLAppUI *appui, const gchar *group_name,
    								const gchar *action_name)
{
	GtkActionGroup *group;
	GtkAction *action;

	g_return_if_fail (GSQL_IS_APPUI (appui));

	group = g_hash_table_lookup (appui->private->action_groups,
	    							group_name);

	if (!group)
	{
		GSQL_DEBUG ("Unable to find action group [%s]", group_name);

		return NULL;
	}

	action = gtk_action_group_get_action (group, action_name);

	if (GTK_IS_ACTION (action))
		return action;

	GSQL_DEBUG ("Unable to find action [%s] in group [%s]", 
	    				action_name, group_name);

	return NULL;
}

/**
 * gsql_appui_get_widget:
 *
 * @appui: a #GSQLAppUI object
 * @widget_name: the name of widget you would like find
 *
 * Return value: a #GtkWidget object or NULL if no widget was found
 *
 */
GtkWidget *
gsql_appui_get_widget (GSQLAppUI *appui, const gchar *widget_name)
{
	GSQL_TRACE_FUNC

	g_return_val_if_fail (GSQL_IS_APPUI (appui), NULL);

	return gtk_ui_manager_get_widget (GTK_UI_MANAGER (appui), widget_name);
}

/**
 * gsql_appui_merge:
 * @appui: a #GSQLAppUI object
 * filename: the file you would like to merge to the UI manager.
 *
 * Return value: merged ID. On error returns -1
 */
gint
gsql_appui_merge (GSQLAppUI *appui, const gchar *filename)
{
	GSQL_TRACE_FUNC

	g_return_val_if_fail (GSQL_IS_APPUI (appui), -1);

	gint id;
	GError *error = NULL;
	
	id = gtk_ui_manager_add_ui_from_file (GTK_UI_MANAGER (appui), 
	    									filename, &error);

	if (error)
	{
		GSQL_DEBUG ("Couldn't merge file [%s]: %s", filename, error->message);

		return -1;
	}

	return id;
}

/**
 * gsql_appui_unmerge:
 *
 * @appui: a #GSQLAppUI object
 * @id: the ID returned by 'gsql_appui_merge(..)'
 *
 */
void
gsql_appui_unmerge (GSQLAppUI *appui, gint id)
{
	GSQL_TRACE_FUNC

	g_return_if_fail (GSQL_IS_APPUI (appui));
	
	gtk_ui_manager_remove_ui(GTK_UI_MANAGER (appui), id);

}

/** gsql_appui_factory_add:
 *
 * @appui: a #GSQLAppUI object
 * @stock_icons: pointer to the array of #GSQLStockIcon
 * @n_elements: number of elements of the @stock_icons
 *
 * Example usage:
 * |[
 *		GSQLStockIcon stock_icons[] = 
 *		{
 *			{ "gsql-my-icon1",	"myicon1.png" },
 *			{ "gsql-my-icon2",	"myicon2.png" }
 *		};
 *
 *		gsql_appui_factory_add (appui, stock_icons, G_N_ELEMENTS(stock_icons));
 * ]|
 *
 */
void
gsql_appui_factory_add (GSQLAppUI *appui, GSQLStockIcon *stock_icons, 
    												gint n_elements)
{
	GSQL_TRACE_FUNC;

	gint i;
	GdkPixbuf *pixbuf;
	GtkIconSet *iconset;

	g_return_if_fail (GSQL_IS_APPUI (appui));
	g_return_if_fail(stock_icons != NULL);
	g_return_if_fail(n_elements > 0);
	
	for (i = 0; i < n_elements; i++)
	{
		pixbuf = gsql_create_pixbuf (stock_icons[i].file);
		
		if (pixbuf)
		{	
			iconset = gtk_icon_set_new_from_pixbuf (pixbuf);
			
			g_object_unref(G_OBJECT(pixbuf));
			pixbuf = NULL;
			
		} else {
			
			GSQL_DEBUG ("Unable to open %s\n",stock_icons[i].file) ;
		}
		
		if (iconset)
		{
			gtk_icon_factory_add(appui->private->icon_factory, 
			    					stock_icons[i].name, iconset);
			
			gtk_icon_set_unref(iconset);
			iconset = NULL;
		}
	}	
}
