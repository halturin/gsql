
SUBDIRS = pixmaps

engine_gladedir = $(PACKAGE_GLADE_DIR)/example
engine_uidir = $(PACKAGE_UI_DIR)/example

engine_glade_DATA = 

engine_ui_DATA = 

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gsql-engine-example.schemas

libgsqldir = $(PACKAGE_ENGINES_DIR)

INCLUDES = \
	$(PACKAGE_CFLAGS)

EXTRA_DIST = \
	$(engine_glade_DATA) \
	$(engine_ui_DATA) \
	$(schema_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/engines/example/$(schema_DATA)


## File created by the gnome-build tools


