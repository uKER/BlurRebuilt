import string
import re
import os

import sys
from socket import *

m_peers  = {}

stateTransitions = []

newDict1 = {}
stateTransitions.append(newDict1)

transitionIndex = 0
activePeer = "Peer 0"

if(len(sys.argv) < 2):
	print "Using default port 3900"
	port = 3900
else:
	port = int(sys.argv[1])


print "Listen on port", port, ": echo diagrams"
print "Press Ctrl + Break to exit."

buf = 16384

sock  = socket(AF_INET, SOCK_DGRAM)
sock.bind(("0.0.0.0", port))

transitionCount = 0

while 1:

	isNewData = 0

	data,addr = sock.recvfrom(buf)
	if not data:
		print "error, exiting"
		break
	else:

		dataString = str(data)

		lines = dataString.split('\n' )

		for line in lines:

			peerIDline = re.search('Join Event ', line)
			transitionLine = re.search('Session Status changed from' ,line)
			dataLine = re.search('Data for ' , line)
			addressLine = re.search('Local Addr: ' , line)
			hostQuitLine = re.search('Host Quit:' , line)
			if peerIDline :
				startIDIndex = line.find('Join Event ')
				startIDIndex += 11
				endIDIndex = line.find('Address Local Addr: ')
				endIDIndex -= 1

				startAddrIndex = endIDIndex + 21
				endAddrIndex = len(line)
				peerID = line[startIDIndex : endIDIndex ]
				peerAddr = line[startAddrIndex : endAddrIndex]


				m_peers [peerID] = peerAddr

				# hack for when we don't get a session status notice.
				transitionIndex += 1
				newDict2 = {}
				stateTransitions.append(newDict2)
				isNewData = 1

			elif transitionLine:
				transitionIndex += 1
				newDict2 = {}
				stateTransitions.append(newDict2)
				isNewData = 1

			elif dataLine:

				startIDIndex = line.find('Data for ')
				startIDIndex += 9
				endIDIndex = len(line)
				endIDIndex -= 1

				activePeer = line[startIDIndex : endIDIndex ]

			elif addressLine:
				startAddrIndex = line.find('Local Addr: ')
				startAddrIndex += 12
				endAddrIndex = len(line)
				connection = line[startAddrIndex : endAddrIndex ]
				if stateTransitions[transitionIndex].has_key(activePeer):
					stateTransitions[transitionIndex][activePeer].append(connection)
				else:
					stateTransitions[transitionIndex] [activePeer] = []
					stateTransitions[transitionIndex] [activePeer].append(connection)

			elif hostQuitLine:
				transitionIndex += 1
				newDict2 = {}
				stateTransitions.append(newDict2)
				print "Host Quit"


	if isNewData == 0:


		#time to print a new picture...

		transitionCount = transitionIndex

		tempFile = open ("temp", "w")

		firstLineString = "digraph  State" + str(transitionCount) + "{"

		tempFile.write(firstLineString)


		stateToPrint = stateTransitions[transitionCount]
		keys = stateToPrint.keys()


		for state in keys:
			size = len (stateToPrint[state])
			count = 0
			while count < size:

				try:
					outputString = "\"" + m_peers[state]+ "\"" +" -> " + "\"" + stateToPrint[state][count]+ "\""
				except:
					outputString = "\"" + state + "\"" +" -> " + "\"" + stateToPrint[state][count]+ "\""

				tempFile.write(outputString)

				count += 1

		lastLineString = "}"
		tempFile.write(lastLineString)

		tempFile.flush()
		tempFile.close()

		# create current pic
		os.system("dot -Tjpg -oPic"+str(transitionCount) +".jpg temp" )

		# create Nth pic
		os.system("dot -Tjpg -oCurrentPic.jpg temp" )
		#end dot exec...
