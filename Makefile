.PHONY: all clean distclean program reset qconfig
EXTRA_CLEAN_FILES = romfs.h romfs.img

all: submodules/nuttx/.config submodules/apps/tof-app romfs.h
	cd submodules/nuttx && $(MAKE) all && arm-none-eabi-size nuttx

clean: submodules/apps/tof-app
	rm -f $(EXTRA_CLEAN_FILES)
	cd submodules/nuttx && $(MAKE) clean

distclean: submodules/apps/tof-app
	rm -f $(EXTRA_CLEAN_FILES)
	cd submodules/nuttx && $(MAKE) distclean
	cd config/tof-l431-nsh && cp -f defconfig.src defconfig
	-rm submodules/apps/tof-app

program: all
	openocd \
        -f interface/stlink-v2-1.cfg \
        -f target/stm32l4x.cfg       \
        -c "adapter speed 50"        \
        -c "program submodules/nuttx/nuttx verify reset exit"

reset:
	openocd \
        -f interface/stlink-v2-1.cfg \
        -f target/stm32l4x.cfg       \
        -c "adapter speed 50"        \
        -c init -c reset -c exit

xprogram: all
	scp submodules/nuttx/nuttx pi@$(RASPI):~/temp
	ssh pi@$(RASPI) 'sudo openocd -f board/st_nucleo_l4.cfg -c "program temp/nuttx verify reset exit" '

xreset:
	ssh pi@$(RASPI) 'sudo openocd -f board/st_nucleo_l4.cfg -c init -c reset -c exit'

qconfig: submodules/nuttx/.config
	cd submodules/nuttx && $(MAKE) qconfig
	cp -v submodules/nuttx/.config config/tof-l431-nsh/defconfig

submodules/apps/tof-app:
	ln -sTf ../../tof-app submodules/apps/tof-app

submodules/nuttx/.config: submodules/apps/tof-app
	cd submodules/nuttx/tools && ./configure.sh ../../config/tof-l431-nsh

romfs.img: $(shell find etc)
	genromfs -f romfs.img -d etc -V 'ROMFS /etc'

romfs.h: romfs.img
	xxd -i romfs.img romfs.h && cp romfs.h config/include
