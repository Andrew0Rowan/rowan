#CC = gcc
CC = armv7a-cros-linux-gnueabi-gcc

CFLAGS = -I. -w -g -funwind-tables 
LDFLAGS = -pthread -lc -lrt -rdynamic 

# This Program
MAINOBJS = 	main.o  pg_exception.o pg_assert.o  

OUTPUT = pp
INPUT  = $(MAINOBJS) 

$(OUTPUT) : $(INPUT)
	$(CC) -o $(OUTPUT) $(CFLAGS) $(LDFLAGS) $(INPUT)
    
# All Phony Targets
.PHONY : clean
# Default starting position
clean :
	rm -f $(INPUT) $(OUTPUT)

