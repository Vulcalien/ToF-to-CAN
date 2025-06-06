#!/bin/sh
ip link set can0 type can bitrate 404040
ifconfig can0 up
