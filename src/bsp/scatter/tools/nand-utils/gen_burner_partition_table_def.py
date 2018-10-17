#!/usr/bin/env python

import argparse
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
	parser.add_argument('pad', help = '0: pad header, 1: no pad, 2: pad tail')
	args = parser.parse_args()
	pad = int(args.pad)

	if pad == 0:
		line_info = "GROUP DEFINE2" + (3 * '\0')
	elif pad == 2:
		line_info = 16 * '\xff'
	elif pad == 1:
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

		data_type = 1

		line_info = struct.pack("I", data_type)
		line_info += struct.pack("I", start_blk)
		line_info += struct.pack("I", end_blk)
		line_info += struct.pack("I", block_num)

		# remove temp files #
		os.remove(args.nand_name + '_header.bin')
	else:
		raise Exception("wrong pad type")

	with open(args.table_file, "ab") as f:
		f.write(line_info)

if __name__ == "__main__":
        sys.exit(main(sys.argv))
