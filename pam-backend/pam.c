/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003  Hongli Lai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <security/pam_appl.h>
#ifdef PAM_MOD_MISC
	#include <security/pam_mod_misc.h>
#else /* PAM_MOD_MISC */
	#include <security/pam_misc.h>
#endif /* PAM_MOD_MISC */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include <grp.h>
#include <sys/types.h>
#ifdef HAVE_FSUID_H
#	include <sys/fsuid.h>
#endif

#include "common.h"


#define PROTOCOL_ASK_PASS		"ASK_PASS\n"		/* Need password */
#define PROTOCOL_INCORRECT_PASSWORD	"INCORRECT_PASSWORD\n"	/* Entered password is incorrect */

/* One of the following messages are printed on exit */
#define PROTOCOL_PASSWORD_FAIL		"PASSWORD_FAIL\n"	/* Entered incorrect password too many times */
#define PROTOCOL_DONE			"DONE\n"		/* Success */
#define PROTOCOL_NO_SUCH_USER		"NO_SUCH_USER\n"	/* USER doesn't exist */
#define PROTOCOL_INIT_ERROR		"INIT_ERROR\n"		/* Unable to initialize PAM */
#define PROTOCOL_ERROR			"ERROR\n"		/* Unknown error */
#define PROTOCOL_AUTH_DENIED		"DENIED\n"		/* User is not allowed to authenticate itself */

#define DEFAULT_USER "root"


static FILE *inf, *outf;
static gboolean Abort = FALSE;
const gchar *new_user;


static int
su_conv (int num_msg, const struct pam_message **msg, struct pam_response **resp, void *data)
{
	int i;
	struct pam_response *reply = NULL;

	reply = g_new0 (struct pam_response, num_msg + 1);

	for (i = 0; i < num_msg; i++) {
	switch (msg[i]->msg_style)
	{
	case PAM_PROMPT_ECHO_ON:
		reply[i].resp_retcode = PAM_SUCCESS;
		break;

	case PAM_PROMPT_ECHO_OFF:
		if (strncasecmp (msg[i]->msg, "password", 8) == 0)
		{
			gchar password[1024];

			fprintf (outf, PROTOCOL_ASK_PASS);
			safe_memset (password, 0, sizeof (password));
			if (!fgets (password, sizeof (password), inf)) {
				safe_memset (password, 0, sizeof (password));
				fprintf (outf, PROTOCOL_ERROR);
				Abort = TRUE;
				return PAM_ABORT;
			}

			if (strlen (password) && password[strlen (password) - 1] == 10)
				password[strlen (password) - 1] = 0;
			reply[i].resp = g_strdup (password);
			safe_memset (password, 0, sizeof (password));
			reply[i].resp_retcode = PAM_SUCCESS;
		}

		break;

	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
		/* Ignore it... */
		reply[i].resp_retcode = PAM_SUCCESS;
		break;
	default:
		break;
	}
	}

	if (resp != NULL) {
		*resp = reply;
	}

	return PAM_SUCCESS;
}


static const struct pam_conv conv =
{
	su_conv,
	NULL
};


extern char *concat (const char *s1, const char *s2, const char *s3);
extern void xputenv (const char *val);
extern void change_identity (const struct passwd *pw);
extern void modify_environment (const struct passwd *pw);


static void
close_pam (pam_handle_t *pamh, int retval)
{
	if (pam_end (pamh, retval) != PAM_SUCCESS)
	{
		/* Closing PAM failed?! */
		fprintf (stderr, "Failed to close PAM\n");
		if (outf)
			fprintf (outf, PROTOCOL_ERROR);
		exit (1);
	}
}


