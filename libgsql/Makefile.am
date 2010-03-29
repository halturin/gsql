INCLUDES = \
	$(PACKAGE_CFLAGS) \
	-DPACKAGE_ENGINES_DIR=\"$(PACKAGE_ENGINES_DIR)\" \
	-DPACKAGE_PLUGINS_DIR=\"$(PACKAGE_PLUGINS_DIR)\"

lib_LTLIBRARIES = \
	libgsql.la 

libgsql_la_SOURCES = \
	common.h

libgsql_la_LIBADD = \
	$(PACKAGE_LIBS)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

libgsqlincludedir=$(includedir)/libgsql

libgsqlinclude_HEADERS = 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgsql.pc

EXTRA_DIST = \
	$(libgsqlinclude_HEADERS) \
	libgsql.pc.in


## File created by the gnome-build tools





