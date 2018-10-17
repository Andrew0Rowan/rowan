#!/usr/bin/env python

import argparse
import os
import struct
import sys

def read(path):
	with open(path, "rb") as f:
		return f.read()

def write(path, data):
	with open(path, "wb") as f:
		f.write(data)

def padding(data, size, pattern = '\0'):
	return data + pattern * (size - len(data))

def align(data, size, pattern = '\0'):
	return padding(data, (len(data) + (size - 1)) & ~(size - 1), pattern)

def gen_nor_bl_img(data):
	data = align(data, 512, '\xff')
	header = (padding(struct.pack("<12sII", "SF_BOOT", 1, 512), 512, '\xff') +
		  padding(struct.pack("<8sIIIIIIII", "BRLYT", 1, 2048, 2048 + len(data),
			0x42424242, 0x00010007, 2048, 2048 + len(data), 1) + '\0' * 140, 512, '\xff') +
		  '\0' * 1024)
	return header + data

def gen_emmc_bl_img(data):
	data = align(data, 512, '\xff')
	header = (padding(struct.pack("<12sII", "EMMC_BOOT", 1, 512), 512, '\xff') +
	          padding(struct.pack("<8sIIIIIIII", "BRLYT", 1, 2048, 2048 + len(data),
	                0x42424242, 0x00010005, 2048, 2048 + len(data), 1) + '\0' * 140, 512, '\xff') +
	          '\0' * 1024)
	return header + data

def gen_sdcard_bl_img(data):
	data = align(data, 512, '\xff')
	header = (padding(struct.pack("<12sII", "SDMMC_BOOT", 1, 512), 512, '\xff') +
		  padding(struct.pack("<8sIIIIIIII", "BRLYT", 1, 2048, 2048 + len(data),
			0x42424242, 0x00010008, 2048, 2048 + len(data), 1) + '\0' * 140, 512, '\xff') +
		  '\0' * 1024)
	return header + data

def get_page_size(nand_name):
	with open("scripts/nand-setting.txt", "r") as f:
		for l in f:
			s = l.split("\t")
			if s[0] == nand_name:
				return int(s[1])
	raise KeyError("not support " + nand_name)

def gen_nand_bl_img(data, nand_name, page_number_bl1, page_number_bl2):
	hdr1 = "temp1.hdr"
	hdr2 = "temp2.hdr"
	page_size = get_page_size(nand_name)
	data = align(data, 128*page_size, '\xff')
	os.system("./tools/bch h {0} null {1} 1 {2} 0 0".format(nand_name, hdr1, page_number_bl1))
	os.system("./tools/bch h {0} null {1} 1 {2} 0 0".format(nand_name, hdr2, page_number_bl2))
	header1 = padding(read(hdr1), 256*page_size, '\xff')
	header2 = padding(read(hdr2), 256*page_size, '\xff')
	os.remove(hdr1)
	os.remove(hdr2)
	return header1 + header2 + data + data

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('type', default='emmc', help='emmc | nand | nor | sdcard')
	parser.add_argument('in_image', help='input file path')
	parser.add_argument('out_image', help='output image path')
	parser.add_argument('nand_name', nargs='?', help='nand chip name')
	parser.add_argument('bl1', type=int, nargs='?', default=0, help='page number of bl1. default=0')
	parser.add_argument('bl2', type=int, nargs='?', default=0, help='page number of bl2. default=0')
	args = parser.parse_args()

	if args.type == 'emmc':
		r = gen_emmc_bl_img(read(args.in_image))
	elif args.type == 'nor':
		r = gen_nor_bl_img(read(args.in_image))
	elif args.type == 'sdcard':
		r = gen_sdcard_bl_img(read(args.in_image))
	elif args.type == 'nand':
		r = gen_nand_bl_img(read(args.in_image), args.nand_name, args.bl1, args.bl2)
	else:
		print("unknown type: " + args.type)
		return 1

	write(args.out_image, r)
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
