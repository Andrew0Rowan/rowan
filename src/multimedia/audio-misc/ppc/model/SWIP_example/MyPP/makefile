###########################################################################
# libmypp.a
###########################################################################
MYPP_SRCS =  \
       ./shift.c \
       ./mypp_api.c

MYPP_INC = \
          -I ./inc

MYPP_CFLAGS  :=-g -Wall -O3 -fPIC $(MCFLAG)

MYPP_LDFLAGS := 
MYPP_LIBS := 

MYPP_OBJS := $(patsubst %.c,%.o,$(MYPP_SRCS))

MYPP_TARGET := libmypp.a

###########################################################################
# build setting
###########################################################################
.PHONY : everything objs clean veryclean rebuild
  
all : $(MYPP_TARGET)

clean :
	find . -name "*.o" | xargs rm -f
	rm -fr $(MYPP_TARGET)

$(MYPP_OBJS):%.o:%.c
	${CC} ${MYPP_CFLAGS} $(MYPP_INC) -c $< -o $@
$(MYPP_TARGET):$(MYPP_OBJS)
	$(AR) cr -o $@ $(MYPP_OBJS) $(MYPP_LDFLAGS) $(MYPP_LIBS)