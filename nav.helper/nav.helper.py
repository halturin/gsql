#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import gobject
try:
    import gtk
except:
    print >> sys.stderr, 'This tool needs GTK support. Quiting...'
    sys.exit()

import navstock

class GSQLNavigationItem(gobject.GObject):

    def __init__(self):
	self.id = 0
	self.stock_name = None
	self.name = None
	self.query = None
	


class GSQLNavigation(gtk.Widget):
    def __init__(self):
	self.a = None


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
	
#	cr = gtk.CellRendererPixbuf()
#	iconstock_combo.pack_start (cr, True)
#	iconstock_combo.add_attribute(cr, "stock-id", 0)
	
#	cr = gtk.CellRendererText()
#	iconstock_combo.pack_start (cr, True)
#	iconstock_combo.add_attribute(cr, "text", 1)  
	
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
	    print dialog.get_filename(), 'selected'
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

