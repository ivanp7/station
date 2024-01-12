ROOT = ..

HEADER = plugin.h
SOURCE = plugin.c
OUT_PLUGIN = demo.station

CFLAGS = -march=native -pipe -std=c17 -Wall -Wextra -Wpedantic -pedantic-errors \
		 -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition \
		 $(shell pkg-config --with-path "${ROOT}" --cflags station) -fPIC -O0 -g3 -ggdb
LFLAGS = $(shell pkg-config --with-path "${ROOT}" --libs station) -fPIC -shared -rdynamic


${OUT_PLUGIN}: ${HEADER} ${SOURCE}
	${CC} -o ${OUT_PLUGIN} ${SOURCE} ${CFLAGS} ${LFLAGS}

clean:
	rm "./${OUT_PLUGIN}"

.phony: clean

