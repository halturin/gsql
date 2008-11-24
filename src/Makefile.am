## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GNOME_UI_CFLAGS) \
	$(PACKAGE_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -O1\
	 -g

gsql_gladedir = $(PACKAGE_GLADE_DIR)
gsql_uidir = $(PACKAGE_UI_DIR)

gsql_glade_DATA = \
	gsql_dialogs.glade
gsql_ui_DATA = gsql.ui \
	gsqlfiles.ui\
	gsqlhelp.ui\
	gsqltool.ui

bin_PROGRAMS = gsql

gsql_SOURCES = \
	main.c\
	gsqlcb.c\
	gsqlcb.h\
	gsql.c\
	gsql.h\
	gsqlmenu.c\
	gsqlmenu.h\
	gsqlconf.c\
	gsqlconf.h\
	gsqlconfcb.c\
	gsqlconfcb.h\
	gsqlmenucb.c               \
	gsqlmenucb.h               \
	gsqlfiles.c               \
	gsqlfiles.h               \
	gsqlhelp.c               \
	gsqlhelp.h               


gsql_LDADD = \
	$(top_builddir)/libgsql/libgsql.la \
	$(PACKAGE_LIBS) \
	$(GNOME_LIBS) \
	$(GNOMEUI_LIBS)


EXTRA_DIST = \
	$(gsql_glade_DATA) \
	$(gsql_ui_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)\
	$(GNOME_CFLAGS)\
	$(GNOMEUI_CFLAGS)

