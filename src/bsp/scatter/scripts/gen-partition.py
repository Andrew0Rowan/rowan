#!/usr/bin/env python

from __future__ import print_function
import binascii
import json
import os
import struct
import sys
import uuid
import xml.dom.minidom

NumberOfPartitionEntries = 128
SizeOfPartitionEntry = 128

def write(path, data):
	with open(path, "wb+") as f:
		f.write(data)

def crc32(data):
        return binascii.crc32(data) & 0xffffffff

def padding(data, size):
	return data + '\0' * (size - len(data))

def gen_gpt(partition):
	pmbr = ('\0' * 446 +
		"\x00\x00\x02\x00\xee\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff" +
		'\0' * 48 + "\x55\xaa")
	entries = ''
	for node in partition.childNodes:
		if node.nodeName != "entry":
			continue
		type = uuid.UUID(node.getAttribute("type"))
		uniq = node.getAttribute("uuid")
		uniq = uniq and uuid.UUID(uniq) or uuid.uuid4()
		start = eval(node.getAttribute("start"))
		end = eval(node.getAttribute("end"))
		attr = node.getAttribute("attributes")
		attr = attr and eval(attr) or 0
		name = node.getAttribute("name")
		entries += struct.pack("<16s16sQQQ72s",
			type.bytes_le,
			uniq.bytes_le,
			start, end, attr,
			name.encode("utf-16le"))
	entries = padding(entries, NumberOfPartitionEntries * SizeOfPartitionEntry)
	lba = eval(partition.getAttribute("lba"))
	lbs = partition.getAttribute("lbs")
	lbs = lbs and eval(lbs) or 512
	FirstUsableLBA = 2 + (NumberOfPartitionEntries * SizeOfPartitionEntry / lbs)
	uniq = partition.getAttribute("uuid")
	uniq = uniq and uuid.UUID(uniq) or uuid.uuid4()
	pmbr = padding(pmbr, lbs)
	header = struct.pack("<8sIIIIQQQQ16sQIII",
		"EFI PART", 0x00010000, 92, 0, 0,
		1, # MyLBA
		1, #lba - 1, # AlternateLBA
		FirstUsableLBA,
		lba - FirstUsableLBA, # LastUsableLBA
		uniq.bytes_le,
		2, NumberOfPartitionEntries, SizeOfPartitionEntry, crc32(entries))
	header = padding(header[:16] +
		struct.pack('I', crc32(header)) +
		header[20:], lbs)
	return pmbr + header + entries

def write_scatter_partition(f, entry):
	if entry.get("file_name", "NONE") != "NONE":
		entry.setdefault("is_download", True)
		entry.setdefault("operation_type", "UPDATE")
		entry.setdefault("type", "NORMAL_ROM")
	f.write(
"""- partition_index: %s
  partition_name: %s
  file_name: %s
  is_download: %s
  type: %s
  linear_start_addr: %s
  physical_start_addr: %s
  partition_size: %s
  region: %s
  storage: %s
  boundary_check: %s
  is_reserved: %s
  operation_type: %s
  d_type: FALSE
  reserve: %s

""" % (entry["partition_index"], entry["partition_name"], entry.get("file_name", "NONE"),
entry.get("is_download", False) and "true" or "false", entry.get("type", "NONE"), hex(entry["linear_start_addr"]),
hex(entry["physical_start_addr"]), hex(entry["partition_size"]), entry.get("region", "EMMC_USER"),
entry.get("storage", "HW_STORAGE_EMMC"), entry.get("boundary_check", True) and "true" or "false",
entry.get("is_reserved", False) and "true" or "false", entry.get("operation_type", "PROTECTED"),
hex(entry.get("reserve", 0))))

