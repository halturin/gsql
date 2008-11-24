
SUBDIRS = pixmaps

engine_gladedir = $(PACKAGE_GLADE_DIR)/mysql
engine_uidir = $(PACKAGE_UI_DIR)/mysql

engine_glade_DATA = 
engine_ui_DATA = engine_mysql.ui



libgsqldir = $(PACKAGE_ENGINES_DIR)

libgsql_LTLIBRARIES = \
	libgsqlengine_mysql.la 

libgsqlengine_mysql_la_SOURCES = \
	engine_mysql.c          \
	engine_session.c          \
	engine_session.h          \
	engine_menu.c          \
	engine_menu.h          \
	engine_menucb.c          \
	engine_menucb.h          \
	engine_conf.h          \
	engine_confcb.h          \
	engine_confcb.c          \
	engine_stock.c          \
	engine_stock.h          \
	engine_conf.c          \
	mysql.c          \
	mysql.h          \
	nav_objects.h          \
	nav_sql.h          \
	mysql_cursor.c          \
	mysql_cursor.h          \
	nav_tree__tables.c          \
	nav_tree__tables.h          \
	mysql_var.c          \
	mysql_var.h          \
	nav_objects.c          \
	nav_tree__users.c          \
	nav_tree__users.h          \
	nav_tree__constraints.c          \
	nav_tree__constraints.h          \
	nav_tree__triggers.c          \
	nav_tree__triggers.h          \
	nav_tree__indexes.c          \
	nav_tree__indexes.h          \
	nav_tree__columns.c          \
	nav_tree__columns.h          \
	nav_tree__views.c          \
	nav_tree__views.h          \
	nav_tree__procedures.c          \
	nav_tree__procedures.h          \
	nav_tree__privileges.c          \
	nav_tree__privileges.h          \
	nav_tree__schemas.c          \
	nav_tree__schemas.h          \
	nav_tree__variables.c          \
	nav_tree__variables.h          \
	nav_tree__processlist.c          \
	nav_tree__processlist.h

libgsqlengine_mysql_la_CFLAGS = \
	$(MYSQL_CFLAGS)

libgsqlengine_mysql_la_LIBADD =          \
	$(PACKAGE_LIBS)          \
	$(MYSQL_LIBS)          \
	$(top_builddir)/libgsql/libgsql.la

INCLUDES = \
	$(PACKAGE_CFLAGS)

EXTRA_DIST = \
	$(engine_glade_DATA) \
	$(engine_ui_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

## File created by the gnome-build tools


