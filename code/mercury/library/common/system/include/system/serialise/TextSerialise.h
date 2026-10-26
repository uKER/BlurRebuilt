#ifndef __TEXT_SERIALISE_H__
#define __TEXT_SERIALISE_H__

#include <system/serialise/I_BinaryStream.h>

namespace Sys
{

// serialize text to a file
// converts \n to \r\n
class TextSerialiser
{
public:
	TextSerialiser(I_BinaryStream& out) : m_stream(out) {}

	inline bool Write(const void *pData, U32 c)
	{
		assert(m_stream.IsWriting());
        return m_stream.Write(pData, c);
	}

	// serialisation functions for all basic core types, checks buffer overflow once per type
#define __TEXT_OUTPUT_STREAM_MACRO(TYPE) \
	FORCE_INLINE TextSerialiser& operator << (const TYPE& _t) \
	{ \
		Util::C_String tmp; \
		tmp << _t; \
		*this << tmp; \
		return *this; \
	}

	// TODO: add support for the other basic types, once Ian has added support from them in Util::C_String
//	__TEXT_OUTPUT_STREAM_MACRO(U8)
//	__TEXT_OUTPUT_STREAM_MACRO(S8)
//	__TEXT_OUTPUT_STREAM_MACRO(U16)
//	__TEXT_OUTPUT_STREAM_MACRO(S16)
	__TEXT_OUTPUT_STREAM_MACRO(U32)
	__TEXT_OUTPUT_STREAM_MACRO(S32)
	__TEXT_OUTPUT_STREAM_MACRO(F32)
//	__TEXT_OUTPUT_STREAM_MACRO(F64)
//	__TEXT_OUTPUT_STREAM_MACRO(U64)
//	__TEXT_OUTPUT_STREAM_MACRO(S64)

	// serialise a c-string
	FORCE_INLINE TextSerialiser& operator << (const char *psz)
	{
		size_t n = strlen(psz);
		for (U32 i=0; i<n; i++)
            *this << psz[i];
		return *this;
	}

	// serialise a Util::C_String
	FORCE_INLINE TextSerialiser& operator << (const Util::C_String &str)
	{
		int n = str.Length();
		const char *psz = str.Chars();
		for (int i=0; i<n; i++)
            *this << psz[i];
		return *this;
	}

	// serialise a character
	FORCE_INLINE TextSerialiser& operator << (const char c)
	{
		static const char cr = '\r';

		if (c == '\n')
			Write((U8*)&cr, 1);

		Write((U8*)&c, 1);
		return *this;
	}

	template<typename T>
	TextSerialiser &operator <<(const T &obj)
	{
		Serialise( *this, (T&)obj );
		return *this;
	}

#undef __TEXT_OUTPUT_STREAM_MACRO

	I_BinaryStream &m_stream;
};

// deserialize a text stream, does either characters or lines at a time.
// converts \r\n to \n
class TextDeserialiser
{
public:
	TextDeserialiser(I_BinaryStream& in) : m_stream(in) {}

	inline bool Read(void *pData, U32 c)
	{
		assert(!m_stream.IsWriting());
        return m_stream.Read(pData, c);
	}

	// read a Util::C_String
	FORCE_INLINE TextDeserialiser& operator >> (Util::C_String &str)
	{
		char c = 0;
		while (1)
		{
			*this >> c;

			str += Util::C_String(c);

			if (c == '\n' || c == 0)
				break;
		}
		return *this;
	}

	// read a character
	FORCE_INLINE TextDeserialiser& operator >> (char &c)
	{
		if (!Read(&c, 1))
			c = 0;

		if (c == '\r')
		{
			// peek the next character to see if it's a \r\n combo
			if (m_stream.m_pCurBuffer >= m_stream.m_pBufferEnd)
			{
				I_BinaryStream::UnderflowContext uc;

				if (!m_stream.Underflow(uc))
				{
					c = 0;
					return *this;
				}
			}
			char cNext = *m_stream.m_pCurBuffer;
			// it's an \r\n combo, so advance an extra character and return '\n'
			if (cNext == '\n')
			{
				m_stream.m_pCurBuffer++;
				c = '\n';
			}
		}		

		return *this;
	}

	template<typename T>
	TextDeserialiser &operator >>(T &obj)
	{
		obj.Deserialise(*this);
		return *this;
	}

	I_BinaryStream&	m_stream;
};

}

#endif

