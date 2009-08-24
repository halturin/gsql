# -*- coding: utf-8 -*-


"""
 * navstock.py
 *
 * GSQL Navigation Helper (is a part of GSQL)
 *
 * Copyright (C) 2009  Taras Halturin  halturin@gmail.com
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

"""

import sys
import os
import gobject
import gtk



stock_dict =  { 'gsql-all-schemas' 			 :'./pixmaps/all.schemas.png' ,
				'gsql-arguments' 			 :'./pixmaps/arguments.png' ,
				'gsql-close' 				 :'./pixmaps/close.png' ,
				'gsql-columns' 				 :'./pixmaps/columns.png' ,
				'gsql-constraints' 			 :'./pixmaps/constraints.png' ,
				'gsql-constraints-p' 		 :'./pixmaps/constraints.primary.png' ,
				'gsql-constraints-f' 		 :'./pixmaps/constraints.foreign.png' ,
				'gsql-constraints-u' 		 :'./pixmaps/constraints.unique.png' ,
				'gsql-constraints-c' 		 :'./pixmaps/constraints.check.png' ,
				'gsql-find' 				 :'./pixmaps/find.png' ,
				'gsql-functions' 			 :'./pixmaps/functions.png' ,
				'gsql-indexes' 				 :'./pixmaps/indexes.png' ,
				'gsql-my-schema' 			 :'./pixmaps/my.schema.png' ,
				'gsql-procedures' 			 :'./pixmaps/procedures.png' ,
				'gsql-privileges' 			 :'./pixmaps/privileges.png' ,
				'gsql-sequences' 			 :'./pixmaps/sequences.png' ,
				'gsql-tables' 				 :'./pixmaps/tables.png' ,
				'gsql-triggers' 			 :'./pixmaps/triggers.png' ,
				'gsql-unknown' 				 :'./pixmaps/unknown.png' ,
				'gsql-users' 				 :'./pixmaps/users.png' ,
				'gsql-views' 				 :'./pixmaps/views.png' }
				
def icon_store_init(self, store):
	
	
	keys = stock_dict.keys()
	keys.sort()
	
	for i in keys:
		stock_image = i
		file_name = stock_dict[i]
		stock_name = str(i).replace('gsql-','GSQL_STOCK_').upper().replace('-','_')
		
		pixbuf = gtk.gdk.pixbuf_new_from_file(file_name)
		iconset = gtk.IconSet(pixbuf)
		self.icon_factory.add(stock_image, iconset)
		
		store.append([stock_name, pixbuf])


