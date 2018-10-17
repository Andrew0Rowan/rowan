import sys
import os
import struct
script_folder, script_name = os.path.split(os.path.realpath(__file__))
sys.path.append(os.path.join(script_folder, "lib"))
import gfh 
import cert

def get_file_sizeb(file_path):
	if not os.path.isfile(file_path):
		return 0
	file_handle = open(file_path, "rb")
	file_handle.seek(0, 2)
	file_size = file_handle.tell()
	file_handle.close()
	return file_size

def concatb(file1_path, file2_path):
	file1_size = get_file_sizeb(file1_path)
	file2_size = get_file_sizeb(file2_path)
	file1 = open(file1_path, "ab+")
	file2 = open(file2_path, "rb")
	file1.write(file2.read(file2_size))
	file2.close()
	file1.close()

class tool_auth:
	def __init__(self, out_path, tool_auth_path):
		self.m_out_path = out_path
		if not os.path.exists(self.m_out_path):
			os.makedirs(self.m_out_path)
		self.m_tool_auth_path = tool_auth_path
		self.m_gfh = gfh.image_gfh()
		self.m_sig_handler = None
	def create_gfh(self, gfh_config):
		self.m_gfh.load_ini(gfh_config)
		return
	def sign(self, key_ini_path):
		#tool auth contains only gfh and signature, no extra content
		self.m_gfh.finalize(0, key_ini_path)
		#write tbs_tool_auth
		tbs_toolauth_file_path = os.path.join(self.m_out_path, "tbs_toolauth.bin")
		tbs_tool_auth_file = open(tbs_toolauth_file_path, "wb")
		tbs_tool_auth_file.write(self.m_gfh.pack())
		tbs_tool_auth_file.close()
		print "===tool_auth signing==="
		if self.m_gfh.get_sig_type() == "SINGLE":
			self.m_sig_handler = cert.sig_single(self.m_gfh.get_pad_type())
			self.m_sig_handler.set_out_path(self.m_out_path)
			self.m_sig_handler.create(key_ini_path, tbs_toolauth_file_path)
			self.m_sig_handler.sign()
			sig_name = "toolauth.sig"
			sig_file_path = os.path.join(self.m_out_path, sig_name)
			self.m_sig_handler.output(self.m_out_path, sig_name)
			#create final toolauth file
			if os.path.isfile(self.m_tool_auth_path):
				os.remove(self.m_tool_auth_path)
			concatb(self.m_tool_auth_path, tbs_toolauth_file_path)
			concatb(self.m_tool_auth_path, sig_file_path)
		else:
			print "unknown signature type"
		#clean up
		os.remove(tbs_toolauth_file_path)
		os.remove(sig_file_path)
		return

def main():
	#parameter parsing
	idx = 1
	key_ini_path = ""
	gfh_config_ini_path = ""
	while idx < len(sys.argv):
		if sys.argv[idx][0] == '-':
			if sys.argv[idx][1] == 'i':
				print "key: " + sys.argv[idx + 1]
				key_ini_path = sys.argv[idx + 1]
				idx += 2
			elif sys.argv[idx][1] == 'g':
				print "gfh: " + sys.argv[idx + 1]
				gfh_config_ini_path = sys.argv[idx + 1]
				idx += 2
			else:
				print "unknown input"
				idx += 2
		else:
			tool_auth_path = sys.argv[idx]
			print "tool_auth_path: " + tool_auth_path
			idx += 1

	if not key_ini_path:
		print "key path is not given!"
		return -1
	if not gfh_config_ini_path:
		print "gfh config path is not given!"
		return -1
	if not tool_auth_path:
		print "tool_auth path is not given!"
		return -1

	out_path = os.path.dirname(os.path.abspath(tool_auth_path))

	tool_auth_obj = tool_auth(out_path, tool_auth_path)
	tool_auth_obj.create_gfh(gfh_config_ini_path)
	tool_auth_obj.sign(key_ini_path)

	return

if __name__ == '__main__':
	main()

