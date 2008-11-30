## Process this file with automake to produce Makefile.in

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = gsql.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = gsql.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
	echo "Updating Gtk icon cache."; \
	        $(gtk_update_icon_cache); \
	else \
	        echo "*** Icon cache not updated.  After (un)install, run this:"; \
	        echo "***   $(gtk_update_icon_cache)"; \
	fi

gsql_pixmapsdir = $(prefix)/@NO_PREFIX_PACKAGE_PIXMAPS_DIR@

gsql_pixmaps_DATA = \
	unknown.png\
	close.png \
	find.png \
	gsql.png \
	msg_dboutput.png \
	msg_error.png \
	msg_normal.png \
	msg_notice.png \
	msg_warning.png \
	object_clone.png \
	object_code.png \
	object_new.png \
	object_revert.png \
	object_save.png \
	object_sql.png \
	session_close.png \
	session_commit.png \
	session_new.png \
	session_rollback.png \
	sql_fetch_all.png \
	sql_fetch_next.png \
	sql_run.png \
	sql_run_at_cursor.png \
	sql_run_step.png \
	sql_showhide_result.png \
	sql_stop.png \
	sql_stop_onerror.png \
	sql_updateable.png \
	all.schemas.png \
	columns.png \
	constraints.png \
	functions.png \
	indexes.png \
	my.schema.png \
	procedures.png \
	sequences.png \
	tables.png \
	triggers.png \
	users.png \
	views.png \
	logo.png \
	constraints.check.png \
	constraints.foreign.png \
	constraints.primary.png \
	constraints.unique.png \
	arguments.png \
	privileges.png


EXTRA_DIST = $(gsql_pixmaps_DATA) \
	$(icon_DATA) \
	$(svgicon_DATA)


