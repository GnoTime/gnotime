#
# Makefile.am
# main makeile am for GnoTime
# 

SUBDIRS = debian doc gconf glade ghtml lib po redhat fedora src

# old, obsolete location ??
# Productivitydir = $(datadir)/gnome/apps/Applications
Productivitydir = $(datadir)/applications
Productivity_in_files = gnotime.desktop.in
Productivity_DATA = $(Productivity_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                   \
	AUTHORS                     \
	COPYING                     \
	ChangeLog                   \
	INSTALL                     \
	NEWS                        \
	TODO                        \
	autogen.sh                  \
	intltool-extract.in         \
	intltool-merge.in           \
	intltool-update.in          \
	$(Productivity_in_files)    \
	$(Productivity_DATA)        \
	$(top_srcdir)/omf.make      \
	$(top_srcdir)/xmldocs.make
	
