#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
 * nav.helper.py
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

__author__  = "Taras Halturin"
__email__   = "halturin@gmail.com"
__license__ = "GPLv2"

import sys
import os
import gobject
import pango

try:
    import gtk
except:
    print >> sys.stderr, 'This tool needs GTK support. Quiting...'
    sys.exit()
    
try:
	import gtksourceview
except:
	print >> sys.stderr, 'GktSourceView are required. Quiting...'

import navstock
import utils

class GSQLNavigationItem(gobject.GObject):
	__gtype_name__ = 'GSQLNavigationItem'
	
	def __init__(self):
		self.id = None
		self.stock_name = None
		self.name = None
		self.query = None
	


class GSQLNavigation(gtk.Widget, gtk.Buildable):
	__gtype_name__ = 'GSQLNavigation'

	def __init__(self):
		self.id = None		# type of navigation tree (oracle, mysql, etc.)
	
	def do_parser_started(self, builder):
		print "do parser started"
		
	def do_parser_finished(self, builder):
		print "do parser finished"
	
	def do_construct_child(self, builder):
		print "do construct child"
		


class MainWindow:
    
    def destroy(self, widget, data=None):
		gtk.main_quit()

    def delete_event(self, widget, event, data=None):
		return False

    def __init__(self):
	
		self.build = gtk.Builder()
		if (self.build.add_from_file('./nav.helper.xml') == 0):
			print >> sys.stderr, 'Couldn\'t open XML-file  with main window'
		self.window = self.build.get_object('main_window')
	
		self.window.connect("delete_event", self.delete_event)
		self.window.connect("destroy", self.destroy)
	
	
		menu = self.build.get_object('menu_open_file')
		menu.connect("activate", self.open_file)
	
		menu = self.build.get_object('menu_about_dialog')
		menu.connect("activate", self.show_about)
	
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
	
	
	

    def open_file(self, item):
		dialog = gtk.FileChooserDialog("Open..",
					None,
					gtk.FILE_CHOOSER_ACTION_OPEN,
					(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
					 gtk.STOCK_OPEN, gtk.RESPONSE_OK))
					
		dialog.set_default_response(gtk.RESPONSE_OK)
		filter = gtk.FileFilter()
		filter.set_name("All files")
		filter.add_pattern("*")
		dialog.add_filter(filter)
	
		filter = gtk.FileFilter()
		filter.set_name("XML files (GtkBuilder)")
		filter.add_pattern("*.xml")
		dialog.add_filter(filter)
	
		response = dialog.run()
		if response == gtk.RESPONSE_OK:
			filename = dialog.get_filename()
			print filename, 'selected'
			ui = gtk.Builder()
			if (ui.add_from_file(filename) == 0):
				print >> sys.stderr, 'Couldn\'t open XML-file with UI'
			
			
		elif response == gtk.RESPONSE_CANCEL:
			print 'Closed, no files selected'
		dialog.destroy()
	
		print (self)
	
    def show_about (self, item):
		about = self.build.get_object('aboutdialog')
		about.run()
		about.hide()
	


if __name__ == "__main__":
    main_window = MainWindow()
    main_window.main()

