export STAGING_DIR=/omap/openwrt/staging_dir/

CC_LINARO=/omap/openwrt/staging_dir/toolchain-arm_cortex-a8+vfpv3_gcc-11.2.0_musl_eabi/bin/arm-openwrt-linux-gcc

I_DIR_LINARO=/omap/openwrt/staging_dir/toolchain-arm_cortex-a8+vfpv3_gcc-11.2.0_musl_eabi/include
L_DIR_LINARO=/omap/openwrt/staging_dir/toolchain-arm_cortex-a8+vfpv3_gcc-11.2.0_musl_eabi/lib

all: zadanie11 zadanie12 zadanie13 zadanie14

zadanie11:
	cd "zadanie 11" && $(CC_LINARO) $(CFLAGS) -I$(I_DIR_LINARO) -L$(L_DIR_LINARO) $(LFLAGS) prog.c -o prog

zadanie12:
	cd "zadanie 12" && $(CC_LINARO) $(CFLAGS) -I$(I_DIR_LINARO) -L$(L_DIR_LINARO) $(LFLAGS) prog.c -o prog

zadanie13:
	cd "zadanie 13" && $(CC_LINARO) $(CFLAGS) -I$(I_DIR_LINARO) -L$(L_DIR_LINARO) $(LFLAGS) prog1.c -o prog1
	cd "zadanie 13" && $(CC_LINARO) $(CFLAGS) -I$(I_DIR_LINARO) -L$(L_DIR_LINARO) $(LFLAGS) prog2.c -o prog2

zadanie14:
	cd "zadanie 14" && $(CC_LINARO) $(CFLAGS) -I$(I_DIR_LINARO) -L$(L_DIR_LINARO) $(LFLAGS) prog.c -o prog

clean:
	rm -f "zadanie 11/prog" "zadanie 12/prog" "zadanie 13/prog1" "zadanie 13/prog2" "zadanie 14/prog"

.PHONY: all zadanie11 zadanie12 zadanie13 zadanie14 clean
