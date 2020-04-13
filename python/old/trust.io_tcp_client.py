#!/usr/bin/env python

import socket
import struct
import sys

import time

CRYPTO_KEY = 0x01020304

TCP_IP = '10.0.0.10'
TCP_PORT = 7
BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

DEBUG = False


def send_cmd_trustio(cmd):
    global CRYPTO_KEY

    # Start our timer
    start_time = time.time()

    # send command
    s.send(cmd)

    # recv crypto challenge
    data = s.recv(4)
    challenge = struct.unpack("<I", data)[0]
    if 0x80000000 & challenge != 0:
        CRYPTO_KEY = 0x80000000 ^ challenge
        if DEBUG:
            print "Updated key to %08X" % CRYPTO_KEY
        response = challenge
    else:
        response = challenge ^ CRYPTO_KEY

    response_unpack = struct.pack("<I", response)

    if DEBUG:
        print "Challenge: %s, %d" % (repr(data), challenge)
        print "Response: %s %d" % (repr(response_unpack), response)

    # respond with crypto response
    s.send(response_unpack)

    resp = s.recv(len(cmd))

    if resp == cmd:
        print `cmd`, "\t", time.time() - start_time

    time.sleep(.1)


def send_cmd(cmd):
    global CRYPTO_KEY

    # Start our timer
    start_time = time.time()

    s.send(cmd)
    resp = s.recv(len(cmd))

    if resp == cmd:
        print `cmd`, "\t", time.time() - start_time

    time.sleep(.1)


for x in range(1000):

    if DEBUG:
        print "GPIO ON"

    if sys.argv[1] == "notio":
        send_cmd("on\r\n")
    else:
        send_cmd_trustio("on\r\n")

    if DEBUG:
        print "GPIO OFF"

    if sys.argv[1] == "notio":
        send_cmd("off\r\n")
    else:
        send_cmd_trustio("off\r\n")

s.close()
