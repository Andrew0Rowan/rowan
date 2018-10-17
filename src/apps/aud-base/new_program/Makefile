TARGET := new_program

SOURCES := main.c

OBJS := $(patsubst %.c,%.o,$(SOURCES))

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^

$(OBJS):%.o:%.c
	$(CC) -o $@ -c $<

clean:
	rm -rf $(TARGET) $(OBJS)

