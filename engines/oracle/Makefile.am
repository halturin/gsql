
SUBDIRS = pixmaps

engine_gladedir = $(PACKAGE_GLADE_DIR)/oracle
engine_uidir = $(PACKAGE_UI_DIR)/oracle

engine_glade_DATA = 
engine_ui_DATA = \
	engine_oracle.ui

libgsqldir = $(PACKAGE_ENGINES_DIR)

libgsql_LTLIBRARIES = \
	libgsqlengine_oracle.la 

libgsqlengine_oracle_la_SOURCES = \
	engine_oracle.c          \
	engine_session.c          \
	engine_session.h          \
	engine_menu.c          \
	engine_menu.h          \
	engine_menucb.c          \
	engine_menucb.h          \
	engine_conf.c          \
	engine_conf.h          \
	engine_confcb.h          \
	engine_confcb.c          \
	engine_stock.c          \
	engine_stock.h          \
	oracle.c          \
	oracle.h          \
	nav_objects.h          \
	nav_sql.h          \
	nav_tree_static.h          \
	oracle_var.c          \
	oracle_var.h          \
	oracle_cursor.c          \
	oracle_cursor.h          \
	nav_tree__tables.c          \
	nav_tree__tables.h          \
	nav_tree__tables_cb.c          \
	nav_tree__tables_cb.h          \
	nav_objects.c          \
	nav_tree__columns.c          \
	nav_tree__columns.h          \
	nav_tree__constraints.c          \
	nav_tree__constraints.h          \
	nav_tree__indexes.c          \
	nav_tree__indexes.h

libgsqlengine_oracle_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(ORACLE_LIBS) 

libgsqlengine_oracle_la_LDFLAGS = \
	$(ORACLE_LDFLAGS) 

INCLUDES = \
	$(PACKAGE_CFLAGS)\
	$(ORACLE_CFLAGS) 

EXTRA_DIST = \
	$(engine_glade_DATA) \
	$(engine_ui_DATA)


## File created by the gnome-build tools
