#
# FILE:
# Makefile.am
#
# FUNCTION:
# Gnotime build file.
#

bin_PROGRAMS = gnotime

gnotime_SOURCES =     \
	active-dialog.c    \
	app.c              \
	projects-tree.c    \
	dialog.c           \
	err.c              \
	err-throw.c        \
	export.c           \
	file-io.c          \
	gconf-gnomeui.c    \
	gconf-io.c         \
	ghtml.c            \
	ghtml-deprecated.c \
	gtt-gsettings-io.c \
	gtt-gsettings-io-p.c \
	idle-dialog.c      \
	journal.c          \
	log.c              \
	main.c             \
	menucmd.c          \
	menus.c            \
	notes-area.c       \
	plug-edit.c        \
	plug-in.c          \
	prefs.c            \
	proj.c             \
	proj-query.c       \
	props-invl.c       \
	props-proj.c       \
	props-task.c       \
	query.c            \
	status-icon.c      \
	timer.c            \
	toolbar.c          \
	util.c             \
	xml-read.c         \
	xml-write.c        \
	dbus.c

noinst_HEADERS =      \
	active-dialog.h    \
	app.h              \
	projects-tree.h    \
	dbus.h             \
	cur-proj.h         \
	dialog.h           \
	err-throw.h        \
	export.h           \
	file-io.h          \
	gconf-gnomeui.h    \
	gconf-io.h         \
	gconf-io-p.h       \
	ghtml.h            \
	ghtml-deprecated.h \
	gtt-gsettings-io.h \
	gtt-gsettings-io-p.h \
	gtt.h              \
	idle-dialog.h      \
	journal.h          \
	log.h              \
	menucmd.h          \
	menus.h            \
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
	${LIBGTKHTML_CFLAGS}  \
	${LIBGLADE_CFLAGS}    \
	${LIBXML2_CFLAGS} \
	${GUILE_CFLAGS} ${X11_CFLAGS}

AM_CPPFLAGS =                                   \
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
	$(LIBXML2_LIBS)       \
	${GUILE_LIBS}         \
	$(INTLLIBS) \
	${GUILE_LDFLAGS} \
	${X11_LIBS} \
	-lm

EXTRA_DIST =         \
	down.xpm          \
	left.xpm          \
	dbus.xml          \
	design.txt

LIBGTK_SRCS= dbus.c dbus.h
BUILT_SOURCES=	dbus-glue.h
dbus-glue.h : dbus.xml Makefile
	dbus-binding-tool --mode=glib-server --output=$@ --prefix=gnotime_dbus $(srcdir)/dbus.xml
dbus.o:	dbus-glue.h


