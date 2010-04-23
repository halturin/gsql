## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(PACKAGE_CFLAGS) \
	-DPACKAGE_ENGINES_DIR=\"$(PACKAGE_ENGINES_DIR)\" \
	-DPACKAGE_PLUGINS_DIR=\"$(PACKAGE_PLUGINS_DIR)\"

AM_CFLAGS =\
	 -Wall\
	 -O1\
	 -g

gsql_gladedir = $(PACKAGE_GLADE_DIR)
gsql_uidir = $(PACKAGE_UI_DIR)

gsql_glade_DATA =
gsql_ui_DATA = \
	gsql.ui

bin_PROGRAMS = gsql

gsql_SOURCES = \
	main.c\
	gsql-app.c \
	gsql-app.h \
	session-manager.c \
	session-manager.h \
	content-manager.c \
	content-manager.h \
	gsql-statusbar.c \
	gsql-statusbar.h \
	gsql-actions.h \
	gsql-callbacks.h \
	gsql-callbacks.c \
	color-hinter.c \
	color-hinter.h


gsql_LDADD = \
	$(top_builddir)/libgsql/libgsql.la \
	$(PACKAGE_LIBS) \
	$(GDL_LIBS)


EXTRA_DIST = \
	$(gsql_glade_DATA) \
	$(gsql_ui_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)\
	$(GDL_CFLAGS)

