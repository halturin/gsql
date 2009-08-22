#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
 * naveditor.py
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
import pango
import gtk

import gsql

class NavEditor:
    
    def __init__(self, main_builder, filename):
	
		self.build = gtk.Builder()
		if (self.build.add_from_file('./ui/navtree_editor.xml') == 0):
			print >> sys.stderr, 'Couldn\'t open XML-file  with main window'
		self.window = self.build.get_object('navtree_editor')
	
		action = self.build.get_object('action_add')
		action.connect('activate', self.action_add)
	
		action = self.build.get_object('action_add_sub')
		action.connect('activate', self.action_add_sub)
	
		action = self.build.get_object('action_remove')
		action.connect('activate', self.action_remove)
	
		store = self.build.get_object('nav_liststore')
	
		self.icon_factory = gtk.IconFactory()
		self.icon_factory.add_default()
	
		navstock.icon_store_init(self, store)
		iconstock_combo = self.build.get_object('iconstock_combo')
		iconstock_combo.set_model(store)
		iconstock_combo.set_row_separator_func(utils.is_row_separator)
	
		self.sql_buffer = gtksourceview.SourceBuffer()
		self.sql_buffer.set_highlight(True)
	
		self.sql_editor = gtksourceview.SourceView(self.sql_buffer)
		self.sql_editor.set_show_line_numbers(True)
		self.sql_editor.set_highlight_current_line(True)
		self.sql_editor.modify_font(pango.FontDescription("Monospace 9"))

	
		scroll = self.build.get_object('scroll_sqleditor')
		scroll.add(self.sql_editor)
		scroll.show_all()

		lm = gtksourceview.SourceLanguagesManager()
		self.sql_buffer.set_language(lm.get_language_from_mime_type('text/x-sql'))
		
		self.window.show()

    def main(self):
		gtk.main()
	
    def action_add(self, action):
		print 'action add', action
	
    def action_add_sub(self, action):
		print 'action add sub'
	
    def action_remove(self, action):
		print 'action remove'
	
	
	

