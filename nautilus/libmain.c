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
#include <bonobo/Bonobo.h>
#include <bonobo-activation/bonobo-activation.h>
#include "gnome-su-component.h"

G_BEGIN_DECLS


#define VIEW_IID_OPENFOLDER	"OAFIID:GNOME_SuperUser_Component_OpenFolder"


static CORBA_Object
image_shlib_make_object (PortableServer_POA poa,
			 const char *iid,
			 gpointer impl_ptr,
			 CORBA_Environment *ev)
{
	GnomeSuComponent *gsc;

	gsc = g_object_new (TYPE_GNOME_SU_COMPONENT, NULL);

	bonobo_activation_plugin_use (poa, impl_ptr);

	return CORBA_Object_duplicate (BONOBO_OBJREF (gsc), ev);
}


static const BonoboActivationPluginObject plugin_list[] = {
	{ VIEW_IID_OPENFOLDER, image_shlib_make_object },
	{ NULL }
};


const BonoboActivationPlugin Bonobo_Plugin_info = {
	plugin_list,
	"GNOME SuperUser Component"
};


G_END_DECLS
