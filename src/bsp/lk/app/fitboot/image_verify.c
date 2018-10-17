#include <debug.h>
#include <malloc.h>
#include <platform.h>
#include <libfdt.h>
#include <blob.h>
#include "image.h"
#include "rsa.h"

int sha256_hash(const void *source, int len, u8 *result);
int calculate_hash_multi_region(const struct image_region region[],
                                int region_count, uint8_t *checksum);
void mod_exp_65537_mont(uintptr_t *r, const uintptr_t *a, const uintptr_t *m);


#define DUMP_DATA 0
#define debug(fmt, args...)
#define MAX_HASH_NODES  64
#define SHA256_SUM_LEN  32
#define PADDING_FF_LEN  182
#define FDT_MAX_DEPTH   32
#define MAX_REGS(x)     (20 +x*7)
#define IS_NAME_MATCH(x,y) !strncmp((x), (y),strlen(y))
#define list_each_subnod(fdt, node, parent)     \
    for (node = fdt_first_subnode(fdt, parent); \
         node >= 0;                 \
         node = fdt_next_subnode(fdt, node))

const uint8_t padding_sha256_rsa2048[RSA2048_BYTES - SHA256_SUM_LEN - PADDING_FF_LEN] = {
	0x00, 0x01, 0x00, 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
};

struct hash_algo hash_algos[] = {
		{
			"sha256",
			SHA256_SUM_LEN,
			RSA2048_BYTES,
			calculate_hash_multi_region,
			padding_sha256_rsa2048,
		}
};

struct sig_algo image_sig_algos[] = {	
		{
			"sha256,rsa2048",
			&hash_algos[0],
			rsa_verify,
		}
};

static void dump_data(uint8_t *buff)
{
	uint i=0;
	
    for (i=0; i<256; i++){
	if ((i%16)==0)
		dprintf(ALWAYS,"\n");
	dprintf(ALWAYS,"0x%02x,",buff[i]);
    }
}

int rsa_check_enabled(void)
{
    return CHECK_RSA;
}

int hash_check_enabled(void)
{
    return CHECK_HASH;
}

int calculate_hash_multi_region(const struct image_region region[],
                                int region_count, uint8_t *checksum)
{

    int ret = 0;
    int32_t i,j,k=0;
    unsigned long inlen=0;
    unsigned char *input_data;
    const unsigned char *tmp_data;

    for (i = 0; i < region_count; i++)
        inlen+=region[i].size;

    input_data = malloc(inlen);

    for (i = 0; i < region_count; i++) {

        tmp_data=region[i].data;

        for (j = 0; j < region[i].size; j++)
            input_data[k++]=tmp_data[j];
    }

    ret=sha256_hash(input_data, inlen, checksum );

    free(input_data);

    return ret;
}

int fdt_getval(const void *blob, int node, const char *prop_name,int default_val)
{
    const uint *tmp;
	int get_val=0;
    int len=0;

    tmp = fdt_getprop(blob, node, prop_name, &len);
    if ((tmp !=0) && (len >= sizeof(int))) {
        get_val = fdt32_to_cpu(tmp[0]);
        return get_val;
    }
    debug("get %s fail\n",prop_name);
	
    return default_val;
}

static void rsa_verify_raw(const uint8_t *bn_n, uint8_t *bn_out, const uint8_t *bn_in)
{
    mod_exp_65537_mont((uintptr_t *)bn_out, (const uintptr_t *)bn_in, (const uintptr_t *)bn_n);
}

static void get_pubkey_info(struct key_prop *pubkey,const void *blob,int node,int *len)
{
	pubkey->exp_len = sizeof(uint64_t);
	pubkey->num_bits = fdt_getval(blob, node, BLOB_NBITS_NODE, 0);
	pubkey->n0inv = fdt_getval(blob, node, BLOB_N0INV_NODE, 0);
	pubkey->rr = fdt_getprop(blob, node, BLOB_RSQU_NODE, NULL);
	pubkey->modulus = fdt_getprop(blob, node, BLOB_MOD_NODE, NULL);
	pubkey->public_exponent = fdt_getprop(blob, node, BLOB_EXP_NODE, len);
	if (!pubkey->public_exponent || *len < sizeof(uint64_t)) {
		pubkey->public_exponent = NULL;
	}
}

