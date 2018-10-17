#!/usr/bin/env python

import ecc_bch
import sys
import os
import struct
import argparse
import pp_tbl

def gen_header(nand_name):
	""" device info:
	name vendor totalsize(MB) pagesize(B) sparesize(B) pageperblock cycle eccstrength
	interface acccon acccon1 dqsdlymux dqsdlyctrl nfidlyctrl pptableenable lbasize
	fastclock usedma
	"""
	with open(os.getcwd() + "/tools/nand-utils/nand_device_list.txt", "r") as f:
		found = 0
		for l in f:
			s = l.split()
			if s[0] == nand_name:
				found = 1
				vendor = s[1]
				total_size = int(s[2])
				page_size = int(s[3])
				#page_shift = len(bin(page_size).replace('0b','')) - 1
				page_shift = 16 if page_size > 512 else 8
				spare_size = int(s[4])
				page_per_block = int(s[5])
				address_cycle = int(s[6])
				ecc_strength = int(s[7])
				interface = int(s[8])
				acccon = eval(s[9])
				acccon1 = eval(s[10])
				dqsdlymux = int(s[11])
				dqsdlyctrl = eval(s[12])
				nfidlyctrl = eval(s[13])
				pptableen = int(s[14])
				lba_size = int(s[15])
				fast_clk = int(s[16])
				use_dma = int(s[17])
				fdm_size = int(s[18])
				fdmecc_size = int(s[19])
				strobesel = int(s[20])

				# meaningless, just set pptableen value as BROM test code #
				if not pptableen:
					pptableen = 0xffff

				break

		if not found:
			raise KeyError("not support " + nand_name)

		#print s[0]
		#print "vendor %s, total size %d(MB) page size %d, page shift %d, spare size %d, page per block %d" \
		#	% (vendor, total_size, page_size, page_shift, spare_size, page_per_block)
		#print "address cycle %d, ecc strength %d, interface %d, acccon 0x%x, acccon1 0x%x" \
		#	% (address_cycle, ecc_strength, interface, acccon, acccon1)
		#print "dqsdlymux %d, dqsdlyctrl 0x%x, nfidlyctrl 0x%x, pptable enable %d, lba size %d" \
		#	% (dqsdlymux, dqsdlyctrl, nfidlyctrl, pptableen, lba_size)
		#print "fast clock enable %d, use dma %d, fdm size %d, fdmecc size %d" \
		#	% (fast_clk, use_dma, fdm_size, fdmecc_size)
		#raise KeyError("not support " + nand_name)

	# nand header is total 440B include ecc parity 140B #
	header_size = 300
	pptbl_size = 128

	header = "NANDCONFIG!" + '\0'
	header += struct.pack("I", total_size)
	header += struct.pack("H", page_size)
	header += struct.pack("H", address_cycle)
	header += struct.pack("H", spare_size)
	header += struct.pack("H", interface)
	header += struct.pack("I", ecc_strength)
	header += struct.pack("H", page_per_block)
	header += struct.pack("H", page_shift)
	header += struct.pack("H", pptableen)
	header += struct.pack("H", dqsdlymux)
	header += struct.pack("I", dqsdlyctrl)
	header += struct.pack("I", acccon)
	header += struct.pack("I", acccon1)
	header += struct.pack("I", nfidlyctrl)
	header += struct.pack("H", lba_size)
	header += struct.pack("H", fast_clk)
	header += struct.pack("H", use_dma)
	header += struct.pack("H", strobesel)
	# pad dummy byte #
	header += '\x5a' * (header_size - len(header) - pptbl_size)
	if pptableen == 1:
		# add pptable #
		if vendor == 'TSB':
			header += str(bytearray(pp_tbl.TSB_PP))
		else:
			raise KeyError(" no %s pptable " % vendor)
	else:
		header += '\xff' * pptbl_size

	# calculate header ecc parity #
	header_size = 300
	ecc_level = 80
	ecc = ecc_bch.bch_enc_14(header, header_size, ecc_level)
	header += ecc

	header_file = nand_name + '_header.bin'
	with open(header_file, "wb") as f:
		f.write(header)

	return (vendor, fdm_size, fdmecc_size, ecc_strength, spare_size, page_size, pptableen, page_per_block)

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('nand_name', help = 'nand device name')
	parser.add_argument('gpt_image', help = 'pmbr + gpt entry image ')
	args = parser.parse_args()

	# generate nand device header #
	gen_header(args.nand_name)

	# pad nand header #
	with open(args.nand_name + '_header.bin', "rb") as f_h:
		h_buf = f_h.read()
	with open(args.gpt_image, "rb+") as f_g:
		f_g.seek(440, 0)
		h_buf = h_buf + f_g.read()
		f_g.seek(0, 0)
		f_g.write(h_buf)

	os.remove(args.nand_name + '_header.bin')

if __name__ == "__main__":
	sys.exit(main(sys.argv))
