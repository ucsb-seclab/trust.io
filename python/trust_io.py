import random
import socket
import struct
from Crypto.Cipher import AES

import time

DH_G = 2
DH_P = 0x00dd45874a7c1eb24021f096ff3dd61ac3


def fast_pow(x, e, m):
    X = x
    E = e
    Y = 1
    while E > 0:
        if E % 2 == 0:
            X = (X * X) % m
            E = E / 2
        else:
            Y = (X * Y) % m
            E = E - 1
    return Y


class TrustIO:
    def __init__(self, debug=False):
        self.debug = debug
        self.key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf" \
                   "\x4f\x3c"
        self.iv = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d" \
                  "\x0e\x0f"
        self.aes_obj = AES.new(self.key, AES.MODE_CBC, self.iv)
        self.freshness_counter = None
        self.sock = None
        self.last_value = None

    def connect(self, ip, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((ip, port))

    def close(self):
        if self.sock is not None:
            self.sock.close()
            self.sock = None

    def send_cmd_trustio(self, cmd, verify_return=True, verify_tuple=(None,
                                                                      None,
                                                                      None)):
        if self.sock is None:
            return False

        # Start our timer
        start_time = time.time()

        # send command
        self.sock.send(cmd)

        if self.debug:
            print "Sent '%s'" % repr(cmd)

        # recv crypto challenge
        data = self.sock.recv(16)
        # print `aes_obj.encrypt("\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00"
        #                        "\x03\x00\x00\x00")`

        # Re-keying?
        if data == '\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' \
                   '\xff\xff':
            if self.debug:
                print "Starting key-pairing protocol!"
            # self.sock.send(data)

            new_key = random.getrandbits(128)
            self.key = str(bytearray.fromhex('{:x}'.format(new_key).zfill(32)))
            if self.debug:
                print "Key: %s" % repr(self.key)

            self.sock.send(self.aes_obj.encrypt(self.key))

            self.aes_obj = AES.new(self.key, AES.MODE_CBC, self.iv)

            if self.debug:
                print "* Updated key! (%s)" % repr(self.key)
            # Get the crypto challenge
            data = self.sock.recv(16)


            # print "Generating local keys..."
            # local_secret = random.getrandbits(128)
            # local_public = pow(DH_G, local_secret, DH_P)
            # print "Secret: %s, Public %s" % (local_secret, local_public)
            #
            # local_public = str(bytearray.fromhex('{:x}'.format(
            #     local_public).zfill(32)))
            # self.sock.send(local_public)
            #
            # print "Reading remote public..."
            # dh_remote = self.sock.recv(16)
            # dh_remote = int(dh_remote.encode('hex'), 16)
            # print repr(dh_remote)
            #
            # print "Calculating key..."
            # key = pow(dh_remote, local_secret, DH_P)
            # print "Key: %d" % key
            # self.key = str(bytearray.fromhex('{:x}'.format(key).zfill(32)))
            # print "Key: %s" % repr(self.key)
            #
            # self.aes_obj = AES.new(self.key, AES.MODE_CBC, self.iv)
            #
            # print "* Updated key! (%s)" % repr(self.key)
            #
            # # Get the crypto challenge
            # data = self.sock.recv(16)

        decrypted_data = self.aes_obj.decrypt(data)
        (dfar, dfsr, value, counter) = struct.unpack("<IIII",
                                                         decrypted_data)
        if self.debug:
            print (dfar, dfsr, value, counter)
        # if 0x80000000 & challenge != 0:
        #     CRYPTO_KEY = 0x80000000 ^ challenge
        #     if DEBUG:
        #         print "Updated key to %08X" % CRYPTO_KEY
        #     response = challenge
        # else:
        #     response = challenge ^ CRYPTO_KEY

        if self.freshness_counter is None:
            self.freshness_counter = counter
        else:
            if counter > self.freshness_counter:
                self.freshness_counter = counter


        # 1 - write, 0 - read
        operation = (dfsr & 1 << 11) >> 11

        # Let's make sure it's permitted
        valid = False
        if dfar == verify_tuple[0] and operation == verify_tuple[1]:
            # write?
            if operation and value == verify_tuple[2]:
                valid = True
            else:
                self.last_value = value
                valid = True

        if valid:
            # Increment our counter and send it back
            response_unpack = struct.pack("<IIII", dfar, dfsr, value,
                                          counter+1)
            response = self.aes_obj.encrypt(response_unpack)
            print "VALID REQUEST!"
        else:
            response_unpack = decrypted_data
            response = data
            print "INVALID REQUEST!"
            print dfar, operation, value
            print verify_tuple

        if self.debug:
            print "Challenge: %s, %d" % (repr(data), value)
            print "Response: %s %s" % (repr(response_unpack), repr(response))

        # respond with crypto response
        self.sock.send(response)

        if verify_return:
            resp = self.sock.recv(len(cmd))

            if resp == cmd:
                print `cmd`, "\t", time.time() - start_time

        time.sleep(.1)
        return True

    def send_cmd(self, cmd):
        if self.sock is None:
            return False

        # Start our timer
        start_time = time.time()

        self.sock.send(cmd)
        resp = self.sock.recv(len(cmd))

        if resp == cmd:
            print `cmd`, "\t", time.time() - start_time

        time.sleep(.1)
        return True

    def verified_read(self, verify_tuple=(None,None,None)):

        start_time = time.time()

        self.send_cmd_trustio("read\r\n", verify_return=False,
                              verify_tuple=verify_tuple)
        read_data = self.sock.recv(4)
        read_data_int = struct.unpack("I", read_data)[0]

        print "read\t", time.time() - start_time

        if read_data_int != self.last_value:
            print "ERROR: %s != %s" % (repr(read_data_int),
                                       repr(self.last_value))
            return None
        else:
            return read_data_int

        # resp = self.sock.recv(len("read\r\n"))
        # print resp
        # decrypted_data = self.aes_obj.decrypt(read_data)
        # (dfar, dfsr, challenge, counter) = struct.unpack("<IIII",
        #                                                  decrypted_data)
        # if self.debug:
        #     print (dfar, dfsr, challenge, counter)
