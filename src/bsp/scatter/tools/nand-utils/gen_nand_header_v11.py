#!/usr/bin/env python

import ecc_bch
import sys
import os
import struct
import argparse

def gen_header(nand_name, lk_start_addr, out_image):
	""" device info:
	name vendor interface pagesize(B) cycle sparesize(B) pageperblock total_blocks
	page_shift block_shift ecc_strength
	"""
	with open(os.getcwd() + "/tools/nand-utils/nand_device_list_v11.txt", "r") as f:
		found = 0
		for l in f:
			s = l.split()
			if s[0] == nand_name:
				found = 1
				vendor = s[1]
				interface = eval(s[2])
				page_size = int(s[3])
				address_cycle = int(s[4])
				spare_size = int(s[5])
				page_per_block = int(s[6])
				total_blocks = int(s[7])
				page_shift = int(s[8])
				block_shift = int(s[9])
				ecc_strength = int(s[10])

		if not found:
			raise KeyError("not support " + nand_name)

		#print s[0]
		#print "vendor %s, page size %d, page shift %d, spare size %d, page per block %d" \
		#	% (vendor, page_size, page_shift, spare_size, page_per_block)
		#print "address cycle %d, ecc strength %d, interface %d block_shift %d total_blocks %d" \
		#	% (address_cycle, ecc_strength, interface, block_shift, total_blocks)

	# step1: generate nand header info #
	header = "BOOTLOADER!" + '\0'
	header += "V006"
	header += "NFIINFO" + '\0'
	header += struct.pack("H", interface)
	header += struct.pack("H", page_size)
	header += struct.pack("H", address_cycle)
	# pad 0 for nand type, means slc nand#
	header += struct.pack("H", 0)
	# pad tlc control #
	header += '\0' * 12
	header += struct.pack("H", spare_size)
	header += struct.pack("H", page_per_block)
	header += struct.pack("H", total_blocks)
	header += struct.pack("H", page_shift)
	header += struct.pack("H", block_shift)
	# pad arm align data #
	header += '\0' * 2
	# pad mlc control #
	header += '\0' * 20
	# pad dummy #
	header += '\0' * (4 + 4 + 28)
	# calculate header ecc parity #
	header_size = 112
	ecc_level = 80
	ecc = ecc_bch.bch_enc_14(header, header_size, ecc_level)
	header += ecc
	# pad 4B dummy #
	header += '\0' * 4
	# double nfb header #
	header += header
	# pad page size align dummy data #
	header += '\0' * (page_size - 512)

	# step2: generate brlyt #
	header += "BRLYT" + ('\0' * 3)
	brlyt_version = 1
	header += struct.pack("I", brlyt_version)
	boot_region_addr = lk_start_addr
	header += struct.pack("I", boot_region_addr)
	main_region_addr = boot_region_addr + (4 * page_per_block)
	header += struct.pack("I", main_region_addr)
	bl_desc_magic = 0x42424242
	header += struct.pack("I", bl_desc_magic)
	bl_desc_dev = 0x2
	header += struct.pack("H", bl_desc_dev)
	bl_desc_type = 0x1
	header += struct.pack("H", bl_desc_type)
	header += struct.pack("I", boot_region_addr)
	header += struct.pack("I", main_region_addr)
	bl_desc_attribute = 1
	header += struct.pack("I", bl_desc_attribute)
	# pad 7 descripters #
	header += '\0' * 7 * 20
	# pad page size align dummy data #
	header += '\0' * (page_size - 180)

	# step3: pad dummy data till GPT LBA #
	# let us have an agreement that GPT alwasy be at LBA 128 for SLC NAND #
	# should be the same LBA in lk and kernel to find GPT #
	gpt_lba = 128
	header += '\0' * page_size * (gpt_lba - 2)

	with open(out_image, "wb") as f:
		f.write(header)

	#return (vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, pptableen, page_per_block)
	return (vendor, 8, 8, ecc_strength, spare_size, page_size, 0, page_per_block)

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('nand_name', help = 'nand device name')
	parser.add_argument('lk_start_addr', help = 'lk start address in page unit')
	parser.add_argument('out_image', help = 'the output header file')
	args = parser.parse_args()
	lk_start_addr = int(args.lk_start_addr)

	# generate nand device header #
	gen_header(args.nand_name, lk_start_addr, args.out_image)

if __name__ == "__main__":
	sys.exit(main(sys.argv))
