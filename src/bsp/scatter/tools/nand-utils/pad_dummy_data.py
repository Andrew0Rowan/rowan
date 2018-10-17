#!/usr/bin/env python

import argparse
import sys
import os

def padding(data, size):
	return data + '\xff' * (size - len(data))

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('in_image', help = 'raw input image which has GPT')
	parser.add_argument('part_size', help = 'partition size')
	args = parser.parse_args()
	part_size = int(args.part_size, 16)
	file_len = os.path.getsize(args.in_image)

	with open(args.in_image, "rb+") as f:
		f.read()
		f.write('\xff' * (part_size - file_len))

if __name__ == "__main__":
	sys.exit(main(sys.argv))
