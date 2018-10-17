#!/usr/bin/env python

import argparse
import csv
import sys
import struct
import os
import math
import gen_nand_header
import gen_nand_header_v11

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('nand_name', help = 'nand device name')
	parser.add_argument('version', help= 'nand header version')
	parser.add_argument('table_file', help = 'partition table file')
	parser.add_argument('image', help = 'image file name')
	parser.add_argument('start_page', help = 'partition start page index')
	parser.add_argument('end_page', help = 'partition end page index')
	parser.add_argument('part_name', help = 'partition name')
	args = parser.parse_args()

	version = float(args.version)
	start_page = int(args.start_page)
	end_page = int(args.end_page)
	if version == 2.0:
		(vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, ppen, ppb) = gen_nand_header.gen_header(args.nand_name)
	elif version == 1.1:
		(vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, ppen, ppb) = gen_nand_header_v11.gen_header(args.nand_name, 192, args.nand_name + '_header.bin')

	file_len = os.path.getsize(args.image)
	if (file_len % (page_size + spare_size)) != 0:
		raise Exception("image size is not full page size aligned")
	block_num = int(math.ceil(float(file_len) / ((page_size + spare_size) * ppb)))
	if (start_page % ppb) != 0:
		raise Exception("partition start page is not block align")
	start_blk = start_page / ppb
	if ((end_page + 1) % ppb) != 0:
		raise Exception("partition end page is not block align")
	end_blk = end_page / ppb
	line_info = [start_blk, end_blk, block_num, '0xffffffff', args.part_name]

	with open(args.table_file, "ab") as csvf:
		csvwriter = csv.writer(csvf, delimiter=';')
		csvwriter.writerow(line_info)

	# remove temp files #
	os.remove(args.nand_name + '_header.bin')

if __name__ == "__main__":
        sys.exit(main(sys.argv))
