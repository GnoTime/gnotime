SUBDIRS = doc glade ghtml src


Productivitydir = $(datadir)/gnome/apps/Applications
Productivity_in_files = gtt.desktop.in
Productivity_DATA = $(Productivity_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 			\
	AUTHORS			\
	COPYING			\
	ChangeLog		\
	NEWS			\
	TODO			\
	$(Productivity_in_files) \
	$(Productivity_DATA) 	\
	$(top_srcdir)/sgmldocs.make

