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
#include "ikbussocket.h"
#include "ikbuscdc.h"

#define CDC_BUF_SIZE 64
const guint8 CDC_I_AM_HERE[] = 
        {IKBUS_DEV_CDC, 0x04, IKBUS_DEV_LOC, IKBUS_MSG_DEV_STAT_READY, 0x00};

const guint8 CDC_ANNOUNCE[] = 
        {IKBUS_DEV_CDC, 0x04, IKBUS_DEV_LOC, IKBUS_MSG_DEV_STAT_READY, 0x01};

const char CDC_IDENTY[] = {
  IKBUS_DEV_CDC, 0x0f, IKBUS_DEV_DIA, IKBUS_MSG_DIA_ACK,
  0x80, 0x00, 0x00, 0x00,
  0x01, /* Hardware version */
  0x01, /* Code index */
  0x01, /* Diagnostic index */
  0x01, /* I-bus index */
  0x21, /* Week */
  0x16, /* Year */
  0xff, /* Vendor */
  0x01  /* Software version*/
};

struct _IKBusCdcPrivate
{
  gchar *ifname;
  GIOChannel *channel;
  IKBusSocket *iksock;
  gint real_tracknum;

  guint8 *stat_resp;              /* Response status to controlling device */
  guint8 *ack_resp;               /* Response acknowledge to controlling device */

  guint8 *tracknum;               /* Current track */
  guint8 *cdnum;                  /* Current disc */
  guint8 *error_mask;
  guint8 *cd_mask;                /* Mask of presence of discs in the changer */
  guint8 *ctrl_task;              /* Command to playback */
  guint8 *ctrl_arg;               /* Additional parameters to playback */

/* Buffers for I/K-bus messages */
  guint8 rx_buf[CDC_BUF_SIZE];    /* Raw data from I/K-bus */
  guint8 tx_buf[CDC_BUF_SIZE];    /* Data ready to be written to I/K-bus */
  guint8 *msg_cmd;                /* I/K-bus command type message */
};

static void 
ikbus_cdc_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (IKBusCdc, ikbus_cdc, G_TYPE_OBJECT,
    G_ADD_PRIVATE (IKBusCdc) G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, ikbus_cdc_initable_iface_init))

enum
{
  PROP_0,
  PROP_IFNAME,
  N_PROP
};

static GParamSpec *obj_properties[N_PROP] = { NULL, };

