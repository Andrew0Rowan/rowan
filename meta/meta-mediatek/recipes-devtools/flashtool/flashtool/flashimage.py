#!/usr/bin/python

import os
import os.path
import sys
import platform
import argparse
import subprocess
import time
import re

################################################################################
# check python version
if sys.version_info[:2] == (2,7):
    pass
else:
    print 'Please install Python 2.7.x to run this script'
    exit(1)
################################################################################

verbose = False
skipFlash = False

fbtool = 'fbtool.py'
fastboot = 'fastboot'

system = platform.system()
machine = platform.machine()
product_out = os.path.abspath('.')

if 'Windows' in system or 'CYGWIN' in system:
    fastboot += '.exe'
elif 'Linux' in system:
    if 'x86_64' in machine:
        fastboot += 'linux-x86_64'
    elif 'arm' in machine or 'aarch64' in machine:
        fastboot += 'linux-arm'
elif 'Darwin' in system:
    fastboot += '-darwin'

# Generate image list from procedure list
def getImageList(procs):
    imgs = []
    try:
        for p in procs:
            if p[0] == 'fastboot' and p[1] == 'flash' and p[3] not in imgs:
                imgs.append(p[3])
    except Exception, e:
        print e
    return imgs

def call(cmd):
    '''cmd: the command list for subprocess.call'''
    if verbose:
        print 'call:', ' '.join(cmd)
    return subprocess.call(cmd)

def check_output(cmd):
    '''cmd: the command list for subprocess.check_output'''
    if verbose:
        print 'check_output:', ' '.join(cmd)
    return subprocess.check_output(cmd, stderr = subprocess.STDOUT, shell=True)

def checkImage(filename, needreboot = True, _verbose = False):
    if _verbose:
        print (filename).rjust(38),':',
    filepath = os.path.join(product_out, filename)
    if os.path.exists(filepath):
        if _verbose:
            print 'PASS'
        return filepath
    if needreboot:
        print 'FAIL'
        call([fastboot, 'reboot'])
        exit(1)
    return None

# return 0 if success
def cmdRun(cmd, dryrun = False):
    ret = 1
    raw_cmd = []
    if cmd[0] == 'daWait':
        daWait()
        print ''
        return 0
    elif cmd[0] == 'fbWait':
        fbWait()
        print ''
        return 0
    elif cmd[0] == 'fastboot': # processing fastboot commands
        if cmd[1] == 'flash':
            # check if image path exits and valid
            filepath = checkImage(cmd[3], False, False)
            if filepath != None:
                if 'CYGWIN' in system:
                    p = subprocess.check_output('cygpath --absolute --mixed %s' % filepath, shell=True)
                    if p:
                        filepath = p.strip()
                raw_cmd += [cmd[0], cmd[1], cmd[2], filepath]
        else:
            raw_cmd += cmd
    else:
        print 'FAIL: Unknown command!'
        return -1
    if dryrun:
        print ' '.join(raw_cmd)
        ret = 0
    else:
        ret = call(raw_cmd)
        if ret == 0 and cmd[0] == 'fastboot' and cmd[1] == 'reboot-bootloader' and not dryrun:
            fbWait()
    return ret

def daWait(secs = 60):
    print 'Waiting for DA mode'
    ret = None
    for i in range(secs):
        print '.',
        sys.stdout.flush()
        time.sleep(1)
        ret = check_output('python %s ' % fbtool)
        if ret != None and len(ret) != 0:
            print ''
            print 'datool - device detected: '
            break
    if ret == None:
        print 'No device detected. Please ensure that datool is running'
        exit(1)

def fbWait(secs = 60):
    print 'Waiting for fastboot mode'
    ret = None
    for i in range(secs):
        print '.',
        sys.stdout.flush()
        time.sleep(1)
        ret = check_output('%s devices' % fastboot)
        if ret != None and len(ret) != 0:
            print ''
            print 'Fastboot - device detected: ', (ret.split())[0]
            break
    if ret == None:
        print 'No device detected. Please ensure that fastboot is running on the target device'
        exit(1)

def cmdReboot(toBootloader = True):
    if toBootloader:
        call([fastboot, 'reboot-bootloader'])
        fbWait()
    else:
        call([fastboot, 'reboot'])

