#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efuse/efuse.h>
#include <efuse/efuse_info.h>
#include <unistd.h>

enum efuse_test_cmd {
    EFUSE_READ = 0,
    EFUSE_WRITE,
    EFUSE_NO_MORE_SENS_DATA,
    EFUSE_CMD_NUM
};
static void hex_dump(const char *prefix, unsigned char *buf, int len)
{
   int i;

   if(!buf || !len)
   	return;

   printf("%s:\n", prefix);
   for(i=0; i<len; i++) {
   	if(i!=0 && !(i%16))
		printf("\n");
   	printf("%02x", *(buf + i));
   }
   printf("\n");
}

#define BUF_SIZE 600
static UnitSpecificWriteToken *efuse_get_token(const char *token_name)
{
    int fd;
    unsigned char *buffer = (unsigned char *)malloc(BUF_SIZE);

    if (!buffer)
    	return NULL;

    fd = open(token_name, O_RDONLY);
    if (fd == -1) {
            free(buffer);
            perror ("open");

            return NULL;
    }

    if(read(fd, buffer, BUF_SIZE) < 0) {
	perror("read");
	buffer = NULL;
    }

    close(fd);
    return (UnitSpecificWriteToken *)buffer;
}

static unsigned char nibbleFromChar(char c)
{
        if(c >= '0' && c <= '9') return c - '0';
        if(c >= 'a' && c <= 'f') return c - 'a' + 10;
        if(c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 255;
}

/* Convert a string of characters representing a hex buffer into a series of bytes of that real value */
static unsigned char *hexStringToBytes(char *inhex, unsigned int efuse_len)
{
        unsigned char *retval;
        unsigned char *p;
        int len, i;
	int str_len = strlen(inhex);
	unsigned char odd;

	for (i=0;i<str_len;i++)
		if (!isxdigit(*(inhex+i))) {
			printf("please input hex value\n");
			return NULL;
		}

	odd = (str_len % 2);
        len = odd?(str_len / 2) + 1:(str_len / 2);
        retval = (unsigned char *)malloc(efuse_len+1);
	if (!retval) {
		printf("hexStringToBytes mem alloc fail\n");
		return NULL;
	}
	memset(retval, 0, efuse_len+1);
        for(i=0, p = (unsigned char *) inhex; i<len; i++) {
		if (odd && i == (len - 1)) {
			printf("odd and last byte\n");
			retval[i] = nibbleFromChar(*p);
		}
		else {
                	retval[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
                	p += 2;
		}
        }
        retval[len] = 0;

        return retval;
}

static void usage(char *main)
{
   printf("%s usage:\n", main);
   printf("ewriter cmd index length [value in hex] [token]\n");
   printf("ex:\n");
   printf("read efuse index 1 with byte length \"32\" -> ewriter 0 1 32\n");
   printf("read efuse index 5 with byte length \"1\" -> ewriter 0 5 1\n");
   printf("write efuse index 1 -> ewriter 1 1 32 0000000011111111222222223333333344444444555555556666666677777777\n");
   printf("write efuse index 5 -> ewriter 1 5 1 1\n");
   printf("write efuse index 21 using verification -> ewriter 1 21 1 1 ./out.bin\n");
   printf("write efuse index 20 with bootloader version 1 -> ewriter 1 20 16 01000000000000000100000000000000\n");
   printf("write efuse index 20 with bootloader version 2 -> ewriter 1 20 16 02000000000000000200000000000000\n");
   printf("disallow sensitive writes -> ewriter 2");
   printf("\n");
}

#define EFUSE_ACTION 1
#define EFUSE_INDEX  2
#define EFUSE_LEN    3
#define EFUSE_VALUE  4
#define EFUSE_TOKEN  5
static unsigned char check_input_data(int argc, char *argv[])
{
    int cmd;

    if (argc > 6 || argc < 2)
    	return 0;

    cmd = atoi(argv[EFUSE_ACTION]);
    if (cmd >= EFUSE_CMD_NUM)
    	return 0;

    if ((cmd == EFUSE_NO_MORE_SENS_DATA) && (argc != 2))
	return 0;

    if ((cmd == EFUSE_READ)) {
    	if (argc != 4)
		return 0;
    }

    if (cmd == EFUSE_WRITE) {
	if (argc < 5 || argc > 6) {
    		return 0;
	}
    }

    return 1;
}

int efuse_read_pubk0(void)
{
    unsigned char efuse_buf[256] = {0};
    int ret;
    size_t len =  efuse_info[SBC_PUBK0_HASH].len;

    ret = tee_fuse_read(SBC_PUBK0_HASH, efuse_buf, len);
    if (ret) {
	printf("failed to read efuse\n ");
    }
    hex_dump("efuse hex", efuse_buf, len);

    return ret;
}

/* actoion (read/write/set no more sensitive data) , efuse index , efuse byte length, efuse value and file name of token */
void main(int argc, char *argv[]){
    int ret = 0, cmd, efuse_idx;
    unsigned int len;
    unsigned char efuse_buf[256], *efuse_value = NULL;
    UnitSpecificWriteToken *token = NULL;

    if(!check_input_data(argc, argv))
    	return usage(argv[0]);

    cmd = atoi(argv[EFUSE_ACTION]);

    switch (cmd){
    case EFUSE_READ:
    	efuse_idx = atoi(argv[EFUSE_INDEX]);
	len = atoi(argv[EFUSE_LEN]);
    	ret = tee_fuse_read(efuse_idx, efuse_buf, len);
	if (ret) {
		printf("failed to read efuse\n ");
		break;
	}
	else
		hex_dump("efuse hex", efuse_buf, len);
    break;
    case EFUSE_WRITE:
    	efuse_idx = atoi(argv[EFUSE_INDEX]);
	len = atoi(argv[EFUSE_LEN]);
	if (argc == 6) {
		token = efuse_get_token(argv[EFUSE_TOKEN]);
		if (!token)
			return;
	}
	efuse_value = hexStringToBytes(argv[EFUSE_VALUE], len);
	if (!efuse_value) {
		printf("invalid efuse value %s\n", argv[EFUSE_VALUE]);
		return;
	}
	hex_dump("efuse hex", efuse_value, len);

	/* For mt8521 board, here it should enable "Fsource" (gpio control) for write operation */
	ret = tee_fuse_write_start();
	if (ret) {
		printf("failed to start to write efuse\n ");
		/* For mt8521 board, here it should disable "Fsource" (gpio control) for write operation */
		break;
	}
    	ret = tee_fuse_write(efuse_idx, efuse_value, len, token);
	tee_fuse_write_end();
	/* For mt8521 board, here it should disable "Fsource" (gpio control) for write operation */
    break;
    case EFUSE_NO_MORE_SENS_DATA:
    	printf("efuse teeFuseNoMoreSensitiveWrites test!\n");
    	ret  = tee_fuse_no_more_sensitive_writes();
	if (ret)
		printf("failed to set no more sensitive write\n");
    break;
    }

    if (efuse_value)
        free(efuse_value);
    if (token)
        free(token);

    printf("status code = %d, which means \"%s\"\n", ret, tee_fuse_error(ret));
    exit(0);
}
