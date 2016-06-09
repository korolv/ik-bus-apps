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


#ifndef _IKBUSCCDC_H_
#define _IKBUSCCDC_H_

#include <glib-object.h>

#define CDC_STAT_STOP                0x00
#define CDC_STAT_PAUSE               0x01
#define CDC_STAT_PLAY                0x02
#define CDC_STAT_FAST_FOR            0x03
#define CDC_STAT_REWIND              0x04
#define CDC_STAT_END                 0x07 /* End of track  Когда заканчивается трек отсылается это сообщение */
#define CDC_STAT_LOAD                0x08 /* и начинается загрузка следующего*/
#define CDC_STAT_CD_CHK              0x09
#define CDC_STAT_NO_MAGAZINE         0x0a

#define CDC_ACK_PAUSE                0x02
#define CDC_ACK_PLAY                 0x09
#define CDC_ACK_PLAY_SC              0x19
#define CDC_ACK_PLAY_RND             0x29
#define CDC_ACK_SC                   0x10
#define CDC_ACK_RND                  0x20

#define CDC_ERR_HIGH_TEMP            1 << 1
#define CDC_ERR_CD_ERROR             1 << 2
#define CDC_ERR_NO_DISC              1 << 3
#define CDC_ERR_NO_DISCS             1 << 4

#define CDC_CD1                      1 << 0
#define CDC_CD2                      1 << 1
#define CDC_CD3                      1 << 2
#define CDC_CD4                      1 << 3
#define CDC_CD5                      1 << 4
#define CDC_CD6                      1 << 5

#define CDC_CMD_STAT_REQ             0x00
#define CDC_CMD_STOP                 0x01
#define CDC_CMD_PAUSE                0x02
#define CDC_CMD_PLAY                 0x03
#define CDC_CMD_FAST                 0x04
#define CDC_CMD_CHNG_TR              0x05
#define CDC_CMD_CHNG_CD              0x06
#define CDC_CMD_SC                   0x07     /* Scan sampling mode */
#define CDC_CMD_RANDOM               0x08     /* Random mode */
#define CDC_CMD_CHNG_TRK             0x0a

G_BEGIN_DECLS

#define IKBUS_TYPE_CDC               (ikbus_cdc_get_type())
#define IKBUS_CDC(obj)               ((G_TYPE_CHECK_INSTANCE_CAST ((obj), IKBUS_TYPE_CDC, IKBusCdc)))
#define IKBUS_CDC_CLASS(klass)       ((G_TYPE_CHECK_CLASS_CAST ((klass), IKBUS_TYPE_CDC, IKBusCdcClass)))
#define IKBUS_IS_CDC(obj)            ((G_TYPE_CHECK_INSTANCE_TYPE ((obj), IKBUS_TYPE_CDC)))
#define IKBUS_IS_CDC_CLASS(klass)    ((G_TYPE_CHECK_CLASS_TYPE ((klass), IKBUS_TYPE_CDC)))
#define IKBUS_CDC_GET_CLASS(obj)     ((G_TYPE_INSTANCE_GET_CLASS ((obj), IKBUS_TYPE_CDC, IKBusCdcClass)))

typedef struct _IKBusCdc        IKBusCdc;
typedef struct _IKBusCdcClass   IKBusCdcClass;
typedef struct _IKBusCdcPrivate IKBusCdcPrivate;

struct _IKBusCdc {
  GObject parent_instance;
  IKBusCdcPrivate *priv;
};

struct _IKBusCdcClass {
  GObjectClass parent_class;
};

GType ikbus_cdc_get_type (void);

IKBusCdc *ikbus_cdc_new (gchar *ifname, GError **error);
void ikbus_cdc_sync_output (IKBusCdc *cdc, GError **error);

void ikbus_cdc_set_track (IKBusCdc *cdc, gint tracknum);
gint ikbus_cdc_get_track (IKBusCdc *cdc);
void ikbus_cdc_set_cd (IKBusCdc *cdc, gint cdnum);
gint ikbus_cdc_get_cd (IKBusCdc *cdc);
gboolean ikbus_cdc_get_random (IKBusCdc *cdc);

guint8 ikbus_cdc_get_cmd_arg (IKBusCdc *cdc);

void ikbus_cdc_set_resp_status (IKBusCdc *cdc, guint8 stat);
void ikbus_cdc_set_resp_request (IKBusCdc *cdc, guint8 req);
void ikbus_cdc_set_error (IKBusCdc *cdc, guint8 errmask);
void ikbus_cdc_insert_cd (IKBusCdc *cdc, guint8 cdnum);
gint ikbus_cdc_remove_cd (IKBusCdc *cdc, guint8 cdnum);

void ikbus_cdc_sync_set (IKBusCdc *cdc,const gchar *first_cmd_name, ...);

void ikbus_cdc_set_random_mid (IKBusCdc *cdc, gboolean rand);
G_END_DECLS
#endif /* _IKBUSCDC_H_ */
