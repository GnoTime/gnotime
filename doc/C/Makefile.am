
SUBDIRS = man

#
# not sure why, but these are not needed.
# gtt_helpdir = $(datadir)/gnome/help/gtt/C
# gtt_help_DATA = gtt/index.html gtt/authors.html topic.dat


# the stuff below is used to build the sgml sources

figs = \
	figures/gtt_mainwin.png  \
	figures/gtt_prefswin.png

docname = gtt
lang = C
omffile = gtt-C.omf
sgml_ents =
include $(top_srcdir)/sgmldocs.make
dist-hook: app-dist-hook
