
bin_PROGRAMS = gnotime

gnotime_SOURCES = 		\
	app.c		\
	ctree.c		\
	ctree-gnome2.c		\
	dialog.c	\
	err.c		\
	err-throw.c	\
	export.c	\
	file-io.c	\
	ghtml.c		\
	ghtml-deprecated.c		\
	idle-dialog.c	\
	idle-timer.c	\
	journal.c	\
	log.c		\
	main.c		\
	menucmd.c	\
	menus.c		\
	myoaf.c		\
	notes-area.c	\
	plug-in.c	\
	prefs.c		\
	proj.c		\
	props-invl.c	\
	props-proj.c	\
	props-task.c	\
	timer.c		\
	toolbar.c	\
	util.c		\
	xml-read.c	\
	xml-write.c

noinst_HEADERS = 	\
	app.h		\
	ctree.h		\
	ctree-gnome2.h		\
	cur-proj.h	\
	dialog.h	\
	err-throw.h	\
	export.h	\
	file-io.h	\
	ghtml.h		\
	ghtml-deprecated.h		\
	gtt.h		\
	idle-dialog.h	\
	idle-timer.h	\
	journal.h	\
	log.h		\
	menucmd.h	\
	menus.h		\
	myoaf.h		\
	notes-area.h	\
	plug-in.h	\
	prefs.h		\
	proj.h		\
	proj_p.h	\
	props-invl.h	\
	props-proj.h	\
	props-task.h	\
	timer.h		\
	toolbar.h	\
	util.h		\
	xml-gtt.h
	

# disable depricated when we find work-around for ctree, property box.
#
#	-DGTT_CTREE_GNOME2 
#	-DGTK_DISABLE_DEPRECATED	
#	-DGNOME_DISABLE_DEPRECATED	

CFLAGS = -g \
	@CFLAGS@ 		\
	${GNOTIME_CFLAGS}	\
	${GCONF_CFLAGS} 	\
	${LIBGTKHTML_CFLAGS} 	\
	${LIBGLADE_CFLAGS} 

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGTTGLADEDIR=\""$(datadir)/gnotime"\" \
	-DPREFIX=\""$(prefix)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)/gnotime"\" \
	-DGTTDATADIR=\""$(datadir)/gnotime"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBDIR=\""$(libdir)/gnotime"\" \
	${GNOME_INCLUDEDIR}	\
	${GUILE_INCS}
						 
gnotime_LDADD = \
	$(GNOTIME_LIBS)	\
	$(LIBGLADE_LIBS)        \
	$(LIBGTKHTML_LIBS)	\
	${GUILE_LIBS}		\
	$(INTLLIBS)

#	$(LIBGNOMEXML_LIBS)        	
#	$(GCONF_LIBS)	 	

EXTRA_DIST = 			\
	down.xpm		\
	left.xpm		\
	design.txt


