import sys
import socket
import time
import struct

# target_cmd = str(sys.argv[3]).lower()
server_address = (sys.argv[1], int(sys.argv[2]))

num_times = 2000
curri = 0
start = time.time()
while curri < num_times:
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # print "Starting to connect."
    sock.connect(server_address)
    # print "Trying to send:" + target_cmd
    sock.sendall("on")
    #print "Closing connection."
    target_cont = sock.recv(1024)
    
    sock.close()
    
    num = struct.unpack('!I', target_cont[:4])[0]
    
    # print hex(num)
    
    curri += 1
   
end = time.time()

print "Total time for 2000 reads:" + str(end-start)
