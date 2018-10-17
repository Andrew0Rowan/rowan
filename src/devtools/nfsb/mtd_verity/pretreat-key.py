#!/usr/bin/python

import os
import struct
import sys
import binascii
import commands

TARGET_H_FILE_PATH = "../src/kernel/linux/v4.4/include/nfsb_key_modulus.h"
LOCAL_KEY = "../src/devtools/nfsb/mtd_verity/rsa.key"

def parse_key(keyfile):
	temp_key_file = "%s.temp" % keyfile
	if os.path.exists(temp_key_file):
		os.remove(temp_key_file)

	cmdline = "openssl rsa -text -in %s -pubout > %s" % (keyfile, temp_key_file)
	os.system(cmdline)
	#check if keyfile generated success
	if not os.path.exists(temp_key_file):
		print "[error] command line excute failed: %s , please check you linux environment" % cmdline
		exit(-1)
	
	#parse keys
	#read keyfile
	lines = ""
	for line in open(temp_key_file):
		lines += line
	
	#replace \r \next
	lines = lines.replace("\n", "")
	lines = lines.replace(" ", "")
	
	#get public key
	#from modulus to publicExponent
	start_idx = lines.find("modulus:") + len("modulus:")
	end_idx = lines.find("publicExponent")
	public_key = lines[start_idx:end_idx]
	if public_key[:2] == "00":
		public_key = public_key[2:]
	public_key = public_key.replace(":", "")
	#print "public_key :" + public_key
	
	#get private key
	#from privateExponent to prime1
	start_idx = lines.find("privateExponent:") + len("privateExponent:")
	end_idx = lines.find("prime1")
	private_key = lines[start_idx:end_idx]
	if private_key[:2] == "00":
		private_key = private_key[2:]
	private_key = private_key.replace(":", "")
	#print "\nprivate_key:" + private_key
	
	os.remove(temp_key_file)

	return (public_key, private_key)

def string_to_bin(string,output_file):
	if os.path.exists(output_file):
		os.remove(output_file)

	if (len(string) != 256*2):
		print "[error] key length is not 256, please check"
		exit(-1)

	of = open(output_file, 'wb')

	for i in range(0,len(string),2):
		num=(int(string[i],16)<<4) + int(string[i+1],16)
		byte=struct.pack('B',num)
        	of.write(byte)
	
	of.close	


def bin_file_to_h_file(in_bin_file,out_h_file):
	if not os.path.exists(in_bin_file):
		print "[error] File %s not exist, please check" % in_bin_file
		exit(-2)

	if os.path.exists(out_h_file):
		os.remove(out_h_file)
	
	of = open(out_h_file, 'w')	

	j = 0
	with open(in_bin_file, 'rb') as f:
        	content = f.read()
        	for i in range(len(content)):
                	if j % 8 == 0:
				of.write("\n    ")
                	j += 1

			of.write("0x%s," % binascii.hexlify(content[i])),
		of.write("\n")

	f.close()
	of.close()

def gen_final_h_file(in_h_file,out_h_file):
        if not os.path.exists(in_h_file):
                print "[error] File %s not exist, please check" % in_h_file
                exit(-2)

        if os.path.exists(out_h_file):
                os.remove(out_h_file)

	inf = open(in_h_file, 'r')
	of = open(out_h_file, 'w')

	of.write("static uint8_t rsa_modulus[256] = \n")
	of.write("{\n")
	of.write(inf.read())
	of.write("};\n")

	inf.close()
	of.close()

def get_top_dir():
	top_path = commands.getoutput('echo ${TOPDIR}')
	print "top_path=%s" % top_path

	test_path = commands.getoutput('echo ${PATH}')
	print "test_path=%s" % test_path
	
	return top_path

if __name__ == "__main__":
	if len(sys.argv) != 2:
        	print("Usage: %s top_dir" % sys.argv[0])
        	sys.exit(2)

	top_path = str(sys.argv[1]);
	key_file_path = top_path + "/" +  LOCAL_KEY
	if not os.path.exists(key_file_path):
		print "[error] File %s not exist, please check" % key_file_path
		exit(-1)

	(img_auth_public_key, img_auth_private_key) = parse_key(key_file_path)

	public_key_out_file = "%s.pub_out" % key_file_path
	private_key_out_file = "%s.pri_out" % key_file_path

	string_to_bin(img_auth_public_key,public_key_out_file)
	string_to_bin(img_auth_private_key,private_key_out_file)

	h_file = "%s.temp" % public_key_out_file 
	bin_file_to_h_file(public_key_out_file,h_file);

	final_h_file = "%s.h" % h_file
	gen_final_h_file(h_file,final_h_file)

	target_h_file = top_path + "/" + TARGET_H_FILE_PATH 
	cmd="cp -f %s %s" %(final_h_file , target_h_file)
	os.system(cmd)

	os.remove(h_file)
	os.remove(final_h_file)

	#os.remove(key_file_path)
