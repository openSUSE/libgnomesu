bin_PROGRAMS = gnomesu
libexec_SCRIPTS = gnomesu-sudo-helper

INCLUDES =\
	-DGETTEXT_PACKAGE="\"gnomesu\""\
	-DLOCALEDIR="\"$(datadir)/locale\""\
	-DGNOMESU_VERSION="\"$(VERSION)\""\
	-DGNOMESU_PREFIX="\"$(prefix)\""\
	-DG_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPRECATED\
	$(LIBGNOMESU_CFLAGS)\
	$(GNOMESU_CFLAGS)\
	-I../src

gnomesu_SOURCES = gnomesu.c
gnomesu_LDFLAGS = $(GNOMESU_LIBS)
gnomesu_LDADD = $(top_srcdir)/src/libgnomesu.la

EXTRA_DIST = $(gnomesu_SOURCES) $(libexec_SCRIPTS)
