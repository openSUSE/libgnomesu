/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003,2004,2005  Hongli Lai
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _UTILS_C_
#define _UTILS_C_

#include <libintl.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "utils.h"
#include "prefix.h"

G_BEGIN_DECLS

extern char **environ;


/* Changes an argument vector into a commandline.
   Escape special characters. */
gchar *
LGSD(create_command) (const gchar **argv)
{
	GString *result;
	gchar *str;
	guint i;

	result = g_string_new ("");
	i = 0;
	while (argv[i] != NULL)
	{
		gchar *tmp;

		tmp = g_strdup (argv[i]);
		LGSD(replace_all) (&tmp, "\"", "\\\"");
		LGSD(replace_all) (&tmp, ">", "\\>");
		LGSD(replace_all) (&tmp, "<", "\\<");
		LGSD(replace_all) (&tmp, " ", "\\ ");
		LGSD(replace_all) (&tmp, "!", "\\!");
		LGSD(replace_all) (&tmp, "$", "\\$");
		LGSD(replace_all) (&tmp, "&", "\\&");
		if (!*tmp)
			g_string_append (result, "\"\" ");
		else
			g_string_append_printf (result, "%s ", tmp);
		g_free (tmp);
		i++;
	}

	str = result->str;
	str[strlen (str) - 1] = 0; /* Get rid of trailing whitespace */
	g_string_free (result, FALSE);
	return str;
}


/* Replace all occurences of from in string str to to. */
void
LGSD(replace_all) (gchar **str, gchar *from, gchar *to)
{
	GString *newstr;
	gchar *found;

	g_return_if_fail (str != NULL);
	g_return_if_fail (from != NULL);
	g_return_if_fail (to != NULL);

	newstr = g_string_new (*str);
	found = strstr (newstr->str, from);
	while (found != NULL)
	{
		gint pos;

		pos = GPOINTER_TO_INT (found) - GPOINTER_TO_INT (newstr->str);
		g_string_erase (newstr, pos, strlen (from));
		g_string_insert (newstr, pos, to);
		found = GINT_TO_POINTER (GPOINTER_TO_INT (found) + strlen (to));
		found = strstr (found, from);
	}

	g_free (*str);
	*str = newstr->str;
	g_string_free (newstr, FALSE);
}


/* Copy the current environment and modify some */
gchar **
LGSD(generate_env) (gchar *user)
{
	GList *list = NULL;
	gchar **env;
	gboolean has_xauth = FALSE, has_iceauth = FALSE, has_home = FALSE, has_user = FALSE;
	gint i = 0;
	struct passwd *pw;

	if (!user)
		user = "root";
	pw = getpwnam (user);

	while (environ[i])
	{
		if (!has_xauth && strncmp (environ[i], "XAUTHORITY=", 11) == 0)
		{
			list = g_list_append (list, g_strdup_printf ("XAUTHORITY=%s/.Xauthority", pw->pw_dir));
			has_xauth = TRUE;
		}
		if (!has_iceauth && strncmp (environ[i], "ICEAUTHORITY=", 13) == 0)
		{
			list = g_list_append (list, g_strdup_printf ("ICEAUTHORITY=%s/.ICEauthority", pw->pw_dir));
			has_iceauth = TRUE;
		}
		if (!has_home && strncmp (environ[i], "HOME=", 5) == 0)
		{
			list = g_list_append (list, g_strdup_printf ("HOME=%s", pw->pw_dir));
			has_home = TRUE;
		}
		if (!has_user && strncmp (environ[i], "USER=", 5) == 0)
		{
			list = g_list_append (list, g_strdup_printf ("USER=%s", user));
			has_user = TRUE;
		} else
			list = g_list_append (list, g_strdup (environ[i]));

		i++;
	}

	if (!has_xauth)
		list = g_list_append (list, g_strdup_printf ("XAUTHORITY=%s/.Xauthority", pw->pw_dir));
	if (!has_iceauth)
		list = g_list_append (list, g_strdup_printf ("ICEAUTHORITY=%s/.ICEauthority", pw->pw_dir));
	if (!has_home)
		list = g_list_append (list, g_strdup_printf ("HOME=%s", pw->pw_dir));
	if (!has_user)
		list = g_list_append (list, g_strdup_printf ("USER=%s", user));

	env = g_new0 (gchar *, g_list_length (list) + 1);
	i = 0;
	for (list = g_list_first (list); list != NULL; list = list->next)
	{
		env[i] = list->data;
		i++;
	}
	g_list_free (list);
	return env;
}


void *
LGSD(safe_memset) (void *s, int c, size_t n)
{
	/* Works around compiler optimizations which removes memset().
	   See http://bugzilla.gnome.org/show_bug.cgi?id=161213 */
	return memset (s, c, n);
}


GList *
LGSD(g_list_addv) (GList *list, const gchar **argv)
{
	guint i, size;

	size = LGSD(count_args) (argv);
	for (i = 0; i < size; i++) {
		list = g_list_append (list, (gpointer) argv[i]);
	}
	return list;
}


gchar **
LGSD(g_list_to_vector) (GList *list, guint *size)
{
	gchar **vec;
	GList *tmp;
	guint i = 0;

	g_return_val_if_fail (list != NULL, NULL);

	vec = g_new0 (gchar *, g_list_length (list) + 1);
	for (tmp = list; tmp; tmp = tmp->next) {
		vec[i] = (gchar *) tmp->data;
		i++;
	}

	if (size)
		*size = i;
	return vec;
}


guint
LGSD(count_args) (const gchar **argv)
{
	guint n = 0;

	g_return_val_if_fail (argv != NULL, 0);

	for (n = 0; *argv; argv++)
		n++;
	return n;
}


/* Initialize gettext stuff */
void
LGSD(libgnomesu_init) (void)
{
	static int initialized = 0;

	if (initialized == 1)
		return;

	initialized = 1;
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}


G_END_DECLS

#endif /* _UTILS_C_ */
