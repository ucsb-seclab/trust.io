import socket
import struct
import base64

from Crypto import Random
from Crypto.Cipher import AES

BS = 16
pad = lambda s: s + (BS - len(s) % BS) * chr(BS - len(s) % BS)
unpad = lambda s : s[0:-ord(s[-1])]


class AESCipher:

    def __init__(self):
        self.key = buffer(bytearray([0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c]))
        self.iv = buffer(bytearray([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f]))

    def encrypt( self, raw ):
        cipher = AES.new( self.key, AES.MODE_CBC, self.iv )
        return cipher.encrypt(raw)

    def decrypt( self, enc ):
        cipher = AES.new(self.key, AES.MODE_CBC, self.iv )
        return cipher.decrypt(enc)
        
HOST, PORT = '', 8967

listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
print "TrustIO Client Service..up and running"
listen_socket.listen(1)

currCry = AESCipher()

while True:
    # print "Waiting for connections"
    client_connection, client_address = listen_socket.accept()
    # print "Got Request from:" + str(client_address)
    encreq = client_connection.recv(BS)
    # print str(len(encreq))
    request = currCry.decrypt(encreq)
    
    # print str(len(request))
    seqnum = struct.unpack('<I', request[-4:])[0]
    # print "Got:" + str(hex(seqnum))
    # increment sequence number
    seqnum += 1
    # print "Sending:" + hex(seqnum)   
    
    # print "Received:" + hex(num)
    # currval = 0xdfdfdfdf 
    # print "Sending:" + hex(currval)
    response = request[:-4] + struct.pack('<I', seqnum)
    # print "Sending Response"
    client_connection.sendall(currCry.encrypt(response))
    #print "Closing Connection"
    client_connection.close()
