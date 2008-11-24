INCLUDES = \
	$(PACKAGE_CFLAGS) \
	$(GNOME_UI_CFLAGS)

libgsqldir = $(PACKAGE_PLUGINS_DIR)

exporter_gladedir = $(PACKAGE_GLADE_DIR)/plugins
exporter_uidir = $(PACKAGE_UI_DIR)/plugins
exporter_pixmapsdir = $(prefix)/@NO_PREFIX_PACKAGE_PIXMAPS_DIR@/plugins

exporter_ui_DATA = 
exporter_glade_DATA = \
	exporter_dialog.glade

exporter_pixmaps_DATA = \
	exporter.png   \
	export_result.png


libgsql_LTLIBRARIES = \
	libgsqlplugin_exporter.la

libgsqlplugin_exporter_la_SOURCES = \
	plugin_exporter.c          \
	plugin_exporter.h          \
	exportercb.c          \
	exportercb.h          \
	export_plain.c          \
	export_csv.c          \
	export_html.c          \
	export_xml.c

libgsqlplugin_exporter_la_CFLAGS = \
	-I$(top_builddir)/libgsql/

libgsqlplugin_exporter_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(top_builddir)/libgsql/libgsql.la

EXTRA_DIST = \
	$(exporter_ui_DATA) \
	$(exporter_glade_DATA) \
	$(exporter_pixmaps_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

## File created by the gnome-build tools





