#
# Makefile.am
# main makeile am for GnoTime
# 

SUBDIRS = data doc gconf ghtml po debian redhat fedora src scripts ui

# old, obsolete location ??
# Productivitydir = $(datadir)/gnome/apps/Applications
Productivitydir = $(datadir)/applications
Productivity_in_files = gnotime.desktop.in
Productivity_DATA = $(Productivity_in_files:.desktop.in=.desktop)

icon_DATA = gnotime_128x128.png
icondir = $(datadir)/pixmaps

EXTRA_DIST =                   \
	AUTHORS                     \
	COPYING                     \
	ChangeLog                   \
	INSTALL                     \
	NEWS                        \
	TODO                        \
	autogen.sh                  \
	$(Productivity_in_files)    \
	$(Productivity_DATA)        \
	$(icon_DATA)                \
	$(top_srcdir)/omf.make      \
	$(top_srcdir)/xmldocs.make



# Satisfy automake strictness check (README must be present).
# The existance of rule (line 1) is enough to satisfy the strictness
# check, but `make dist` actually needs the file, so we just copy it.
# Note that automake 1.16.4 (from 2021) natively allows .md extension
# for README and a few others.
README: README.md
	cp $< $@

