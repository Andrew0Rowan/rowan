CUR_SOURCE=${wildcard *.c}
CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}
all:$(CUR_OBJS)

$(CUR_OBJS):%.o:%.c
	$(CC) -I$(DIR_INC) -D_FILE_OFFSET_BIT=64 -c $^ -o $(ROOT_DIR)/$(OBJS_DIR)/$@
