#ifndef ARTERYIMPL_H
#define ARTERYIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

//
// System includes
//

#ifdef PLATFORM_PS3
#include <netex/net.h>
#include <sys/synchronization.h>
#include <sys/ppu_thread.h>
#include <sys/poll.h>
#endif

#include <System/Debug/Debug.h>
#include <System/Artery/Artery.h>
#include <System/Memory/MemoryTracker.h>

//
// Macros
//

#define ARTERY_FOURCC(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

//
// Constants
//

const int kArteryListenPort			= 6317;
const int kArteryBroadcastPort		= 6318;
const int kArteryBroadcastReplyPort	= 6319;

const ArteryU32	kAtHeader		= ARTERY_FOURCC('A', 'R', 'T', 'E');
const ArteryU32	kAtSearch		= ARTERY_FOURCC('F', 'I', 'N', 'D');
const ArteryU32	kAtSearchReply	= ARTERY_FOURCC('R', 'P', 'L', 'Y');

const int kArteryMaxServiceIdLength	= 32;

//
// Broadcast packets
//

struct ArteryBroadcastMessage
{
	ArteryU32	length;
	ArteryU32	tag;
	ArteryU32	request;
};

struct ArteryBroadcastReply
{
	ArteryU32	length;
	ArteryU32	tag;
	ArteryU32	request;
	char		game[64];
	char		machine[64];
	char		platform[16];
};

//
// Artery string stuff
//

void ArteryStrCpy(char* dst, const char* src, int maxLength);

//
// ArteryQueue
//

template<typename Type, Type* Type::*nextMember>
class ArteryQueue
{
public:
	inline ArteryQueue() :
		_head(0),
		_next(&_head)
	{
	}

	inline ~ArteryQueue()
	{
	}

public:
	void Push(Type* item)
	{
		item->*nextMember	= 0;
		*_next				= item;
		_next				= &(item->*nextMember);
	}

	Type* Pop()
	{
		Type* item = _head;

		if ((item != 0) && ((_head = item->*nextMember) == 0))
			_next = &_head;

		return item;
	}

public:
	bool IsEmpty() const
	{
		return _head == 0;
	}

private:
	Type*	_head;
	Type**	_next;
};

//
// ArteryMsgQueue
//

class ArteryMsgQueue
{
public:
	ArteryMsgQueue();
	~ArteryMsgQueue();

public:
	bool Push(const ArteryMessage& msg);
	bool Pop(ArteryMessage& msg);

public:
	bool IsEmpty() const;

public:
	void Drain();

private:
	struct Link
	{
		Link* next;
		ArteryMessage msg;
	};

private:
	ArteryQueue<Link, &Link::next> _q;
};

//
// ArterySync
//

#include <System/Artery/ArterySync.h>

//
// ArteryPeer
//

class ArteryService;

class ArteryPeer
{
public:
	ArteryPeer();
	~ArteryPeer();

public:
	bool IsConnected(ArteryU32 salt);
	bool CanReceive(ArteryU32 salt);
	int EstimatedSendBacklog(ArteryU32 salt);

public:
	bool Send(ArteryU32 salt, const ArteryMessage& msg);

public:
	bool TryReceive(ArteryU32 salt, ArteryMessage& msg);
	bool Receive(ArteryU32 salt, ArteryMessage& msg);
	bool Receive(ArteryU32 salt, ArteryMessage& msg, int timeMsec);

public:
	void Close(ArteryU32 salt);

public:
	bool Wait(ArteryU32 salt, ArteryWaitContext& wc);
	bool Wait(ArteryU32 salt, ArteryWaitContext& wc, int timeMsec);

public:
	void WakeUp(ArteryU32 salt);

public:
	bool Create(ArteryService* service, int peerIndex);
	void Destroy();

public:
	void Clear();

public:
	void ManageServer(ArterySocket& socket);
	bool ManageClient(ArterySocket& socket, const char* serviceId, ArteryCallback callback, void* context, const ArterySockAddr& target);
	void Update();

public:
	bool DoSend();
	bool DoReceive();

private:
	static void ThreadProc(void* context);

private:
	ArteryService*	_service;
	int				_peerIndex;
	ArteryU32		_salt;

	bool			_isActive;
	bool			_isConnected;
	bool			_isFinished;
	bool			_isServer;
	bool			_isConnecting;

	ArterySockAddr	_target;
	char			_serviceId[kArteryMaxServiceIdLength];
	ArteryCallback	_callback;
	void*			_context;

	ArteryThread	_thread;
	ArterySocket	_socket;

	ArteryMutex		_mutex;
	ArteryCondVar	_condVar;

	ArteryMsgQueue	_sendQ;
	ArteryMsgQueue	_receiveQ;

	ArteryU32		_receivedMessageIndex;

	ArteryU32		_sendBacklog;

private:
	ArteryMessage	_sendMsg;
	ArteryU8*		_sendData;
	int				_sendRemaining;

private:
	ArteryMessage	_receiveMsg;
	ArteryU32		_receiveLength;
	ArteryU8*		_receiveData;
	int				_receiveRemaining;

friend class ArteryConnection;
friend class ArteryService;
};

//
// ArteryService
//

#include "ArteryService.h"

#endif // ARTERYIMPL_H
