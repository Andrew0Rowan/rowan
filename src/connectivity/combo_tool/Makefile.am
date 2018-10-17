bin_PROGRAMS = wmt_loader 6620_launcher power_on_wifi wmt_loopback stp_dump
wmt_loader_SOURCES = combo_loader/yocto_loader.c
wmt_loader_LDADD = ${LIBGD}
wmt_loopback_SOURCES = src/wmt_loopback.c
6620_launcher_SOURCES = src/yocto_stp_uart_launcher.c
6620_launcher_LDADD = -lpthread ${LIBGD}
AM_CFLAGS = -ldl

power_on_wifi_SOURCES = src/power_on_wifi.c
stp_dump_SOURCES = src/stp_dump/os_linux.c \
                   src/stp_dump/eloop.c \
                   src/stp_dump/stp_dump.c
