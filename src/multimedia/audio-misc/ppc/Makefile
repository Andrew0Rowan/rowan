###########################################################################
# libppc_resource.a
###########################################################################
PPCSWIP_SRCS =  \
       ./model/src/ppc_swip.c

PPCSWIP_INC = \
          -I model/swip_api \
          -I model/src \
          -I model/inc

PPCSWIP_CFLAGS  :=-g -Wall -O3

PPCSWIP_LDFLAGS :=
PPCSWIP_LIBS :=

PPCSWIP_OBJS := $(patsubst %.c,%.o,$(PPCSWIP_SRCS))

PPCSWIP_TARGET := libppc_resource.a

###########################################################################
# libppcmodel.so
###########################################################################
PPCMODEL_SRCS =  \
       ppc_control.cpp

PPCMODEL_INC =  \
          -I include \
          -I model/swip_api \
          -I model/src \
          -I model/inc

PPCMODEL_CFLAGS  := -g -Wall -O3 -fPIC

#PPCMODEL_CFLAGS  += -DPPC_EQ_MODE_ENABLE

PPCMODEL_LDFLAGS := -fPIC -shared -Wl,--no-undefined

PPCMODEL_LIBS := \
    -L ./ \
    -L model/libs/${base_libdir} \
    -lppc \
    -lppc_resource \
    -lcommon \
    -ladd \
    -lchmix \
    -lgaincontrol \
    -linput \
    -loutput \
    -lswip_common \
    -lpeq \
    -lmastervolume \
    -lfilter \
    -ldelay \
    -lmbdrc_mtk \

PPCMODEL_OBJS := $(patsubst %.cpp,%.o,$(PPCMODEL_SRCS))
PPCMODEL_TARGET := libppcmodel.so


.PHONY : everything objs clean veryclean rebuild

all : $(PPCSWIP_TARGET) $(PPCMODEL_TARGET)

clean :
	find . -name "*.o" | xargs rm -f
	rm -fr $(PPCSWIP_TARGET)  $(PPCMODEL_TARGET)

$(PPCSWIP_OBJS):%.o:%.c
	${CC} ${PPCSWIP_CFLAGS} $(PPCSWIP_INC) -c $< -o $@
$(PPCSWIP_TARGET):$(PPCSWIP_OBJS)
	$(AR) cr -o $@ $(PPCSWIP_OBJS) $(PPCSWIP_LDFLAGS) $(PPCSWIP_LIBS)

$(PPCMODEL_OBJS):%.o:%.cpp
	${CXX} ${PPCMODEL_CFLAGS} $(PPCMODEL_INC) -c $< -o $@
$(PPCMODEL_TARGET) : $(PPCMODEL_OBJS)
	$(CXX) -o $@ $(PPCMODEL_OBJS) $(PPCMODEL_LDFLAGS) $(PPCMODEL_LIBS)

install :
	install -d ${DESTDIR}/${libdir}
	install -d ${DESTDIR}/${includedir}
	install -m 755 ./include/ppc_control.h ${DESTDIR}/${includedir}
	install -m 755 ./libppcmodel.so ${DESTDIR}/${libdir}
	install -m 755 ./model/libs/${base_libdir}/libmbdrc_mtk.so ${DESTDIR}/${libdir}
