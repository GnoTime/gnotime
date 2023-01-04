#
# FILE:
# Makefile.am
#
# FUNCTION:
# Gnotime build file.
#

bin_PROGRAMS = gnotime

gnotime_SOURCES =                \
	gtt_activation_dialog.c  \
	gtt_application_window.c \
	gtt_dbus.c               \
	gtt_err_throw.c          \
	gtt_export.c             \
	gtt_file_io.c            \
	gtt_ghtml.c              \
	gtt_ghtml_deprecated.c   \
	gtt_gsettings_gnomeui.c  \
	gtt_gsettings_io.c       \
	gtt_help_popup.c         \
	gtt_idle_dialog.c        \
	gtt_idle_timer.c         \
	gtt_journal.c            \
	gtt_log.c                \
	gtt_menu_commands.c      \
	gtt_menus.c              \
	gtt_myoaf.c              \
	gtt_notes_area.c         \
	gtt_plug_in.c            \
	gtt_plug_in_edit.c       \
	gtt_preferences.c        \
	gtt_project.c            \
	gtt_project_queries.c    \
	gtt_projects_tree.c      \
	gtt_props_dlg_interval.c \
	gtt_props_dlg_project.c  \
	gtt_props_dlg_task.c     \
	gtt_queries.c            \
	gtt_signal_handlers.c    \
	gtt_status_icon.c        \
	gtt_timer.c              \
	gtt_toolbar.c            \
	gtt_util.c               \
	gtt_xml_read.c           \
	gtt_xml_write.c          \
	main.c

noinst_HEADERS =      \
	active-dialog.h    \
	app.h              \
	projects-tree.h    \
	dbus.h             \
	cur-proj.h         \
	gtt_help_popup.h           \
	err-throw.h        \
	gtt_export.h           \
	file-io.h          \
	gconf-gnomeui.h    \
	gconf-io.h         \
	gconf-io-p.h       \
	ghtml.h            \
	ghtml-deprecated.h \
	gtt.h              \
	gtt_idle_dialog.h      \
	gtt_journal.h          \
	log.h              \
	menucmd.h          \
	menus.h            \
	myoaf.h            \
	notes-area.h       \
	plug-in.h          \
	prefs.h            \
	proj.h             \
	proj_p.h           \
	proj-query.h       \
	props-invl.h       \
	props-proj.h       \
	props-task.h       \
	query.h            \
	status-icon.h      \
	timer.h            \
	toolbar.h          \
	util.h             \
	xml-gtt.h

# disable depricated when we find work-around for ctree, property box.
#
#	-DGTT_CTREE_GNOME2
#	-DGTK_DISABLE_DEPRECATED
#	-DGNOME_DISABLE_DEPRECATED

AM_CFLAGS = -g -Wall     \
	@CFLAGS@              \
	${GLIB_CFLAGS}     \
	${GTK_CFLAGS}     \
	${LIBGNOME_CFLAGS}     \
	${LIBGNOMEUI_CFLAGS}     \
	${GCONF_CFLAGS}       \
	${LIBGNOMEVFS_CFLAGS} \
	${LIBGTKHTML_CFLAGS}  \
	${LIBGLADE_CFLAGS}    \
	${LIBXML2_CFLAGS} \
	${GUILE_CFLAGS} ${X11_CFLAGS}

INCLUDES =                                   \
	$(LIBQOF_CFLAGS)                          \
	$(LIBDBUS_CFLAGS)                         \
	$(XSS_EXTENSION_CFLAGS)                   \
	-I$(includedir)                           \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"  \
	-DGTTGLADEDIR=\""$(datadir)/gnotime"\"    \
	-DPREFIX=\""$(prefix)"\"                  \
	-DSYSCONFDIR=\""$(sysconfdir)/gnotime"\"  \
	-DGTTDATADIR=\""$(datadir)/gnotime"\"     \
	-DDATADIR=\""$(datadir)"\"                \
	-DLIBDIR=\""$(libdir)/gnotime"\"          \
	-DWITH_DBUS=@WITH_DBUS@ \
	${GLIB_CFLAGS}     \
	${GTK_CFLAGS}     \
	${LIBGNOME_CFLAGS}     \
	${LIBGNOMEUI_CFLAGS}     \
	${GCONF_CFLAGS}       \
	${GUILE_CFLAGS}

gnotime_LDADD =          \
	-lm \
	${GLIB_LIBS}     \
	${GTK_LIBS}     \
	${LIBGNOME_LIBS}     \
	${LIBGNOMEUI_LIBS}     \
	${GCONF_LIBS}       \
	$(LIBQOF_LIBS)        \
	$(LIBDBUS_LIBS)       \
	$(XSS_EXTENSION_LIBS) \
	$(LIBGLADE_LIBS)      \
	$(LIBGTKHTML_LIBS)    \
	${LIBGNOMEVFS_LIBS}   \
	$(LIBXML2_LIBS)       \
	${GUILE_LIBS}         \
	$(INTLLIBS) \
	${GUILE_LDFLAGS} \
    ${X11_LIBS}

EXTRA_DIST =         \
	down.xpm          \
	left.xpm          \
	dbus.xml          \
	design.txt

LIBGTK_SRCS= gtt_dbus.c dbus.h
BUILT_SOURCES=	dbus-glue.h
dbus-glue.h : dbus.xml Makefile
	dbus-binding-tool --mode=glib-server --output=$@ --prefix=gnotime_dbus $(srcdir)/dbus.xml
dbus.o:	dbus-glue.h


