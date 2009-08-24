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

try:
	import gtksourceview
except:
	print >> sys.stderr, 'GktSourceView are required. Quiting...'

import gsql
import navstock

class NavEditor:
    
	def __init__(self, main_builder, filename):
		self.main = main_builder
		self.navtree = gtk.Builder()
		self.navtree.add_from_file(filename)
    
		self.build = gtk.Builder()
		if (self.build.add_from_file('./ui/navtree_editor.xml') == 0):
			print >> sys.stderr, 'Couldn\'t open XML-file  with main window'
		self.widget = self.build.get_object('navtree_editor')
	
		action = self.build.get_object('action_add')
		action.connect('activate', self.action_add)
	
		action = self.build.get_object('action_add_sub')
		action.connect('activate', self.action_add_sub)
	
		action = self.build.get_object('action_remove')
		action.connect('activate', self.action_remove)
		
		action = self.build.get_object('action_stock_choose')
		action.connect('activate', self.action_stock_choose)
	
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
		
		self.widget.show()

	def action_add(self, action):
		print 'action add', action
	
	def action_add_sub(self, action):
		print 'action add sub'
	
	def action_remove(self, action):
		print 'action remove'

	def action_stock_choose(self, action):
		print 'action stock choose'

		dialog = self.main.get_object('dialog_stock_choose')
		dialog.set_default_response(gtk.RESPONSE_OK)
		response = dialog.run()
		
		if response == gtk.RESPONSE_OK:
			print 'stock selected'
		else:
			print 'choosing stock canceled'
		
		dialog.hide()
	
	

