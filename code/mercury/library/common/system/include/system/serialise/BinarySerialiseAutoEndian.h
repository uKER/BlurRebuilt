#ifndef __BINARY_SERIALISE_AUTO_ENDIAN_H__
#define __BINARY_SERIALISE_AUTO_ENDIAN_H__

#pragma warning (disable:4512) // assignment operator could not generated

#include <core/utility/endian.h>
#include <system/Serialise.h>

namespace Sys
{

// serialize a binary stream and switch endian-ness
// NOTE - this is slower than the BinarySerialiser as it will check whether to endian swap for every <<
class BinarySerialiserAutoEndian
{
public:
	BinarySerialiserAutoEndian(I_BinaryStream& out) : m_stream(out), m_EndianSwap(out.GetEndianSwapFlag()) {}

	inline bool Write(const void *pData, U32 c)
	{
		assert(m_stream.IsWriting());
        return m_stream.Write(pData, c);
	}

	// serialisation functions for all basic core types, checks buffer overflow once per type
#define __BINARY_OUTPUT_STREAM_MACRO(TYPE, SIZE, STYPE) \
	FORCE_INLINE BinarySerialiserAutoEndian& operator << (TYPE _t) \
	{ \
		STYPE t = (STYPE&)_t; \
		if(m_EndianSwap) \
		{ \
			Ut::DoEndianSwap::EndianSwap(t); \
		} \
		else \
		{ \
			Ut::NoEndianSwap::EndianSwap(t); \
		} \
		if ((m_stream.m_pBufferEnd-m_stream.m_pCurBuffer)<(int)SIZE) \
			Write((U8*)&t, SIZE); \
		else \
		{ \
			*((STYPE*)m_stream.m_pCurBuffer) = t; \
			m_stream.m_pCurBuffer += SIZE; \
		} \
		return *this; \
	}

	__BINARY_OUTPUT_STREAM_MACRO(U8,  1, U8)
	__BINARY_OUTPUT_STREAM_MACRO(S8,  1, U8)
	__BINARY_OUTPUT_STREAM_MACRO(char,1, U8)
	__BINARY_OUTPUT_STREAM_MACRO(U16, 2, U16)
	__BINARY_OUTPUT_STREAM_MACRO(S16, 2, U16)
	__BINARY_OUTPUT_STREAM_MACRO(U32, 4, U32)
	__BINARY_OUTPUT_STREAM_MACRO(S32, 4, U32)
	__BINARY_OUTPUT_STREAM_MACRO(F32, 4, U32)
	__BINARY_OUTPUT_STREAM_MACRO(F64, 8, U64)
	__BINARY_OUTPUT_STREAM_MACRO(U64, 8, U64)
	__BINARY_OUTPUT_STREAM_MACRO(S64, 8, U64)

#undef __BINARY_OUTPUT_STREAM_MACRO

	template<typename T>
	BinarySerialiserAutoEndian &operator <<(T &obj)
	{
		Serialise( *this, obj );
		return *this;
	}

	template<typename T>
	BinarySerialiserAutoEndian &operator <<(const T &obj)
	{
		Serialise( *this, (T&)obj );
		return *this;
	}

	I_BinaryStream &m_stream;
	const bool m_EndianSwap;
};

// deserialize a binary stream and switch endian-ness
class BinaryDeserialiserAutoEndian
{
public:

	BinaryDeserialiserAutoEndian(I_BinaryStream& in) : m_stream(in), m_EndianSwap(in.GetEndianSwapFlag()) {}

	inline bool Read(void *pData, U32 c)
	{
		assert(!m_stream.IsWriting());
        return m_stream.Read(pData, c);
	}

	// deserialisation functions for all basic core types, checks buffer underflow once per type
#define __BINARY_INPUT_STREAM_MACRO(TYPE, SIZE, STYPE) \
	FORCE_INLINE BinaryDeserialiserAutoEndian& operator >> (TYPE &t) \
	{ \
		STYPE _t; \
		if ((m_stream.m_pBufferEnd-m_stream.m_pCurBuffer)<(int)SIZE) \
			Read((U8*)&_t, SIZE); \
		else \
		{ \
			_t = *((STYPE*)m_stream.m_pCurBuffer); \
			m_stream.m_pCurBuffer += SIZE; \
		} \
		if(m_EndianSwap) \
		{ \
			Ut::DoEndianSwap::EndianSwap(_t); \
		} \
		else \
		{ \
			Ut::NoEndianSwap::EndianSwap(_t); \
		} \
		t = (TYPE&)_t; \
		return *this; \
	}
	
	__BINARY_INPUT_STREAM_MACRO(U8,  1, U8)
	__BINARY_INPUT_STREAM_MACRO(S8,  1, U8)
	__BINARY_INPUT_STREAM_MACRO(char,1, U8)
	__BINARY_INPUT_STREAM_MACRO(U16, 2, U16)
	__BINARY_INPUT_STREAM_MACRO(S16, 2, U16)
	__BINARY_INPUT_STREAM_MACRO(U32, 4, U32)
	__BINARY_INPUT_STREAM_MACRO(S32, 4, U32)
	__BINARY_INPUT_STREAM_MACRO(F32, 4, U32)
	__BINARY_INPUT_STREAM_MACRO(F64, 8, U64)
	__BINARY_INPUT_STREAM_MACRO(U64, 8, U64)
	__BINARY_INPUT_STREAM_MACRO(S64, 8, U64)

#undef __BINARY_INPUT_STREAM_MACRO


	template<typename T>
	BinaryDeserialiserAutoEndian &operator >>(T &obj)
	{
		Deserialise( *this, obj );
		return *this;
	}

	I_BinaryStream&	m_stream;
	const bool m_EndianSwap;
};

}

#endif

