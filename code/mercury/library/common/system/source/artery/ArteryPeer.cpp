#include "SystemPch.h"
#include "ArteryImpl.h"

//
// ArteryAlloc
//

ArteryU8* ArteryAlloc(int bytes)
{
	return (ArteryU8*)BizNonTrackedMalloc(bytes);
}

void ArteryFree(ArteryU8* mem)
{
	if (mem)
		BizNonTrackedFree(mem);
}

#ifndef CFG_GOLD

//
// Artery string stuff
//

void ArteryStrCpy(char* dst, const char* src, int maxLength)
{
	if (maxLength > 0)
	{
		for(; (*src != '\0') && (maxLength > 1); maxLength--)
			*dst++ = *src++;

		*dst = '\0';
	}
}

//
// ArteryMsgQueue
//

ArteryMsgQueue::ArteryMsgQueue()
{
}

ArteryMsgQueue::~ArteryMsgQueue()
{
	Drain();
}

bool ArteryMsgQueue::Push(const ArteryMessage& msg)
{
	Link* link = ArteryObjectAlloc<Link>();

	if (!link)
		return false;

	link->next	= 0;
	link->msg	= msg;

	_q.Push(link);

	return true;
}

bool ArteryMsgQueue::Pop(ArteryMessage& msg)
{
	Link* link = _q.Pop();

	if (!link)
		return false;

	msg = link->msg;

	ArteryObjectFree(link);

	return true;
}

bool ArteryMsgQueue::IsEmpty() const
{
	return _q.IsEmpty();
}

void ArteryMsgQueue::Drain()
{
	while(Link* link = _q.Pop())
		ArteryObjectFree(link);
}

//
// ArteryPeer
//

ArteryPeer::ArteryPeer() :
	_service(0),
	_peerIndex(-1),
	_salt(0),
	_isActive(false),
	_isConnected(false),
	_isFinished(false),
	_isServer(false),
	_isConnecting(false),
	_callback(0),
	_context(0),
	_receivedMessageIndex(0),
	_sendBacklog(0),
	_sendData(0),
	_sendRemaining(0),
	_receiveLength(0),
	_receiveData((ArteryU8*)&_receiveLength),
	_receiveRemaining(sizeof(_receiveLength))
{
	*_serviceId = '\0';
}

ArteryPeer::~ArteryPeer()
{
}

//
// ArteryPeer connection interface functions
//

bool ArteryPeer::IsConnected(ArteryU32 salt)
{
	_mutex.Wait();

	bool result = (salt == _salt) && _isConnected;

	_mutex.Release();

	return result;
}

bool ArteryPeer::CanReceive(ArteryU32 salt)
{
	_mutex.Wait();

	bool result = (salt == _salt) && !_receiveQ.IsEmpty();

	_mutex.Release();

	return result;
}

int ArteryPeer::EstimatedSendBacklog(ArteryU32 salt)
{
	_mutex.Wait();

	int result = (salt == _salt) ? (_sendBacklog + _sendMsg.MessageLength()) : 0;

	_mutex.Release();

	return result;
}

bool ArteryPeer::Send(ArteryU32 salt, const ArteryMessage& msg) // TODO: probably needs to kick off send
{
	_mutex.Wait();

	bool result = (salt == _salt) && _isConnected && _sendQ.Push(msg);

	if (result)
	{
		_sendBacklog += msg.MessageLength();

		_service->WakeUp();
	}

	_mutex.Release();

	return result;
}

bool ArteryPeer::TryReceive(ArteryU32 salt, ArteryMessage& msg)
{
	_mutex.Wait();

	bool result = (salt == _salt) && _receiveQ.Pop(msg);

	_mutex.Release();

	return result;
}

bool ArteryPeer::Receive(ArteryU32 salt, ArteryMessage& msg)
{
	_mutex.Wait();

	bool result = false;

	if (salt == _salt)
	{
		if (_receiveQ.IsEmpty() && _isConnected)
			_condVar.Wait();

		result = _receiveQ.Pop(msg);
	}

	_mutex.Release();

	return result;
}

bool ArteryPeer::Receive(ArteryU32 salt, ArteryMessage& msg, int timeMsec)
{
	_mutex.Wait();

	bool result = false;

	if (salt == _salt)
	{
		if (_receiveQ.IsEmpty() && _isConnected)
			_condVar.Wait(timeMsec);

		result = _receiveQ.Pop(msg);
	}

	_mutex.Release();

	return result;
}

