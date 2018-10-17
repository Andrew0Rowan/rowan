TARGET=ewriter
SRC=$(wildcard *.c)
OBJS=${SRC:%.c=%.o}
LDFLAGS+=-ltz_efuse
LDFLAGS+=-ltz_uree

.PHONY: all install clean

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $<

clean:
	rm -rf ${TARGET} *.o
