
SUBDIRS = \
	pixmaps

engine_glade_DATA = 

engine_gladedir = \
	$(PACKAGE_GLADE_DIR)/firebird

EXTRA_DIST = \
	$(engine_glade_DATA)\
	$(engine_ui_DATA) \
	$(schema_DATA)

engine_ui_DATA = \
	engine_firebird.ui

engine_uidir = \
	$(PACKAGE_UI_DIR)/firebird

libgsqldir = $(PACKAGE_ENGINES_DIR)

libgsql_LTLIBRARIES = \
	libgsqlengine_firebird.la 

libgsqlengine_firebird_la_SOURCES = \
	engine.c                   \
	engine_conf.c                   \
	engine_conf.h                   \
	engine_confcb.c                   \
	engine_confcb.h                   \
	engine_menu.c                   \
	engine_menu.h                   \
	engine_menucb.c                   \
	engine_menucb.h                   \
	engine_session.c                   \
	engine_session.h                   \
	engine_stock.c                   \
	engine_stock.h                   \
	firebird.c                   \
	firebird_cursor.c                   \
	firebird_cursor.h                   \
	firebird_var.c                   \
	firebird_var.h                   \
	nav_objects.c                   \
	nav_objects.h                   \
	nav_sql.h                   \
	nav_tree__columns.c                   \
	nav_tree__columns.h                   \
	nav_tree__constraints.c                   \
	nav_tree__constraints.h                   \
	nav_tree__indexes.c                   \
	nav_tree__indexes.h                   \
	nav_tree__schemas.c                   \
	nav_tree__schemas.h                   \
	nav_tree__tables.c                   \
	nav_tree__tables.h                   \
	nav_tree__triggers.c                   \
	nav_tree__triggers.h                   \
	nav_tree__users.c                   \
	nav_tree__users.h                   \
	nav_tree__views.c                   \
	nav_tree__views.h                   \
	nav_tree__exceptions.c                   \
	nav_tree__exceptions.h                   \
	nav_tree__generators.h                   \
	nav_tree__generators.c                   \
	nav_tree__procedures.c                   \
	nav_tree__procedures.h                   \
	nav_tree__arguments.c                   \
	nav_tree__arguments.h                   \
	nav_tree__functions.c                   \
	nav_tree__functions.h

libgsqlengine_firebird_la_LDFLAGS = \
	-lfbclient

libgsqlengine_firebird_la_CFLAGS = \
	$(FIREBIRD_CFLAGS)

libgsqlengine_firebird_la_LIBADD =                         \
	$(PACKAGE_LIBS)                                     \
	$(top_builddir)/libgsql/libgsql.la

schema_DATA = 

schemadir = \
	@GCONF_SCHEMA_FILE_DIR@

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

INCLUDES = \
	$(PACKAGE_CFLAGS)

## File created by the gnome-build tools

