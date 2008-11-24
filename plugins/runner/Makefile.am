INCLUDES = \
	$(PACKAGE_CFLAGS) \
	$(GNOME_UI_CFLAGS)

libgsqldir = $(PACKAGE_PLUGINS_DIR)

runner_gladedir = $(PACKAGE_GLADE_DIR)/plugins
runner_uidir = $(PACKAGE_UI_DIR)/plugins
runner_pixmapsdir = $(prefix)/@NO_PREFIX_PACKAGE_PIXMAPS_DIR@/plugins

runner_ui_DATA = 
runner_glade_DATA = \
	runner_dialog.glade

runner_pixmaps_DATA = \
	runner.png   \
	runner_logo.png


libgsql_LTLIBRARIES = \
	libgsqlplugin_runner.la

libgsqlplugin_runner_la_SOURCES = \
	plugin_runner.c          \
	plugin_runner.h          \
	thread_run.c          \
	thread_run.h

libgsqlplugin_runner_la_CFLAGS = \
	-I$(top_builddir)/libgsql/

libgsqlplugin_runner_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(top_builddir)/libgsql/libgsql.la

EXTRA_DIST = \
	$(runner_ui_DATA) \
	$(runner_glade_DATA) \
	$(runner_pixmaps_DATA)

AM_CPPFLAGS = \
	$(PACKAGE_CFLAGS)

## File created by the gnome-build tools