if __name__ == '__main__':
    # parse args
    parser = argparse.ArgumentParser( \
        description = '''
Auto device flasher, Python 2.7.x required
''',
        formatter_class = argparse.RawTextHelpFormatter)
    parser.add_argument('partition', nargs='?', default = 'all', \
                        help = 'partition to flash [default: all] , not include test partition')
    parser.add_argument('-d', '--dryrun', action = 'store_true', default = False, \
                        help = 'dryrun for debug, no image would be flashed')
    parser.add_argument('-u', '--user', action = 'store_true', default = False, \
                        help = 'Flash user data partition')
    parser.add_argument('-b', '--boot', action = 'store_true', default = False, \
                        help = 'Flash boot partition')
    parser.add_argument('-t', '--test', action = 'store_true', default = False, \
                        help = 'Flash test partition')
    parser.add_argument('-v', '--verbose', action = 'store_true', default = False, \
                        help = 'print more information while flashing')
    parser.add_argument('--toolsdir', default = None, \
                        help = '''\
The tools dir where to find fbtool and fastboot.
Path priority order:
    1. --toolsdir specified
    2. current directory
    3. $PATH
''')
    parser.add_argument('--productdir', default = None, \
                        help = '''\
The product out directory where to find images.
Path priority order:
    1. --productdir specified
    2. current directory
''')

    args = parser.parse_args()
    verbose = args.verbose
    if args.dryrun:
        verbose = True

    print ''
    parser.print_usage()
    print ''
    print ''.center(80, '*')
    print ('Running flasher on ' + platform.platform()).center(80)
    print ''.center(80, '*')
    print ''

    try:
        from flashproc import getFlashProc
    except ImportError, e:
        print 'ImportError:',e
        print ''
        exit(1)
    try:
        from flashproc import getFlashUserProc
    except:
        getFlashUserProc = getFlashProc
    try:
        from flashproc import getFlashBootProc
    except:
        getFlashBootProc = getFlashProc
    try:
        from flashproc import getFlashTestProc
    except:
        getFlashTestProc = getFlashProc

    # check flash tools
    toolsdir = ''
    try:
        if args.toolsdir:
            toolsdir = os.path.abspath(args.toolsdir);
            fbtool = os.path.join(toolsdir, fbtool)
            fastboot = os.path.join(toolsdir, fastboot)
            if not os.path.exists(fbtool) or not os.path.exists(fastboot):
                raise Exception(str(toolsdir))
        else:
            toolsdir = os.path.abspath('.')
            if os.path.exists(os.path.join(toolsdir, fbtool)) and os.path.exists(os.path.join(toolsdir, fastboot)):
                fbtool = os.path.join(toolsdir, fbtool)
                fastboot = os.path.join(toolsdir, fastboot)
    except Exception, e:
        print 'Can not find fbtool or fastboot in %s' % str(e)
        print ''
        exit(1)

    devProduct='DEFAULT'

    procs = getFlashProc(devProduct)
    if args.user:
        procs = getFlashUserProc(devProduct)
    if args.boot:
        procs = getFlashBootProc(devProduct)
    if args.test:
        procs = getFlashTestProc(devProduct)
    if procs:
        if verbose:
            print 'Flash procedure'.center(80)
            print ''.center(80, '-')
            for p in procs:
                print 'fastboot',' '.join(p)
            print ''
    else:
        print 'Can not retrieve flash procedure according to product type of', devProduct
        print 'Exit !'
        exit(1)

    # check image path
    if args.productdir:
        # take user specific product directory
        product_out = os.path.abspath(args.productdir)
    else:
        # check current directory
        product_out = os.path.abspath('.')
        for img in getImageList(procs):
            if checkImage(img, False, False) is None:
                product_out = None
                break
        if product_out is None:
            product_out = os.getenv('PRODUCT_OUT', '.')
    product_out = os.path.abspath(product_out)

    print ''.center(80, '*')
    print '* flash images under:'.ljust(78), '*'
    print '*    ', product_out.ljust(72), '*'
    print ''.center(80, '*')
    print ''

    # check images
    print 'Checking image'.center(80)
    print ''.center(80, '-')
    images = getImageList(procs)
    try:
        for img in images:
            checkImage(img, _verbose = True)
    except Exception, e:
        print e
        time.sleep(2)
        cmdReboot(False)
        raw_input('Fail, press enter to exit: ')
        exit(1)

    # flash images
    print ''
    print 'Start flashing'.center(80)
    print ''.center(80, '-')
    try:
        for proc in procs:
            if skipFlash == False:
                if 0 != cmdRun(proc, args.dryrun):
                    raise Exception('<<FAILED>> %s' % ' '.join(proc))
            else:
                if proc[0] == 'fastboot' and proc[1] == 'flash':
                    print 'Skip flashing', proc[3]
    except Exception, e:
        print e
        time.sleep(2)
        cmdReboot(False)
        exit(1)

    time.sleep(2)
    cmdReboot(False)
    print ''
    if system == "Windows":
        raw_input('Success, press enter to exit: ')
    else:
        print 'Success'
