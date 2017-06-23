# libgnomesu #

[![Build Status](https://travis-ci.org/openSUSE/libgnomesu.svg?branch=master)](https://travis-ci.org/openSUSE/libgnomesu)

libgnomesu is a library for providing superuser privileges to GNOME 
applications. It supports sudo, consolehelper, PAM and su.

libgnomesu will use one the following services (in order of priority),
depending on which one is available:
- consolehelper
- PAM
- su

libgnomesu will use the su-based backend as final fallback, if no other
services are available. This service uses a backend which is a modified
version of GNU su.

libgnomesu comes with the GUI frontend 'gnomesu'.


You can find API documentation in the folder 'doc'.
