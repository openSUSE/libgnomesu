INCLUDES =\
	-DG_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPREATED\
	$(LIBGNOMESU_CFLAGS)\
	$(NAUTILUS_CFLAGS)\
	-I../src

bonobodir = $(libdir)/bonobo
bonobo_LTLIBRARIES = libgnomesu-nautilus.la

libgnomesu_nautilus_la_SOURCES =\
	libmain.c\
	gnome-su-component.h\
	gnome-su-component.c

libgnomesu_nautilus_la_LIBADD = $(NAUTILUS_LIBS) -L$(top_srcdir)/src -lgnomesu
libgnomesu_nautilus_la_LDFLAGS = -module -avoid-version

appregistrydir = $(datadir)/application-registry
appregistry_DATA = gnomesu-nautilus.applications

mimedir = $(datadir)/mime-info
mime_DATA = gnomesu-nautilus.keys

server_in_files = GNOME_SuperUser_Component.server.in.in

serverdir = $(libdir)/bonobo/servers
server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files)
	sed -e "s|\@BONOBODIR\@|$(bonobodir)|" $< > $@

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = $(server_in_files) $(appregistry_DATA) $(mime_DATA)
CLEANFILES = $(server_DATA) $(server_DATA).in
