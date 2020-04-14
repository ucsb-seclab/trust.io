import socket
import struct

HOST, PORT = '', 8967

listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
print "TrustIO Client Service..up and running"
listen_socket.listen(1)
while True:
    # print "Waiting for connections"
    client_connection, client_address = listen_socket.accept()
    # print "Got Request from:" + str(client_address)
    request = client_connection.recv(4)
    num = struct.unpack('!I', request[:4])[0]
    # print "Received:" + hex(num)
    currval = 0xdfdfdfdf 
    # print "Sending:" + hex(currval)
    response = struct.pack('!I', currval)
    # print "Sending Response"
    client_connection.sendall(response)
    #print "Closing Connection"
    client_connection.close()
