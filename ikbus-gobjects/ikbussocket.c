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

#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include "ikbus.h"
#include "ikbussocket.h"

typedef enum
{
  STATE_NONE,
  STATE_SOCKET,
  STATE_CONNECTED
} IKBusSocketState;

struct _IKBusSocketPrivate
{
  gchar *ifname;
  IKBusSocketAddres sock_addr;
  IKBusSocketAddres conn_addr;
  gint fd;
  IKBusSocketState state;
};

static void ikbus_socket_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (IKBusSocket, ikbus_socket, G_TYPE_OBJECT,
    G_ADD_PRIVATE (IKBusSocket) G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, ikbus_socket_initable_iface_init))

enum
{
  PROP_0,
  PROP_IFNAME,
  PROP_SOCK_ADDR,
  PROP_CONN_ADDR,
  N_PROP
};

static GParamSpec *obj_properties[N_PROP] = { NULL, };

static void
ikbus_socket_finalize (GObject *object)
{
  IKBusSocket *sock = IKBUS_SOCKET (object);

  g_free (sock->priv->ifname);
  G_OBJECT_CLASS (ikbus_socket_parent_class)->finalize (object);
}

static void
ikbus_socket_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  IKBusSocket *sock = IKBUS_SOCKET (object);

  switch (property_id)
    {
      case PROP_IFNAME:
        g_value_set_string (value, sock->priv->ifname);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ikbus_socket_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  IKBusSocket *sock = IKBUS_SOCKET (object);

  switch (property_id)
    {
      case PROP_IFNAME:
        g_free (sock->priv->ifname);
        sock->priv->ifname = g_strdup (g_value_get_string (value));
        break;
      case PROP_SOCK_ADDR:
        sock->priv->sock_addr = g_value_get_uchar (value);
        break;
      case PROP_CONN_ADDR:
        sock->priv->conn_addr = g_value_get_uchar (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

gboolean
ikbus_socket_connect (IKBusSocket* sock,
                      IKBusSocketAddres addr,
                      IKBusSocketAddres conn,
                      GError **error)
{
  struct sockaddr_ikbus ikbus_addr;
  struct ifreq ifr;
  struct ikbus_filter filter;

  g_return_val_if_fail (IKBUS_IS_SOCKET (sock), FALSE);

  if (sock->priv->state != STATE_SOCKET)
  {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                         "Unsuitable state of the I/K-bus socket");
    return FALSE;
  }

  /* Set up incoming filter */
    filter.id_rx = addr;
    filter.id_tx = conn;
    if (setsockopt (sock->priv->fd, SOL_IKBUS, IKBUS_FILTER, &filter, sizeof (filter))< 0)
    {
      int errsv = errno;
      g_set_error (error,
                   G_IO_ERROR,
                   g_io_error_from_errno (errsv),
                   "Error setting socket options: %s", g_strerror (errsv));
    }

  /* Choose net interface */
  ikbus_addr.ifindex = 0;
  if (sock->priv->ifname != NULL)
  {
    g_stpcpy (ifr.ifr_name, sock->priv->ifname);
    ioctl (sock->priv->fd, SIOCGIFINDEX, &ifr);
    ikbus_addr.ifindex = ifr.ifr_ifindex;
  }
  ikbus_addr.ikbus_family = AF_IKBUS;

  /* Bind I/K-bus socket */
  if (bind (sock->priv->fd, (struct sockaddr *) &ikbus_addr, sizeof (ikbus_addr)) < 0)
  {
    int errsv = errno;
    g_set_error (error,
                 G_IO_ERROR,
                 g_io_error_from_errno (errsv),
                 "Error binding to %s: %s", sock->priv->ifname,g_strerror (errsv));
    return FALSE;
  }

  sock->priv->state = STATE_CONNECTED;
  sock->priv->sock_addr = addr;
  sock->priv->conn_addr = conn;
  return TRUE;
}

gint
ikbus_socket_get_fd (IKBusSocket* sock)
{
  

  g_return_val_if_fail (IKBUS_IS_SOCKET (sock), -1);

  return sock->priv->fd;
}

gint
ikbus_socket_read (IKBusSocket *sock, guint8 *buf)
{
  gint ret = -1;
  g_return_val_if_fail (IKBUS_IS_SOCKET (sock), ret);

  if (sock->priv->state == STATE_CONNECTED)
    ret = read (sock->priv->fd, buf, IKBUS_MAX_FRAME_SIZE);

  return ret;
}

gint
ikbus_socket_write (IKBusSocket *sock, const guint8 *buf, gint nbytes)
{
  gint ret = -1;
  g_return_val_if_fail (IKBUS_IS_SOCKET (sock), ret);

  if (sock->priv->state == STATE_CONNECTED)
    ret = write (sock->priv->fd, buf, nbytes);

  return ret;
}

static gboolean
ikbus_socket_initable_init (GInitable *initable,
                            GCancellable *cancellable,
                            GError  **error)
{
  IKBusSocket *sock;
  gint sock_fd;

  g_return_val_if_fail (IKBUS_IS_SOCKET (initable), FALSE);
  sock = IKBUS_SOCKET (initable);

  if (sock->priv->state >= STATE_SOCKET)
    return TRUE;

  sock_fd = socket (PF_IKBUS, SOCK_RAW, 0);
  if (sock_fd < 0)
  {
    int errsv = errno;
    g_set_error (error,
                 G_IO_ERROR,
                 g_io_error_from_errno (errsv),
                 "Fail to create I/K-bus socket");
    return FALSE;
  }

  sock->priv->fd = sock_fd;
  sock->priv->state = STATE_SOCKET;

  return TRUE;
}

static void
ikbus_socket_initable_iface_init (GInitableIface *iface)
{
  iface->init = ikbus_socket_initable_init;
}

static void
ikbus_socket_class_init (IKBusSocketClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ikbus_socket_finalize;
  object_class->get_property = ikbus_socket_get_property;
  object_class->set_property = ikbus_socket_set_property;

  obj_properties[PROP_IFNAME] = g_param_spec_string ("ifname",
                                    "Interface name",
                                    "The name of the I/K-bus network interface",
                                    NULL, /* default */
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  obj_properties[PROP_SOCK_ADDR] = g_param_spec_uchar ("sockaddr",
                                    "Socket address",
                                    "Socket address of the I/K-bus network",
                                    0x00, 0xff, 0xff, /* default */
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  obj_properties[PROP_CONN_ADDR] = g_param_spec_uchar ("connaddr",
                                    "Connection address",
                                    "Interlocutor address of the I/K-bus network",
                                    0x00, 0xff, 0xff, /* default */
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROP, obj_properties);
}

static void
ikbus_socket_init (IKBusSocket *sock)
{
  sock->priv = ikbus_socket_get_instance_private (sock);
  sock->priv->state = STATE_NONE;
}

IKBusSocket*
ikbus_socket_new (gchar *ifname, GError **error)
{
  return IKBUS_SOCKET (g_initable_new (IKBUS_TYPE_SOCKET, NULL, 
                                        error, "ifname", ifname, NULL));
}
