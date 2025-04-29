
import time
import can
import struct
from can.bus import BusState

class PositionReader(can.Listener):

    coordinates = []

    def on_message_received(self, msg):
        print(msg)
        #data = struct.unpack("<BhBxxxx", msg.data)
        #print(data)


def main():

    print ("Starting scanner....")

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

