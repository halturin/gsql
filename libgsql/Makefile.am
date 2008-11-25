INCLUDES = \
        $(PACKAGE_CFLAGS)

lib_LTLIBRARIES = \
	libgsql.la 

libgsql_la_SOURCES = \
	workspace.c \
	workspace.h \
	engines.c \
	engines.h \
	conf.c \
	conf.h \
	session.c \
	session.h \
	editor.c \
	editor.h \
	common.c \
	common.h \
	menu.c \
	menu.h \
	utils.c \
	utils.h \
	stock.c \
	stock.h \
	plugins.c \
	plugins.h \
	navigation.h \
	content.h \
	content.c \
	notify.c \
	notify.h \
	cursor.c \
	cursor.h \
	navigation.c \
	sqleditor.h \
	sqleditor.c \
	cvariable.c \
	cvariable.h \
	type_datetime.c \
	type_datetime.h

libgsql_la_LIBADD = \
	$(PACKAGE_LIBS)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

libgsqlincludedir=$(includedir)/libgsql

libgsqlinclude_HEADERS = \
	common.h \
	conf.h \
	content.h \
	cursor.h \
	cvariable.h \
	editor.h \
	engines.h \
	menu.h \
	navigation.h \
	notify.h \
	plugins.h \
	session.h \
	sqleditor.h \
	stock.h \
	type_datetime.h \
	utils.h \
	workspace.h 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgsql.pc

EXTRA_DIST = \
	$(libgsqlinclude_HEADERS) \
	libgsql.pc.in


## File created by the gnome-build tools





