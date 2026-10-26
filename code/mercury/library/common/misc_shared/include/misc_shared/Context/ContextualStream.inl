#include <btl/utility/string.h>
#include <core/utility/endian.h>
#include <system/serialise/Serialise.h>

#include <limits>


namespace Sys
{

class C_ContextualStreamPos
{
	friend C_ContextualStreamLen operator - (const C_ContextualStreamPos& end, const C_ContextualStreamPos& start);

public:
	C_ContextualStreamPos() : m_pPos(NULL), m_pEnd(NULL) {}
	C_ContextualStreamPos(const C_ContextualStreamPos& copy) : m_pPos(copy.m_pPos), m_pEnd(copy.m_pEnd) {}
	C_ContextualStreamPos(const Sys::I_BinaryStream& stream) : m_pPos(stream.m_pCurBuffer), m_pEnd(stream.m_pBufferEnd) {}

	~C_ContextualStreamPos() {}

	C_ContextualStreamPos& operator = (const C_ContextualStreamPos& copy)
	{
		m_pPos = copy.m_pPos;
		m_pEnd = copy.m_pEnd;
		return *this;
	}

private:
	const U8* m_pPos; ///< stream position
	const U8* m_pEnd; ///< stream end, used for validation

};

/**
@param end final position marker.
@param start original position marker.
@returns kStreamLenInvalid if the markers are incompatible.
*/
inline C_ContextualStreamLen operator - (const C_ContextualStreamPos& end, const C_ContextualStreamPos& start)
{
	return ((end.m_pEnd == start.m_pEnd) && (end.m_pPos >= start.m_pPos)) ? static_cast<C_ContextualStreamLen>(end.m_pPos - start.m_pPos) : kStreamLenInvalid;
}

/**
@param lhs value to compare.
@param rhs value to compare.
@returns false if both are valid but different.
*/
inline bool CompareStreamLen(const C_ContextualStreamLen& lhs, const C_ContextualStreamLen& rhs)
{
	return ((lhs == kStreamLenInvalid) || (rhs == kStreamLenInvalid)) ? true : (lhs == rhs);
}

/**
@param flag flag to test.
@returns true if the flag is set.
*/
inline bool C_ContextualStream::GetFlag(const E_Flag flag) const 
{
	return (m_Flags & flag) != 0;
}

/**
@param flag flag to affect.
@param value true to set or false to clear.
*/
inline void C_ContextualStream::SetFlag(const E_Flag flag, const bool value) 
{
	if (value)
		m_Flags |= flag;
	else
		m_Flags &= ~flag;
}

/**
@param pDest buffer to copy to.
@param numBytes number of bytes to read.
*/
void C_ContextualStream::Read(void* pDest, const U32 numBytes)
{
	assert(!m_BinaryStream.IsWriting());
	if (!m_BinaryStream.Read(pDest, numBytes))
		SetFlag(E_Flag_HasErrors);
}

/**
@param pSrc buffer to copy from.
@param numBytes number of bytes to write.
*/
void C_ContextualStream::Write(const void* pSrc, const U32 numBytes) 
{
	assert(m_BinaryStream.IsWriting());
	if (!m_BinaryStream.Write(pSrc, numBytes))
		SetFlag(E_Flag_HasErrors);
}

/**
@param T_ContextInterface should derive from Sys::C_Anything.
@returns NULL if this context interface is not supported.
*/
template <class T_ContextInterface>
T_ContextInterface* C_ContextualStream::GetContextItem() const 
{
	// TODO: [Oscar] use QueryInterface if we want multiple-inheritence
	return Sys::AnythingCast<T_ContextInterface>(GetContextItem(T_ContextInterface::ClassUid()));
}

/**
For consistency with existing code, the ostream operator calls 
Serialise(C_ContextualStream&, obj) which may be implemented anywhere.
@param obj value to write.
@returns ostream object.
*/
template <typename T>
inline C_ContextualStream& C_ContextualStream::operator << (const T& obj) 
{
	Serialise(*this, obj); 
	return *this;
}

/**
For consistency with existing code, the istream operator calls 
Deserialise(C_ContextualStream&, obj) which may be implemented anywhere.
@param obj value to read.
@returns istream object.
*/
template <typename T>
inline C_ContextualStream& C_ContextualStream::operator >> (T& obj)
{
	Deserialise(*this, obj);
	return *this;
}

/**
@returns value read.
*/
template <typename T_ProxyType> 
inline T_ProxyType C_ContextualStream::ReadAs()
{
	T_ProxyType proxy;
	Deserialise(*this, proxy);
	return proxy;
}

/**
@param value value to read.
*/
template <typename T_ProxyType, typename T_DataType>
inline void C_ContextualStream::ReadAs(T_DataType& value)
{
	T_ProxyType proxy;
	Deserialise(*this, proxy);
	value = static_cast<T_DataType>(proxy);
}

/**
@param value value to write.
*/
template <typename T_ProxyType, typename T_DataType> 
inline void C_ContextualStream::WriteAs(const T_DataType& value)
{
	const T_ProxyType proxy = static_cast<T_ProxyType>(value);
	Serialise(*this, proxy);
}


// implement Serialise and Deserialise on C_ContextualStream for single-byte types
#define SERIALISE_CONTEXTUAL_STREAM_BYTE(byteType) \
inline void Serialise(C_ContextualStream& ostream, const byteType value) \
{ \
	StaticAssert(sizeof(value) == 1, byteType_must_be_one_byte); \
	ostream.Write(&value, sizeof(value)); \
} \
inline void Deserialise(C_ContextualStream& ostream, byteType& value) \
{ \
	StaticAssert(sizeof(value) == 1, byteType_must_be_one_byte); \
	ostream.Read(&value, sizeof(value)); \
}

SERIALISE_CONTEXTUAL_STREAM_BYTE(U8)
SERIALISE_CONTEXTUAL_STREAM_BYTE(S8)
SERIALISE_CONTEXTUAL_STREAM_BYTE(char)

#undef SERIALISE_CONTEXTUAL_STREAM_BYTE

// implement Serialise and Deserialise on C_ContextualStream for multi-byte integer types
#define SERIALISE_CONTEXTUAL_STREAM_INTEGER(integerType) \
inline void Serialise(C_ContextualStream& ostream, integerType value) \
{ \
	Ut::NativeToBigEndian::EndianSwap(value); \
	ostream.Write(&value, sizeof(value)); \
} \
inline void Deserialise(C_ContextualStream& istream, integerType& value) \
{ \
	istream.Read(&value, sizeof(value)); \
	Ut::BigEndianToNative::EndianSwap(value); \
}

SERIALISE_CONTEXTUAL_STREAM_INTEGER(U16)
SERIALISE_CONTEXTUAL_STREAM_INTEGER(U32)
SERIALISE_CONTEXTUAL_STREAM_INTEGER(U64)
SERIALISE_CONTEXTUAL_STREAM_INTEGER(S16)
SERIALISE_CONTEXTUAL_STREAM_INTEGER(S32)
SERIALISE_CONTEXTUAL_STREAM_INTEGER(S64)

#undef SERIALISE_CONTEXTUAL_STREAM_INTEGER

// implement Serialise and Deserialise on C_ContextualStream for multi-byte floating point types
#define SERIALISE_CONTEXTUAL_STREAM_FLOAT(floatType, suffix) \
inline void Serialise(C_ContextualStream& ostream, floatType value) \
{ \
	Ut::NativeToBigEndian::EndianSwap##suffix(&value); \
	ostream.Write(&value, sizeof(value)); \
} \
inline void Deserialise(C_ContextualStream& istream, floatType& value) \
{ \
	istream.Read(&value, sizeof(value)); \
	Ut::BigEndianToNative::EndianSwap##suffix(&value); \
}

SERIALISE_CONTEXTUAL_STREAM_FLOAT(F32, D)
SERIALISE_CONTEXTUAL_STREAM_FLOAT(F64, Q)

#undef SERIALISE_CONTEXTUAL_STREAM_FLOAT

// implement Serialise and Deserialise on C_ContextualStream for boolean types
#define SERIALISE_CONTEXTUAL_STREAM_BOOL(boolType) \
inline void Serialise(C_ContextualStream& ostream, const boolType value) \
{ \
	U8 zeroOrOne = (value == true) ? 1 : 0; \
	Ut::NativeToBigEndian::EndianSwap(zeroOrOne); \
	ostream.Write(&zeroOrOne, sizeof(zeroOrOne)); \
} \
inline void Deserialise(C_ContextualStream& istream, boolType& value) \
{ \
	U8 zeroOrOne = 0; \
	istream.Read(&zeroOrOne, sizeof(zeroOrOne)); \
	Ut::BigEndianToNative::EndianSwap(zeroOrOne); \
	value = (zeroOrOne != 0); \
}

SERIALISE_CONTEXTUAL_STREAM_BOOL(bool)

#undef SERIALISE_CONTEXTUAL_STREAM_BOOL

// implement Serialise and Deserialise on C_ContextualStream for Btl::String types
template <typename DataType, typename Buffer>
inline void Serialise(C_ContextualStream& ostream, const Btl::StringBase<DataType, Buffer>& value)
{
	assert(value.Length() <= static_cast<S32>(std::numeric_limits<U16>::max()));

	const U16 length = static_cast<U16>(value.Length());
	ostream << length;
	
	if (length > 0)
		ostream.Write(value.Chars(), length);
}

template <typename DataType, typename Buffer>
inline void Deserialise(C_ContextualStream& istream, Btl::StringBase<DataType, Buffer>& value)
{
	U16 length = 0;
	istream >> length;

	value.Resize(static_cast<S32>(length));
	assert(value.Length() >= static_cast<S32>(length));

	if (length > 0)
		istream.Read(value.NonConstChars(), length);
}

} // namespace Sys
