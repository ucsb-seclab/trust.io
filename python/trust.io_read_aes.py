#!/usr/bin/env python

import socket
import struct
import sys

import time

from trust_io import TrustIO

TCP_IP = '10.0.0.10'
TCP_PORT = 7

tio = TrustIO(debug=False)
tio.connect(TCP_IP, TCP_PORT)

print "* Turning lights on..."
tio.send_cmd_trustio("on\r\n", verify_tuple=(0x41200000, 1, 0xffff))


for x in range(2000):
    if x%2 == 0:
        print "* Turning lights on..."
        tio.send_cmd_trustio("on\r\n", verify_tuple=(0x41200000, 1, 0xffff))
    else:
        print "* Turning lights off..."
        tio.send_cmd_trustio("off\r\n", verify_tuple=(0x41200000, 1, 0))
    print "* Verified read..."
    val = tio.verified_read(verify_tuple=(0x41200000, 0, 0))
    print "** Read: ", val

print "* Turning lights off..."
tio.send_cmd_trustio("off\r\n", verify_tuple=(0x41200000, 1, 0))

# print "* Verified read..."
# val = tio.verified_read()
# print "** Read: ", val

tio.close()
