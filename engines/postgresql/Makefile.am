AM_CFLAGS = -Werror
SUBDIRS = pixmaps

engine_gladedir = $(PACKAGE_GLADE_DIR)/postgresql
engine_uidir = $(PACKAGE_UI_DIR)/postgresql

engine_glade_DATA = 
engine_ui_DATA = engine_pgsql.ui

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = 

libgsqldir = $(PACKAGE_ENGINES_DIR)

libgsql_LTLIBRARIES = \
	libgsqlengine_pgsql.la 

libgsqlengine_pgsql_la_SOURCES = \
	engine.c \
	engine_stock.c \
	engine_menucb.c \
	engine_menu.c \
	engine_confcb.c \
	engine_conf.c \
	engine_session.c \
	pgsql.c \
	pgsql_cursor.c \
	pgsql_var.c \
	nav_objects.c \
	nav_tree_languages.c \
	nav_tree_databases.c \
	nav_tree_processlist.c \
	nav_tree_procedures.c \
	nav_tree_privileges.c \
	nav_tree_variables.c \
	nav_tree_constraints.c \
	nav_tree_views.c \
	nav_tree_types.c \
	nav_tree_sequences.c \
	nav_tree_triggers.c \
	nav_tree_indexes.c \
	nav_tree_columns.c \
	nav_tree_tables.c \
	nav_tree_users.c \
	nav_tree_schemas.c \
	engine_confcb.h \
	engine_conf.h \
	engine_menucb.h \
	engine_menu.h \
	engine_session.h \
	engine_stock.h \
	nav_objects.h \
	nav_sql.h \
	nav_tree_columns.h \
	nav_tree_constraints.h \
	nav_tree_databases.h \
	nav_tree_indexes.h \
	nav_tree_languages.h \
	nav_tree_privileges.h \
	nav_tree_procedures.h \
	nav_tree_processlist.h \
	nav_tree_schemas.h \
	nav_tree_sequences.h \
	nav_tree_tables.h \
	nav_tree_triggers.h \
	nav_tree_types.h \
	nav_tree_users.h \
	nav_tree_variables.h \
	nav_tree_views.h \
	pgsql_cursor.h \
	pgsql.h \
	pgsql_var.h


libgsqlengine_pgsql_la_CFLAGS = \
	$(PGSQL_CFLAGS)

libgsqlengine_pgsql_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(PGSQL_LIBS) \
	$(top_builddir)/libgsql/libgsql.la

INCLUDES = \
	$(PACKAGE_CFLAGS)

EXTRA_DIST = \
        $(engine_glade_DATA) \
        $(engine_ui_DATA) \
        $(schema_DATA)
                        
##install-data-local:
##	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/engines/postgresql/$(schema_DATA)


## File created by the gnome-build tools


