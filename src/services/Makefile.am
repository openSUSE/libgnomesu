noinst_LTLIBRARIES = libservices.la

INCLUDES=\
	$(LIBGNOMESU_CFLAGS)\
	-DLOCALEDIR="\"$(datadir)/locale\""\
	-DLIBEXECDIR="\"$(libexecdir)\""\
	-DGETTEXT_PACKAGE="\"$(GETTEXT_PACKAGE)\""\
	-DG_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPRECATED\
	-DGLADEDIR="\"$(pkgdatadir)/glade\""\
	-I$(srcdir)/..

libservices_la_SOURCES =\
	service.h\
	consolehelper.c consolehelper.h\
	pam.c	pam.h\
	su.c	su.h

EXTRA_DIST=$(libservices_la_SOURCES)
