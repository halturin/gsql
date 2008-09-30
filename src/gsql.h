/***************************************************************************
 *            gsql.h
 *
 *  Tue Jul 25 08:42:58 2006
 *  Copyright  2006  Halturin Taras
 *  halturin@gmail.com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef _GSQL_H
#define _GSQL_H

G_BEGIN_DECLS

void 
gsql_window_create (void);

GtkWidget *
create_dialog_logon (void);

GtkDialog *
create_dialog_close_session (void);

void
gsql_window_clean_exit();

G_END_DECLS

#endif				/* _GSQL_H */
