/*
 * Copyright 2016 Vladimir Korol <vovabox@mail.ru>
 *
 * This file is part of ikbus-apps.
 *
 * ikbus-apps is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ikbus-apps is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ikbus-apps. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _IKBUSSOCKET_H_
#define _IKBUSSOCKET_H_

#include <glib-object.h>
#include "ikbusframe.h"

G_BEGIN_DECLS

#define IKBUS_TYPE_SOCKET               (ikbus_socket_get_type())
#define IKBUS_SOCKET(obj)               ((G_TYPE_CHECK_INSTANCE_CAST ((obj), IKBUS_TYPE_SOCKET, IKBusSocket)))
#define IKBUS_SOCKET_CLASS(klass)       ((G_TYPE_CHECK_CLASS_CAST ((klass), IKBUS_TYPE_SOCKET, IKBusSocketClass)))
#define IKBUS_IS_SOCKET(obj)            ((G_TYPE_CHECK_INSTANCE_TYPE ((obj), IKBUS_TYPE_SOCKET)))
#define IKBUS_IS_SOCKET_CLASS(klass)    ((G_TYPE_CHECK_CLASS_TYPE ((klass), IKBUS_TYPE_SOCKET)))
#define IKBUS_SOCKET_GET_CLASS(obj)     ((G_TYPE_INSTANCE_GET_CLASS ((obj), IKBUS_TYPE_SOCKET, IKBusSocketClass)))

typedef struct _IKBusSocket        IKBusSocket;
typedef struct _IKBusSocketClass   IKBusSocketClass;
typedef struct _IKBusSocketPrivate IKBusSocketPrivate;
typedef guint8  IKBusSocketAddres;

struct _IKBusSocket {
  GObject parent_instance;
  IKBusSocketPrivate *priv;
};

struct _IKBusSocketClass {
  GObjectClass parent_class;
};

GType ikbus_socket_get_type (void);

IKBusSocket* ikbus_socket_new (gchar *ifname, GError **error);

gboolean ikbus_socket_connect (IKBusSocket *sock, IKBusSocketAddres addr, 
                               IKBusSocketAddres conn, GError **error);
gint ikbus_socket_get_fd (IKBusSocket *sock);
gint ikbus_socket_read (IKBusSocket *sock, guint8 *buf);
gint ikbus_socket_write (IKBusSocket *sock, const guint8 *buf, gint nbytes);
G_END_DECLS

#endif /* _IKBUSSOCKET_H_ */