static void rsa_verify_with_pubkey(struct sig_info *info,
                                  uint8_t *sig,uint sig_len, int node,uint8_t *buf)
{
	int i;
	int length;
	struct key_prop prop;
	const uint8_t *pubk ,*sign_tmp;
	const void *blob = info->pubkey;
	uint8_t __attribute__((aligned(8))) key[256], sign[256], plain[256];

    if (node < 0) {
        debug("%s: Skipping invalid node", __func__);
        return -EBADF;
    }

	get_pubkey_info(&prop,blob,node,&length);

    pubk=prop.modulus;
    for (i=0; i<256; i++)
        key[255-i]=pubk[i];

#if DUMP_DATA
    dprintf(ALWAYS,"start to dump public key ===>>>\n");
    dump_data(key);
    dprintf(ALWAYS,"\n end of dump public key <<<===\n");
#endif

    sign_tmp=sig;
    for (i = 0; i < 256; i++)
        sign[255 - i] = sign_tmp[i];

    rsa_verify_raw(key,plain ,sign);

    for (i = 0; i < 256; i++)
        buf[255 - i] = plain[i];

#if DUMP_DATA
    dprintf(ALWAYS,"\nstart to dump rsa_hash ===>>>\n");
    dump_data(buf);
    dprintf(ALWAYS,"\nend of dump public rsa_hash <<<===\n");
#endif

}

int rsa_verify(struct sig_info *info,
               const struct fdt_region region[], int region_count,
               uint8_t *sig, uint sig_len)
{
    int ret=0,i;
    int pad_len;
	const uint8_t *padding;
    uint8_t buf[sig_len];
    uint8_t hash[info->algo->hash_info->pad_len];
    struct image_region hash_region[region_count];

	/* genarte hash region*/
    for (i = 0; i < region_count; i++) {
        hash_region[i].data = info->fit_image + region[i].offset;
        hash_region[i].size = region[i].size;
    }


    if (info->algo->hash_info->hash_len >
        info->algo->hash_info->pad_len) {
        dprintf(CRITICAL,"%s: invlaid checksum-algorithm %s for %s\n",
                __func__, info->algo->hash_info->hash, info->algo->rsa);
        return -EINVAL;
    }

    /* checksum hash*/
    ret = info->algo->hash_info->hash_cal(hash_region, region_count, hash);
    if (ret < 0) {
        dprintf(CRITICAL,"%s: Error in checksum calculation\n", __func__);
        return -EINVAL;
    }

#if DUMP_DATA
	dprintf(ALWAYS,"start to dump calculate_hash ===>>\n");
	dump_data(hash);
	dprintf(ALWAYS,"\n end of dump calculate_hash <<===\n");
#endif

    if (info->req_offset != -1) {
        /*rsa calculate hash*/
        rsa_verify_with_pubkey(info, sig, sig_len,
                                     info->req_offset,&buf[0]);

        padding = info->algo->hash_info->hash_padding;
        pad_len = info->algo->hash_info->pad_len - info->algo->hash_info->hash_len;

		/**
		*padding check:
		*padding len is 224bytes: (first 2 and last 20bytes) are not 0xff ,all other is padding with 0xff :
		* { 0,0x1,0xff,0xff,...0xff,0x00, 0x30, 0x31, 0x30,0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01
		*  , 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20}
		**/
		for(i=0;i<pad_len;i++)
		{
			if(i<2){
				if(buf[i] != padding[i]){
					dprintf(CRITICAL,"RSA check padding fail first two!\n");
					return -EINVAL;
				}
			}
			else if(i>203){
				if(buf[i] != padding[i-203 +1]){
					dprintf(CRITICAL,"RSA check padding fail last 20!\n");
					return -EINVAL;
				}
			}
			else {
					if(buf[i] != 0xff){
					dprintf(CRITICAL,"RSA check padding fail not 0xff!\n");
					return -EINVAL;
				}

			}
		}

        /* compare calculate hash and rsa hash. */
        if (memcmp((uint8_t *)buf + pad_len, hash, sig_len - pad_len)) {
            dprintf(CRITICAL,"RSA check hash fail!\n");
            return -EACCES;
        }

        if (!ret)
            return ret;
    }

    return ret;
}

int fit_image_get_data(const void *fit, int noffset,
                       const void **data, size_t *size)
{
    int len;

    *data = fdt_getprop(fit, noffset, FDT_DATA_NODE, &len);
    if (*data != NULL) {
        *size = len;
        return 0;

    } else
        *size = 0;

    return -1;

}

