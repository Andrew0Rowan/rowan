LOCAL_INCLUDE := -Iinclude

LIBWEBSOCKETS_TEST_TARGET := libwebsockets_test
LIBWEBSOCKETS_TEST_SRC := sample/libwebsockets_test.c
LINK_WEBSOCKETS := -lwebsockets

OPUS_TEST_TARGET := opus_test
OPUS_TEST_SRC := sample/opus_test.c
LINK_OPUS := -lopus

PAL_SAMPLE_TARGET := pal_sample
PAL_SAMPLE_SRC := sample/pal_sample.c sample/platform_linux.c
LINK_PAL_LIB := -Llib -lpal ${LINK_OPUS} ${LINK_WEBSOCKETS} -lcjson -lportaudio -pthread

# Pal Dynamic Library
PAL_DLIB := lib/libpal.so
# Test Resource
SAMPLE_PCM := sample/sample.pcm
SAMPLE_OPUS := sample/sample.opus

all: ${PAL_SAMPLE_TARGET}
#${LIBWEBSOCKETS_TEST_TARGET} ${OPUS_TEST_TARGET} 
.PHONY : all

${LIBWEBSOCKETS_TEST_TARGET}:
	${CC} -o $@ ${LOCAL_INCLUDE} ${LIBWEBSOCKETS_TEST_SRC} ${LINK_WEBSOCKETS}

${OPUS_TEST_TARGET}:
	${CC} -o $@ ${LOCAL_INCLUDE} ${OPUS_TEST_SRC} ${LINK_OPUS}

${PAL_SAMPLE_TARGET}:
	${CC} -o $@ ${LOCAL_INCLUDE} ${PAL_SAMPLE_SRC} ${LINK_PAL_LIB}

install:
	install -d $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(libdir)
	install -d $(DESTDIR)$(includedir)
#	install -m 755 ${LIBWEBSOCKETS_TEST_TARGET} $(DESTDIR)$(bindir)
#	install -m 755 ${OPUS_TEST_TARGET} $(DESTDIR)$(bindir)
	install -m 755 ${PAL_SAMPLE_TARGET} $(DESTDIR)$(bindir)
	install -m 755 ${PAL_DLIB} $(DESTDIR)$(libdir)
	install -m 644 include/pal.h $(DESTDIR)$(includedir)
	install -m 644 include/pal_platform.h $(DESTDIR)$(includedir)
#	install -m 644 ${SAMPLE_PCM} $(DESTDIR)$(bindir)
#	install -m 644 ${SAMPLE_OPUS} $(DESTDIR)$(bindir)
.PHONY : install
	
clean:
	rm -f ${LIBWEBSOCKETS_TEST_TARGET}
	rm -f ${OPUS_TEST_TARGET}
	rm -f ${PAL_SAMPLE_TARGET}
.PHONY : clean
