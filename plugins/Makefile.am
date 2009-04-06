
SUBDIRS = 
##\
##	exporter \
##	runner

##if HAVE_PLUGIN_VTE
##SUBDIRS += vte
##endif

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gsql-plugins.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/plugins/$(schema_DATA)

EXTRA_DIST = \
	$(schema_DATA)

## File created by the gnome-build tools



