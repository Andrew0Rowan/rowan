#!/usr/bin/env python

import argparse
import sys
import os
import math
import ecc_bch

def pad_ecc(in_image, out_image, sector_size, fdm_size, fdmecc_size, ecc_level, spare_size, page_size, bad_swap):
	# file size check #
	file_len = os.path.getsize(in_image)
	if (file_len % sector_size) != 0:
		raise Exception("input image size is not sector size aligned")
	sector_num = file_len / sector_size

	# calculate parity byte number #
	parity_byte = int(math.ceil(float(ecc_level) * 14 / 8))

	# bad block mark control setting #
	if bad_swap != 0:
		bad_sec = page_size / (sector_size + spare_size)
		bad_pos = page_size % (sector_size + spare_size)
		sec_per_page = page_size / sector_size
		print "bad sector %d, bad position %d, sector number per page %d " \
			% (bad_sec, bad_pos, sec_per_page)

	f_in = open(in_image, "rb")
	f_out = open(out_image, "wb")

	for i in range(sector_num):
		percent = float(i) / float(sector_num) * 100
		sys.stdout.writelines("%.1f"%percent)
		sys.stdout.writelines("%\r")
		sys.stdout.flush();
		f_in.seek(sector_size * i, 0)
		f_out.seek((sector_size + spare_size) * i, 0)
		sector_data = f_in.read(sector_size)
		sector_data += '\xff' * fdmecc_size
		# bad mark swap #
		if bad_swap != 0:
			if (i % sec_per_page) == bad_sec:
				temp_buf = bytearray(sector_data)
				temp_byte = temp_buf[bad_pos]
				temp_buf[bad_pos] = temp_buf[sector_size]
				temp_buf[sector_size] = temp_byte
				sector_data = str(temp_buf)
		parity_data = ecc_bch.bch_enc_14(sector_data, sector_size + fdmecc_size, ecc_level)
		sector_data += '\xff' * (fdm_size - fdmecc_size)
		sector_data += parity_data
		sector_data += '\xff' * (spare_size - fdm_size - parity_byte)
		f_out.write(sector_data)

	f_in.close()
	f_out.close()

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('in_image', help = 'raw input image')
	parser.add_argument('out_image', help = 'ecc padded image')
	parser.add_argument('sector_size', help = 'sector size')
	parser.add_argument('fdm_size', help = 'FDM size')
	parser.add_argument('fdmecc_size', help = 'FDM ECC size')
	parser.add_argument('ecc_level', help = 'ECC level')
	parser.add_argument('spare_size', help = 'spare size per sector')
	parser.add_argument('page_size', help = 'page size')
	parser.add_argument('bad_swap', help = 'whether do bad mark swap')
	args = parser.parse_args()

	sector_size = int(args.sector_size)
	fdm_size = int(args.fdm_size)
	fdmecc_size = int(args.fdmecc_size)
	ecc_level = int(args.ecc_level)
	spare_size = int(args.spare_size)
	page_size = int(args.page_size)
	parity_byte = int(math.ceil(float(ecc_level) * 14 / 8))
	bad_swap = int(args.bad_swap)

	print "secsize %d, fdmsz %d, fdmeccsz %d, lel %d, spare %d, parity_byte %d page size %d bad_swap %d" \
		% (sector_size, fdm_size, fdmecc_size, ecc_level, spare_size, parity_byte, page_size, bad_swap)

	pad_ecc(args.in_image, args.out_image, sector_size, fdm_size, fdmecc_size, ecc_level, spare_size, page_size, bad_swap)

if __name__ == "__main__":
	sys.exit(main(sys.argv))
