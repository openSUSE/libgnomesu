libexec_PROGRAMS = gnomesu-backend
noinst_LTLIBRARIES = libcommon.la

INCLUDES = $(GNOMESU_BACKEND_CFLAGS)

gnomesu_backend_SOURCES = \
	basename.c\
	closeout.c closeout.h\
	pathmax.h\
	malloc.c\
	realloc.c\
	su.c\
	sys2.h\
	system.h\
	version-etc.c version-etc.h\
	xalloc.h\
	xmalloc.c\
	xstrdup.c

gnomesu_backend_LDFLAGS = $(GNOMESU_BACKEND_LIBS) libcommon.la -lcrypt
gnomesu_backend_DEPENDENCIES = libcommon.la

libcommon_la_SOURCES = common.c
libcommon_la_LDFLAGS = $(GNOMESU_BACKEND_LIBS)

installed_su=$(libexecdir)/`echo gnomesu-backend | sed $(transform)`

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
	echo chown root:root $(installed_su); \
	chown root:root $(installed_su) || printErr; \
	echo chmod a=xs $(installed_su); \
	chmod a=xs $(installed_su) || printErr
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
	echo chown root:root $(installed_su); \
	chown root:root $(installed_su) || printWarn; \
	echo chmod a=xs $(installed_su); \
	chmod a=xs $(installed_su) || true
endif

EXTRA_DIST = $(gnomesu_backend_SOURCES) $(libcommon_la_SOURCES)