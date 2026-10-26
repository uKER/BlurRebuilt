#ifndef SYSTEM_ARTERY_ARTERY_H
#define SYSTEM_ARTERY_ARTERY_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Types
//

#if defined(PLATFORM_PC)

typedef signed __int8 ArteryI8;
typedef unsigned __int8 ArteryU8;

typedef signed __int16 ArteryI16;
typedef unsigned __int16 ArteryU16;

typedef signed __int32 ArteryI32;
typedef unsigned __int32 ArteryU32;

typedef signed __int64 ArteryI64;
typedef unsigned __int64 ArteryU64;

typedef float ArteryF32;

#elif defined (PLATFORM_PS3) || defined(PLATFORM_LINUX)

typedef signed char ArteryI8;
typedef unsigned char ArteryU8;

typedef signed short ArteryI16;
typedef unsigned short ArteryU16;

typedef signed int ArteryI32;
typedef unsigned int ArteryU32;

typedef signed long long ArteryI64;
typedef unsigned long long ArteryU64;

typedef float ArteryF32;

#else

#error platform unsupported - see SteveC

#endif

//
// Includes
//

#if defined(PLATFORM_PC)
#include "System/Artery/WsSocket.h"
typedef WsSocket ArterySocket;
typedef WsSockAddr ArterySockAddr;
#elif defined(PLATFORM_PS3)
#include <cell/atomic.h>
#include "System/Artery/BkSocket.h"
typedef BkSocket ArterySocket;
typedef BkSockAddr ArterySockAddr;
#elif defined(PLATFORM_LINUX)
#include <arpa/inet.h>
#include "System/Artery/BkSocket.h"
typedef BkSocket ArterySocket;
typedef BkSockAddr ArterySockAddr;
#else
typedef int ArterySocket;
typedef int ArterySockAddr;
#endif

//
// ArteryAlloc
//

ArteryU8* ArteryAlloc(int bytes);
void ArteryFree(ArteryU8* mem);

template<typename Type> inline Type* ArteryObjectAlloc()
{
	ArteryU8* mem = ArteryAlloc(sizeof(Type));

	if (!mem)
		return 0;

	return new((void*)mem) Type();
}

template<typename Type> inline void ArteryObjectFree(Type* obj)
{
	if (obj)
	{
		obj->~Type();

		ArteryFree((ArteryU8*)obj);
	}
}

//
// ArteryMessage
//

#include "System/Artery/ArteryMessage.h"

/* ArteryStartup
 *
 * Must be call before any other Artery functions are used. Responsible for spinning up
 * the worker thread, and initialising any transports.
 *
 * Returns false if artery could not be started.
 */

bool ArteryStartup(bool startServer, const char* gameName);

/* ArteryStartupEx
 *
 * Must be call before any other Artery functions are used. Responsible for spinning up
 * the worker thread, and initialising any transports. This version allows you
 *
 * Returns false if artery could not be started.
 */

bool ArteryStartupEx(bool startServer, const char* gameName, int maxPeers);

/* ArteryCleanup
 *
 * Stops Artery from processing messages, shuts down all worker threads, and transports. Blocks
 * until complete.
 */

void ArteryCleanup();

/* ArteryCallback
 *
 * Prototype for all callbacks in Artery. Supplies the remote peer the callback is associated with,
 * along with a context value defined by the user.
 */

typedef void (*ArteryCallback)(class ArteryConnection con, void* context);

/* ArteryRegister
 *
 * Associates a callback function with an id. When a connection identifies itself using this id the
 * callback function will be invoked.
 *
 * Returns false on out of memory, or if a callback is already registered under that id.
 */

bool ArteryRegister(const char* id, ArteryCallback callback, void* context);

/* ArteryConnect
 *
 * Initiates a connection to the specified target, then returns immediately. When the connection is finally
 * established, or is unable to complete, the callback function will be invoked.
 *
 * Returns false if there are not enough free resources to start the operation.
 */

bool ArteryConnect(ArteryConnection& con, const ArterySockAddr& target, const char* id, ArteryCallback callback, void* context);

/* ArteryEnumeratePeers
 *
 * Enumerate all potential peers on the network, this function does not return immediately.
 */

struct ArteryPeerInfo
{
	ArterySockAddr	address;
	char			game[64];
	char			machine[64];
	char			platform[16];
};

typedef bool (*ArteryEnumeratePeersCallback)(const ArteryPeerInfo& info, void* context);

bool ArteryEnumeratePeers(ArteryEnumeratePeersCallback callback, void* context);

/* ArteryWaitContext
 *
 * Maintains the connection wait state over multiple calls to ArteryConnection::Wait(...)
 */

struct ArteryWaitContext
{
	ArteryU32 messageIndex;

public:
	ArteryWaitContext() :
		messageIndex((ArteryU32)-1)
	{
	}
};

/* ArteryConnection
 *
 * Interface to remote peer.
 */

class ArteryConnection
{
public:
	ArteryConnection();

public:
	bool IsConnected();
	bool CanReceive();
	int EstimatedSendBacklog();

public:
	bool Send(const ArteryMessage& msg);

public:
	bool TryReceive(ArteryMessage& msg);
	bool Receive(ArteryMessage& msg);
	bool Receive(ArteryMessage& msg, int timeMsec);

public:
	void Close();

public:
	bool Wait(ArteryWaitContext& wc);
	bool Wait(ArteryWaitContext& wc, int timeMsec);

public:
	void WakeUp();

private:
	ArteryConnection(class ArteryPeer* peer);

private:
	class ArteryPeer*	_peer;
	ArteryU32			_salt;

friend class ArteryPeer;
friend class ArteryService;
};

#endif // SYSTEM_ARTERY_ARTERY_H
