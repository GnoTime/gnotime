
bin_PROGRAMS = gnotime

gnotime_SOURCES =     \
	active-dialog.c    \
	app.c              \
	ctree.c            \
	ctree-gnome2.c     \
	dialog.c           \
	err.c              \
	err-throw.c        \
	export.c           \
	file-io.c          \
	gconf-gnomeui.c    \
	gconf-io.c         \
	ghtml.c            \
	ghtml-deprecated.c \
	idle-dialog.c      \
	idle-timer.c       \
	journal.c          \
	log.c              \
	main.c             \
	menucmd.c          \
	menus.c            \
	myoaf.c            \
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
	timer.c            \
	toolbar.c          \
	util.c             \
	xml-read.c         \
	xml-write.c

noinst_HEADERS =      \
	active-dialog.h    \
	app.h              \
	ctree.h            \
	ctree-gnome2.h     \
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
	gtt.h              \
	idle-dialog.h      \
	idle-timer.h       \
	journal.h          \
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
	${GNOTIME_CFLAGS}     \
	${GCONF_CFLAGS}       \
	${LIBGNOMEVFS_CFLAGS} \
	${LIBGTKHTML_CFLAGS}  \
	${LIBGLADE_CFLAGS}    \
	${LIBXML2_CFLAGS}

INCLUDES =                                   \
	-I../lib                                  \
	-I../lib/qof                              \
	-I$(includedir)                           \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"  \
	-DGTTGLADEDIR=\""$(datadir)/gnotime"\"    \
	-DPREFIX=\""$(prefix)"\"                  \
	-DSYSCONFDIR=\""$(sysconfdir)/gnotime"\"  \
	-DGTTDATADIR=\""$(datadir)/gnotime"\"     \
	-DDATADIR=\""$(datadir)"\"                \
	-DLIBDIR=\""$(libdir)/gnotime"\"          \
	${GNOME_INCLUDEDIR}                       \
	${GUILE_INCS}
						 
gnotime_LDADD =         \
	$(GNOTIME_LIBS)      \
	../lib/qof/libqof.la \
	../lib/libqofsql/libqofsql.la \
	$(LIBGLADE_LIBS)     \
	$(LIBGTKHTML_LIBS)   \
	${LIBGNOMEVFS_LIBS}  \
	$(LIBXML2_LIBS)      \
	${GUILE_LIBS}        \
	$(INTLLIBS)

#	$(LIBGNOMEXML_LIBS)        	
#	$(GCONF_LIBS)	 	

EXTRA_DIST = 			\
	down.xpm		\
	left.xpm		\
	design.txt


                                                                                

