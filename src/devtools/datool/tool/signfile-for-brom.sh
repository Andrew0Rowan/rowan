#!/bin/bash

#
# Copyright (C) 2015 MediaTek Inc. All rights reserved.
# Tristan Shieh <tristan.shieh@mediatek.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

if [ $# -lt 2 -o $# -gt 3 ]
then
       echo "Usage: $0 private-key.pem filename [pss]"
       exit -1
fi

TMPFILE1=$(mktemp)
TMPFILE2=$(mktemp)

python -c "
import hashlib

f = open('$2', 'rb')
b = f.read()
f.close()

d = hashlib.sha256(b).digest()
if '$3' != 'pss':
	b = '\0\0'
	for i in range(0, len(d), 2):
	       b += d[i + 1] + d[i]
	b += '\0' * 222
else:
	b = d

f = open('${TMPFILE1}', 'wb')
f.write(b)
f.close()
"

if [ -z "$3" -o "$3" != "pss" ]; then
openssl rsautl -sign -inkey $1 -raw -in ${TMPFILE1} -out ${TMPFILE2}
else
openssl pkeyutl -sign -inkey $1 -in ${TMPFILE1} -out ${TMPFILE2} -pkeyopt digest:sha256 -pkeyopt rsa_padding_mode:pss -pkeyopt rsa_pss_saltlen:32
fi
RET=$?

python -c "
f = open('${TMPFILE2}', 'rb')
d = f.read()
f.close()
b = ''
if '$3' != 'pss':
	for i in range(0, len(d), 2):
		b += d[i + 1] + d[i]
else:
	b = d;
f = open('$2.sign', 'wb')
f.write(b)
f.close()
"

if [ "$3" = "pss" ]; then
        echo "Signature file $2.sign with pss padding is generated"
else
        echo "Signature file $2.sign with legacy padding is generated"
fi

rm -f ${TMPFILE1} ${TMPFILE2}
exit ${RET}
