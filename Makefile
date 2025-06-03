.PHONY: all demo clean distclean program reset qconfig

ROMFS := config/include/romfs.h

all: submodules/nuttx/.config submodules/apps/tof-app $(ROMFS)
	cd submodules/nuttx && $(MAKE) all && arm-none-eabi-size nuttx

demo:
	$(MAKE) -C demo

clean: submodules/apps/tof-app
	-rm $(ROMFS)
	cd submodules/nuttx && $(MAKE) clean

distclean: submodules/apps/tof-app
	rm -f $(EXTRA_CLEAN_FILES)
	cd submodules/nuttx && $(MAKE) distclean
	cd config/tof-l431-nsh && cp -f defconfig.src defconfig
	-rm submodules/apps/tof-app

program: all
	openocd \
        -f interface/stlink.cfg \
        -f target/stm32l4x.cfg  \
        -c "adapter speed 50"   \
        -c "program submodules/nuttx/nuttx verify reset exit"

reset:
	openocd \
        -f interface/stlink.cfg \
        -f target/stm32l4x.cfg  \
        -c "adapter speed 50"   \
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

$(ROMFS): etc/init.d/rcS
	genromfs -f romfs.img -d etc -V 'ROMFS /etc'
	xxd -i romfs.img $@
	-rm romfs.img