enum {
  REQ_STATUS,
  STOP,
  PAUSE,
  PLAY,
  FAST,
  REWIND,
  NEXT,
  PREVIOUS,
  DISC,
  SCAN_ON,
  SCAN_OFF,
  RANDOM_ON,
  RANDOM_OFF,
  CHNG_TRK,
  LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

static void
ikbus_cdc_finalize (GObject *object)
{
  IKBusCdc *g_cdc= IKBUS_CDC (object);

  g_free (g_cdc->priv->ifname);
  g_io_channel_shutdown (g_cdc->priv->channel, FALSE, NULL);
  G_OBJECT_CLASS (ikbus_cdc_parent_class)->finalize (object);
}

static void
ikbus_cdc_dispose (GObject *object)
{
  IKBusCdc *g_cdc= IKBUS_CDC (object);

  g_clear_object (&g_cdc->priv->iksock);
  G_OBJECT_CLASS (ikbus_cdc_parent_class)->dispose (object);
}

static void
ikbus_cdc_get_property (GObject *object,
                        guint property_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  IKBusCdc *g_cdc= IKBUS_CDC (object);

  switch (property_id)
    {
      case PROP_IFNAME:
        g_value_set_string (value, g_cdc->priv->ifname);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ikbus_cdc_set_property (GObject *object,
                        guint property_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  IKBusCdc *g_cdc = IKBUS_CDC (object);

  switch (property_id)
    {
      case PROP_IFNAME:
        if (g_cdc->priv->ifname == NULL)
          g_cdc->priv->ifname = g_strdup (g_value_get_string (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ikbus_action (IKBusCdc *cdc)
{
  switch (*cdc->priv->msg_cmd)
    {
    /* Response to status request */
    case IKBUS_MSG_DEV_STAT_REQ:
      ikbus_socket_write (cdc->priv->iksock, CDC_I_AM_HERE, 5);
      break;

    /* Control playback */
    case IKBUS_MSG_CD_CTL:

      switch (*cdc->priv->ctrl_task) 
        {

        case CDC_CMD_STAT_REQ:
          ikbus_cdc_sync_output (cdc, NULL);
          g_signal_emit (cdc, signals[REQ_STATUS], 0);
          break;

        case CDC_CMD_STOP:
          g_signal_emit (cdc, signals[STOP], 0);
          *cdc->priv->stat_resp = CDC_STAT_STOP;
          *cdc->priv->ack_resp = CDC_ACK_PAUSE;
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        case CDC_CMD_PAUSE:
          g_signal_emit (cdc, signals[PAUSE], 0);
          *cdc->priv->stat_resp = CDC_STAT_NO_MAGAZINE;
          *cdc->priv->ack_resp = CDC_ACK_PAUSE;
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        case CDC_CMD_PLAY:
          g_signal_emit (cdc, signals[PLAY], 0);
          *cdc->priv->stat_resp = CDC_STAT_PLAY;
          *cdc->priv->ack_resp = CDC_ACK_PLAY;
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        case CDC_CMD_FAST:
          if (*cdc->priv->ctrl_arg == 0) {
            g_signal_emit (cdc, signals[REWIND], 0, *cdc->priv->ctrl_arg);
            *cdc->priv->stat_resp = CDC_STAT_REWIND;
          }
          else {
            g_signal_emit (cdc, signals[FAST], 0, *cdc->priv->ctrl_arg);
            *cdc->priv->stat_resp = CDC_STAT_FAST_FOR;
          }
          *cdc->priv->ack_resp = CDC_ACK_PLAY;
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        case CDC_CMD_CHNG_TR:
        case CDC_CMD_CHNG_TRK:
          if (*cdc->priv->ctrl_arg == 0)
            g_signal_emit (cdc, signals[NEXT], 0);
          else
            g_signal_emit (cdc, signals[PREVIOUS], 0);
          break;

        case CDC_CMD_CHNG_CD:
          g_signal_emit (cdc, signals[DISC], 0);
          break;

        case CDC_CMD_SC:
          if (*cdc->priv->ctrl_arg == 1)
          {
            *cdc->priv->ack_resp |= CDC_ACK_SC;
            g_signal_emit (cdc, signals[SCAN_ON], 0);
          }
          else
          {
            *cdc->priv->ack_resp &= ~CDC_ACK_SC;
            g_signal_emit (cdc, signals[SCAN_OFF], 0);
          }
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        case CDC_CMD_RANDOM:
          if (*cdc->priv->ctrl_arg == 1)
          {
            *cdc->priv->ack_resp |= CDC_ACK_RND;
            g_signal_emit (cdc, signals[RANDOM_ON], 0);
          }
          else
          {
            *cdc->priv->ack_resp &= ~CDC_ACK_RND;
            g_signal_emit (cdc, signals[RANDOM_OFF], 0);
          }
          ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
          break;

        default:
          g_warning ("Unknown CDC command 0x%02X\n", *cdc->priv->ctrl_task);
        }
      break;

    /* Identity request */
    case IKBUS_DIA_READ_IDENT:
      ikbus_socket_write (cdc->priv->iksock, CDC_IDENTY, 16);
      break;


    default:
      g_warning ("Unknown CDC message 0x%02X\n", *cdc->priv->msg_cmd);
    }
}

static gboolean
ikbus_cdc_receiving (G_GNUC_UNUSED GIOChannel *source,
                    G_GNUC_UNUSED GIOCondition condition,
                    gpointer data)
{
  IKBusCdc *cdc = IKBUS_CDC (data);
  gint n;

  n = ikbus_socket_read (cdc->priv->iksock, cdc->priv->rx_buf);
  if ((n > 4) && (n < 8))
        ikbus_action (cdc);

  return TRUE;
}

static gboolean
ikbus_cdc_timeout (gpointer data)
{
  IKBusCdc *cdc = IKBUS_CDC (data);

  ikbus_socket_write (cdc->priv->iksock, CDC_I_AM_HERE, 5);
  return TRUE;
}

static gboolean
ikbus_cdc_initable_init (GInitable *initable,
                         GCancellable *cancellable,
                         GError  **error)
{
  g_return_val_if_fail (IKBUS_IS_CDC (initable), FALSE);
  IKBusCdc *g_cdc = IKBUS_CDC (initable);

  g_cdc->priv->iksock = ikbus_socket_new (g_cdc->priv->ifname, error);
  if (NULL == g_cdc->priv->iksock)
    return FALSE;

  if (FALSE == ikbus_socket_connect (g_cdc->priv->iksock, IKBUS_DEV_CDC, IKBUS_DEV_LOC, error))
    return FALSE;

  g_cdc->priv->channel = g_io_channel_unix_new (ikbus_socket_get_fd (g_cdc->priv->iksock));

  /* NULL encoding means the stream is binary safe */
  g_io_channel_set_encoding (g_cdc->priv->channel, NULL, NULL);
  /* no buffering */
  g_io_channel_set_buffered (g_cdc->priv->channel, FALSE);

  if (!g_io_add_watch (g_cdc->priv->channel, 
                      G_IO_IN, (GIOFunc) ikbus_cdc_receiving, g_cdc))
  {
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_INITIALIZED,
                 "Fail to add watch CDC channel");
    return FALSE;
  }
  g_io_channel_unref (g_cdc->priv->channel);

  if (!g_timeout_add (3800, ikbus_cdc_timeout, g_cdc))
  {
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_INITIALIZED,
                 "Fail to add CDC timeout");
    return FALSE;
  }

  ikbus_socket_write (g_cdc->priv->iksock, CDC_ANNOUNCE, 5);

  return TRUE;
}

static void
ikbus_cdc_initable_iface_init (GInitableIface *iface)
{
  iface->init = ikbus_cdc_initable_init;
}

static void
ikbus_cdc_class_init (IKBusCdcClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ikbus_cdc_finalize;
  object_class->dispose = ikbus_cdc_dispose;
  object_class->get_property = ikbus_cdc_get_property;
  object_class->set_property = ikbus_cdc_set_property;

  obj_properties[PROP_IFNAME] = g_param_spec_string ("ifname",
                                    "Interface name",
                                    "The name of the I/K-bus network interface",
                                    NULL, /* default */
                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROP, obj_properties);

  signals[REQ_STATUS] = g_signal_new ("req-status",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[STOP] = g_signal_new ("stop",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[PLAY] = g_signal_new ("play",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[FAST] = g_signal_new ("fast-forward",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[REWIND] = g_signal_new ("rewind",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[NEXT] = g_signal_new ("next",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[PREVIOUS] = g_signal_new ("previous",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[DISC] = g_signal_new ("change-disc",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               1, G_TYPE_UCHAR);

  signals[SCAN_ON] = g_signal_new ("scan-on",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  
  signals[SCAN_OFF] = g_signal_new ("scan-off",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[RANDOM_ON] = g_signal_new ("random-on",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);

  signals[RANDOM_OFF] = g_signal_new ("random-off",
                               IKBUS_TYPE_CDC,
                               G_SIGNAL_RUN_FIRST,
                               0,
                               NULL,
                               NULL,
                               g_cclosure_marshal_VOID__VOID,
                               G_TYPE_NONE,
                               0);
}

static void
ikbus_cdc_init (IKBusCdc *cdc)
{
  cdc->priv = ikbus_cdc_get_instance_private (cdc);

  /* Bind cdc fields to buffer's addresses */
  cdc->priv->msg_cmd = cdc->priv->rx_buf + 3;
  cdc->priv->ctrl_task = cdc->priv->rx_buf + 4;
  cdc->priv->ctrl_arg  = cdc->priv->rx_buf + 5;

  cdc->priv->stat_resp  = cdc->priv->tx_buf + 4;
  cdc->priv->ack_resp   = cdc->priv->tx_buf + 5;
  cdc->priv->error_mask = cdc->priv->tx_buf + 6;
  cdc->priv->cd_mask    = cdc->priv->tx_buf + 7;
  cdc->priv->cdnum  = cdc->priv->tx_buf + 9;
  cdc->priv->tracknum = cdc->priv->tx_buf + 10;

  /* Set default values for TX message */
  cdc->priv->tx_buf[IKBUS_FRM_SENDER] = IKBUS_DEV_CDC; /* Sender address */
  cdc->priv->tx_buf[IKBUS_FRM_SIZE] = 10; /* Message length */
  cdc->priv->tx_buf[IKBUS_FRM_RECEIVER] = IKBUS_DEV_RAD; /* Receiver address */

  cdc->priv->tx_buf[IKBUS_FRM_CMD] = IKBUS_MSG_CD_STAT;
  *cdc->priv->stat_resp = CDC_STAT_STOP;
  *cdc->priv->ack_resp = CDC_ACK_PAUSE;
  *cdc->priv->error_mask = 0;
  *cdc->priv->cd_mask = 0;
  *cdc->priv->cdnum = 0;
  *cdc->priv->tracknum = 0;

  cdc->priv->real_tracknum = 0;
}

IKBusCdc*
ikbus_cdc_new (gchar *ifname, GError **error)
{
  return IKBUS_CDC (g_initable_new (IKBUS_TYPE_CDC, NULL, 
                                    error, "ifname", ifname, NULL));
}

void
ikbus_cdc_sync_output (IKBusCdc *cdc, GError **error)
{
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  ikbus_socket_write (cdc->priv->iksock, cdc->priv->tx_buf, 11);
}

static guint8
ikbus_cdc_hex_like_dec (guint8 hex_num)
{
  return (hex_num%10 + (hex_num/10)*16)%160;
}

void
ikbus_cdc_set_track (IKBusCdc *cdc, gint tracknum)
{
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  cdc->priv->real_tracknum = tracknum;
  *cdc->priv->tracknum = ikbus_cdc_hex_like_dec ((guint8) tracknum);
}

gint
ikbus_cdc_get_track (IKBusCdc *cdc)
{
  g_return_val_if_fail (IKBUS_IS_CDC (cdc), -1);

  return (gint) cdc->priv->real_tracknum;
}

void
ikbus_cdc_set_cd (IKBusCdc *cdc, gint cdnum)
{
  guint8 tmp_mask = 0;
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  if ((cdnum >= 1) && (cdnum <= 6))
  {
    tmp_mask = 1 << (cdnum - 1);
    if (tmp_mask & (*cdc->priv->cd_mask))
            *cdc->priv->cdnum = (guint8) cdnum;
  }
}

gint
ikbus_cdc_get_cd (IKBusCdc *cdc)
{
  g_return_val_if_fail (IKBUS_IS_CDC (cdc), -1);

  return (gint) *cdc->priv->cdnum;
}

guint8
ikbus_cdc_get_cmd_arg (IKBusCdc *cdc)
{
  g_return_val_if_fail (IKBUS_IS_CDC (cdc), 0);

  return *cdc->priv->ctrl_arg;
}

void
ikbus_cdc_set_resp_status (IKBusCdc *cdc, guint8 stat)
{
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  *cdc->priv->stat_resp = stat;
}

void
ikbus_cdc_set_resp_request (IKBusCdc *cdc, guint8 req)
{
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  *cdc->priv->ack_resp = req;
}

void
ikbus_cdc_set_error (IKBusCdc *cdc, guint8 errmask)
{
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  *cdc->priv->error_mask = errmask;
}

void
ikbus_cdc_insert_cd (IKBusCdc *cdc, guint8 cdnum)
{
  guint8 tmp_mask = 0;
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  if ((cdnum >= 1) && (cdnum <= 6))
  {
    tmp_mask = 1 << (cdnum - 1);
    if (*cdc->priv->cd_mask == 0)
    {
      *cdc->priv->cdnum = cdnum;
      *cdc->priv->stat_resp = CDC_STAT_STOP;
    }
    *cdc->priv->cd_mask |= tmp_mask;
  }
}

gint
ikbus_cdc_remove_cd (IKBusCdc *cdc, guint8 cdnum)
{
  guint8 tmp_mask = 0;
  g_return_val_if_fail (IKBUS_IS_CDC (cdc), -1);

  if ((cdnum >= 1) && (cdnum <= 6))
  {
    tmp_mask = 1 << (cdnum - 1);
    *cdc->priv->cd_mask &= ~tmp_mask;
    if (*cdc->priv->cd_mask == 0)
    {
      *cdc->priv->cdnum = 0;
      *cdc->priv->stat_resp = CDC_STAT_NO_MAGAZINE;
    }
    else if (*cdc->priv->cdnum == cdnum)
    {
      guint8 i;
      for (i = 0; (i < 6) && !(1 & (*cdc->priv->cd_mask >> i)); i++);
      *cdc->priv->cdnum = i + 1;
    }
  }
  return *cdc->priv->cdnum;
}

void
ikbus_cdc_sync_set (IKBusCdc *cdc,const gchar *first_cmd_name, ...)
{
  va_list var_args;
  g_return_if_fail (IKBUS_IS_CDC (cdc));

  va_start (var_args, first_cmd_name);
  if (first_cmd_name)
    {
      const gchar *name;
      int tmp_diasc;
      name = first_cmd_name;
      tmp_diasc = ikbus_cdc_get_cd (cdc);
      do
        {
          gint value;
          value = va_arg (var_args, gint);
          if (!g_strcmp0(name, "track"))
            {
              ikbus_cdc_set_track (cdc, value);
            }
          else if (!g_strcmp0(name, "disc"))
            {
              tmp_diasc = value;
            }
          else if (!g_strcmp0(name, "random"))
            {
              ikbus_cdc_set_random_mid (cdc, (gboolean) value);
            }
          else if (!g_strcmp0(name, "sampling"))
            {
              
            }
        }
      while ((name = va_arg (var_args, const gchar *)));
      ikbus_cdc_set_cd (cdc, tmp_diasc); /* first set cdmask value, after disk */
      ikbus_cdc_sync_output (cdc, NULL);
    }
  va_end (var_args);
}

void
ikbus_cdc_set_random_mid (IKBusCdc *cdc, gboolean rand)
{
  const guint8 mid_press_button_random[] = 
        {IKBUS_DEV_MID, 0x06, IKBUS_DEV_RAD, IKBUS_MSG_BUTTON, 0x00, 0x00, 0x09};
  const guint8 mid_release_button_random[] = 
        {IKBUS_DEV_MID, 0x06, IKBUS_DEV_RAD, IKBUS_MSG_BUTTON, 0x00, 0x00, 0x49};

  g_return_if_fail (IKBUS_IS_CDC (cdc));

  ikbus_socket_write (cdc->priv->iksock, mid_press_button_random, 7);
  g_usleep (150000);
  ikbus_socket_write (cdc->priv->iksock, mid_release_button_random, 7);
}
