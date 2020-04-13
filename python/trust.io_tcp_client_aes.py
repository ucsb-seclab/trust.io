#!/usr/bin/env python

import socket
import struct
import sys

import time

from trust_io import *

CRYPTO_KEY = 0x01020304

from Crypto.Cipher import AES

key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"
iv = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
aes_obj = AES.new(key, AES.MODE_CBC, iv)
freshness_counter = None


TCP_IP = '10.0.0.10'
TCP_PORT = 7
BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

DEBUG = True


tio = TrustIO(debug=DEBUG)
tio.connect(TCP_IP, TCP_PORT)

for x in range(1000):

    if DEBUG:
        print "GPIO ON"

    if sys.argv[1] == "notio":
        tio.send_cmd("on\r\n")
    else:
        tio.send_cmd_trustio("on\r\n")

    # time.sleep(.5)
    if DEBUG:
        print "GPIO OFF"

    if sys.argv[1] == "notio":
        tio.send_cmd("off\r\n")
    else:
        tio.send_cmd_trustio("off\r\n")

s.close()
