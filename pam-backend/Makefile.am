if HAVE_PAM

libexec_PROGRAMS = gnomesu-pam-backend

INCLUDES = $(GNOMESU_PAM_BACKEND_CFLAGS) -I$(top_srcdir)/su-backend

gnomesu_pam_backend_SOURCES = pam.c
gnomesu_pam_backend_CFLAGS = \
	$(INCLUDES) \
	$(SUID_CFLAGS) \
	$(AM_CFLAGS)
gnomesu_pam_backend_LDFLAGS = \
	$(SUID_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(GNOMESU_PAM_BACKEND_LIBS)\
	$(top_builddir)/su-backend/libcommon.la \
	-lpam
gnomesu_pam_backend_DEPENDENCIES = $(top_builddir)/su-backend/libcommon.la

if INSTALL_PAM
pamdir = $(PAMDIR)
pam_DATA = gnomesu-pam
endif

installed_exe=$(DESTDIR)$(libexecdir)/`echo gnomesu-pam-backend | sed '$(transform)'`

if SETUID_ERROR
install-data-local: gnomesu-pam-backend
	@printErr() { \
		echo; \
		echo "########################################################"; \
		echo "ERROR: you don't have permission to create setuid root executables."; \
		echo "Please run 'make install' again as root."; \
		echo "########################################################"; \
		echo; \
		exit 1; \
	}; \
	echo chown root$(ROOTGROUP) $(installed_exe); \
	chown root$(ROOTGROUP) $(installed_exe) || printErr; \
	echo chmod u=rxs,go=xs $(installed_exe); \
	chmod u=rxs,go=xs $(installed_exe) || printErr
else
install-data-local: gnomesu-pam-backend
	@printWarn() { \
		echo; \
		echo "########################################################"; \
		echo "WARNING: you don't have permission to create setuid root executables."; \
		echo "As a result, gnomesu-pam-backend may not work correctly."; \
		echo "Please run 'make install' as root."; \
		echo "########################################################"; \
		echo; \
	}; \
	echo chown root$(ROOTGROUP) $(installed_exe); \
	chown root$(ROOTGROUP) $(installed_exe) || printWarn; \
	echo chmod u=rxs,go=xs $(installed_exe); \
	chmod u=rxs,go=xs $(installed_exe) || true
endif

endif

EXTRA_DIST = pam.c gnomesu-pam
