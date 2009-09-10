INCLUDES = \
	$(PACKAGE_CFLAGS) \
	$(GNOME_UI_CFLAGS)

libgsqldir = $(PACKAGE_PLUGINS_DIR)

tunnel_gladedir = $(PACKAGE_GLADE_DIR)/plugins
tunnel_uidir = $(PACKAGE_UI_DIR)/plugins
tunnel_pixmapsdir = $(prefix)/@NO_PREFIX_PACKAGE_PIXMAPS_DIR@/plugins

tunnel_ui_DATA = 
tunnel_glade_DATA = 

tunnel_pixmaps_DATA = \
	tunnel.png

libgsql_LTLIBRARIES = \
	libgsqlplugin_tunnel.la

libgsqlplugin_tunnel_la_SOURCES = \
	plugin_tunnel.c          \
	plugin_tunnel.h

libgsqlplugin_tunnel_la_CFLAGS = \
	-I$(top_builddir)/libgsql/ \
	$(PLUGIN_TUNNEL_CFLAGS)

libgsqlplugin_tunnel_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(PLUGIN_TUNNEL_LIBS) \
	$(top_builddir)/libgsql/libgsql.la

EXTRA_DIST = \
	$(tunnel_ui_DATA) \
	$(tunnel_glade_DATA) \
	$(tunnel_pixmaps_DATA)

## File created by the gnome-build tools





