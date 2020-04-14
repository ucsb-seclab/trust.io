import sys
import socket
import time

# target_cmd = str(sys.argv[3]).lower()
server_address = (sys.argv[1], int(sys.argv[2]))

num_times = 1000
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
    sock.recv(1024)
    sock.close()
    time.sleep(0.25)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # print "Starting to connect."
    sock.connect(server_address)
    # print "Trying to send:" + target_cmd
    sock.sendall("off")
    sock.recv(1024)
    #print "Closing connection."
    sock.close()
    time.sleep(0.25)
    
    curri += 1
   
end = time.time()

print "Total time for 1000 blinks:" + str(end-start)
