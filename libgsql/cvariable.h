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
 
#ifndef _CVARIABLE_H
#define _CVARIABLE_H

#include <glib.h>
#include <glib-object.h>

typedef struct _GSQLVariable GSQLVariable;
typedef struct _GSQLVariableClass GSQLVariableClass;
typedef struct _GSQLVariablePrivate GSQLVariablePrivate;

#define GSQL_VARIABLE_TYPE 			(gsql_variable_get_type ())
#define GSQL_VARIABLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQL_VARIABLE_TYPE, GSQLVariable))
#define GSQL_VARIABLE_CLASS(klass)	(G_TYPE_CHECK_INSTANCE_CAST ((klass), GSQL_VARIABLE_TYPE, GSQLVariableClass))

#define GSQL_IS_VARIABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQL_VARIABLE_TYPE))
#define GSQL_IS_VARIABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSQL_VARIABLE_TYPE))


struct _GSQLVariable
{
	GObject parent;
	gchar   *field_name;
	
	GType   value_type;
	gulong  value_length;
	
	gboolean is_array;
	gulong   array_length;
	
	gpointer value;
	
	/* There is pointer to a raw data. For example, casting to GSQL_TYPE_DATETIME */
	/* from the original RDBMS type. */
	gpointer raw;
	void	 (*raw_to_value) (GSQLVariable *variable);
	
	gpointer spec;
	
	GSQLVariablePrivate *private;
};

struct _GSQLVariableClass
{
	GObjectClass parent;
	
	/* Signals */
	void (*on_free) (GSQLVariable *cursor);
};

G_BEGIN_DECLS

GType gsql_variable_get_type (void);

GSQLVariable *
gsql_variable_new (void);

G_END_DECLS

#endif /* _CVARIABLE_H */

