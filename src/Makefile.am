SUBDIRS = ui

lib_LTLIBRARIES = libgnomesu.la
 
INCLUDES=\
	$(LIBGNOMESU_CFLAGS)\
	-DLOCALEDIR="\"$(datadir)/locale\""\
	-DLIBEXEC="\"$(libexecdir)\""\
	-DGETTEXT_PACKAGE="\"$(GETTEXT_PACKAGE)\""\
	-DG_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPRECATED\
	-DGLADEDIR="\"$(pkgdatadir)/glade\""

libgnomesu_la_SOURCES =\
	$(libgnomesu_include_HEADERS)\
	libgnomesu.c\
	service.h\
	utils.c	utils.h\
	consolehelper.c consolehelper.h\
	sudo.c	sudo.h\
	pam.c	pam.h\
	su.c	su.h

libgnomesu_la_LIBADD = -L../src $(LIBGNOMESU_LIBS)
libgnomesu_la_LDFLAGS = -version-info 0:0:0
 
libgnomesu_includedir = $(includedir)/libgnomesu-1.0/libgnomesu
libgnomesu_include_HEADERS =\
	libgnomesu.h

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = libgnomesu.schemas

if SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
endif

 
EXTRA_DIST=$(libgnomesu_la_SOURCES) $(schema_DATA)
