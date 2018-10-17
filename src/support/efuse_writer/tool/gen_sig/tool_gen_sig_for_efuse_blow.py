import os
import sys
import struct
import shutil
import subprocess
import binascii

'''
    structure of eFuse write permission 
    typedef struct {
        u32 magic;                  /* constant 0x719ea32c in big endian */
        u32 length;                 /* length of rest of this message in big endian */
        u8 hrid[8];                 /* HRID fuse value */
        u32 fuseIndex;              /* fuse index in big endian */
        u8 fuseValueLen;            /* byte length of fuse value, must match arg in teeFuseWrite */
        u8 fuseValue[fuseValueLen]; /* the fuse value, must match arg in teeFuseWrite */
        u8 pubkey[varies];          /* RSA public key in PKCS#8 DER format */  
        u8 sig[2048/8];             /* RSA w/ SHA256 signature and PSS padding over all previous fields */
    } UnitSpecificWriteToken;
'''

# show tool usage
def print_cmd_usage():
    print "usage: python tool_gen_sig_for_efuse_blow.py -id dev_hrid.bin -idx num -val efuse_value.bin -pubk pubk.der -prik prik.der -out out.bin"
    print "       -id   => binary file contains device id, 8 bytes"
    print "       -idx  => number of efuse index, decimal"
    print "       -val  => binary file contains efuse value for the efuse index"
    print "       -pubk => binary file contains rsa public  key (PKCS8 format, DER)"
    print "       -prik => binary file contains rsa private key (PKCS8 format, DER)"
    print "       -out  => binary file contains UnitSpecificWriteToken"
    return    

# main function
def main():
    # parse intput parameters
    if len(sys.argv) != 13:
        print_cmd_usage()
    efuse_idx = 0
    efuse_val_sz = 0
    tmp_bin_filename = "__tmp.bin"
    tmp_hash_bin_filename = "__tmp_hash.bin"
    tmp_sig_bin_filename  = "__tmp_sig.bin"
    dev_id_filename = ""
    efuse_val_filename = ""
    pubk_der_filename  = ""
    prik_der_filename  = ""
    out_filename  = ""
    idx = 1
	  
    while idx < len(sys.argv):
    	if sys.argv[idx] == "-id":
    	    dev_id_filename = sys.argv[idx+1]
    	    if os.stat(dev_id_filename).st_size != 8:
    	    	  print "error: incorrect device id size"
    	    	  return -1
    	elif sys.argv[idx] == "-idx":
    	    efuse_idx = int(sys.argv[idx+1])
    	elif sys.argv[idx] == "-val":
      	  efuse_val_filename = sys.argv[idx+1]
      	  efuse_val_sz = os.stat(efuse_val_filename).st_size
      	  if efuse_val_sz <= 0 or efuse_val_sz >= 256:
      	  	print "error: efuse value file size should be less than 256"
      	  	return -1
    	elif sys.argv[idx] == "-pubk":
      	  pubk_der_filename = sys.argv[idx+1]
      	  if os.stat(pubk_der_filename).st_size == 0:
    	    	  print "error: public key file size is zero"
    	    	  return -1
    	elif sys.argv[idx] == "-prik":
      	  prik_der_filename = sys.argv[idx+1]
      	  if os.stat(prik_der_filename).st_size == 0:
    	    	  print "error: private key file size is zero"
    	    	  return -1
    	elif sys.argv[idx] == "-out":
      	  out_filename = sys.argv[idx+1]
    	else:
          print "error: unknow parameter ", sys.argv[idx]
    	idx+=2
    
    # 0. prepare source binary file
    f_src = open(tmp_bin_filename, "wb")
    
    # 1 magic = 0x719ea32c
    magic_no = struct.pack('>L', 0x719ea32c)
    f_src.write(magic_no)
    print "magic" + "(" , len(magic_no), "): " + '0x%4x' % (0x719ea32c)

    # 2 length = 1*8 (hrid) + 4 (fuseIndex) + 1 (fuseValueLen) + fuseValueLen (fuseValue) + size of pubk (DER format) + 256 (sig)
    token_len = 1*8 + 4 + 1 + os.stat(efuse_val_filename).st_size + os.stat(pubk_der_filename).st_size + 256
    f_src.write(struct.pack('>L', token_len))
    print "length (", len(struct.pack('>L', token_len)), "): ", '0x%04x' % token_len, "(", token_len , ")"
    
    # 3 hrid
    f_dev_id = open(dev_id_filename, "rb")
    hrid = f_dev_id.read()
    f_src.write(hrid);
    f_dev_id.close();
    print "hrid (", len(hrid), "): 0x" + binascii.b2a_hex(hrid)

    # 4 fuseIndex
    f_src.write(struct.pack('>L', efuse_idx))
    print "fuseIndex (", len(struct.pack('>L', efuse_idx)),"): ", '0x%04x' % efuse_idx, "(", efuse_idx , ")"

    # 5 fuseValueLen
    f_src.write(struct.pack('>B', efuse_val_sz))
    print "fuseValueLen (", len(struct.pack('>B', efuse_val_sz)), "): ", '0x%02x' % efuse_val_sz, "(", efuse_val_sz , ")"
    
    # 6 fuseValue
    f_efuse_value = open(efuse_val_filename, "rb")
    fuseValue = f_efuse_value.read()
    f_src.write(fuseValue);
    f_efuse_value.close();
    print "fuseValue (", len(fuseValue),"): 0x" + binascii.b2a_hex(fuseValue)

    # 7 pubkey
    f_pubk_der = open(pubk_der_filename, "rb")
    f_src.write(f_pubk_der.read());
    f_pubk_der.close();
    f_src.close()
    print "pubkey (",  os.stat(pubk_der_filename).st_size, ")"

    # 8 sig
    # 8.1 sha256
    cmd = "openssl dgst -sha256 -binary -out " + tmp_hash_bin_filename + " " + tmp_bin_filename
    try:
    	subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError as e:
	  	print "error: fail to perform sha256"
	  	return -1

    # 8.1 rsa sign
    cmd = "openssl pkeyutl -sign -in " + tmp_hash_bin_filename + " -keyform DER -inkey " + prik_der_filename + " -out " + tmp_sig_bin_filename + " -pkeyopt digest:sha256 -pkeyopt rsa_padding_mode:pss -pkeyopt rsa_pss_saltlen:32"
    try:
    	subprocess.check_call(cmd, shell=True)
    except subprocess.CalledProcessError as e:
	  	print "error: fail to perform rsa sign with pss-sha256"
	  	return -1

    # 9 generate final signature
    f_out = open(out_filename, "wb")
    f_src = open(tmp_bin_filename, "rb")
    f_out.write(f_src.read());
    f_src.close();
    f_src = open(tmp_sig_bin_filename, "rb")
    f_out.write(f_src.read());
    f_src.close();
    f_out.close()
    print "sig (",  os.stat(tmp_sig_bin_filename).st_size, ")"

    print "succeed to generate", out_filename, "(", os.stat(out_filename).st_size, ")"

    return 0

# program entry
if __name__ == '__main__':
    main()