static struct passwd *
init (int argc, char **argv)
{
	int infd, outfd;
	struct passwd *pw;


	if (argv[1] && strcmp (argv[1], "--version") == 0)
	{
		g_print ("%s\n", VERSION);
		exit (0);
	}

	if (!g_getenv ("_GNOMESU_PAM_BACKEND_START") || strcmp (g_getenv ("_GNOMESU_PAM_BACKEND_START"), "1") != 0)
	{
		fprintf (stderr, "This program is for internal use only! Never run this program directly!\n");
		exit (1);
	}
	unsetenv ("_GNOMESU_PAM_BACKEND_START");


	/* Parse arguments */
	if (argc < 5) {
		fprintf (stderr, "Too little arguments.\n");
		exit (1);
	}

	new_user = argv[3];
	if (new_user[0] == '\0')
		new_user = DEFAULT_USER;

	infd = atoi (argv[1]);
	outfd = atoi (argv[2]);

	if (infd <= 2 || outfd <= 2) {
		fprintf (stderr, "Invalid file descriptors.\n");
		exit (1);
	}

	inf = fdopen (infd, "r");
	if (!inf) {
		fprintf (stderr, "Cannot fopen() INFD\n");
		exit (1);
	}

	outf = fdopen (outfd, "w");
	if (!outf) {
		fprintf (stderr, "Cannot fopen() OUTFD\n");
		exit (1);
	}
	setlinebuf (outf);


	/* Check whether the given user exists */
	pw = getpwnam (new_user);
	if (!pw) {
		fprintf (outf, PROTOCOL_NO_SUCH_USER);
		exit (1);
	}

	/* Make a copy of the password information and point pw at the local
	   copy instead.  Otherwise, some systems (e.g. Linux) would clobber
	   the static data through the getlogin call from log_su.  */
	pw = g_memdup (pw, sizeof (struct passwd));
	pw->pw_name = g_strdup (pw->pw_name);
	pw->pw_dir = g_strdup (pw->pw_dir);
	pw->pw_shell = g_strdup (pw->pw_shell);
	endpwent ();
	return pw;
}


/*  Usage: gnomesu-pam-backend <INFD> <OUTFD> <USER> <COMMAND> [ARG1] [ARGn..]
 *  gnomesu-pam-backend uses file descriptors INFD and OUTFD to communicate with the parent.
 *  INFD is used to retrieve the password. OUTFD is used to print messages (see the PROTOCOL_* macros).
 */

int
main (int argc, char *argv[])
{
	struct passwd *pw;
	gboolean authenticated = FALSE;
	pam_handle_t *pamh = NULL;
	int retval, i;

	pw = init (argc, argv);

	if (pam_start ("gnomesu-pam", new_user, &conv, &pamh) != PAM_SUCCESS) {
		fprintf (outf, PROTOCOL_INIT_ERROR);
		exit (1);
	}

	if (pam_set_item (pamh, PAM_RUSER, g_get_user_name ()) != PAM_SUCCESS) {
		fprintf (outf, PROTOCOL_INIT_ERROR);
		exit (1);
	}

	/* Ask for password up to 3 times */
	for (i = 0; i < 3; i++)	{
		/* Start the authentication */
		retval = pam_authenticate (pamh, 0);

		if (retval != PAM_AUTH_ERR || Abort)
			break;
		else
			fprintf (outf, PROTOCOL_INCORRECT_PASSWORD);
	}
	if (i >= 3)
		return 1;

	if (retval == PAM_SUCCESS) {
		/* Is the user permitted to access this account? */
		retval = pam_acct_mgmt (pamh, 0);

		if (retval == PAM_SUCCESS)
			authenticated = TRUE;
		else
			fprintf (outf, PROTOCOL_AUTH_DENIED);
	} else
		fprintf (outf, PROTOCOL_PASSWORD_FAIL);


	if (Abort)
	{
		close_pam (pamh, retval);
		fprintf (outf, PROTOCOL_ERROR);
		return 1;
	}

	if (authenticated)
	{
		char **command = argv + 4;
		pid_t pid;
		int exitCode = 1, status;

		modify_environment (pw);
		#ifdef HAVE_SETFSUID
		setfsuid (pw->pw_uid);
		#endif /* HAVE_SETFSUID */
		change_identity (pw);
		initgroups (pw->pw_name, pw->pw_gid);
		setgid (pw->pw_gid);
		setuid (pw->pw_uid);

		retval = pam_setcred (pamh, PAM_ESTABLISH_CRED);
		if (retval != PAM_SUCCESS)
			fprintf (stderr, "Warning: %s\n", pam_strerror (pamh, retval));

		pam_open_session (pamh, 0);
		pid = fork ();
		switch (pid)
		{
		case -1:
			fprintf (outf, PROTOCOL_ERROR);
			fclose (inf);
			fclose (outf);
			outf = NULL;
			break;
		case 0:
			change_identity (pw);
			execvp (command[0], command);
			_exit (1);
			break;
		default:
			fprintf (outf, PROTOCOL_DONE);
			fclose (inf);
			fclose (outf);
			outf = NULL;
			waitpid (pid, &status, 0);
			exitCode = WEXITSTATUS (status);
			break;
		}
		pam_close_session (pamh, 0);
		close_pam (pamh, retval);

		/* evecvp() failed */
		return exitCode;

	} else
	{
		close_pam (pamh, retval);
		return 1;
	}
}
