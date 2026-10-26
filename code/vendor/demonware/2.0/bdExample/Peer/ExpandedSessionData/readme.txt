Expanded Session Data Example.

Keys:

	Q: quit
	
	K: Connect to event parser


This is a more involved example than any of the others.
This example makes uses of the SessionData class to propogate
information on the connection graph of the p2p session.

There are 2 interesting classes.

SessionData & SessionEventParser

SessionData inherits from bdSessionData and encapsulates 
the conenction graph for each peer.

SessionEventParser inherits from bdSessionListener and
reacts to certain callbacks by sending BD_INFO logs.

The SessionEventParser also registers a RemoteLogSubscriber
which transmits these event messages to the remote log listener.

In this example the remote log listener is the parseLog.py script
which parses the events and generates a digraph file for every new 
state, it then uses the dot tool to create a jpeg representing
the connection graph.

Finally graph.html is a simple webpage that displays the most recently
generated jpeg [called CurrentPic.jpg] and meta refreshes every 500ms.

This example is a work in progress.