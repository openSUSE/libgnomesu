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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <glib.h>
#include <glade/glade.h>

G_BEGIN_DECLS


/* These functions are private and should not be used by applications */

gchar *__libgnomesu_create_command (gchar **argv);
void __libgnomesu_replace_all (gchar **str, gchar *from, gchar *to);
gchar **__libgnomesu_generate_env (gchar *user);

guint __libgnomesu_count_args (gchar **argv);

GladeXML *__libgnomesu_load_glade (gchar *basename);

void __libgnomesu_init (void);


G_END_DECLS

#endif /* _UTILS_H_ */
