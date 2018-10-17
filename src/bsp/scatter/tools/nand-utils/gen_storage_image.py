#!/usr/bin/env python

import argparse
import sys
import os
import math
import pp_tbl
from gen_nand_header import gen_header
from pad_ecc_by_sector import pad_ecc
from randomizer import randomizer

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('nand_name', help = 'nand device name')
	parser.add_argument('in_image', help = 'raw input image which has GPT')
	parser.add_argument('out_image', help = 'the output storage image ')
	parser.add_argument('rand_en', help = 'if randomizer is enabled')
	args = parser.parse_args()

	rand_en = int(args.rand_en)

	header_file = args.nand_name + '_header.bin'
	header_size = 440

	with open(args.in_image, "rb") as f_in:
		f_in.seek(header_size, 0)
		in_data = f_in.read()

	(vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, ppen, ppb) = gen_header(args.nand_name)
	with open(header_file, "rb+") as hf:
		hf_data = hf.read()
		hf.write(in_data)

	ecc_file = args.out_image
	rand_file = args.out_image
	if rand_en == 1:
		ecc_file = args.nand_name + '_ecc.bin'
	if ppen == 1:
		paired = raw_input("insert empty paired page ? Y/y or N/n ")
		if paired in ['Y', 'y']:
			rand_file = args.nand_name + '_rand.bin'
	sector_size = 1024 if page_size > 512 else 512
	sector_spare_size = spare_size / (page_size / sector_size)
	pad_ecc(header_file, ecc_file, sector_size, fdm_size, fdmecc_size, ecc_strength, sector_spare_size, page_size, 1)

	if rand_en == 1:
		randomizer(ecc_file, rand_file, "SS", 1, page_size, spare_size, sector_size, ppb, ppen, vendor)

	# insert empty paired page #
	if ppen == 1 and paired in ['Y', 'y']:
		if rand_en == 1:
			pp_file = rand_file
		else:
			pp_file = ecc_file
		if vendor == 'TSB':
			PP = pp_tbl.TSB_PP
		else:
			raise Exception("unsupport %s pp table " % vendor)
		f_pp = open(pp_file, "rb")
		f_out = open(args.out_image, "wb")

		if (os.path.getsize(header_file) % page_size) != 0:
			raise Exception("input image size is not page size aligned")
		page_number = os.path.getsize(header_file) / page_size
		block_number = int(math.ceil(float(page_number) / (ppb / 2)))
		print "page num %d  block num %d" % (page_number, block_number)
		for i in range(ppb * block_number):
			if page_number > 0 and (i % ppb) in PP:
				f_out.write(f_pp.read(page_size + spare_size))
				page_number -= 1
			else:
				f_out.write('\xff' * (page_size + spare_size))
		f_pp.close()
		f_out.close()
		# remove temp files #
		os.remove(pp_file)

	if rand_en == 1:
		os.remove(ecc_file)

	os.remove(header_file)

if __name__ == "__main__":
        sys.exit(main(sys.argv))
