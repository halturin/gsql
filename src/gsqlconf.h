/***************************************************************************
 *            gsqlconf.h
 *
 *  Mon Oct 16 18:46:39 2006
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
 
#ifndef _GSQLCONF_H
#define _GSQLCONF_H

#include <glade/glade.h>
#include <libgsql/common.h>


#define GSQL_CONF_UI_RESTORE_SIZE_POS		GSQL_CONF_ROOT_KEY "/ui/resotre_size_and_position"
#define GSQL_CONF_UI_SIZE_X					GSQL_CONF_ROOT_KEY "/ui/window_size_x"
#define GSQL_CONF_UI_SIZE_Y					GSQL_CONF_ROOT_KEY "/ui/window_size_y"
#define GSQL_CONF_UI_POS_X					GSQL_CONF_ROOT_KEY "/ui/window_pos_x"
#define GSQL_CONF_UI_POS_Y					GSQL_CONF_ROOT_KEY "/ui/window_pos_y"
#define GSQL_CONF_UI_MAXIMIZED				GSQL_CONF_ROOT_KEY "/ui/window_maximized"

G_BEGIN_DECLS

void
gsql_conf_init();
    
void
gsql_conf_dialog();

G_END_DECLS

#endif /* _GSQLCONF_H */
