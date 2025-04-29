#!/bin/sh
openocd \
    -s /usr/share/openocd/scripts \
        -f interface/stlink-v2-1.cfg \
        -f target/stm32f7x.cfg \
     -c "program submodules/nuttx/nuttx verify reset exit"
