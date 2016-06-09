/* 
 * File:   ikbus.h
 * Author: Vladimir Korol
 *
 * Created on March 4, 2015
 */

#ifndef _IK_BUS_H_
#define _IK_BUS_H_

#define AF_IKBUS 41
#define PF_IKBUS AF_IKBUS

/* TX_IDx1 LENx1 RX_IDx1 CMDx1 DATAx32 CHK_SUMx1 */
#define IKBUS_MAX_DLEN 35
#define IKBUS_MAX_FRAME_SIZE 37
#define SOL_IKBUS 200
#define IKBUS_ID_ALL 0xff

enum {
    IKBUS_FILTER = 1,    /* set 0 .. n filter(s) */
    IKBUS_ERR_FILTER,    /* set filter for error frames       */
    IKBUS_RECV_OWN_MSGS, /* receive my own msgs (default:off) */
    IKBUS_ONLY_DATA,     /* Only data part in stren */
};

/*
 * struct sockaddr_ikbus - the sockaddr structure for I/K-BUS sockets
 * @ikbus_family:  address family number AF_IKBUS.
 * @ifindex: I/K-bus network interface index.
 */
struct sockaddr_ikbus {
	unsigned short int ikbus_family;
	int ifindex;
} ikbus_addr;

/*
 * struct ikbus_filter - filter for receive frames
 * @id_rx: identifier of receiver, 0xff - all
 * @id_tx: identifier of sender, 0xff - all
 */
struct ikbus_filter {
	u_int8_t id_rx;
	u_int8_t id_tx;
};

struct ikbus_head {
	u_int8_t rx_id;
	u_int8_t len;
	u_int8_t tx_id;
	u_int8_t cmd;
};

struct ikbus_frame {
	struct ikbus_head *head;
	u_int8_t *data;
};

#endif /* _IK_BUS_H_ */
