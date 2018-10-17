#ifndef _CFG_ETHERNET_FILE_H
#define _CFG_ETHERNET_FILE_H

typedef struct
{
	unsigned char macaddr[6];
    unsigned char Reserved1[64 - 6];       // Reserved
}File_Ethernet_Struct;

#define CFG_FILE_ETHERNET_REC_SIZE   sizeof(File_Ethernet_Struct)
#define CFG_FILE_ETHERNET_REC_TOTAL   1

#endif