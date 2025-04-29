/*
 * wireless.h
 */

#pragma once

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/wireless/ieee802154/ieee802154_device.h>

class XBeeConnection {
 public:
    XBeeConnection(uint16_t panid) : m_handle(1), m_panid_low(panid & 0xff), m_panid_high(panid >> 8) {};
    int open();
    int open_wonly();
    int open_ronly();
    int receive_packet(uint16_t & src, uint8_t * packet, int len);
    void send_packet(uint16_t dest, uint8_t * packet, int len);
    void broadcast_packet(uint8_t * packet, int len)
    {
        send_packet(0xffff, packet, len);
    };
    void send_can_packet(uint16_t dest, uint16_t can_id, uint8_t * packet);
    void broadcast_can_packet(uint16_t can_id, uint8_t * packet)
    {
        send_can_packet(0xffff, can_id, packet);
    };

 private:
    int m_handle;
    uint8_t m_panid_low, m_panid_high;
    int m_fd;

};

