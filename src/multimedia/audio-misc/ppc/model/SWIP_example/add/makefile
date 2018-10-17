###########################################################################
# libadd.a
###########################################################################
ADD_SRCS =  \
       ./src/add_api.c

ADD_INC = \
          -I ./inc

ADD_CFLAGS  :=-g -Wall -O3 -fPIC $(MCFLAG)

ADD_LDFLAGS := 
ADD_LIBS := 

ADD_OBJS := $(patsubst %.c,%.o,$(ADD_SRCS))

ADD_TARGET := libadd.a

###########################################################################
# build setting
###########################################################################
.PHONY : everything objs clean veryclean rebuild
  
all : $(ADD_TARGET)

clean :
	find . -name "*.o" | xargs rm -f
	rm -fr $(ADD_TARGET)

$(ADD_OBJS):%.o:%.c
	${CC} ${ADD_CFLAGS} $(ADD_INC) -c $< -o $@
$(ADD_TARGET):$(ADD_OBJS)
	$(AR) cr -o $@ $(ADD_OBJS) $(ADD_LDFLAGS) $(ADD_LIBS)