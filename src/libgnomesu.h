/* libgnomesu - Library for providing superuser privileges to GNOME apps.
 * Copyright (C) 2003  Hongli Lai
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

#ifndef _LIBGNOMESU_H_
#define _LIBGNOMESU_H_

#include <glib.h>

G_BEGIN_DECLS


/* One simple, easy-to-use function */
gboolean gnome_su_exec (gchar *commandline);


/* More complex functions */
gboolean gnome_su_spawn_command_sync (gchar *user, gchar *commandline);
gboolean gnome_su_spawn_command_async (gchar *user, gchar *commandline, int *pid);
gboolean gnome_su_spawn_sync (gchar *user, gchar **argv);
gboolean gnome_su_spawn_async (gchar *user, gchar **argv, int *pid);


G_END_DECLS

#endif /* _LIBGNOMESU_H_ */
