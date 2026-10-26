#!/bin/python

import sys
from socket import *

if(len(sys.argv) < 3):
    print "syntax:", sys.argv[0], " <port> <logfile>"
    sys.exit(-1)

port = int(sys.argv[1])
file = open(sys.argv[2], "w")

print "Listen on port", port, ": echo and write to", sys.argv[2]
print "Press Ctrl + Break to exit."

buf = 16384

sock  = socket(AF_INET, SOCK_DGRAM)
sock.bind(("0.0.0.0", port))

while 1:
	data,addr = sock.recvfrom(buf)
	if not data:
		print "error, exiting"
		break
	else:
		out = "%s: %s" % (addr, data)
		print out,
		file.write(out)

sock.close()
f.close()

