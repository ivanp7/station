STATION_PATH = ..

HEADER = plugin.h
SOURCE = plugin.c

OUT_PLUGIN = demo.station

DL_LOADER = /lib64/ld-linux-x86-64.so.2

CFLAGS = -march=native -pipe -std=c17 -Wall -Wextra -Wpedantic -pedantic-errors \
		 -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition \
		 $(shell PKG_CONFIG_PATH="${STATION_PATH}" pkg-config --cflags station) -fPIC -O0 -g3 -ggdb
LFLAGS = $(shell PKG_CONFIG_PATH="${STATION_PATH}" pkg-config --libs station) -fPIC -shared -rdynamic -DDL_LOADER="\"${DL_LOADER}\"" -Wl,-e,plugin_main


${OUT_PLUGIN}: ${HEADER} ${SOURCE}
	${CC} -o ${OUT_PLUGIN} ${SOURCE} ${CFLAGS} ${LFLAGS}

clean:
	rm "./${OUT_PLUGIN}"

.phony: clean

