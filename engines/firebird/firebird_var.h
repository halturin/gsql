/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2010  Smyatkin Maxim <smyatkinmaxim@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

 
#ifndef FIREBIRD_VAR_H
#define FIREBIRD_VAR_H

#include <ibase.h>
#include <string.h>


#include <libgsql/cvariable.h>

typedef struct _GSQLEFirebirdVariable  GSQLEFirebirdVariable;

typedef struct _Firebird_FIELD Firebird_FIELD;

typedef PARAMVARY VARY2;

struct _Firebird_FIELD {
  gchar *name;
  gint type;
  ISC_QUAD blob_id;
  gint size;
  gshort *ind;
  gshort scale;
  gshort subtype;
};

struct _GSQLEFirebirdVariable 
{
  Firebird_FIELD *field;
  GSQLWorkspace  *workspace;
  isc_db_handle  dbhandle;
  isc_tr_handle  trhandle;
  ISC_QUAD       blob_id;

};

void
firebird_variable_free (GSQLEFirebirdVariable *var);

void
firebird_cursor_show_error (GSQLWorkspace *workspace, ISC_STATUS status[20], gboolean is_sql_error);

//G_END_DECLS

#endif /* FIREBIRD_VAR_H */

#ifdef VMS
#define FB_ALIGN(n,b)              (n)
#endif

#ifdef sun
#ifdef sparc
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif
#endif

#ifdef hpux
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifdef _AIX
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif
 
#if (defined(_MSC_VER) && defined(WIN32)) 
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifndef FB_ALIGN
#define FB_ALIGN(n,b)          ((n+1) & ~1)
#endif

#ifndef ISC_INT64_FORMAT

/* Define a format string for printf.  Printing of 64-bit integers
   is not standard between platforms */

#if (defined(_MSC_VER) && defined(WIN32))
#define	ISC_INT64_FORMAT	"I64"
#else
#define	ISC_INT64_FORMAT	"ll"
#endif
#endif
