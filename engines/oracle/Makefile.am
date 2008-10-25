
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
	oracle_var.c          \
	oracle_var.h          \
	oracle_cursor.c          \
	oracle_cursor.h          \
	nav_tree__tables.c          \
	nav_tree__tables.h          \
	nav_objects.c          \
	nav_tree__columns.c          \
	nav_tree__columns.h          \
	nav_tree__constraints.c          \
	nav_tree__constraints.h          \
	nav_tree__indexes.c          \
	nav_tree__indexes.h          \
	nav_tree__triggers.h          \
	nav_tree__triggers.c          \
	nav_tree__depend.c          \
	nav_tree__depend.h          \
	nav_tree__mviews.c          \
	nav_tree__mviews.h          \
	nav_tree__schemas.c          \
	nav_tree__schemas.h          \
	nav_tree__procedures.c          \
	nav_tree__procedures.h          \
	nav_tree__arguments.c          \
	nav_tree__arguments.h          \
	nav_tree__packages.c          \
	nav_tree__packages.h          \
	nav_tree__sequences.c          \
	nav_tree__sequences.h          \
	nav_tree__synonyms.c          \
	nav_tree__synonyms.h          \
	nav_tree__dblinks.c          \
	nav_tree__dblinks.h          \
	nav_tree__libraries.c          \
	nav_tree__libraries.h          \
	nav_tree__privileges.c          \
	nav_tree__privileges.h          \
	nav_tree__roles.c          \
	nav_tree__roles.h          \
	nav_tree__resourcs.c          \
	nav_tree__resourcs.h          \
	nav_tree__tablespaces.c          \
	nav_tree__tablespaces.h          \
	nav_tree__users.c          \
	nav_tree__users.h          \
	nav_tree__srv_inst.c          \
	nav_tree__srv_inst.h          \
	nav_tree__rollback.c          \
	nav_tree__rollback.h          \
	nav_tree__redo_logs.c          \
	nav_tree__redo_logs.h          \
	nav_tree__recycle.c          \
	nav_tree__recycle.h          \
	nav_tree__operators.c          \
	nav_tree__operators.h          \
	nav_tree__indextypes.c          \
	nav_tree__indextypes.h          \
	nav_tree__views.c          \
	nav_tree__views.h          \
	nav_tree__objects.c          \
	nav_tree__objects.h          \
	nav_tree__queues.c          \
	nav_tree__queues.h          \
	nav_tree__dimens.c          \
	nav_tree__dimens.h          \
	nav_tree__storetabs.c          \
	nav_tree__storetabs.h

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
