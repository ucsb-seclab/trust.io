import socket
import sys
import os

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
    if str(target_str).startswith("on"):
        # print "Turning ON"
        os.system("echo 1 > /sys/class/gpio/gpio489/value")
    else:
        # print "Turning OFF"
        os.system("echo 0 > /sys/class/gpio/gpio489/value")
    connection.send("OK")
    connection.close()
