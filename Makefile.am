
SUBDIRS = debian doc gconf glade ghtml lib po redhat src

Productivitydir = $(datadir)/gnome/apps/Applications
Productivity_in_files = gnotime.desktop.in
Productivity_DATA = $(Productivity_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                   \
	AUTHORS                     \
	COPYING                     \
	ChangeLog                   \
	NEWS                        \
	TODO                        \
	autogen.sh                  \
	intltool-extract.in         \
	intltool-merge.in           \
	intltool-update.in          \
	$(Productivity_in_files)    \
	$(Productivity_DATA)        \
	$(top_srcdir)/sgmldocs.make

