libexec_PROGRAMS = gnomesu-backend
noinst_LTLIBRARIES = libcommon.la

AM_CPPFLAGS = $(GNOMESU_BACKEND_CFLAGS)

gnomesu_backend_SOURCES = \
	basename.c\
	closeout.c closeout.h\
	errmsg.c errmsg.h\
	pathmax.h\
	su.c\
	system.h\
	version-etc.c version-etc.h\
	xalloc.h\
	xmalloc.c\
	xstrdup.c\
	xstrdup.h

gnomesu_backend_LDFLAGS = $(GNOMESU_BACKEND_LIBS) libcommon.la -lcrypt
gnomesu_backend_CFLAGS = $(AM_CFLAGS)
gnomesu_backend_DEPENDENCIES = libcommon.la

libcommon_la_SOURCES = \
	common.c\
	common.h\
	xalloc.h\
	xmalloc.c\
	xstrdup.c\
	xstrdup.h
libcommon_la_LDFLAGS = $(GNOMESU_BACKEND_LIBS)

installed_su=$(DESTDIR)$(libexecdir)/`echo gnomesu-backend | sed '$(transform)'`

if SETUID_ERROR
install-data-local: gnomesu-backend
	@printErr() { \
		echo; \
		echo "########################################################"; \
		echo "ERROR: you don't have permission to create setuid root executables."; \
		echo "Please run 'make install' again as root."; \
		echo "########################################################"; \
		echo; \
		exit 1; \
	}; \
	echo chown root$(ROOTGROUP) $(installed_su); \
	chown root$(ROOTGROUP) $(installed_su) || printErr; \
	echo chmod u=rxs,go=xs $(installed_su); \
	chmod u=rxs,go=xs $(installed_su) || printErr
else
install-data-local: gnomesu-backend
	@printWarn() { \
		echo; \
		echo "########################################################"; \
		echo "WARNING: you don't have permission to create setuid root executables."; \
		echo "As a result, gnomesu-backend may not work correctly."; \
		echo "Please run 'make install' as root."; \
		echo "########################################################"; \
		echo; \
	}; \
	echo chown root$(ROOTGROUP) $(installed_su); \
	chown root$(ROOTGROUP) $(installed_su) || printWarn; \
	echo chmod u=rxs,go=xs $(installed_su); \
	chmod u=rxs,go=xs $(installed_su) || true
endif

EXTRA_DIST = $(gnomesu_backend_SOURCES) $(libcommon_la_SOURCES)
