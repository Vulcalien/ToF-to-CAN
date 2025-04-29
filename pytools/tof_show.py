
import time
import can
import struct
from can.bus import BusState
import curses

x = [0, 10, 30, 50, 10, 30, 50]
y = [0,  4,  2,  4,  6,  8,  6]

stdscr = curses.initscr()
stdscr.keypad(1)
curses.raw()
curses.noecho()
curses.cbreak()
win = curses.newwin(0, 0)
win.keypad(1)

class PositionReader(can.Listener):

    coordinates = []

    def on_message_received(self, msg):
        #print(msg)
        data = struct.unpack("<BhBxxxx", msg.data)
        #print(data)
        sensor_id = data[0]
        win.addstr(y[sensor_id],x[sensor_id],"%3d  " % (data[1]))
        win.refresh()


def main():


    bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=404040)
    # filter robot position

    can_filters = [{"can_id": 0x0670, "can_mask": 0xfff0, "extended": False}]
    bus.set_filters(can_filters)

    notifier = can.Notifier(bus, [ PositionReader() ])
    msg = can.Message(arbitration_id=0x67f,is_extended_id=False,data=[0xff, 2, 7, 0, 0x38, 0, 0x0f, 3])
    bus.send(msg)
    msg = can.Message(arbitration_id=0x3e3,is_extended_id=False,data=[0, 0, 0, 0, 0, 0, 0, 0])
    while True:
        bus.send(msg)
        time.sleep(1)

main()