int calculate_hash_one_region(const void *data, int data_len, const char *algo,
                              uint8_t *value, int *value_len)
{
    if (strcmp(algo, "sha256") == 0) {
        //dprintf(ALWAYS, "start cal size 0x%x sha256: %lld (us)\n", data_len,current_time_hires());
        sha256_hash((unsigned char *)data, data_len,
               (unsigned char *)value);
        //dprintf(ALWAYS, "end cal size 0x%x sha256: %lld (us)\n", data_len,current_time_hires());
        *value_len = SHA256_SUM_LEN;
    }  else {
		dprintf(ALWAYS," Unsupported hash alogrithm\n");
        return -1;
    }

    return 0;
}

int fit_get_node_value(const void *fit, int noffset, uint8_t **value,
                 int *value_len)
{
    int len;

    *value = (uint8_t *)fdt_getprop(fit, noffset, FDT_VAL_NODE, &len);

    if (*value != NULL) {
        *value_len = len;
        return 0;
    } else {
        dprintf(CRITICAL,"fit_get_node_value  NULL !!\n");
        *value_len = 0;
    }

    return -1;
}

int fit_get_hash_algo(const void *fit, int noffset, char **algo)
{
    int len;

    *algo = (char *)fdt_getprop(fit, noffset, FDT_ALGO_NODE, &len);
    if (*algo != NULL)
        return 0;
    else
        dprintf(CRITICAL,"fdt_getprop algo NULL !!\n");

    return -1;
}

static int fit_image_check_hash(const void *fit, int noffset, const void *data,
                                size_t size)
{
    uint8_t value[FIT_MAX_HASH_LEN];
    int value_len;
    char *algo;
    uint8_t *image_hash;
    int image_hash_len;

    if (fit_get_hash_algo(fit, noffset, &algo)) {
		dprintf(ALWAYS,"get hash alogrithm fail \n");
        return -1;
    }

    if (fit_get_node_value(fit, noffset, &image_hash,&image_hash_len)) {

		dprintf(ALWAYS,"get hash value fail \n");
        return -1;
    }

#if DUMP_DATA
    dprintf(ALWAYS,"\nstart dump img hash %d===>\n",image_hash_len);
    dump_data(image_hash);
#endif

    if (calculate_hash_one_region(data, size, algo, value, &value_len)) {

		dprintf(ALWAYS,"calsulate new hash fail !! \n");
        return -1;
    }

#if DUMP_DATA
	dprintf(ALWAYS,"\nstart dump cal hash %d===>\n",value_len);
	dump_data(value);
#endif

    if (value_len == image_hash_len) {
        if (memcmp(value, image_hash, value_len) != 0) {
	    	dprintf(ALWAYS,"compare hash value fail !! \n");
            return -1;
        }
	else
	    dprintf(SPEW,"check integrity success! \n");

    } else {

		dprintf(ALWAYS,"hash length error ! \n");
        return -1;
    }

    return 0;
}

static int fit_verify_prepare(struct sig_info *sign_info,
                              const void *fit, int noffset, int required_keynode)
{
    char *algo_name;

    if (fit_get_hash_algo(fit, noffset, &algo_name)) {
        return -1;
    }

    memset(sign_info, '\0', sizeof(*sign_info));
    sign_info->fit_image = (void *)fit;
    sign_info->algo = image_get_sig_algo(algo_name);
    sign_info->pubkey = &blob[0];
    sign_info->req_offset = required_keynode;
    dprintf(SPEW,"%s\n", algo_name);

    if (!sign_info->algo) {
        return -1;
    }

    return 0;
}

int fit_image_integrity_verify(const void *fit, int image_noffset)
{
    const void  *data;
    size_t      size;
    int     noffset = 0;

    /* Get image data and data length */
    if (fit_image_get_data(fit, image_noffset, &data, &size)) {

	dprintf(ALWAYS,"fit_image_get_data fail ! \n");
       return 0;
    }

    /* Process all hash subnodes of the component image node */
    list_each_subnod(fit, noffset, image_noffset) {
        const char *node_name = fit_get_name(fit, noffset, NULL);
        if(IS_NAME_MATCH(node_name,FDT_HASH_NODE)){
            if (fit_image_check_hash(fit, noffset, data, size))
                return 0;
        }
    }

    return 1;
}

int subimage_check_integrity(const void *fit, int rd_noffset)
{

    if (!fit_image_integrity_verify(fit, rd_noffset)) {

		dprintf(ALWAYS,"check integrity fail ! \n");
        return -EACCES;
    }
 
    return 0;
}
struct sig_algo *image_get_sig_algo(const char *name) {
    uint i;