void ArteryPeer::Close(ArteryU32 salt)
{
	_mutex.Wait();

	if (salt == _salt)
	{
		_isConnected = false;

		_condVar.Broadcast();
	}

	_mutex.Release();
}

bool ArteryPeer::Wait(ArteryU32 salt, ArteryWaitContext& wc)
{
	_mutex.Wait();

	bool result = false;

	if (salt == _salt)
	{
		if (_isConnected && (_receiveQ.IsEmpty() || (wc.messageIndex == _receivedMessageIndex)))
			_condVar.Wait();

		result = !_receiveQ.IsEmpty() && (wc.messageIndex != _receivedMessageIndex);

		wc.messageIndex = _receivedMessageIndex;
	}

	_mutex.Release();

	return result;
}

bool ArteryPeer::Wait(ArteryU32 salt, ArteryWaitContext& wc, int timeMsec)
{
	_mutex.Wait();

	bool result = false;

	if (salt == _salt)
	{
		if (_isConnected && (_receiveQ.IsEmpty() || (wc.messageIndex == _receivedMessageIndex)))
			_condVar.Wait(timeMsec);

		result = !_receiveQ.IsEmpty() && (wc.messageIndex != _receivedMessageIndex);

		wc.messageIndex = _receivedMessageIndex;
	}

	_mutex.Release();

	return result;
}

void ArteryPeer::WakeUp(ArteryU32 salt)
{
	_mutex.Wait();

	if (salt == _salt)
		_condVar.Broadcast();

	_mutex.Release();
}

//
// ArteryPeer create/destroy
//

bool ArteryPeer::Create(ArteryService* service, int peerIndex)
{
	_service	= service;
	_peerIndex	= peerIndex;
	_salt		= 1;

	if (!_mutex.Create() || !_condVar.Create(&_mutex))
	{
		Destroy();
		return false;
	}

	return true;
}

void ArteryPeer::Destroy()
{
	Clear();

	_service	= 0;
	_peerIndex	= -1;
	_salt		= 0;

	_mutex.Destroy();
	_condVar.Destroy();
}

void ArteryPeer::Clear()
{
	_salt++;

	_thread.Close();
	_socket.Close();

	_isActive		= false;
	_isConnected	= false;
	_isFinished		= false;
	_isServer		= false;
	_isConnecting	= false;

	_target		= ArterySockAddr();
	*_serviceId	= '\0';
	_callback	= 0;
	_context	= 0;

	_sendQ.Drain();
	_receiveQ.Drain();

	_receivedMessageIndex	= 0;
	_sendBacklog			= 0;

	_sendMsg.Clear();
	_receiveMsg.Clear();

	_sendData			= 0;
	_sendRemaining		= 0;
	_receiveLength		= 0;
	_receiveData		= (ArteryU8*)&_receiveLength;
	_receiveRemaining	= sizeof(_receiveLength);
}

//
// ArteryPeer management
//

void ArteryPeer::ManageServer(ArterySocket& socket)
{
	_mutex.Wait();

	if (!_isActive && !_service->_shutdown)
	{
		_isActive		= true;
		_isConnected	= true;
		_isFinished		= false;
		_isServer		= true;

		_socket.Attach(socket);

		if (!_thread.Create(&ArteryPeer::ThreadProc, this))
			_isFinished = true;
	}

	_mutex.Release();
}

bool ArteryPeer::ManageClient(ArterySocket& socket, const char* serviceId, ArteryCallback callback, void* context, const ArterySockAddr& target)
{
	bool result = false;

	_mutex.Wait();

	if (!_isActive && !_service->_shutdown)
	{
		_isActive		= true;
		_isConnected	= true;
		_isFinished		= false;
		_isServer		= false;
		_isConnecting	= true;

		_target			= target;
		_callback		= callback;
		_context		= context;

		if (serviceId)
			ArteryStrCpy(_serviceId, serviceId, kArteryMaxServiceIdLength);

		_socket.Attach(socket);

		if (_thread.Create(&ArteryPeer::ThreadProc, this))
			result = true;
		else
		{
			_isFinished = true;

			_service->WakeUp();
		}
	}

	_mutex.Release();

	return result;
}

