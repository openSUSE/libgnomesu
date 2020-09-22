AC_PREQ([2.53])
AC_INIT([libgnomesu], [2.0.5])
AM_INIT_AUTOMAKE([1.11 foreign dist-xz no-dist-gzip])
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
AC_LANG([C])

AC_PROG_CPP
AC_PROG_CC
AC_ISC_POSIX
AC_STDC_HEADERS
AM_PROG_AR
AM_DISABLE_STATIC
AM_PROG_LIBTOOL

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET


PKG_CHECK_MODULES(LIBGNOMESU,
[
	glib-2.0
	gtk+-3.0
])

AC_MSG_CHECKING(for forkpty())
AC_TRY_LINK(
	[void forkpty (void);],
	[forkpty ();],
	AC_MSG_RESULT(yes),
	[
		AC_MSG_RESULT(no)
		oLIBS="$LIBS"
		LIBS="$LIBS -lutil"
		AC_MSG_CHECKING(for forkpty() in -lutil)
		AC_TRY_LINK(
			[void forkpty (void);],
			[forkpty ();],
			[
				AC_MSG_RESULT(yes)
				LIBGNOMESU_LIBS="$LIBGNOMESU_LIBS -lutil"
			],
			[
				AC_MSG_RESULT(no)
				AC_MSG_ERROR(The function forkpty() is not available on your system.)
			])
		LIBS="$oLIBS"
	])


PKG_CHECK_MODULES(GNOMESU,[
       gio-2.0 >= 2.26.0
])
AC_SUBST(GNOMESU_CFLAGS)
AC_SUBST(GNOMESU_LIBS)


PKG_CHECK_MODULES(GNOMESU_PAM_BACKEND,[
	glib-2.0
])
GNOMESU_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS"
GNOMESU_BACKEND_LIBS="$GNOMESU_PAM_BACKEND_LIBS"


GETTEXT_PACKAGE=libgnomesu-1.0
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS="ar az bg bs ca cs da de el en_CA en_GB es eu fa fi fr gu hi hr hu id ie it ja ko lt lv mk ms nb ne nl nn pa pl pt_BR pt ru rw sk sq sr@latin sr sv tr uk vi zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT
AC_PROG_INTLTOOL


# FreeBSD doesn't have a 'root' group, but it has a 'wheel' group instead
AC_MSG_CHECKING(whether there's a 'root' group)
AC_RUN_IFELSE([
	AC_LANG_SOURCE(
[[#include <grp.h>
#include <sys/types.h>

int main() {
	struct group *grp = getgrnam ("root");
	return !grp;
}]]
	)
], [
	AC_MSG_RESULT(yes)
	ROOTGROUP=:root
], [
	AC_MSG_RESULT(no)
	AC_MSG_CHECKING(whether there's a 'wheel' group)
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
		#include <grp.h>
		#include <sys/types.h>

		int main() {
			struct group *grp = getgrnam ("wheel");
			return !grp;
		}]]
		)
	], [
		AC_MSG_RESULT(yes)
		ROOTGROUP=:wheel
	], AC_MSG_RESULT(no))
])
AC_SUBST(ROOTGROUP)


AC_CHECK_FUNCS(syslog)
if test $ac_cv_func_syslog = no; then
  # syslog is not in the default libraries.  See if it's in some other.
  for lib in bsd socket inet; do
    AC_CHECK_LIB($lib, syslog, [AC_DEFINE(HAVE_SYSLOG)
      GNOMESU_BACKEND_LIBS="$GNOMESU_BACKEND_LIBS -l$lib"
      GNOMESU_PAM_BACKEND_LIBS="$GNOMESU_PAM_BACKEND_LIBS -l$lib"
      break])
  done
fi

# Check for PAM
LIBS=-lpam
have_pam=no
AC_CHECK_FUNC(pam_start, [
	# FreeBSD has pam_mod_misc.h instead of pam_misc.h
	AC_CHECK_HEADER(security/pam_misc.h, have_pam=yes, [
		AC_CHECK_HEADER(security/pam_mod_misc.h, [
			have_pam=yes
			GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DPAM_MOD_MISC"
		])
	])
])
AM_CONDITIONAL(HAVE_PAM, test $have_pam = yes)

if test $have_pam = yes; then
	LIBGNOMESU_CFLAGS="$LIBGNOMESU_CFLAGS -DHAVE_PAM"
	LIBGNOMESU_LIBS="$LIBGNOMESU_LIBS -lpam"
fi

AC_SUBST(LIBGNOMESU_CFLAGS)
AC_SUBST(LIBGNOMESU_LIBS)


