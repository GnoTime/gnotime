
# XXX need to figure out how to install gconf schemas
# where do they live in filesystem ?

# to install schemas, need to do a
# gconftool --install-schema-file=FILENAME

EXTRA_DIST = \
	gnotime.schemas.in

schemasdir = $(sysconfdir)/gconf/schemas
schemas_in_files = gnotime.schemas.in
schemas_DATA = gnotime.schemas

@INTLTOOL_SCHEMAS_RULE@

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA) ;
