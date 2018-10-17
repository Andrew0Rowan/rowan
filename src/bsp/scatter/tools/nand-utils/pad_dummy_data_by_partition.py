#!/usr/bin/env python

import argparse
import sys
import os
import math
import pp_tbl
import gen_nand_header
import gen_nand_header_v11
from pad_ecc_by_sector import pad_ecc

def padding(data, size):
	return data + '\xff' * (size - len(data))

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('nand_name', help = 'nand device name')
	parser.add_argument('in_image', help = 'raw input image')
	parser.add_argument('part_size', help = 'partition size')
	parser.add_argument('version', help= 'nand header version')
	args = parser.parse_args()
	version = float(args.version)

	if version == 2.0:
		(vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, ppen, ppb) = gen_nand_header.gen_header(args.nand_name)
	elif version == 1.1:
		(vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, ppen, ppb) = gen_nand_header_v11.gen_header(args.nand_name, 192, args.nand_name + '_header.bin')

	# input image page size alignment #
	file_len = os.path.getsize(args.in_image)
	pages = int(int(args.part_size) / page_size)
	part_size = pages * (page_size + spare_size)
	with open(args.in_image, "rb+") as f:
		f.read()
		f.seek(0, 2)
		f.write('\xff' * (part_size - file_len))

	os.remove(args.nand_name + '_header.bin')

if __name__ == "__main__":
        sys.exit(main(sys.argv))
