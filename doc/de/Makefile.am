## Process this file with automake to produce Makefile.in.

EXTRA_DIST = topic.dat 

gtt_helpdir = $(datadir)/gnome/help/gtt/de
gtt_help_DATA = topic.dat

install-data-hook:
	( cd $(DESTDIR)$(gtt_helpdir) && \
	  rm -f index.html && \
	  ln -s ../C/index.html . )