    for (i = 0; i < ARRAY_SIZE(image_sig_algos); i++) {
        if (!strcmp(image_sig_algos[i].rsa, name))
            return &image_sig_algos[i];
    }

    return NULL;
}

static int fit_get_hashed_node_name(const void *fit, int noffset,char *nod_name[])
{
	int prop_len=0;
	int i = 0;

	const char *prop_start, *prop_end, *prop_name;
	prop_start = fdt_getprop(fit, noffset, FDT_HASHED_NODE, &prop_len);
	prop_end = prop_start ? prop_start + prop_len : prop_start;
	prop_name=prop_start;

	while(prop_name < prop_end){
		nod_name[i] = (char *)prop_name;
		prop_name += strlen(prop_name) + 1;
		i++;
	}

	return i;
}

static int is_recorded_string(const char *string, char *const recorder[], int rec_num)
{
	int i=0;
	int result=0;
	while(i < rec_num)
	{
		if (!strcmp(recorder[i], string))
		{
			result=1;
			break;
		}
		i++;
	}

    return result;
}

#define get_base(x) fdt_off_dt_struct(x)

int fdt_prop_check(const void * fit,uint offset,char * const invalid_list [ ],int invalid_num,int default_val)
{
	int  def_val=0;
	const char *string=NULL;
	const struct fdt_property *property=NULL;

	if(default_val >= 2)
		def_val=1;
	property = fdt_get_property_by_offset(fit, offset, NULL);
	string = fdt_string(fit, fdt32_to_cpu(property->nameoff));
	if (is_recorded_string(string, invalid_list, invalid_num))
		def_val = 0;

	return def_val;
}

int fdt_check_range(const void * fit,int reg_num ,int max_reg_num,uint nextoffset)
{
	if (nextoffset != fdt_size_dt_struct(fit))
		return -FDT_ERR_BADLAYOUT;

	if (reg_num >= max_reg_num)
		return -FDT_ERR_BADVALUE;

	return 0;
}

int fdt_parsing_regions(const void * fit,char * const record_list [ ],int record_num,char * const property_list [ ],
	int property_num,struct fdt_region region [ ],int max_regions_num)
{
	int stack[FDT_MAX_DEPTH];
	char path[200]={'\0'};
	int path_len=sizeof(path);
	int path_level = -1;
	char *ppath;
	uint nextoffset = 0;
	uint previousoffset = 0;
	uint32_t tag;
	int region_num = 0;
	int region_offset = -1;
	int record = 0;
	ppath = path;

	do {
		const char *node_name=NULL;
		int node_name_len=0;
		int is_region_recorded = 0;
		uint stopoffset = 0;
		uint startoffset=0;

		startoffset = nextoffset;
		tag = fdt_next_tag(fit, startoffset, &nextoffset);
		stopoffset = nextoffset;

		/* Start of node*/
		if(tag == FDT_BEGIN_NODE){
			path_level++;
			if (path_level == FDT_MAX_DEPTH)
				return -FDT_ERR_BADSTRUCTURE;

			node_name = fdt_get_name(fit, startoffset, &node_name_len);
			if (ppath - path + 2 + node_name_len >= path_len)
				return -FDT_ERR_NOSPACE;
			if (ppath != path + 1)
				*ppath++ = '/';
			strcpy(ppath, node_name);
			ppath += node_name_len;
			stack[path_level] = record;
			if (record == 1)
				stopoffset = startoffset;
			if (is_recorded_string(path, record_list, record_num))
				record = 2;
			else if (record)
				record--;
			else
				stopoffset = startoffset;
			is_region_recorded = record;
		}
		/* End of node */
		else if(tag == FDT_END_NODE){
			is_region_recorded = record;
			record = stack[path_level--];
			for(;ppath > path;)
			{
				if(*--ppath == '/') {
					*ppath = '\0';
					break;
				}
			}

		}
		/* Property node: contain: name , size, content */
		else if(tag == FDT_PROP){
			is_region_recorded=fdt_prop_check(fit,startoffset,property_list,property_num,record);
			stopoffset = startoffset;

		}
		/* nop  node*/
		else if(tag == FDT_NOP){
			stopoffset = startoffset;
			is_region_recorded = record >= 2;
		}
		/* End of fdt */
		else if(tag == FDT_END){
			is_region_recorded = 1;
		}

		if (is_region_recorded && region_offset == -1) {
			previousoffset = region[region_num - 1].offset +region[region_num - 1].size - get_base(fit);
			if (region_num && (region_num <= max_regions_num) && (startoffset == previousoffset))
				region_offset = region[--region_num].offset - get_base(fit);
			else
				region_offset = startoffset;
		}

		if (!is_region_recorded && region_offset != -1) {
			if (region_num < max_regions_num) {
				region[region_num].offset = get_base(fit) + region_offset;
				region[region_num].size = stopoffset - region_offset;
			}
			region_num++;
			region_offset = -1;
		}
	} while (tag != FDT_END);

	//end handle
	if (region_num < max_regions_num) {
		region[region_num].offset = get_base(fit) + region_offset;
		region[region_num].size = nextoffset - region_offset;
		region_num++;
	}

	if(fdt_check_range(fit,region_num ,max_regions_num, nextoffset))
		return 0;

	return region_num;
}

