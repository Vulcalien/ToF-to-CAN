# ToF-to-CAN: a CAN interface for the VL53L5CX Time-of-Flight sensor
This firmware offers a CAN interface for the
[VL53L5CX](https://www.st.com/en/imaging-and-photonics-solutions/vl53l5cx.html)
Time-of-Flight (ToF) sensor, controlled via *I2C* by a
[STM32L432KC](https://www.st.com/en/microcontrollers-microprocessors/stm32l432kc.html)
microcontroller.

The project is built on top of the [NuttX](https://nuttx.apache.org/)
embedded operating system.

## Building
### Installing dependencies
Make sure the following packages are installed: # TODO

Retrieve all git submodules:

```sh
git submodule update --init --depth=1
```
### Generating the binary
Run `make ID=<sensor-id>` to compile the firmware. If the commands fails
with an error, try running the following commands:

```sh
cp config/tof-l431-nsh/defconfig submodules/nuttx/.config
make ID=<sensor-id>
```

After building the firmware, the binary `submodules/nuttx/nuttx.bin`
should be present. Flash this file by running `make program
ID=<sensor-id>` (which uses OpenOCD) or another flashing software.

## Usage
TODO

## Examples
The `demo` directory contains examples of how to use the interface.

If the computer is able to connect to the sensor via CAN bus, the
following commands can be used to run the demos:

```sh
cd demo
sudo ./can-config.sh
make
```

After selecting which demo to run, press the Enter key to request data
from the sensor. Use *ctrl^C* to quit the demo.

## License
The source code of the application, contained in the `tof-app`
directory, is licensed under the GNU General Public License, either
version 3 of the License or any later version.

### Dependencies
This project depends on the following, each licensed under their
respective terms:
- the VL53L5CX driver in the `tof-app/lib/vl53l5cx` directory
- the NuttX operating system, included as a git submodule
- files in the `config` directory
