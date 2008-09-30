/***************************************************************************
 *            stock.h
 *
 *  Fri Sep  7 01:43:47 2007
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
 
#ifndef _STOCK_H
#define _STOCK_H

#include <glib.h>
#include <gtk/gtk.h>


#define GSQL_STOCK_ALL_SCHEMAS			"gsql-all-schemas"
#define GSQL_STOCK_ARGUMENTS			"gsql-arguments"
#define GSQL_STOCK_CLOSE				"gsql-close"
#define GSQL_STOCK_COLUMNS				"gsql-columns"
#define GSQL_STOCK_CONSTRAINT			"gsql-constraints"
#define GSQL_STOCK_CONSTRAINT_P			"gsql-constraints-p"
#define GSQL_STOCK_CONSTRAINT_F			"gsql-constraints-f"
#define GSQL_STOCK_CONSTRAINT_U			"gsql-constraints-u"
#define GSQL_STOCK_CONSTRAINT_C			"gsql-constraints-c"
#define GSQL_STOCK_FIND					"gsql-find"
#define GSQL_STOCK_FUNCTIONS			"gsql-functions"
#define GSQL_STOCK_GSQL					"gsql-gsql"
#define GSQL_STOCK_INDEXES				"gsql-indexes"
#define GSQL_STOCK_LOGO					"gsql-logo"
#define GSQL_STOCK_MOUTPUT				"gsql-msg-output"
#define GSQL_STOCK_MERROR				"gsql-msg-error"
#define GSQL_STOCK_MNORMAL				"gsql-msg-normal"
#define GSQL_STOCK_MNOTICE				"gsql-msg-notice"
#define GSQL_STOCK_MWARNING				"gsql-msg-warning"
#define GSQL_STOCK_MY_SCHEMA			"gsql-my-schema"
#define GSQL_STOCK_OBJ_CLONE			"gsql-object-clone"
#define GSQL_STOCK_OBJ_CODE				"gsql-object-code"
#define GSQL_STOCK_OBJ_NEW				"gsql-object-new"
#define GSQL_STOCK_OBJ_REVERT			"gsql-object-revert"
#define GSQL_STOCK_OBJ_SAVE				"gsql-object-save"
#define GSQL_STOCK_OBJ_SQL				"gsql-object-sql"
#define GSQL_STOCK_PROCEDURES			"gsql-procedures"
#define GSQL_STOCK_PRIVILEGES			"gsql-privileges"
#define GSQL_STOCK_SEQUENCES			"gsql-sequences"
#define GSQL_STOCK_SESSION_CLOSE		"gsql-session-close"
#define GSQL_STOCK_SESSION_COMMIT		"gsql-session-commit"
#define GSQL_STOCK_SESSION_NEW			"gsql-session-new"
#define GSQL_STOCK_SESSION_ROLLBACK		"gsql-session-rollback"
#define GSQL_STOCK_SQL_FETCH_ALL		"gsql-sql-fetch-all"
#define GSQL_STOCK_SQL_FETCH_NEXT		"gsql-sql-fetch-next"
#define GSQL_STOCK_SQL_RUN_AT_CURSOR	"gsql-sql-run-at-cursor"
#define GSQL_STOCK_SQL_RUN				"gsql-sql-run"
#define GSQL_STOCK_SQL_RUN_STEP			"gsql-sql-run-step"
#define GSQL_STOCK_SQL_SHOW_HIDE		"gsql-sql-show-hide"
#define GSQL_STOCK_SQL_STOP_ON_ERR		"gsql-sql-stop-on-err"
#define GSQL_STOCK_SQL_STOP				"gsql-sql-stop"
#define GSQL_STOCK_SQL_UPDATEABLE		"gsql-sql-updateable"
#define GSQL_STOCK_TABLES				"gsql-tables"
#define GSQL_STOCK_TRIGGERS				"gsql-triggers"
#define GSQL_STOCK_UNKNOWN				"gsql-unknown"
#define GSQL_STOCK_USERS				"gsql-users"
#define GSQL_STOCK_VIEWS				"gsql-views"




typedef struct _GSQLStockIcon GSQLStockIcon;

struct _GSQLStockIcon
{
	const char *name;
	const char *file;
};

G_BEGIN_DECLS

void
gsql_stock_init ();

void
gsql_factory_add (GSQLStockIcon *stock_icons, gint n_elements);

G_END_DECLS

#endif /* _STOCK_H */

 