int fit_check_sign(const void *fit, int noffset, int required_keynode)
{
    int node_num;
    char path[200];
    int image_sign_len;
    uint8_t *image_sign;
    const uint32_t *strings;
    struct sig_info sign_info;
    char *const exc_prop[] = {"data"};
    char *node_name[MAX_HASH_NODES];

	/*get sign info and sign image*/
    if (fit_verify_prepare(&sign_info, fit, noffset, required_keynode))
        return -1;

    if (fit_get_node_value(fit, noffset, &image_sign,&image_sign_len))
        return -1;

#if DUMP_DATA
	dprintf(ALWAYS,"\nstart to dump sign (len=%d) ===>>\n",image_sign_len);
	dump_data(image_sign);
	dprintf(ALWAYS,"\n end of dump sign <<====\n");
#endif

	/*get node name  of fdt image*/
	node_num=fit_get_hashed_node_name(fit,noffset,node_name);


	/*parsing  regions*/
    struct fdt_region fdt_regions[MAX_REGS(node_num)];

    node_num = fdt_parsing_regions(fit, node_name, node_num,
                             exc_prop, ARRAY_SIZE(exc_prop),
                             fdt_regions, (MAX_REGS(node_num) - 1));
    if (node_num < 0) {
        return -1;
    }

    strings = fdt_getprop(fit, noffset, FDT_HASHED_STR, NULL);
    if (strings) {
        fdt_regions[node_num].offset = fdt_off_dt_strings(fit) +
                                    fdt32_to_cpu(strings[0]);
        fdt_regions[node_num].size = fdt32_to_cpu(strings[1]);
        node_num++;
    }

	/*signature verify */
    if (sign_info.algo->sig_verify(&sign_info, fdt_regions, node_num, image_sign,
                          image_sign_len)) {
        return -1;
    }

    return 0;
}

static int fit_verify_configed_sign(const void *fit, int conf_noffset,
                                    const void *sig_blob, int sig_offset)
{
    int noffset;
    int ret = -1;

    list_each_subnod(fit, noffset, conf_noffset) {
        const char *node_name = fit_get_name(fit, noffset, NULL);
		if(IS_NAME_MATCH(node_name,FDT_SIG_NODE))
		{
            ret = fit_check_sign(fit, noffset, sig_offset);
            if (ret) {

				dprintf(ALWAYS,"check sign fail ! \n");
            }
	    else {
				dprintf(SPEW,"check sign success ! \n");
                break;
            }
        }
    }

    return ret;
}

int fit_verify_sign(const void *fit, int conf_noffset)
{
	const char *req_node=NULL;
	int ret;
    int noffset;
    int sig_node;
    const void *sig_blob=&blob[0];

    sig_node = fdt_subnode_offset(sig_blob, 0, FDT_SIG_NODE);
    if (sig_node < 0) {
        dprintf(CRITICAL," No sign node (signature): %s\n",fdt_strerror(sig_node));
        return -1;
    }

    list_each_subnod(sig_blob, noffset, sig_node) {
        req_node = fdt_getprop(sig_blob, noffset, BLOB_REQ_NODE, NULL);
		if((req_node !=NULL)&&(!strcmp(req_node, "conf"))){
	        ret = fit_verify_configed_sign(fit, conf_noffset, sig_blob,noffset);
	        if (ret) {
	            printf("Failed to verify '%s'\n",fit_get_name(sig_blob, noffset, NULL));
	        }
			return ret;
	    }
	}

	return -1;
}
