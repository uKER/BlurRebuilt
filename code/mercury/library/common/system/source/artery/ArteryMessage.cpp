#include "SystemPch.h"
#include "ArteryImpl.h"

//
// ArteryEndian
//

enum ArteryEndian
{
	kArteryLittleEndian,
	kArteryBigEndian
};

#if defined(PLATFORM_PS3) || defined(PLATFORM_WII) || defined(PLATFORM_SPU)
	inline ArteryEndian ArteryGetEndian() { return kArteryBigEndian; }
#elif defined(PLATFORM_PC) || defined(PLATFORM_LINUX)
	inline ArteryEndian ArteryGetEndian() { return kArteryLittleEndian; }
#else
	inline ArteryEndian ArteryGetEndian()
	{
		static const U16 test = 0x00FF;

		return (*((U8*)&test) != 0) ? kArteryLittleEndian : kArteryBigEndian;
	}
#endif

inline ArteryU64 Swizzle(ArteryU64 v)
{
	return (
			((v &  0x00000000000000FFULL) << 56) |
			((v &  0x000000000000FF00ULL) << 40) |
			((v &  0x0000000000FF0000ULL) << 24) |
			((v &  0x00000000FF000000ULL) << 8) |
			((v &  0x000000FF00000000ULL) >> 8) |
			((v &  0x0000FF0000000000ULL) >> 24) |
			((v &  0x00FF000000000000ULL) >> 40) |
			((v &  0xFF00000000000000ULL) >> 56)
		);
}

inline ArteryI64 Swizzle(ArteryI64 v)	{ return Swizzle((ArteryU64)v);	}

//
// ArteryMessage
//

ArteryMessage::Impl ArteryMessage::Private::_null = { 1, 0, { 0 } };

bool ArteryMessage::Set(const void* data, int length)
{
	if (length <= 0)
	{
		Clear();
		return true;
	}

	if ((_impl->references == 1) && (_impl->length == length))
	{
		if (data)
			memcpy(_impl->Data(), data, length);

		return true;
	}

	Impl* impl = CreateImpl(length);

	if (!impl)
	{
		Clear();
		return false;
	}

	if (data)
		memcpy(impl->Data(), data, length);

	_impl->Release();
	_impl = impl;

	return true;
}

void ArteryMessage::Clear()
{
	_impl->Release();
	_impl = &Private::_null;
	_impl->Acquire();
}

ArteryMessage::Impl* ArteryMessage::CreateImpl(int length)
{
	if ((length < 0) || (length > (int)kArteryMaxMessageLength))
		return 0;

	Impl* impl = (Impl*)ArteryAlloc(Private::kImplSize + sizeof(ArteryU32) + length);

	if (!impl)
		return 0;

	impl->references			= 1;
	impl->length				= length;
	*(ArteryU32*)impl->message	= htonl(length);

	return impl;
}

//
// ArteryMessageBuilder
//

ArteryMessageBuilder::ArteryMessageBuilder() :
	_data(0),
	_length(0),
	_capacity(0)
{
}

ArteryMessageBuilder::~ArteryMessageBuilder()
{
	ArteryFree(_data);
}

void ArteryMessageBuilder::Clear()
{
	ArteryFree(_data);
	_data = 0;
	_length = 0;
	_capacity = 0;
}

bool ArteryMessageBuilder::Build(ArteryMessage& msg)
{
	return msg.Set(_data, _length);
}

bool ArteryMessageBuilder::Write(const void* data, int length)
{
	if (!Expand(length))
		return false;

	memcpy(_data + _length, data, length);

	_length += length;

	return true;
}

bool ArteryMessageBuilder::Expand(int length)
{
	int requiredCapacity = _length + length;

	if (requiredCapacity <= _capacity)
		return true;

	int newCapacity = 3 * ((requiredCapacity + 16) >> 1);

	ArteryU8* newData = ArteryAlloc(newCapacity);

	if (!newData)
		return false;

	memcpy(newData, _data, _length);

	ArteryFree(_data);

	_data		= newData;
	_capacity	= newCapacity;

	return true;
}

