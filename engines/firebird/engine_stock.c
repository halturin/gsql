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



#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgsql/stock.h>
#include <libgsql/common.h>

#include "engine_stock.h"

static GSQLStockIcon stock_icons[] = {
	{ GSQLE_FIREBIRD_STOCK_FIREBIRD,		 "firebird.png" },
	{ GSQLE_FIREBIRD_STOCK_EXCEPTIONS,	 	 "exception.png" },
};

void
engine_stock_init ()
{
	GSQL_TRACE_FUNC;

	gsql_factory_add (stock_icons, G_N_ELEMENTS(stock_icons));
	return;
}
