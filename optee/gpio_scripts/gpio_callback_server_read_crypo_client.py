import socket
import struct
import base64
import sys
import time
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
        raw = raw
        cipher = AES.new( self.key, AES.MODE_CBC, self.iv )
        return cipher.encrypt(raw)

    def decrypt( self, enc ):
        cipher = AES.new(self.key, AES.MODE_CBC, self.iv )
        return cipher.decrypt(enc)
        
currCry = AESCipher()

# target_cmd = str(sys.argv[3]).lower()
server_address = (sys.argv[1], int(sys.argv[2]))

num_times = 20
curri = 0
start = time.time()
while curri < num_times:
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # print "Starting to connect."
    sock.connect(server_address)
    # print "Trying to send:" + target_cmd
    
    con = "a"*12 + struct.pack('!I', 14)
    
    sock.sendall(currCry.encrypt(con))    
    
    #print "Closing connection."
    encont = sock.recv(BS)
    
    sock.close()
    
    target_cont = currCry.decrypt(encont)
    
    num = struct.unpack('!I', target_cont[-4:])[0]
    
    print num
    
    curri += 1
   
end = time.time()

print "Total time for 2000 reads:" + str(end-start)