# SysV needs -lsec, older versions of Linux need -lshadow for shadow passwords.
LIBS=
AC_SEARCH_LIBS(getspnam, [shadow sec])
GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS $LIBS"
AC_EGREP_HEADER(sp_pwdp, shadow.h, [AC_CHECK_FUNC(getspnam, GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_SHADOW_H")])

# SCO-ODT-3.0 is reported to need -lufc for crypt.
# NetBSD needs -lcrypt for crypt.
AC_SEARCH_LIBS(crypt, [ufc crypt], [LIB_CRYPT="$ac_cv_search_crypt"])
GNOMESU_BACKEND_LIBS="$GNOMESU_BACKEND_LIBS $LIB_CRYPT"

# From Jim Meyering.
# Determine whether malloc accepts 0 as its argument.
# If it doesn't, arrange to use the replacement function.
# xmalloc.c requires that this symbol be defined so it doesn't
# mistakenly use a broken malloc -- as it might if this test were omitted.
GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_DONE_WORKING_MALLOC_CHECK=1"
AC_CACHE_CHECK([for working malloc], jm_cv_func_working_malloc,
  [AC_TRY_RUN([
    char *malloc ();
    int
    main ()
    {
      exit (malloc (0) ? 0 : 1);
    }
	  ],
	 jm_cv_func_working_malloc=yes,
	 jm_cv_func_working_malloc=no,
	 dnl When crosscompiling, assume malloc is broken.
	 jm_cv_func_working_malloc=no)
  ])
if test $jm_cv_func_working_malloc = no; then
    GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DRPL_MALLOC -Dmalloc=rpl_malloc"
fi


# From Jim Meyering.
# Determine whether realloc works when both arguments are 0.
# If it doesn't, arrange to use the replacement function.
# xmalloc.c requires that this symbol be defined so it doesn't
# mistakenly use a broken realloc -- as it might if this test were omitted.
GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_DONE_WORKING_REALLOC_CHECK=1"
AC_CACHE_CHECK([for working realloc], jm_cv_func_working_realloc,
  [AC_TRY_RUN([
    char *realloc ();
    int
    main ()
    {
      exit (realloc (0, 0) ? 0 : 1);
    }
	  ],
	 jm_cv_func_working_realloc=yes,
	 jm_cv_func_working_realloc=no,
	 dnl When crosscompiling, assume realloc is broken.
	 jm_cv_func_working_realloc=no)
  ])
if test $jm_cv_func_working_realloc = no; then
    GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DRPL_REALLOC -Drealloc=rpl_realloc"
fi

AC_CHECK_FUNC(initgroups, [
  GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_INITGROUPS"
  GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DHAVE_INITGROUPS"
])
AC_CHECK_FUNC(endgrent, [
  GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_ENDGRENT"
  GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DHAVE_ENDGRENT"
])
AC_CHECK_FUNC(endpwent, [
  GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_ENDPWENT"
  GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DHAVE_ENDPWENT"
])
AC_CHECK_HEADER(syslog.h, GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_SYSLOG_H")
AC_CHECK_HEADER(sys/fsuid.h, [
	GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DHAVE_FSUID_H"
	AC_CHECK_FUNC(setfsuid, GNOMESU_PAM_BACKEND_CFLAGS="$GNOMESU_PAM_BACKEND_CFLAGS -DHAVE_SETFSUID")
])

AC_CHECK_HEADER(error.h, [
	AC_CHECK_FUNC(error, GNOMESU_BACKEND_CFLAGS="$GNOMESU_BACKEND_CFLAGS -DHAVE_ERROR_H")
])


AC_SUBST(GNOMESU_BACKEND_CFLAGS)
AC_SUBST(GNOMESU_BACKEND_LIBS)
AC_SUBST(GNOMESU_PAM_BACKEND_CFLAGS)
AC_SUBST(GNOMESU_PAM_BACKEND_LIBS)

AC_MSG_CHECKING(whether a setuid error should abort installation)
AC_ARG_ENABLE(setuid-error,
    [  --disable-setuid-error   Don't make 'make install' fail if it's unable to make
                           gnomesu-backend setuid root.],
    [case "${enableval}" in
     yes) setuid_error=yes  ;;
     no)  setuid_error=no ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-setuid-error) ;;
     esac],
    [setuid_error=yes])
AC_MSG_RESULT($setuid_error)
AM_CONDITIONAL(SETUID_ERROR, test $setuid_error = yes)


AC_ARG_ENABLE(install-pam, [  --disable-install-pam    Do not install PAM entry. (default=yes)],,enableval=yes)
AM_CONDITIONAL(INSTALL_PAM, test "$enableval" = "yes")

AC_ARG_WITH(pam-dir, [  --with-pam-dir=PATH     where to install PAM entries to (default=/etc/pam.d)],,withval=/etc/pam.d)
if test "$withval" != ""; then
	PAMDIR=$withval
else
	PAMDIR=/etc/pam.d
fi
AC_SUBST(PAMDIR)

AC_ARG_VAR([SUID_CFLAGS],
	   [CFLAGS used for binaries which are usually with the suid bit])
AC_ARG_VAR([SUID_LDFLAGS],
	   [LDFLAGS used for binaries which are usually with the suid bit])

AM_BINRELOC


AC_OUTPUT([
libgnomesu-1.0.pc
Makefile
src/Makefile
src/services/Makefile
su-backend/Makefile
pam-backend/Makefile
tools/Makefile
po/Makefile.in
doc/Makefile
])
