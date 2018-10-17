#!/usr/bin/env python
# -*- coding: utf-8 -*
try:
    import pyserial as serial
    import pyserial.tools.list_ports as ser_tools
except ImportError:
    import serial
    import serial.tools.list_ports as ser_tools
import sys, time, struct, logging
from optparse import OptionParser
try:
    # python2
    import ConfigParser as configparser
except ImportError:
    # python3
    import configparser

class Fbtool:
    def __init__(self, config_file, meid=False):
        self.TGT_CFG_SBC_EN = 0x00000001
        self.TGT_CFG_SLA_EN = 0x00000002
        self.TGT_CFG_DAA_EN = 0x00000004
        self.E_ERROR = 0x1000
        self.__ser = None
        self.__connect_type = 'UNKNOWN'
        self.__meid = meid
        cfg = configparser.ConfigParser()
        cfg.read(config_file)
        self.__da1_path = cfg.get('DA1_Path_Addr', 'da1_path')
        self.__da1_addr = int(cfg.get('DA1_Load_Addr', 'da1_addr'), 16)
        self.__da1_jump_64 = int(cfg.get('DA1_JUMP_64', 'da1_jump_64'), 16)
        self.__da2_path = cfg.get('DA2_Path_Addr', 'da2_path')
        self.__da2_addr = int(cfg.get('DA2_Wrapper_Addr', 'da2_addr'), 16)
        self.__auth_path = cfg.get('Auth', 'auth_path')
        self.__cert_path = cfg.get('Cert', 'cert_path')
        logging.debug('da1_path: %s' %(self.__da1_path))
        logging.debug('da1_addr: 0x%x' %(self.__da1_addr))
        logging.debug('da1_jump_64: 0x%x' %(self.__da1_jump_64))
        logging.debug('da2_path: %s' %(self.__da2_path))
        logging.debug('ad2_addr: 0x%x' %(self.__da2_addr))
        logging.debug('auth_path: %s' %(self.__auth_path))
        logging.debug('cert_path: %s' %(self.__cert_path))

    def __del__(self):
        # compatible with pySerial 2.6.
        # isOpen() is deprecated since version 3.0, 3.0 uses is_open
        if self.__ser and self.__ser.isOpen():
            self.__ser.close()

    def __match_usb_br(self, vid, pid):
        if vid == 0x0e8d and pid == 0x0003:
            self.__connect_type = 'BROM'
            return True
        return False

    def __match_usb_pl(self, vid, pid):
        if ((vid == 0x0e8d and pid == 0x2000) or (vid == 0x0e8d and pid == 0x3000)):
            self.__connect_type = 'PRELOADER'
            return True
        return False

    def __match_usb_auto(self, vid, pid):
        if self.__match_usb_br(vid, pid):
            return True
        if self.__match_usb_pl(vid, pid):
            return True
        return False

    def __open_usb_device(self, match_usb_func, comport, vid, pid):
        if match_usb_func(vid, pid):
            time.sleep(0.1)
            try:
                self.__ser = serial.Serial(comport, 115200)
            except serial.SerialException as e:
                logging.debug('%s, retry...' %(str(e)))
            else:
                logging.info('Got %s' %(comport))
                return True
        return False

    def __find_usb_device(self, match_usb_func):
        while True:
            ports = ser_tools.comports()
            if serial.VERSION < '3':
                ports_list = list(ports)
                for port in ports_list:
                    if 'USB' in port[2]:
                        if sys.platform.startswith('win'):
                            idx = port[2].index('VID_')+4
                            vid = int(port[2][idx : idx + 4], 16)
                            idx = port[2].index('PID_')+4
                            pid = int(port[2][idx : idx + 4], 16)
                        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
                            idx = port[2].index('VID:PID=') + 8
                            vid = int(port[2][idx : idx + 4], 16)
                            pid = int(port[2][idx + 5 : idx + 13], 16)
                        elif sys.platform.startswith('darwin'):
                            raise EnvironmentError('Unsupport macOS')
                        else:
                            raise EnvironmentError('Unsupported platform')
                        if self.__open_usb_device(match_usb_func, port[0], vid, pid):
                            return
            else:
                for port in ports:
                    if self.__open_usb_device(match_usb_func, port.device, port.vid, port.pid):
                        return

    def __read8(self):
        return struct.unpack('!B', self.__ser.read())[0]

    def __read16(self):
        return struct.unpack('!H', self.__ser.read(2))[0]

    def __read32(self):
        return struct.unpack('!I', self.__ser.read(4))[0]

    def __write8(self, data, echo):
        self.__ser.write(struct.pack('!B', data))
        if echo:
            return struct.unpack('!B', self.__ser.read())[0] == data
        return True

    def __write16(self, data, echo):
        self.__ser.write(struct.pack('!H', data))
        if echo:
            return struct.unpack('!H', self.__ser.read(2))[0] == data
        return True

    def __write32(self, data, echo):
        self.__ser.write(struct.pack('!I', data))
        if echo:
            return struct.unpack('!I', self.__ser.read(4))[0] == data
        return True

    def __start_cmd(self):
        cmd = (0xa0, 0x0a, 0x50, 0x05)
        echo_cmd = (0x5f, 0xf5, 0xaf, 0xfa)

        i = 0
        while (i < len(cmd)):
            self.__write8(cmd[i], False)
            if self.__read8() != echo_cmd[i]:
                i = 0
                self.__ser.flushInput()
            else:
                i = i + 1
        time.sleep(0.1)
        # self.__ser.flush()
        self.__ser.flushInput()
        self.__ser.flushOutput()
        if self.__connect_type == 'BROM':
            logging.info('Connect brom')
        elif self.__connect_type == 'PRELOADER':
            logging.info('Connect preloader')

    def __load_binary(self, path):
        logging.info("Loading file: %s" %path)
        with open(path, 'rb') as f:
            return f.read()

    def __checksum(self, data, length):
        checksum = 0
        for i in range(0, length, 2):
            checksum ^= struct.unpack('<H', data[i:i+2])[0]
        checksum &= 0xFFFF
        return checksum

    def __get_target_config(self):
        if not self.__write8(0xd8, True):
            return -1, None
        cfg = self.__read32()
        status = self.__read16()

        if status >= self.E_ERROR:
            return status, None
        return 0, cfg

    def __send_auth(self, cfg):
        if self.TGT_CFG_DAA_EN & cfg == 0:
            return 0
        else:
            if self.TGT_CFG_SBC_EN & cfg == 0:
                logging.error('daa=1, sbc=0')
                return -2
            if self.__auth_path == '':
                logging.error('no auth file')
                return -3
            auth = self.__load_binary(self.__auth_path)
            auth_len = len(auth)
            logging.debug("auth file size: 0x%x" %(auth_len))

            if not self.__write8(0xe2, True):
                return -4
            if not self.__write32(len(auth), True):
                return -5

            status = self.__read16()
            if status >= self.E_ERROR:
                return status
            self.__ser.write(auth)

            # compare checksum
            if self.__read16() != self.__checksum(auth, auth_len):
                return -6

            status = self.__read16()
            if status >= self.E_ERROR:
                return status
            return 0

    def __strip_pl_hdr(self, pl, pl_len):
        # EMMC_HEADER_V1
        identifier, ver, dev_rw_unit = struct.unpack('12s2I', pl[:20])
        # GFH_FILE_INFO_V1
        gfh = pl[:56]
        gfh_offset = 0

        if identifier.strip(b'\0') == b'EMMC_BOOT' and ver == 1:
            logging.debug('emmc_hdr: identifier:%s, ver:0x%08x, dev_rw_unit:0x%08x' %(identifier, ver, dev_rw_unit))
            # BR_Layout_v1 size: 40
            if dev_rw_unit + 40 > pl_len:
                logging.error('EMMC HDR error. dev_rw_unit=0x%x, brlyt_size=0x%x, pl_len=0x%x'
                    %(dev_rw_unit, brlyt_size, pl_len))
                return False, None, None

            brlyt_identifier, brlyt_ver = struct.unpack('8sI', pl[dev_rw_unit:dev_rw_unit + 12])
            logging.debug('brlyt_identifier: %s, brlyt_ver=0x%x' %(brlyt_identifier, brlyt_ver))
            if brlyt_identifier.strip(b'\0') != b'BRLYT' or brlyt_ver != 1:
                logging.error('BRLYT error. ver=0x%x, identifier=%s' %(brlyt_ver, brlyt_identifier))
                return False, None, None
            # BL_Descriptor
            bl_begin_dev_addr = struct.unpack('I', pl[dev_rw_unit + 28 : dev_rw_unit + 32])[0]
            if bl_begin_dev_addr + 56 > pl_len:
                logging.error('BRLYT error. bl_begin_dev_addr=0x%x' %bl_begin_dev_addr)
                return False, None, None
            # GFH_FILE_INFO_v1
            gfh = pl[bl_begin_dev_addr:bl_begin_dev_addr + 56]
            gfh_offset = bl_begin_dev_addr

        gfh_struct =struct.unpack('I2H12sIH2B7I', gfh)

        gfh_magic_ver = gfh_struct[0]
        gfh_type = gfh_struct[2]
        gfh_identifier = gfh_struct[3]
        gfh_file_len = gfh_struct[9]
        gfh_jump_offset = gfh_struct[13]
        gfh_sig_len = gfh_struct[12]
        if (gfh_magic_ver & 0x00FFFFFF) == 0x004D4D4D and gfh_type == 0 and gfh_identifier.strip(b'\0') == b'FILE_INFO':
            if gfh_file_len < gfh_jump_offset + gfh_sig_len:
                logging.error('GFH error. pl_len=0x%x, file_len=0x%x, jump_offset=0x%x, sig_len=0x%x'
                                %(pl_len, gfh_file_len, gfh_jump_offset, gfh_sig_len))
                return False, None, None
            logging.debug('gfh: magic_ver: 0x%08x' %gfh_struct[0])
            logging.debug('gfh: size: 0x%04x' %gfh_struct[1])
            logging.debug('gfh: type: 0x%04x' %gfh_struct[2])
            logging.debug('gfh: identifier: %s' %gfh_struct[3])
            logging.debug('gfh: file_ver: 0x%08x' %gfh_struct[4])
            logging.debug('gfh: file_type: 0x%04x' %gfh_struct[5])
            logging.debug('gfh: flash_dev: 0x%02x' %gfh_struct[6])
            logging.debug('gfh: sig_type: 0x%02x' %gfh_struct[7])
            logging.debug('gfh: load_addr: 0x%08x' %gfh_struct[8])
            logging.debug('gfh: file_len: 0x%08x' %gfh_struct[9])
            logging.debug('gfh: max_size: 0x%08x' %gfh_struct[10])
            logging.debug('gfh: content_offset: 0x%08x' %gfh_struct[11])
            logging.debug('gfh: sig_len: 0x%08x' %gfh_struct[12])
            logging.debug('gfh: jump_offset: 0x%08x' %gfh_struct[13])
            logging.debug('gfh: attr: 0x%08x' %gfh_struct[14])
            strip_pl = pl[gfh_offset + gfh_jump_offset:]
            strip_pl_len = gfh_file_len - gfh_jump_offset - gfh_sig_len
            return (True, strip_pl, strip_pl_len)
        else:
            return (True, pl, pl_len)

    def __send_da(self, addr, da, da_len, sig, sig_len):
        if not self.__write8(0xd7, True):
            return -1
        if not self.__write32(addr, True):
            return -2
        logging.debug('len: 0x%x' %(da_len + sig_len))
        if not self.__write32(da_len + sig_len, True):
            return -3
        if not self.__write32(sig_len, True):
            return -4
        status = self.__read16()
        if status >= self.E_ERROR:
            return status

        if da_len > 0:
            self.__ser.write(da)
        if sig_len > 0:
            self.__ser.write(sig)

        checksum = self.__checksum(da, da_len) ^ self.__checksum(sig, sig_len)
        data = self.__read16()
        logging.debug('checksum: 0x%x - 0x%x' %(checksum, data))
        if data != checksum:
            return -5
        status = self.__read16()
        if status >= self.E_ERROR:
            return status

        return 0

    def __jump_da(self, addr):
        if not self.__write8(0xd5, True):
            return -1
        if not self.__write32(addr, True):
            return -2
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        return 0

    def __jump_da_ex(self, addr):
        if not self.__write8(0xde, True):
            return -1
        if not self.__write32(addr, True):
            return -2
        if not self.__write8(0x1, True):
            return -3
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        if not self.__write8(0x64, True):
            return -4
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        return 0

    def __get_meid(self):
        if not self.__write8(0xe1, True):
            return -1
        len = self.__read32()
        logging.debug('meid len: 0x%x' %len)
        data = struct.unpack('!'+str(len)+'B', self.__ser.read(len))
        meid_str = lambda s: ''.join(map(lambda c: '%02x' %c, s))
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        logging.info(meid_str(data));
        return 0

    def __send_cert(self, data, len):
        if not self.__write8(0xe0, True):
            return -1
        if not self.__write32(len, True):
            return -2
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        self.__ser.write(data)
        checksum = self.__checksum(data, len)
        data = self.__read16()
        if checksum != data:
            logging.error("checksum: 0x%x - 0x%x" %(checksum, data))
            return -3
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        return 0

    def __reboot_platform(self):
        if not self.__write8(0xd4, True):
            return -1
        if not self.__write32(0x10007000, True):
            return -2
        if not self.__write32(0x1, True):
            return -3
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        if not self.__write32(0x22000004, True):
            return -4
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        if not self.__write8(0xd4, True):
            return -5
        if not self.__write32(0x10007014, True):
            return -6
        if not self.__write32(0x1, True):
            return -7
        status = self.__read16()
        if status >= self.E_ERROR:
            return status
        if not self.__write32(0x1209, True):
            return -8
        return 0

    def start(self):
        self.__find_usb_device(self.__match_usb_auto)
        self.__start_cmd()

        # get meid
        if self.__meid:
            status = self.__get_meid()
            if status != 0:
                logging.error('get meid (%d)' %status)
                return -1
            return 0

        # send cert
        if self.__cert_path != '':
            cert = self.__load_binary(self.__cert_path)
            cert_len = len(cert)
            logging.debug('cert_len: 0x%x' %cert_len)
            status = self.__send_cert(cert, cert_len)
            if status != 0:
                logging.error('send cert (%d)' %status)
                return -1
            logging.info('Reboot...')
            status = self.__reboot_platform()
            if status != 0:
                logging.error('reboot platform (%d)' %status)
                return -1
            return 0

        if self.__connect_type == 'BROM':
            status, cfg = self.__get_target_config()
            if status != 0:
                logging.error('get target config (%s)' %status)
                return -1
            logging.debug('cfg=0x%x' %cfg)

            status = self.__send_auth(cfg)
            if status != 0:
                logging.error('send auth (%d)' %status)
                return -1

            da1 = self.__load_binary(self.__da1_path)
            da1_len = len(da1)
            logging.debug('da1 length: 0x%x' %da1_len)
            status, strip_pl, strip_pl_len = self.__strip_pl_hdr(da1, da1_len)
            if not status:
                logging.error('strip pl hdr')
                return -1

            sig_da1 = None
            sig_da1_len = 0
            if self.TGT_CFG_DAA_EN & cfg:
                sig_da1 = self.__load_binary(self.__da1_path + '.sign')
                sig_da1_len = len(sig_da1)

            logging.debug('strip_pl_len: 0x%x' %strip_pl_len)
            logging.info('Send %s' %self.__da1_path)
            status = self.__send_da(self.__da1_addr, strip_pl, strip_pl_len, sig_da1, sig_da1_len)
            if status != 0:
                logging.error('send da1 (%d)' %status)
                return -1
            logging.info('Jump da')
            if self.__da1_jump_64 == 0:
                status = self.__jump_da(self.__da1_addr)
            else:
                status = self.__jump_da_ex(self.__da1_addr)
            if status != 0:
                logging.error('jump da1 (%d)' %status)
                return -1

            self.__ser.close()
            if self.__da2_path == '':
                return 0

            # handshake to preloader
            self.__find_usb_device(self.__match_usb_pl)
            self.__start_cmd()

        # load da2 (lk)
        da2 = self.__load_binary(self.__da2_path)
        da2_len = len(da2)
        sig_da2 = self.__load_binary(self.__da2_path + '.sign')
        sig_da2_len = len(sig_da2)
        logging.info('Send %s' %self.__da2_path)
        status = self.__send_da(self.__da2_addr, da2, da2_len, sig_da2, sig_da2_len)
        if status != 0:
            logging.error('send da2 (%d)' %status)
            return -1
        logging.info('Jump da2')
        status = self.__jump_da(self.__da2_addr)
        if status != 0:
            logging.error('jump da2 (%d)' %status)
            return -1

        self.__ser.close()
        return 0


if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-f', '--file', dest='configfile', help='read config file',
                    metavar='FILE', default='dl_addr.ini')
    parser.add_option('-d', '--debug', action='store_true', dest='debuglog',
                    default=False, help='enable debug log')
    parser.add_option('-m', '--meid', action='store_true', dest='meid',
                    default=False, help='get meid')
    options, args = parser.parse_args()
    config_file = options.configfile
    meid = options.meid
    debug = options.debuglog
    if debug:
        logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')
    else:
        logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

    logging.info('pySerial version: (%s)' %serial.VERSION)
    if serial.VERSION < '2.6':
        logging.error('pySerial version(%s) is lower than 2.6, please upgrade!' %serial.VERSION)
    logging.info('Use config file: %s' %(config_file))
    logging.info('Waiting to connect platform...')
    fbtool = Fbtool(config_file, meid)
    fbtool.start()
