SUBDIRS = services

lib_LTLIBRARIES = libgnomesu.la

INCLUDES=\
	$(LIBGNOMESU_CFLAGS)\
	-DLOCALEDIR="\"$(datadir)/locale\""\
	-DLIBEXECDIR="\"$(libexecdir)\""\
	-DGETTEXT_PACKAGE="\"$(GETTEXT_PACKAGE)\""\
	-DG_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPRECATED\
	-DGLADEDIR="\"$(pkgdatadir)/glade\""

libgnomesu_la_SOURCES =\
	$(libgnomesu_include_HEADERS)\
	libgnomesu.c\
	prefix.c prefix.h\
	utils.c	utils.h\
	gnomesu-auth-dialog.c

libgnomesu_la_LIBADD = -L../src $(LIBGNOMESU_LIBS) services/libservices.la
libgnomesu_la_LDFLAGS = -version-info 1:0:1

libgnomesu_includedir = $(includedir)/libgnomesu-1.0/libgnomesu
libgnomesu_include_HEADERS =\
	libgnomesu.h\
	gnomesu-auth-dialog.h

 
EXTRA_DIST=$(libgnomesu_la_SOURCES) auth-icon.csource
