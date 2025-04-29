/*
 * can_utils.cpp
 */

#include "can_utils.h"

int CAN::open(int mode)
{
    int can_fd = ::open("/dev/can0", mode | O_NOCTTY);
    if (can_fd < 0) {
        perror("/dev/can0 open");
    }
    return can_fd;
}


int CAN::write(int fd, int can_id, unsigned char *msg, int len)
{
    struct can_msg_s txmsg;
    memset(&txmsg, 0, sizeof(struct can_msg_s));
    int msglen = can_bytes2dlc(len);
    txmsg.cm_hdr.ch_id     = can_id;
    txmsg.cm_hdr.ch_rtr    = false;
    txmsg.cm_hdr.ch_dlc    = msglen;
    txmsg.cm_hdr.ch_tcf    = 0;

    memcpy(txmsg.cm_data, msg, len);

    int msgsize = CAN_MSGLEN(msglen);
    int nbytes  = ::write(fd, &txmsg, msgsize);
    if (nbytes == msgsize)
        return 0;
    else {
        perror("CAN SEND");
        return -1;
    }
}


