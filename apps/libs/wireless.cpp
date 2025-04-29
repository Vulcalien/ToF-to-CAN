/*
 * wireless.cpp
 */

#include <poll.h>
#include "wireless.h"

int XBeeConnection::open()
{
    m_fd = ::open("/dev/ieee0", O_RDWR | O_NOCTTY);
    return m_fd;
}

int XBeeConnection::open_wonly()
{
    m_fd = ::open("/dev/ieee0", O_WRONLY | O_NOCTTY);
    return m_fd;
}

int XBeeConnection::open_ronly()
{
    m_fd = ::open("/dev/ieee0", O_RDONLY | O_NOCTTY);
    return m_fd;
}

void XBeeConnection::send_packet(uint16_t dest, uint8_t * packet, int len)
{
    struct mac802154dev_txframe_s tx_frame;

    tx_frame.meta.srcmode = IEEE802154_ADDRMODE_SHORT;

    tx_frame.meta.destaddr.mode = IEEE802154_ADDRMODE_SHORT;
    tx_frame.meta.destaddr.saddr[0] = dest & 0xff;
    tx_frame.meta.destaddr.saddr[1] = dest >> 8;
    tx_frame.meta.destaddr.panid[0] = m_panid_low;
    tx_frame.meta.destaddr.panid[1] = m_panid_high;

    tx_frame.meta.handle = m_handle;
    m_handle++;
    tx_frame.meta.flags.ackreq = dest != 0xffff;
    tx_frame.meta.flags.usegts = 0;
    tx_frame.meta.flags.indirect = false;
    tx_frame.meta.ranging = IEEE802154_NON_RANGING;

    tx_frame.payload = packet;
    tx_frame.length = len;

    if (write(m_fd, &tx_frame, sizeof(struct mac802154dev_txframe_s)) < 0) {
        perror("sending to /dev/ieee0");
    }

}

void XBeeConnection::send_can_packet(uint16_t dest, uint16_t can_id, uint8_t * packet)
{
    uint8_t buffer[11];
    buffer[0] = can_id & 0xff;
    buffer[1] = can_id >> 8;
    memcpy(buffer + 2, packet, 8);
    uint8_t cks = 0;
    for (int i = 0; i < 10;i++)
        cks += buffer[i];
    buffer[10] = cks;
    send_packet(dest, buffer, 11);
}

int XBeeConnection::receive_packet(uint16_t & src, uint8_t * packet, int len)
{
    mac802154dev_rxframe_s buf;
    int result = read(m_fd, &buf, sizeof(buf));

    if (result < 0) return result;

    src = buf.meta.src.saddr[0] | (buf.meta.src.saddr[1] << 8);

    int rx_len;
    if (buf.length <= len)
        rx_len = buf.length;
    else
        rx_len = len;

    memcpy(packet, buf.payload, rx_len);

    return rx_len;
}

