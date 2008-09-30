## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GNOME_UI_CFLAGS) \
	$(PACKAGE_CFLAGS)

AM_CFLAGS =\
	$(ORACLE_CFLAGS)\
	 -Wall\
	 -O1\
	 -g

gsql_gladedir = $(PACKAGE_GLADE_DIR)
gsql_uidir = $(PACKAGE_UI_DIR)

gsql_glade_DATA = \
	gsql_dialogs.glade
gsql_ui_DATA = gsql.ui \
	gsqlfiles.ui\
	gsqlhelp.ui

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


gsql_LDADD =                      \
	$(PACKAGE_LIBS)           \
	$(top_builddir)/libgsql/libgsql.la 


EXTRA_DIST = \
	$(gsql_glade_DATA) \
	$(gsql_ui_DATA)

