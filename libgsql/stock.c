/***************************************************************************
 *            stock.c
 *
 *  Fri Sep  7 01:43:35 2007
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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgsql/stock.h>
#include <libgsql/common.h>

static GSQLStockIcon stock_icons[] = 
{
	{ GSQL_STOCK_ALL_SCHEMAS,			"all.schemas.png" },
	{ GSQL_STOCK_ARGUMENTS,				"arguments.png" },
	{ GSQL_STOCK_CLOSE,					"close.png" },
	{ GSQL_STOCK_COLUMNS,				"columns.png" },
	{ GSQL_STOCK_CONSTRAINT,			"constraints.png" },
	{ GSQL_STOCK_CONSTRAINT_P,			"constraints.primary.png" },
	{ GSQL_STOCK_CONSTRAINT_F,			"constraints.foreign.png" },
	{ GSQL_STOCK_CONSTRAINT_U,			"constraints.unique.png" },
	{ GSQL_STOCK_CONSTRAINT_C,			"constraints.check.png" },
	{ GSQL_STOCK_FIND,					"find.png" },
	{ GSQL_STOCK_FUNCTIONS,				"functions.png" },
	{ GSQL_STOCK_GSQL,					"gsql.png" },
	{ GSQL_STOCK_INDEXES,				"indexes.png" },
	{ GSQL_STOCK_LOGO,					"logo.png" },
	{ GSQL_STOCK_MOUTPUT,				"msg_dboutput.png" },
	{ GSQL_STOCK_MERROR,				"msg_error.png" },
	{ GSQL_STOCK_MNORMAL,				"msg_normal.png" },
	{ GSQL_STOCK_MNOTICE,				"msg_notice.png" },
	{ GSQL_STOCK_MWARNING,				"msg_warning.png" },
	{ GSQL_STOCK_MY_SCHEMA,				"my.schema.png" },
	{ GSQL_STOCK_OBJ_CLONE,				"object_clone.png" },
	{ GSQL_STOCK_OBJ_CODE,				"object_code.png" },
	{ GSQL_STOCK_OBJ_NEW,				"object_new.png" },
	{ GSQL_STOCK_OBJ_REVERT,			"object_revert.png" },
	{ GSQL_STOCK_OBJ_SAVE,				"object_save.png" },
	{ GSQL_STOCK_OBJ_SQL,				"object_sql.png" },
	{ GSQL_STOCK_PROCEDURES,			"procedures.png" },
	{ GSQL_STOCK_PRIVILEGES,			"privileges.png" },
	{ GSQL_STOCK_SEQUENCES,				"sequences.png" },
	{ GSQL_STOCK_SESSION_CLOSE,			"session_close.png" },
	{ GSQL_STOCK_SESSION_COMMIT,		"session_commit.png" },
	{ GSQL_STOCK_SESSION_NEW,			"session_new.png" },
	{ GSQL_STOCK_SESSION_ROLLBACK,		"session_rollback.png" },
	{ GSQL_STOCK_SQL_FETCH_ALL,			"sql_fetch_all.png" },
	{ GSQL_STOCK_SQL_FETCH_NEXT,		"sql_fetch_next.png" },
	{ GSQL_STOCK_SQL_RUN_AT_CURSOR,		"sql_run_at_cursor.png" },
	{ GSQL_STOCK_SQL_RUN,				"sql_run.png" },
	{ GSQL_STOCK_SQL_RUN_STEP,			"sql_run_step.png" },
	{ GSQL_STOCK_SQL_SHOW_HIDE,			"sql_showhide_result.png" },
	{ GSQL_STOCK_SQL_STOP_ON_ERR,		"sql_stop_onerror.png" },
	{ GSQL_STOCK_SQL_STOP,				"sql_stop.png" },
	{ GSQL_STOCK_SQL_UPDATEABLE,		"sql_updateable.png" },
	{ GSQL_STOCK_TABLES,				"tables.png" },
	{ GSQL_STOCK_TRIGGERS,				"triggers.png" },
	{ GSQL_STOCK_UNKNOWN,				"unknown.png" },
	{ GSQL_STOCK_USERS,					"users.png" },
	{ GSQL_STOCK_VIEWS,					"views.png" }
};

static const GtkStockItem stock_items[] =
{
	{ GSQL_STOCK_SESSION_CLOSE, N_("Close session"), 0, 0, NULL },
	{ GSQL_STOCK_SESSION_NEW, N_("New session"), 0, 0, NULL },
	{ GSQL_STOCK_SESSION_COMMIT, N_("Commit"), 0, 0, NULL },
	{ GSQL_STOCK_SESSION_ROLLBACK, N_("Rollback"), 0, 0, NULL }
};

static GtkIconFactory *gsql_icon_factory;

void
gsql_stock_init ()
{
	GSQL_TRACE_FUNC

	gsql_icon_factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(gsql_icon_factory);
	
	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));
	gtk_stock_add_static(stock_items, G_N_ELEMENTS(stock_items));
	
}

void
gsql_factory_add (GSQLStockIcon *stock_icons, gint n_elements)
{
	GSQL_TRACE_FUNC

	gint i;
	GdkPixbuf *pixbuf;
	GtkIconSet *iconset;
	
	g_return_if_fail(stock_icons != NULL);
	g_return_if_fail(n_elements > 0);
	
	for (i = 0; i < n_elements; i++)
	{
		pixbuf = create_pixbuf (stock_icons[i].file);
		if (pixbuf)
		{	
			iconset = gtk_icon_set_new_from_pixbuf (pixbuf);
			
			g_object_unref(G_OBJECT(pixbuf));
			pixbuf = NULL;
			
		} else {
			printf ("can't open %s\n",stock_icons[i].file) ;
		}
		
		if (iconset)
		{
			gtk_icon_factory_add(gsql_icon_factory, stock_icons[i].name, iconset);
			
			gtk_icon_set_unref(iconset);
			iconset = NULL;
		}
	}	
}

	