def write_scatter(f, partition, d):
	f.write(
"""############################################################################################################
#
#  General Setting
#
############################################################################################################
- general: MTK_PLATFORM_CFG
  info:
    - config_version: %s
      platform: %s
      project: %s
      storage: %s
      boot_channel: %s
      block_size: %s
      skip_pmt_operate: %s
############################################################################################################
#
#  Layout Setting
#
############################################################################################################
""" % (d["MTK_PLATFORM_CFG"]["config_version"], d["MTK_PLATFORM_CFG"]["platform"], d["MTK_PLATFORM_CFG"]["project"],
       d["MTK_PLATFORM_CFG"]["storage"], d["MTK_PLATFORM_CFG"]["boot_channel"], d["MTK_PLATFORM_CFG"]["block_size"],
       d["MTK_PLATFORM_CFG"].get("skip_pmt_operate", True) and "true" or "false"))
	d["PRELOADER"]["partition_index"] = "SYS0"
	d["MBR"]["partition_index"] = "SYS1"
	write_scatter_partition(f, d["PRELOADER"])
	write_scatter_partition(f, d["MBR"])
	i = 2
	for node in partition.childNodes:
		if node.nodeName != "entry":
			continue
		start = eval(node.getAttribute("start"))
		end = eval(node.getAttribute("end"))
		name = node.getAttribute("name")
		if name not in d:
			continue
		entry = d[name]
		entry["partition_name"] = name
		entry["partition_index"] = "SYS%d" % i
		i += 1
		entry["linear_start_addr"] = start * 512  
		entry["physical_start_addr"] = start * 512
                if end != start:
			entry["partition_size"] = (end + 1 - start) * 512
		else:
			entry["partition_size"] = 0
		write_scatter_partition(f, entry)
	if (d["MTK_PLATFORM_CFG"].get("skip_pmt_operate", True) and "true" or "false") == "false":
		d["sgpt"]["partition_index"] = "SYS%d" % i
		write_scatter_partition(f, d["sgpt"])


def sanity_check(path, partition):
	err = 0
	lba = eval(partition.getAttribute("lba"))
	lbs = partition.getAttribute("lbs")
	lbs = lbs and eval(lbs) or 512
	FirstUsableLBA = 2 + (NumberOfPartitionEntries * SizeOfPartitionEntry / lbs)
	usable = (FirstUsableLBA, lba - FirstUsableLBA)
	used = {}
	for node in partition.childNodes:
		if node.nodeName != "entry":
			continue
		name = node.getAttribute("name")
		start = eval(node.getAttribute("start"))
		end = eval(node.getAttribute("end"))
		if start > end:
			print("%s: error: partition '%s': start lba (%d) > end lba (%d)" %
				(path, name, start, end), file = sys.stderr)
			err += 1
		if start < usable[0] or end > usable[1]:
			print("%s: error: partition '%s': (%d...%d) out of usable range (%d...%d)" %
				(path, name, start, end, usable[0], usable[1]), file = sys.stderr)
			err += 1
		for i in used:
			if (used[i][0] <= start and start <= used[i][1] or
				used[i][0] <= end and end <= used[i][1]):
				print("%s: error: partition '%s': (%d...%d) overlapped with partition '%s' (%d...%d)" %
					(path, name, start, end, i, used[i][0], used[i][1]), file = sys.stderr)
				err += 1
		used[name] = (start, end)
	return err

def main(argv):
	if len(argv) == 5: # normal argument list len(argv) = 5 #
		print ("len:%d .py=%s .xml=%s .json=%s MBR=%s .txt=%s" %
			(len(argv), argv[0], argv[1], argv[2], argv[3], argv[4]))
	if len(argv) <= 3: # len(argv) = 4 for mt2701 spi nor boot (scatter.txt isn't needed) #
		print("Usage: len:%d,%s partition_*.xml scatter_*.json [MBR] [scatter.txt] " % (len(argv), argv[0]))
		exit(1)
	root = xml.dom.minidom.parse(argv[1])
	for partition in root.childNodes:
		if partition.nodeName == "partition":
			break
	else:
		raise Exception("partition not found")
	if sanity_check(argv[1], partition):
		return 1
	write(argv[3], gen_gpt(partition))
	if len(argv) == 5:
		with open(os.path.join(os.path.dirname(__file__), argv[2]), "r") as f:
			d = json.load(f)
		with open(argv[4], "w") as f:
			write_scatter(f, partition, d)
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
