/* su for GNU.  Run a shell with substitute user and group IDs.
   Copyright (C) 1992-1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*
   Compile-time options:
   -DSYSLOG_SUCCESS     Log successful su's (by default, to root) with syslog.
   -DSYSLOG_FAILURE     Log failed su's (by default, to root) with syslog.

   -DSYSLOG_NON_ROOT    Log all su's, not just those to root (UID 0).
   Never logs attempted su's to nonexistent accounts.

   Based on su written by David MacKenzie <djm@gnu.ai.mit.edu>.
 */

/*  Usage: gnomesu-backend <INFD> <OUTFD> <USER> <COMMAND> [ARG1] [ARGn..]
 *  gnomesu-backend uses file descriptors INFD and OUTFD to communicate with the parent.
 *  INFD is used to retrieve the password. OUTFD is used to print messages (see the PROTOCOL_* macros).
 */


#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#define GNU_PACKAGE PROGRAM_NAME

#define PROTOCOL_ASK_PASS		"ASK_PASS\n"		/* Need password */
#define PROTOCOL_INCORRECT_PASSWORD	"INCORRECT_PASSWORD\n"	/* Entered password is incorrect */

/* One of the following messages are printed on exit */
#define PROTOCOL_PASSWORD_FAIL		"PASSWORD_FAIL\n"	/* Entered incorrect password too many times */
#define PROTOCOL_DONE			"DONE\n"		/* Success */
#define PROTOCOL_NO_SUCH_USER		"NO_SUCH_USER\n"	/* USER doesn't exist */
#define PROTOCOL_ERROR			"ERROR\n"		/* Unknown error */
#define PROTOCOL_AUTH_DENIED		"DENIED\n"		/* User is not allowed to authenticate itself */


/* Hide any system prototype for getusershell.
   This is necessary because some Cray systems have a conflicting
   prototype (returning `int') in <unistd.h>.  */
#define getusershell _getusershell_sys_proto_

#include "system.h"
#include "errmsg.h"

#undef getusershell

#if HAVE_SYSLOG_H && HAVE_SYSLOG
# include <syslog.h>
#else
# undef SYSLOG_SUCCESS
# undef SYSLOG_FAILURE
# undef SYSLOG_NON_ROOT
#endif

#ifndef _POSIX_VERSION
struct passwd *getpwuid ();
struct group *getgrgid ();
uid_t getuid ();
# include <sys/param.h>
#endif /* not _POSIX_VERSION */

#ifndef HAVE_ENDGRENT
# define endgrent() ((void) 0)
#endif

#ifndef HAVE_ENDPWENT
# define endpwent() ((void) 0)
#endif

#if HAVE_SHADOW_H
# include <shadow.h>
#endif

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "su"

#define AUTHORS "David MacKenzie"

/* The shell to run if none is given in the user's passwd entry.  */
#define DEFAULT_SHELL "/bin/sh"

/* The user to become if none is specified.  */
#define DEFAULT_USER "root"

char *crypt(const char *key, const char *salt);
char *getusershell ();
void endusershell ();
void setusershell ();

char *base_name ();
char *xstrdup (char *pw_name);

extern char **environ;

/* The name this program was run with.  */
char *program_name;

FILE *inf, *outf;


#if defined (SYSLOG_SUCCESS) || defined (SYSLOG_FAILURE)
/* Log the fact that someone has run su to the user given by PW;
   if SUCCESSFUL is nonzero, they gave the correct password, etc.  */

static void
log_su (const struct passwd *pw, int successful)
{
  const char *new_user, *old_user, *tty;

# ifndef SYSLOG_NON_ROOT
  if (pw->pw_uid)
    return;
# endif
  new_user = pw->pw_name;
  /* The utmp entry (via getlogin) is probably the best way to identify
     the user, especially if someone su's from a su-shell.  */
  old_user = getlogin ();
  if (old_user == NULL)
    {
      /* getlogin can fail -- usually due to lack of utmp entry.
	 Resort to getpwuid.  */
      struct passwd *pwd = getpwuid (getuid ());
      old_user = (pwd ? pwd->pw_name : "");
    }
  tty = ttyname (2);
  if (tty == NULL)
    tty = "none";
  /* 4.2BSD openlog doesn't have the third parameter.  */
  openlog (base_name (program_name), 0
# ifdef LOG_AUTH
	   , LOG_AUTH
# endif
	   );
  syslog (LOG_NOTICE,
# ifdef SYSLOG_NON_ROOT
	  "%s(to %s) %s on %s",
# else
	  "%s%s on %s",
# endif
	  successful ? "" : "FAILED SU ",
# ifdef SYSLOG_NON_ROOT
	  new_user,
# endif
	  old_user, tty);
  closelog ();
}
#endif

/* Ask the user for a password.
   Return 1 if the user gives the correct password for entry PW,
   0 if not.  Return 1 without asking for a password if run by UID 0
   or if PW has an empty password.  */

