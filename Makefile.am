## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = po libgsql src pixmaps data doc engines plugins

gsqldocdir = ${prefix}/doc/gsql
gsqldoc_DATA = \
	AUTHORS

gnomemenudir = $(datadir)/applications/
gnomemenu_DATA = gsql.desktop

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gsql.schemas


# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done


install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = $(gsqldoc_DATA) \
	$(schema_DATA)

