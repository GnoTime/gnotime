
bin_PROGRAMS = gtt

gtt_SOURCES = 		\
	app.c		\
	ctree.c		\
	dialog.c	\
	err.c		\
	err-throw.c	\
	file-io.c	\
	ghtml.c		\
	idle-timer.c	\
	journal.c	\
	log.c		\
	main.c		\
	menucmd.c	\
	menus.c		\
	myoaf.c		\
	plug-in.c	\
	prefs.c		\
	proj.c		\
	props-invl.c	\
	props-proj.c	\
	props-task.c	\
	shorts.c	\
	timer.c		\
	toolbar.c	\
	util.c		\
	xml-read.c	\
	xml-write.c

noinst_HEADERS = 	\
	app.h		\
	ctree.h		\
	cur-proj.h	\
	dialog.h	\
	err-throw.h	\
	file-io.h	\
	ghtml.h		\
	gtt.h		\
	idle-timer.h	\
	journal.h	\
	log.h		\
	menucmd.h	\
	menus.h		\
	myoaf.h		\
	plug-in.h	\
	prefs.h		\
	proj.h		\
	proj_p.h	\
	props-invl.h	\
	props-proj.h	\
	props-task.h	\
	shorts.h	\
	timer.h		\
	toolbar.h	\
	util.h		\
	xml-gtt.h
	

# remove GTK_ENABLE_BROKEN when gnome-2.0 works for real
CFLAGS = -DGTK_ENABLE_BROKEN	\
	@CFLAGS@ 		\
	${GNOTIME_CFLAGS}	\
	${GCONF_CFLAGS} 	\
	${GTKHTML2_CFLAGS} 	\
	${LIBGLADE_CFLAGS} 

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGTTGLADEDIR=\""$(datadir)/gnotime"\" \
	-DGTTDATADIR=\""$(datadir)/gnotime"\" \
	${GNOME_INCLUDEDIR}	\
	${GUILE_INCS}
						 
gtt_LDADD = \
	$(GNOTIME_LIBS)	\
	$(LIBGLADE_LIBS)        \
	$(GTKHTML2_LIBS)	\
	$(INTLLIBS)

#	$(LIBXML2_LIBS)        	\
#	$(GCONF_LIBS)	 	\
#	${GUILE_LIBS}		\

EXTRA_DIST = 			\
	down.xpm		\
	left.xpm		\
	design.txt