static int
correct_password (const struct passwd *pw)
{
  char unencrypted[1024], *encrypted, *correct;
#ifdef HAVE_SHADOW_H
  /* Shadow passwd stuff for SVR3 and maybe other systems.  */
  struct spwd *sp = getspnam (pw->pw_name);

  endspent ();
  if (sp)
    correct = sp->sp_pwdp;
  else
#endif
    correct = pw->pw_passwd;

  if (getuid () == 0 || correct == 0 || correct[0] == '\0')
    return 1;

  fprintf (outf, PROTOCOL_ASK_PASS);
  memset (unencrypted, 0, sizeof (unencrypted));
  if (fgets (unencrypted, sizeof (unencrypted), inf) == NULL)
    {
      fprintf (outf, PROTOCOL_ERROR);
      exit (1);
    }

  if (strlen (unencrypted) && unencrypted[strlen (unencrypted) - 1] == 10)
      unencrypted[strlen (unencrypted) - 1] = 0;

  encrypted = crypt (unencrypted, correct);
  memset (unencrypted, 0, sizeof (unencrypted));
  return strcmp (encrypted, correct) == 0;
}


char *concat (const char *s1, const char *s2, const char *s3);
void xputenv (const char *val);
void change_identity (const struct passwd *pw);
void modify_environment (const struct passwd *pw);


int
main (int argc, char **argv)
{
  const char *new_user;
  char **command = NULL;
  struct passwd *pw;
  struct passwd pw_copy;
  int infd, outfd, i;
  struct rlimit rlp;

  if (argv[1] && strcmp (argv[1], "--version") == 0)
  {
      printf ("%s\n", VERSION);
      return 0;
  }

  if (!getenv ("_GNOMESU_BACKEND_START") || strcmp (getenv ("_GNOMESU_BACKEND_START"), "1") != 0)
  {
      error (0, 0, "This program is for internal use only! Never run this program directly!");
      return 1;
  }
  unsetenv ("_GNOMESU_BACKEND_START");

  program_name = argv[0];


  /* Parse arguments */
  if (argc < 5)
  {
      error (0, 0, "Too little arguments.");
      return 1;
  }
  new_user = argv[3];
  if (new_user[0] == '\0')
      new_user = DEFAULT_USER;

  infd = atoi (argv[1]);
  outfd = atoi (argv[2]);
  if (infd <= 2 || outfd <= 2)
  {
      error (0, 0, "Invalid file descriptors.");
      return 1;
  }
  inf = fdopen (infd, "r");
  if (!inf)
  {
      error (0, 0, "Cannot fopen() INFD");
      return 1;
  }
  outf = fdopen (outfd, "w");
  if (!outf)
  {
      error (0, 0, "Cannot fopen() OUTFD");
      return 1;
  }
  setlinebuf (outf);

  command = argv + 4;

  pw = getpwnam (new_user);
  if (pw == 0)
  {
    fprintf (outf, PROTOCOL_NO_SUCH_USER);
    return 1;
  }
  endpwent ();

  /* Make sure pw->pw_shell is non-NULL.  It may be NULL when NEW_USER
     is a username that is retrieved via NIS (YP), but that doesn't have
     a default shell listed.  */
  if (pw->pw_shell == NULL || pw->pw_shell[0] == '\0')
    pw->pw_shell = (char *) DEFAULT_SHELL;

  /* Make a copy of the password information and point pw at the local
     copy instead.  Otherwise, some systems (e.g. Linux) would clobber
     the static data through the getlogin call from log_su.  */
  pw_copy = *pw;
  pw = &pw_copy;
  pw->pw_name = xstrdup (pw->pw_name);
  pw->pw_dir = xstrdup (pw->pw_dir);
  pw->pw_shell = xstrdup (pw->pw_shell);

  /* Ask for password up to 3 times */
  for (i = 0; i < 3; i++)
  {
      if (!correct_password (pw))
        {
#ifdef SYSLOG_FAILURE
          log_su (pw, 0);
#endif
          usleep (2500000);
          fprintf (outf, PROTOCOL_INCORRECT_PASSWORD);
          if (i >= 2)
          {
              fprintf (outf, PROTOCOL_PASSWORD_FAIL);
              return 1;
          }
        }
      else
        {
#ifdef SYSLOG_SUCCESS
          log_su (pw, 1);
#endif
          break;
        }
  }

  modify_environment (pw);
  change_identity (pw);

  fprintf (outf, PROTOCOL_DONE);
  fclose (inf);
  fclose (outf);

  /* Close all file handles except stdin/out/err */
  getrlimit (RLIMIT_NOFILE, &rlp);
  for (i = 3; i < (int) rlp.rlim_cur; i++)
      close (i);

  execvp (command[0], command);
  /* This should never be reached! */
  return 1;
}
