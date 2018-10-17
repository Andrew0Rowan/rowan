#!/usr/bin/env python

#to do:
#extract pmbr + gpt header + gpt entry
#add txt file output
#add picture file output

import sys
import os

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument('in_image', help = 'input image with GPT')
	args = parser.parse_args()

if __name__ == "__main__":
	sys.exit(main(sys.argv))
