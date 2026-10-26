#ifndef SYSTEM_ARTERY_ARTERYMESSAGE_H
#define SYSTEM_ARTERY_ARTERYMESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

const ArteryU32 kArteryMaxMessageLength = 64 * 1024 * 1024;

//
// ArteryMessage
//

class ArteryMessage
{
public:
	inline ArteryMessage() :
		_impl(&Private::_null)
	{
		_impl->Acquire();
	}

	inline ArteryMessage(int length) :
		_impl(&Private::_null)
	{
		_impl->Acquire();

		if (!Set(0, length))
		{
			assert(0);
		}
	}

	inline ArteryMessage(const void* data, int length) :
		_impl(&Private::_null)
	{
		_impl->Acquire();

		if (!Set(data, length))
		{
			assert(0);
		}
	}

	inline ArteryMessage(const ArteryMessage& rhs) :
		_impl(rhs._impl)
	{
		_impl->Acquire();
	}

	inline ~ArteryMessage()
	{
		_impl->Release();
	}

public:
	inline ArteryMessage& operator=(const ArteryMessage& rhs)
	{
		rhs._impl->Acquire();
		_impl->Release();

		_impl = rhs._impl;

		return *this;
	}

	inline operator bool() const
	{
		return _impl != &Private::_null;
	}

public:
	inline int DataLength() const
	{
		return _impl->length;
	}

	inline void* Data() const
	{
		return _impl->Data();
	}

public:
	inline int MessageLength() const
	{
		return _impl->length + sizeof(ArteryU32);
	}

	inline void* Message() const
	{
		return _impl->message;
	}

public:
	bool Set(const void* data, int length);
	void Clear();

private:
	struct Impl
	{
		int			references;
		int			length;
		ArteryU8	message[1];

		inline void Acquire()
		{
#if defined(PLATFORM_PC)
			InterlockedIncrement((LONG*)&references);
#elif defined(PLATFORM_PS3)
			cellAtomicIncr32((ArteryU32*)&references);
#else
			references++;
#endif
		}

		inline void Release()
		{
#if defined(PLATFORM_PC)
			if (InterlockedDecrement((LONG*)&references) <= 0)
#elif defined(PLATFORM_PS3)
			if (cellAtomicDecr32((ArteryU32*)&references) <= 1)
#else
			if (--references <= 0)
#endif
			{
				ArteryFree((ArteryU8*)this);
			}
		}

		inline ArteryU8* Data()
		{
			return message + sizeof(ArteryU32);
		}
	};

private:
	struct Private
	{
		static Impl _null;
		static const int kImplSize = sizeof(int) * 2;
	};

private:
	static Impl* CreateImpl(int length);

private:
	Impl* _impl;
};

//
// ArteryMessageBuilder
//

class ArteryMessageBuilder
{
public:
	ArteryMessageBuilder();
	~ArteryMessageBuilder();

public:
	bool Build(ArteryMessage& msg);
	void Clear();

public:
	bool Write(const void* data, int length);

public:
	bool Write(const ArteryI8& value);
	bool Write(const ArteryU8& value);
	bool Write(const ArteryI16& value);
	bool Write(const ArteryU16& value);
	bool Write(const ArteryI32& value);
	bool Write(const ArteryU32& value);
	bool Write(const ArteryI64& value);
	bool Write(const ArteryU64& value);
	bool Write(const ArteryF32& value);
	bool Write(const char* text);

private:
	bool Expand(int length);

private:
	ArteryU8*	_data;
	int			_length;
	int			_capacity;
};

//
// ArteryMessageReader
//

class ArteryMessageReader
{
public:
	ArteryMessageReader(const ArteryMessage& msg);

public:
	bool Read(void* data, int length);

public:
	bool Read(ArteryI8& value);
	bool Read(ArteryU8& value);
	bool Read(ArteryI16& value);
	bool Read(ArteryU16& value);
	bool Read(ArteryI32& value);
	bool Read(ArteryU32& value);
	bool Read(ArteryI64& value);
	bool Read(ArteryU64& value);
	bool Read(ArteryF32& value);
	bool Read(const char*& text);
	bool Read(const ArteryU8*& data, int length);

private:
	ArteryMessage	_msg;
	ArteryU8*		_current;
	ArteryU8*		_end;
};

#endif // SYSTEM_ARTERYMESSAGE_H
