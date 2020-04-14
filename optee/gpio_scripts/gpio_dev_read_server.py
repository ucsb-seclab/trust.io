import socket
import sys
import os
import struct

server_address = ('', 8923)
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Starting to bind."
sock.bind(server_address)

sock.listen(1)

while True:
    # Wait for a connection
    # print "Waiting for connection."
    connection, client_address = sock.accept()
    # print "Received connection"
    target_str = connection.recv(1024)
    
    os.system("cat /sys/class/gpio/gpio489/value > tmp_val.txt")
    fp = open("tmp_val.txt", "r")
    cont = fp.read()
    fp.close()
    cont = str(cont).strip()
    cont = int(cont)
    response = struct.pack('!I', cont)
    connection.send(response)
    connection.close()