bool ArteryMessageBuilder::Write(const ArteryI8& value)
{
	return Write(&value, sizeof(value));
}

bool ArteryMessageBuilder::Write(const ArteryU8& value)
{
	return Write((const ArteryI8&)value);
}

bool ArteryMessageBuilder::Write(const ArteryI16& value)
{
	ArteryI16 netValue = htons(value);

	return Write(&netValue, sizeof(netValue));
}

bool ArteryMessageBuilder::Write(const ArteryU16& value)
{
	return Write((const ArteryI16&)value);
}

bool ArteryMessageBuilder::Write(const ArteryI32& value)
{
	ArteryI32 netValue = htonl(value);

	return Write(&netValue, sizeof(netValue));
}

bool ArteryMessageBuilder::Write(const ArteryU32& value)
{
	return Write((const ArteryI32&)value);
}

bool ArteryMessageBuilder::Write(const ArteryI64& value)
{
	ArteryI64 netValue;
	
	if (ArteryGetEndian() == kArteryBigEndian)
		netValue = Swizzle(value);
	else
		netValue = value;

	return Write(&netValue, sizeof(netValue));
}

bool ArteryMessageBuilder::Write(const ArteryU64& value)
{
	return Write((const ArteryI64&)value);
}

bool ArteryMessageBuilder::Write(const ArteryF32& value)
{
	return Write(*(const ArteryI32*)&value);
}

bool ArteryMessageBuilder::Write(const char* text)
{
	ArteryU32 length = (ArteryU32)(strlen(text) + 1);

	if (!Write(length))
		return false;

	return Write(text, length);
}

//
// ArteryMessageReader
//

ArteryMessageReader::ArteryMessageReader(const ArteryMessage& msg) :
	_msg(msg),
	_current((ArteryU8*)msg.Data()),
	_end((ArteryU8*)msg.Data() + msg.DataLength())
{
}

bool ArteryMessageReader::Read(void* data, int length)
{
	if ((_current + length) > _end)
	{
		_current = _end;
		return false;
	}

	if (data)
		memcpy(data, _current, length);

	_current += length;

	return true;
}

bool ArteryMessageReader::Read(ArteryI8& value)
{
	return Read(&value, sizeof(value));
}

bool ArteryMessageReader::Read(ArteryU8& value)
{
	return Read((ArteryI8&)value);
}

bool ArteryMessageReader::Read(ArteryI16& value)
{
	ArteryI16 netValue;

	if (!Read(&netValue, sizeof(netValue)))
		return false;

	value = ntohs(netValue);

	return true;
}

bool ArteryMessageReader::Read(ArteryU16& value)
{
	return Read((ArteryI16&)value);
}

bool ArteryMessageReader::Read(ArteryI32& value)
{
	ArteryI32 netValue;

	if (!Read(&netValue, sizeof(netValue)))
		return false;

	value = ntohl(netValue);

	return true;
}

bool ArteryMessageReader::Read(ArteryU32& value)
{
	return Read((ArteryI32&)value);
}

bool ArteryMessageReader::Read(ArteryI64& value)
{
	ArteryI64 netValue;

	if (!Read(&netValue, sizeof(netValue)))
		return false;

	if (ArteryGetEndian() == kArteryBigEndian)
		value = Swizzle(netValue);
	else
		value = netValue;

	return true;
}

bool ArteryMessageReader::Read(ArteryU64& value)
{
	return Read((ArteryI64&)value);
}

bool ArteryMessageReader::Read(ArteryF32& value)
{
	return Read(*(ArteryI32*)&value);
}

bool ArteryMessageReader::Read(const char*& text)
{
	ArteryU32 length;

	if (!Read(length))
		return false;

	const char* current = (const char*)_current;

	if (!Read(0, length))
		return false;

	text = current;

	return true;
}

bool ArteryMessageReader::Read(const ArteryU8*& data, int length)
{
	const char* current = (const char*)_current;

	if (!Read(0, length))
		return false;

	data = (ArteryU8*)current;

	return true;
}
