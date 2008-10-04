/***************************************************************************
 *            sqleditor.h
 *
 *  Copyright  2008  Taras Halturin
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
 
#ifndef _SQLEDITOR_H
#define _SQLEDITOR_H

#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/session.h>


typedef struct _GSQLEditor			GSQLEditor;
typedef struct _GSQLEditorPrivate	GSQLEditorPrivate;
typedef struct _GSQLEditorClass		GSQLEditorClass;


#define GSQL_EDITOR_TYPE 			(gsql_editor_get_type ())
#define GSQL_EDITOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_EDITOR_TYPE, GSQLEditor))
#define GSQL_EDITOR_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_EDITOR_TYPE, GSQLEditorClass))

#define GSQL_IS_EDITOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_EDITOR_TYPE))
#define GSQL_IS_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_EDITOR_TYPE))

#define GSQL_CONF_SQL_FETCH_STEP	GSQL_CONF_ROOT_KEY "/sql_fetch_step"
#define GSQL_CONF_SQL_FETCH_MAX		GSQL_CONF_ROOT_KEY "/sql_fetch_max"

#define GSQL_EDITOR_FETCH_STEP_DEFAULT  100
#define GSQL_EDITOR_FETCH_MAX_DEFAULT  10000

typedef GtkActionGroup * (*GSQLEditorFActionCB) (void);

#include <libgsql/common.h>

struct _GSQLEditor
{
	GtkVPaned parent;
	
	GSQLCursor *cursor;
	GSQLEditorPrivate *private;
};


struct _GSQLEditorClass
{
	GtkVPanedClass parent;
};


G_BEGIN_DECLS

GType gsql_editor_get_type (void);

GSQLEditor *
gsql_editor_new (GtkWidget *source);

void
gsql_editor_run_sql (GSQLEditor *editor);

void
gsql_editor_merge_actions (gchar *ui_addons, GtkActionGroup *action_addons);


G_END_DECLS


#endif /* _CONTENT_H */

 
