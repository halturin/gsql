/***************************************************************************
 *            content.h
 *
 *  Mon Nov  5 23:47:09 2007
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
 
#ifndef _CONTENT_H
#define _CONTENT_H

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _GSQLContent			GSQLContent;
typedef struct _GSQLContentPrivate	GSQLContentPrivate;
typedef struct _GSQLContentClass	GSQLContentClass;


#define GSQL_CONTENT_TYPE 			(gsql_content_get_type ())
#define GSQL_CONTENT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_CONTENT_TYPE, GSQLContent))
#define GSQL_CONTENT_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_CONTENT_TYPE, GSQLContentClass))

#define GSQL_IS_CONTENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_CONTENT_TYPE))
#define GSQL_IS_CONTENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_CONTENT_TYPE))

#include <libgsql/common.h>
#include <libgsql/session.h>


struct _GSQLContent
{
	GtkContainer parent;
	GSQLSession *session;
	
	GSQLContentPrivate *private;
};


struct _GSQLContentClass
{
	GtkContainerClass parent;
	
	/* signals */
	void (*on_changed)	(GSQLContent *content, gboolean status);
	void (*on_close)	(GSQLContent *content, gboolean force);
	
	void (*on_save)		(GSQLContent *content, gboolean save_as);
	void (*on_revert)   (GSQLContent *content);
};




G_BEGIN_DECLS



GType gsql_content_get_type (void);

GSQLContent *
gsql_content_new (GSQLSession *session, const gchar *stock);

void
gsql_content_set_child (GSQLContent *content, GtkWidget *child);

gboolean
gsql_content_get_changed (GSQLContent *content);

void
gsql_content_set_changed (GSQLContent *content, gboolean changed);

gchar *
gsql_content_get_name (GSQLContent *content);

gchar *
gsql_content_get_display_name (GSQLContent *content);

void 
gsql_content_set_name (GSQLContent *content, gchar *name);

void 
gsql_content_set_display_name (GSQLContent *content, gchar *name);

void 
gsql_content_set_name_full (GSQLContent *content, gchar *name, gchar *display_name);

void
gsql_content_set_stock (GSQLContent *content, const gchar *stock);

gchar *
gsql_content_get_stock (GSQLContent *content);

GtkWidget *
gsql_content_get_header_label (GSQLContent *content);

GtkWidget *
gsql_content_get_menu_label (GSQLContent *content);

G_END_DECLS


#endif /* _CONTENT_H */

 
