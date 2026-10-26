#include "SystemPch.h"
#include "ArteryImpl.h"

/*

	TODO
	----

	struct ArteryThrottle
	{
		// Any values set to zero are left unchanged

		int maxSendBytes;
		int maxSendMessages;

		int maxReceiveBytes;
		int maxReceiveMessages;
	};

	void ArteryConnection::SetThrottle(ArteryThrottle* throttle);

	bool ArteryConnection::CanSend();
	bool ArteryConnection::TrySend(const ArteryMessage& msg);

	struct ArteryStatus
	{
		int sendTotalBytes;
		int sendTotalMessages;

		int sendBacklogBytes;
		int sendBacklogMessages;

		int receiveTotalBytes;
		int receiveTotalMessages;

		int receiveBacklogBytes;
		int receiveBacklogMessages;
	};

	bool ArteryConnection::Status(ArteryStatus* status);

*/

#ifndef CFG_GOLD

ArteryService gArteryService;

//
// Artery interface functions
//

bool ArteryStartup(bool startServer, const char* gameName)
{
	// By default we allow 8 peers to connect to a running service
	return gArteryService.Initialise(startServer, gameName, 8);
}

bool ArteryStartupEx(bool startServer, const char* gameName, int maxPeers)
{
	return gArteryService.Initialise(startServer, gameName, maxPeers);
}

void ArteryCleanup()
{
	gArteryService.Destroy();
}

bool ArteryRegister(const char* id, ArteryCallback callback, void* context)
{
	return gArteryService.Register(id, callback, context);
}

bool ArteryConnect(ArteryConnection& con, const ArterySockAddr& target, const char* id, ArteryCallback callback, void* context)
{
	return gArteryService.Connect(target, id, callback, context, con);
}

bool ArteryEnumeratePeers(ArteryEnumeratePeersCallback callback, void* context)
{
#if defined(PLATFORM_PC)
	ArterySocket skt;

	if (!skt.Broadcast(0))
		return false;

	WSAEVENT event = skt.Event();

	ArteryBroadcastMessage msg;

	msg.length	= htonl(sizeof(msg));
	msg.tag		= htonl(kAtHeader);
	msg.request	= htonl(kAtSearch);

	DWORD startTime	= GetTickCount();
	DWORD sendTime	= startTime - 1000;

	for(;;)
	{
		DWORD time = GetTickCount();

		if ((time - startTime) >= 250)
			break;

		if ((time - sendTime) >= 50)
		{
			if (skt.SendTo(&msg, sizeof(msg), ArterySockAddr(INADDR_BROADCAST, kArteryBroadcastPort)) != sizeof(msg))
				return false;

			sendTime = time;
		}

		WSAWaitForMultipleEvents(1, &event, FALSE, 25, FALSE);

		WsSockAddr addr;
		ArteryBroadcastReply rply;

		for(;;)
		{
			int len = skt.ReceiveFrom(&rply, sizeof(rply), addr);

			if (len <= 0)
				break;

			if ((len != sizeof(rply)) || (len != ntohl(rply.length)) || (kAtHeader != ntohl(rply.tag)) || (kAtSearchReply != ntohl(rply.request)))
				continue;

			ArteryPeerInfo info;

			info.address = WsSockAddr(addr.Addr(), kArteryListenPort);

			_snprintf(info.game, sizeof(info.game), "%s", rply.game);
			_snprintf(info.machine, sizeof(info.machine), "%s", rply.machine);
			_snprintf(info.platform, sizeof(info.platform), "%s", rply.platform);

			if (!callback(info, context))
				return true;
		}
	}

	return true;
#else
	return false;
#endif
}

//
// ArteryConnection
//

ArteryConnection::ArteryConnection() :
	_peer(0),
	_salt((ArteryU32)-1)
{
}

bool ArteryConnection::IsConnected()
{
	return _peer ? _peer->IsConnected(_salt) : false;
}

bool ArteryConnection::CanReceive()
{
	return _peer ? _peer->CanReceive(_salt) : false;
}

int ArteryConnection::EstimatedSendBacklog()
{
	return _peer ? _peer->EstimatedSendBacklog(_salt) : 0;
}

bool ArteryConnection::Send(const ArteryMessage& msg)
{
	return _peer ? _peer->Send(_salt, msg) : false;
}

bool ArteryConnection::TryReceive(ArteryMessage& msg)
{
	return _peer ? _peer->TryReceive(_salt, msg) : false;
}

bool ArteryConnection::Receive(ArteryMessage& msg)
{
	return _peer ? _peer->Receive(_salt, msg) : false;
}

bool ArteryConnection::Receive(ArteryMessage& msg, int timeMsec)
{
	return _peer ? _peer->Receive(_salt, msg, timeMsec) : false;
}

void ArteryConnection::Close()
{
	if (_peer)
	{
		_peer->Close(_salt);

		_peer = 0;
	}
}

bool ArteryConnection::Wait(ArteryWaitContext& wc)
{
	return _peer ? _peer->Wait(_salt, wc) : false;
}

bool ArteryConnection::Wait(ArteryWaitContext& wc, int timeMsec)
{
	return _peer ? _peer->Wait(_salt, wc, timeMsec) : false;
}

void ArteryConnection::WakeUp()
{
	if (_peer)
		_peer->WakeUp(_salt);
}

ArteryConnection::ArteryConnection(class ArteryPeer* peer) :
	_peer(peer),
	_salt(peer->_salt)
{
}

#else // !CFG_GOLD

bool ArteryStartup(bool startServer, const char* gameName)
{
	return false;
}

void ArteryCleanup()
{
}

bool ArteryRegister(const char* id, ArteryCallback callback, void* context)
{
	return false;
}

bool ArteryConnect(ArteryConnection& con, const ArterySockAddr& target, const char* id, ArteryCallback callback, void* context)
{
	return false;
}

bool ArteryEnumeratePeers(ArteryEnumeratePeersCallback callback, void* context)
{
	return false;
}

//
// ArteryConnection
//

ArteryConnection::ArteryConnection() :
	_peer(0),
	_salt(-1)
{
}

bool ArteryConnection::IsConnected()
{
	return false;
}

bool ArteryConnection::CanReceive()
{
	return false;
}

int ArteryConnection::EstimatedSendBacklog()
{
	return 0;
}

bool ArteryConnection::Send(const ArteryMessage& msg)
{
	return false;
}

bool ArteryConnection::TryReceive(ArteryMessage& msg)
{
	return false;
}

bool ArteryConnection::Receive(ArteryMessage& msg)
{
	return false;
}

bool ArteryConnection::Receive(ArteryMessage& msg, int timeMsec)
{
	return false;
}

void ArteryConnection::Close()
{
}

bool ArteryConnection::Wait(ArteryWaitContext& wc)
{
	return false;
}

bool ArteryConnection::Wait(ArteryWaitContext& wc, int timeMsec)
{
	return false;
}

void ArteryConnection::WakeUp()
{
}

ArteryConnection::ArteryConnection(class ArteryPeer* peer) :
	_peer(0),
	_salt(-1)
{
}

#endif // !CFG_GOLD
