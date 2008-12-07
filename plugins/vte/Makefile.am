INCLUDES = \
	$(PACKAGE_CFLAGS) \
	$(GNOME_UI_CFLAGS)

libgsqldir = $(PACKAGE_PLUGINS_DIR)

vte_gladedir = $(PACKAGE_GLADE_DIR)/plugins
vte_uidir = $(PACKAGE_UI_DIR)/plugins
vte_pixmapsdir = $(prefix)/@NO_PREFIX_PACKAGE_PIXMAPS_DIR@/plugins

vte_ui_DATA = vte.ui
vte_glade_DATA = vte_dialog_conf.glade

vte_pixmaps_DATA = \
	vte-terminal.png

libgsql_LTLIBRARIES = \
	libgsqlplugin_vte.la 

libgsqlplugin_vte_la_SOURCES = \
	vtecb.c                 \
	vtecb.h                 \
	vte_conf.c                 \
	vte_conf.h                 \
	plugin_vte.h                 \
	plugin_vte.c

libgsqlplugin_vte_la_CFLAGS = \
	-I$(top_builddir)/libgsql/ \
	$(PLUGIN_VTE_CFLAGS)

libgsqlplugin_vte_la_LIBADD = \
	$(PACKAGE_LIBS)      \
	$(PLUGIN_VTE_LIBS) \
	$(top_builddir)/libgsql/libgsql.la 


EXTRA_DIST = \
	$(vte_ui_DATA) \
	$(vte_glade_DATA) \
	$(vte_pixmaps_DATA)


## File created by the gnome-build tools





