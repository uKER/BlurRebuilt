#include "SystemPch.h"
#include <Core/Core.h>
#include <System/Debug/Debug.h>
#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include <System/Debug/VariablesNet.h>

#ifndef CFG_GOLD

extern ArteryLock gDebugPeerCs;

struct DvBuffer
{
	static const int kCapacity = 48 * 1024;

	U8	_data[kCapacity];

	U32 _read;
	U32 _write;

	ArteryLock _lock;

public:
	DvBuffer() :
		_read(0),
		_write(0)
	{
	}

public:
	void Push(const char* channel, ArteryU32 severity, const char* text)
	{
		ArteryU32 chBytes		= strlen(channel) + 1;
		ArteryU32 txBytes		= strlen(text) + 1;
		ArteryU32 totalBytes	= sizeof(ArteryU32) + chBytes + sizeof(ArteryU32) + sizeof(ArteryU32) + txBytes;

		// Drop the message if it's very large

		if (totalBytes >= (kCapacity / 4))
			return;

		_lock.Wait();

		// Free space for message

		while(Available() < totalBytes)
			Pop();

		// Add message to buffer

		Write((const U8*)&chBytes, sizeof(ArteryU32));
		Write((const U8*)channel, chBytes);
		Write((const U8*)&severity, sizeof(ArteryU32));
		Write((const U8*)&txBytes, sizeof(ArteryU32));
		Write((const U8*)text, txBytes); 

		_lock.Release();
	}

	void Send(ArteryConnection con)
	{
		if (_read == _write)
			return;

		ArteryMessageBuilder b;

		if (!b.Write(kDvmDebugLog))
			return;

		if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
			return;

		_lock.Wait();

		bool result = true;

		if (_read < _write)
		{
			if (!b.Write(_data + _read, _write - _read))
				result = false;
		}
		else
		{
			U32 split = kCapacity - _read;

			if (!b.Write(_data + _read, split) || !b.Write(_data, _write))
				result = false;
		}

		_lock.Release();

		if (!result)
			return;

		ArteryMessage m;

		if (b.Build(m))
			con.Send(m);
	}

private:
	void Pop()
	{
		if (_read == _write)
			return;

		// Skip channel

		_read = (_read + 4) % kCapacity;

		while(_data[_read] != '\0')
			_read = (_read + 1) % kCapacity;

		_read = (_read + 1) % kCapacity;

		// Skip severity

		_read = (_read + 4) % kCapacity;

		// Skip text

		_read = (_read + 4) % kCapacity;

		while(_data[_read] != '\0')
			_read = (_read + 1) % kCapacity;

		_read = (_read + 1) % kCapacity;
	}

	U32 Available()
	{
		if (_write >= _read)
			return kCapacity + _read - _write - 1;

		return _read - _write - 1;
	}

	void Write(const U8* data, U32 length)
	{
		U32 split = kCapacity - _write;

		if (length <= split)
		{
			memcpy(_data + _write, data, length);
			_write = (_write + length) % kCapacity;
		}
		else
		{
			U32 left = length - split;

			memcpy(_data + _write, data, split);
			memcpy(_data, data + split, left);

			_write = left;
		}
	}
};

DvBuffer gDvBuffer;

namespace Debug
{
	void AddDvBuffer(const char* channel, ArteryU32 severity, const char* text)
	{
		gDvBuffer.Push(channel, severity, text);
	}

	void SendDvBuffer(ArteryConnection con)
	{
		gDvBuffer.Send(con);
	}
}

#endif // CFG_GOLD
