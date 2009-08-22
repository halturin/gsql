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

import naveditor
import navstock
import utils

import gsql

class MainWindow:
    
    def destroy(self, widget, data=None):
		gtk.main_quit()

    def delete_event(self, widget, event, data=None):
		return False

    def __init__(self):
	
		self.build = gtk.Builder()
		if (self.build.add_from_file('./ui/nav.helper.xml') == 0):
			print >> sys.stderr, 'Couldn\'t open XML-file  with main window'
		self.window = self.build.get_object('main_window')
	
		self.window.connect("delete_event", self.delete_event)
		self.window.connect("destroy", self.destroy)
	
	
		menu = self.build.get_object('menu_open_file')
		menu.connect("activate", self.open_file)
	
		menu = self.build.get_object('menu_about_dialog')
		menu.connect("activate", self.show_about)
		
		self.window.show()

    def main(self):
		gtk.main()

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
			
			notebook = self.build.get_object('main_notebook')
			
			editor = NavEditor(self.build, filename)
			
			label = gtk.Label(os.path.basename(filename))
			label.set_tooltip_text (filename)
			notebook.append_page (editor, label)
			
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