void ArteryPeer::Update()
{
	_mutex.Wait();

	if (_isActive)
	{
		if (_isConnected)
		{
			if (_isConnecting)
			{
				int result = _socket.TestConnection(_target);

				if (result < 0)
				{
					_isConnected	= false;
					_isConnecting	= false;

					_condVar.Broadcast();
				}
				else if (result > 0)
				{
					_isConnecting = false;
				}
			}

			if (!_isConnecting)
			{
				if (!DoSend() || !DoReceive())
				{
					_isConnected = false;

					_condVar.Broadcast();
				}
			}
		}

		if (_isFinished)
			Clear();
	}

	_mutex.Release();
}

bool ArteryPeer::DoSend()
{
	for(;;)
	{
		if (!_sendMsg)
		{
			if (!_sendQ.Pop(_sendMsg))
				break;

			_sendBacklog -= _sendMsg.MessageLength();

			_sendData		= (ArteryU8*)_sendMsg.Message();
			_sendRemaining	= _sendMsg.MessageLength();
		}

		int sent = _socket.Send(_sendData, _sendRemaining);

		if (sent < 0)
			return false;
		else if (sent == 0)
			break;

		_sendData		+= sent;
		_sendRemaining	-= sent;

		if (_sendRemaining <= 0)
			_sendMsg.Clear();
	}

	return true;
}

bool ArteryPeer::DoReceive()
{
	for(;;)
	{
		int received = _socket.Receive(_receiveData, _receiveRemaining);

		if (received < 0)
			return false;
		else if (received == 0)
			break;

		_receiveData		+= received;
		_receiveRemaining	-= received;

		if (_receiveRemaining <= 0)
		{
			if (_receiveMsg == 0)
			{
				_receiveLength = ntohl(_receiveLength);

				if ( /*(_receiveLength < 0) ||*/ (_receiveLength > kArteryMaxMessageLength)) // JASOND: Intel compiler: "warning #186: pointless comparison of unsigned integer with zero"
					return false;

				if (!_receiveMsg.Set(0, _receiveLength))
					return false;

				_receiveData		= (ArteryU8*)_receiveMsg.Data();
				_receiveRemaining	= _receiveMsg.DataLength();
			}
			else
			{
				_receiveLength		= 0;
				_receiveData		= (ArteryU8*)&_receiveLength;
				_receiveRemaining	= sizeof(_receiveLength);

				_receivedMessageIndex++;

				bool result = _receiveQ.Push(_receiveMsg);

				_receiveMsg.Clear();

				_condVar.Broadcast();

				if (!result)
					return false;
			}
		}
	}

	return true;
}

//
// ArteryPeer thread proc
//

void ArteryPeer::ThreadProc(void* context)
{
	MEMORY_TRACKER_PUSH("mercury.artery");

	ArteryPeer* peer = (ArteryPeer*)context;

	ArteryConnection con(peer);

	if (!peer->_isServer)
	{
		ArteryMessage msg(peer->_serviceId, (int)strlen(peer->_serviceId) + 1);

		if (msg && con.Send(msg))
		{
			while(con.IsConnected())
			{
				ArteryMessage msg;

				if (con.Receive(msg) && msg)
				{
					ArteryMessageReader rdr(msg);

					ArteryU32 tag;

					if (!rdr.Read(tag) || (tag != ARTERY_FOURCC('H', 'E', 'L', 'O')))
						con.Close();

					break;
				}
			}
		}
		else
			con.Close();
	}
	else
	{
		ArteryMessage msg("HELO", 4);

		if (msg && con.Send(msg))
		{
			while(con.IsConnected())
			{
				ArteryMessage msg;

				if (con.Receive(msg) && msg)
				{
					const char* peerAnnounce = (const char*)msg.Data();

					if (*(peerAnnounce + msg.DataLength() - 1) == '\0')
					{
						ArteryStrCpy(peer->_serviceId, peerAnnounce, kArteryMaxServiceIdLength);

						peer->_service->SetupPeerCallback(peer);
					}

					break;
				}
			}
		}
		else
			con.Close();
	}

	if (peer->_callback)
		peer->_callback(con, peer->_context);

	peer->_isFinished = true;

	peer->_service->WakeUp();

	MEMORY_TRACKER_POP("mercury.artery");
}

#endif // CFG_GOLD
