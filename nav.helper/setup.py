try:
    from setuptools import setup, Extention
except:
    from distutils.core import setup, Extension

extention = Extension(
		    name = 'gsql',
		    include_dirs = ['/usr/include/pygtk-2.0',
				    '/usr/include/glib-2.0', 
				    '/usr/lib/glib-2.0/include',
				    '/usr/include/gtk-2.0',
				    '/usr/lib/gtk-2.0/include',
				    '/usr/include/atk-1.0',
				    '/usr/include/cairo',
				    '/usr/include/pango-1.0',
				    '/usr/include/pixman-1',
				    '/usr/include/freetype2',
				    '/usr/include/directfb',
				    '/usr/include/libpng12',
				    '../libgsql'],
		    libraries = ['gtk-x11-2.0','gdk-x11-2.0',
		    'atk-1.0','pangoft2-1.0',
		    'gdk_pixbuf-2.0','m',
		    'pangocairo-1.0',
		    'gio-2.0','cairo',
		    'pango-1.0','freetype',
		    'fontconfig','gmodule-2.0',
		    'gobject-2.0','glib-2.0'],
                    sources = ['gsqlmodule.c', 'navtree.c'])

setup (name = 'gsql',
       version = '0.1.0',
       description = 'This is a GSQLNavTree package',
       ext_modules = [extention])
