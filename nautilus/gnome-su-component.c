/* 
 * Gnome SuperUser
 * Copyright (C) 2003  Hongli Lai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdlib.h>
#include "libgnomesu.h"
#include "gnome-su-component.h"

G_BEGIN_DECLS


static void
impl_Bonobo_Listener_event (PortableServer_Servant servant,
			    const CORBA_char *event_name,
			    const CORBA_any *args,
			    CORBA_Environment *ev)
{
	GnomeSuComponent *gsc;
	const CORBA_sequence_CORBA_string *list;
	gchar **argv;
	gint i;

	gsc = GNOME_SU_COMPONENT (bonobo_object_from_servant (servant));

	if (!CORBA_TypeCode_equivalent (args->_type, TC_CORBA_sequence_CORBA_string, ev)) {
		return;
	}

	list = (CORBA_sequence_CORBA_string *)args->_value;

	g_return_if_fail (gsc != NULL);
	g_return_if_fail (list != NULL);


	if (strcmp (event_name, "OpenFolder") != 0)
		return;


	argv = g_new0 (gchar *, list->_length + 3);
	argv[0] = "nautilus";
	argv[1] = "--no-desktop";
	for (i = 0; i < list->_length; i++)
		argv[i + 2] = list->_buffer[i];
	gnomesu_spawn_async (NULL, argv, NULL);
	g_free (argv);
}


/* initialize the class */
static void
gnome_su_component_class_init (GnomeSuComponentClass *class)
{
	POA_Bonobo_Listener__epv *epv = &class->epv;
	epv->event = impl_Bonobo_Listener_event;
}


static void
gnome_su_component_init (GnomeSuComponent *gsc)
{
}


BONOBO_TYPE_FUNC_FULL (GnomeSuComponent,
		       Bonobo_Listener,
		       BONOBO_TYPE_OBJECT,
		       gnome_su_component);


G_END_DECLS
