/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */


#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <malloc.h>
#include <endian.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#pragma pack(push)
#pragma pack(1)
struct ubick_header {
	char magic[4];
	uint8_t version;
	uint8_t hash_type;
	uint32_t block_size;
	uint32_t block_interval;
	uint32_t block_count;
	uint8_t hash_salt[32];
	uint8_t rsa_sig[256];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct ubi_ec_hdr {
	uint32_t  magic;
	char    version;
	char    padding1[3];
	uint64_t  ec; /* Warning: the current limit is 31-bit anyway! */
	uint32_t  vid_hdr_offset;
	uint32_t  data_offset;
	uint32_t  image_seq;
	char    padding2[32];
	uint32_t  hdr_crc;
};
#pragma pack(pop)

#define UBI_CRC32_INIT 0xFFFFFFFFU
extern uint32_t crc32(uint32_t val, const void *ss, int len);

int getfilesize(char* path)
{
	int result;
	struct stat buf;
	result = stat(path,&buf);
	if(result == 0)
		return buf.st_size;
	else
		return 0;
}

/*
mtd_verity steps
1.get ubi image size and fill it to final UBI image
2.generate ubick_header
3.calculate sha256 hash according block_size,block_interval and block_count
4.generate hash_slat
5.encrypt slated hash with verified key
6.output final UBI image
*/
static void usage(const char* name)
{
	fprintf(stderr,"Usage:\n"
		"%s [input image] [block size] [block interval] [block count] [privkey] [output image]\n"
		"Options:\n"
		"input image: path of input UBI image\n"
		"block size: size of hash block\n"
		"block interval: size of hash interval\n"
		"block count: number of hash block\n"
		"privkey: path of privkey\n"
		"output image: path of output UBI image\n"
		"NOTE:block_size=block_internval=blockcount=-1 means check whole image\n\n",name);
}

#define RSA_LEN 256
#define HASH_LEN 32
static void RSA_Private_Encrypt(const char* key,uint8_t* in,uint8_t* out,int flen,int padding)
{
	OpenSSL_add_all_algorithms();
	FILE* rsa_fp=NULL;
	RSA* rsa=NULL;
	uint8_t in_tmp[RSA_LEN];
	uint8_t out_tmp[RSA_LEN];
	int i=0;
	
	rsa_fp= fopen(key,"r");
	rsa = PEM_read_RSAPrivateKey(rsa_fp,NULL,NULL,NULL);
	RSA_private_encrypt(RSA_LEN,in,out,rsa,padding);
}

void dump_hex(const char* name,uint8_t buff[],int len)
{
	int i=0;
	printf("%s content is:",name);
	for(;i<len;i++)
	{
		if(i%16 == 0) printf("\n");
		printf("0x%x ",buff[i]);
	}
	printf("\n");
}

#define RSA_GAP 0x10
#define SIGN_PADDED_SIZE 4096
int main(int argc,char **argv)
{
	struct ubick_header * header = NULL;
	struct stat in_img_sb,out_img_sb;
	char* in_image = NULL, *out_image = NULL, *privkey_file = NULL;
	char* modulus_file = NULL;
	uint32_t block_size,block_interval,block_count;
	int in_image_fd=-1,out_image_fd=-1;
	//mpuint *privkey = NULL,*modulus = NULL;
	uint32_t in_image_size;
	uint8_t *buf;
	uint8_t hash_value[HASH_LEN];
	SHA256_CTX sha256_state;
	int i;
	unsigned char sign_buffer[SIGN_PADDED_SIZE]={0};

	if(argc != 7)
	{
		usage(argv[0]);
		return -1;
	}

	in_image = argv[1];
	block_size = atoi(argv[2]);
	block_interval = atoi(argv[3]);
	block_count = atoi(argv[4]);
	privkey_file = argv[5];
	out_image = argv[6];

	if(stat(in_image,&in_img_sb)!=0)
	{
		printf("Could not stat input UBI image %s\n",in_image);
		return -1;
	}

	if(block_size == -1)
	{
		if(block_interval != -1 || block_count !=-1)
		{
			printf("block_size,block_interval and block_count must be all -1 if want to check whole image\n");
			return -1;
		}
	}
	
	in_image_size = getfilesize(in_image);
	if(block_size==-1)
	{
		/*check whole image*/
		block_size = in_image_size;
		block_interval = 0;
		block_count = 1;
	}
	if(block_size*block_count+block_interval*(block_count-1) > in_image_size)
	{
		block_count = in_image_size/(block_size+block_interval);
		printf("block size is too large, trim it to %d\n",block_count);
	}

	in_image_fd = open(in_image,O_RDONLY);
	buf = malloc(in_image_size);
	read(in_image_fd,buf,in_image_size);
	close(in_image_fd);

	/*hack set UBI image size to ubi_ec_hdr unused field and recal crc32*/
	*(uint32_t*)&buf[sizeof(struct ubi_ec_hdr)-2*sizeof(uint32_t)] = htobe32(in_image_size);
	*(uint32_t*)&buf[sizeof(struct ubi_ec_hdr)-sizeof(uint32_t)] = htobe32(crc32(UBI_CRC32_INIT,buf,sizeof(struct ubi_ec_hdr)-sizeof(uint32_t)));
	
	out_image_fd = open(out_image, O_CREAT|O_TRUNC|O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	write(out_image_fd,buf,in_image_size);

	header = malloc(sizeof(struct ubick_header));
	memcpy(header->magic,"UBIC",sizeof(header->magic));
	header->block_size = htobe32(block_size);
	header->block_interval = htobe32(block_interval);
	header->block_count = htobe32(block_count);
	header->hash_type = 0;
	header->version = 0;

	/*generate hash_slat*/
	memcpy(header->hash_salt,buf,sizeof(header->hash_salt));

	SHA256_Init(&sha256_state);

	for(i=0;i<block_count;i++)
	{
		SHA256_Update(&sha256_state,&buf[i*block_size+i*block_interval],block_size);
	}

	SHA256_Update(&sha256_state,(uint8_t*)header,sizeof(*header)-sizeof(header->rsa_sig));

	SHA256_Final(hash_value,&sha256_state);

	memset(header->rsa_sig,0,RSA_LEN);
	memcpy(header->rsa_sig+RSA_GAP,hash_value,HASH_LEN);
	
	RSA_Private_Encrypt(privkey_file,header->rsa_sig,header->rsa_sig,RSA_LEN,RSA_NO_PADDING);
	
	dump_hex("header",header,sizeof(*header));
	
	memcpy(sign_buffer,header,sizeof(*header));
	write(out_image_fd,sign_buffer,SIGN_PADDED_SIZE);

	close(out_image_fd);
	
//#define RSA_TEST
#ifdef RSA_TEST
	{
		extern uint8_t *rsa_encryptdecrypt(const uint8_t *sig, const uint8_t *e, const uint8_t *n);
		#include "nfsb_key_modulus.h"
		uint8_t* decrypted_content = NULL;
		uint8_t in_tmp[RSA_LEN];
		uint8_t out_tmp[RSA_LEN];
		int i=0;
		uint8_t excepted_e[4] = {0x00, 0x01, 0x00, 0x01};
		decrypted_content = rsa_encryptdecrypt(header->rsa_sig,excepted_e,rsa_modulus);
		dump_hex("decrypted content",decrypted_content,RSA_LEN);
	}
#endif
	
	return 0;
	
}

