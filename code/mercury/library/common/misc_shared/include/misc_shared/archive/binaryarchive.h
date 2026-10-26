#ifndef ARCHIVE_BINARY_ARCHIVE_H
#define ARCHIVE_BINARY_ARCHIVE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "serialisationinfo.h"
#include "core/utility/endian.h"
#include "system/serialise/i_binarystream.h"
#include "btl/utility/types.h"

namespace Arch
{

class BinaryOutputArchive;
class BinaryInputArchive;

// write archive to a binary stream and switch endian-ness
template<typename ENDIANSWITCH>
class BinaryOutputArchive_t
{
private:
	FRIEND_CLASS BinaryOutputArchive;

	BinaryOutputArchive_t(Sys::I_BinaryStream& out) : m_stream(out) {}

	void Open()
	{
		Write("ARCH", 4);
		m_version = 0;
		*this << (S32) m_version;
	}

	void Close()
	{
	}

public:

	/// Write unformatted data to the archive. Returns true for success.
	inline bool Write(const void *pData, U32 nBytes)
	{
		assert(m_stream.IsWriting());
        return m_stream.Write(pData, nBytes);
	}

	inline void Align(U32 bytes)
	{
		U64 size = m_stream.GetPosition();
		U64 aligned = Btl::AlignUp(size, (U64)bytes);
		Write(0, U32(aligned - size));
	}

#define __BINARY_OUTPUT_STREAM_MACRO(TYPE, STYPE) \
	BinaryOutputArchive_t& operator << (TYPE t) {SerialiseAtom<TYPE, STYPE>(t); return *this;} \
	BinaryOutputArchive_t& operator & (TYPE t) {SerialiseAtom<TYPE, STYPE>(t); return *this;}

	__BINARY_OUTPUT_STREAM_MACRO(U8,  U8)
	__BINARY_OUTPUT_STREAM_MACRO(S8,  U8)
	__BINARY_OUTPUT_STREAM_MACRO(char,U8)
	__BINARY_OUTPUT_STREAM_MACRO(bool,U8)
	__BINARY_OUTPUT_STREAM_MACRO(U16, U16)
	__BINARY_OUTPUT_STREAM_MACRO(S16, U16)
	__BINARY_OUTPUT_STREAM_MACRO(U32, U32)
	__BINARY_OUTPUT_STREAM_MACRO(S32, U32)
	__BINARY_OUTPUT_STREAM_MACRO(F32, U32)
	__BINARY_OUTPUT_STREAM_MACRO(F64, U64)
	__BINARY_OUTPUT_STREAM_MACRO(U64, U64)
	__BINARY_OUTPUT_STREAM_MACRO(S64, U64)

#undef __BINARY_OUTPUT_STREAM_MACRO

	/// Write an object to the archive.
	/// Calls the object's Serialise function.
	template<typename T>
	BinaryOutputArchive_t& operator << (T& obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;

		if (Arch::SerialisationInfo<T_NC>::GetSerialisationType() != Arch::SERIALISATION_TYPE_PRIMITIVE)
		{
			*this << (U16) 'AR';
			*this << (U16) Arch::SerialisationInfo<T_NC>::GetVersion();
		}

		U32 currentVersion = GetVersion();
		SetVersion(Arch::SerialisationInfo<T_NC>::GetVersion());

		Serialise(*this, const_cast<T_NC&>(obj));
		
		SetVersion(currentVersion);

		return *this;
	}

	/// Write an object through a pointer.
	/// Derived classes may be written through a base class pointer, if the types were registered as polymorphic.
	template<typename T>
	BinaryOutputArchive_t& operator << (T* obj)
	{
		Serialise(*this, obj);
		return *this;
	}

	/// Write a string of characters to the archive.
	BinaryOutputArchive_t& operator << (const char* p)
	{
		U32 n = (U32) strlen(p);
		*this << n;
		Write(p, n);
		return *this;
	}

	/// Write a string of characters to the archive.
	BinaryOutputArchive_t& operator << (char* p)
	{
		U32 n = (U32) strlen(p);
		*this << n;
		Write(p, n);
		return *this;
	}

	/// Write a name-value pair.
	/// For a binary archive, the name is ignored.
	template<typename T>
	BinaryOutputArchive_t& operator << (const Nvp<T>& obj)
	{
		*this << obj.value;
		return *this;
	}

	/// Write a name-value pair.
	/// For a binary archive, the name is ignored.
	template<typename T>
	BinaryOutputArchive_t& operator << (Nvp<T>& obj)
	{
		*this << obj.value;
		return *this;
	}

	BinaryOutputArchive_t& operator << (SerialisationType t)
	{
		*this << (U8) t;
		return *this;
	}

	/// Equivalent to << operator.
	template<typename T>
	BinaryOutputArchive_t& operator & (T& obj)
	{
		*this << obj;
		return *this;
	}

	/// Equivalent to << operator.
	template<typename T>
	BinaryOutputArchive_t& operator & (T* obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;
		*this << (T_NC*) obj;
		return *this;
	}

	template<typename T>
	BinaryOutputArchive_t& operator & (Nvp<T> obj)
	{
		*this << obj.value;
		return *this;
	}

	/// The version number of the object currently being written.
	U16 GetVersion() const {return m_version;}

	bool GetEndianSwap() const
	{
		return ENDIANSWITCH::swap;
	}

private:
	// serialisation functions for all basic core types, checks buffer overflow once per type
	template <typename TYPE, typename STYPE>
	void SerialiseAtom(TYPE _t)
	{
		U32 SIZE = sizeof(TYPE);
		STYPE t = (STYPE&)_t;
		ENDIANSWITCH::EndianSwap(t);
		if ((m_stream.m_pBufferEnd-m_stream.m_pCurBuffer)<(int)SIZE)
			Write((U8*)&t, SIZE);
		else
		{
			*((STYPE*)m_stream.m_pCurBuffer) = t;
			m_stream.m_pCurBuffer += SIZE;
		}
	}

	void SetVersion(U16 i) {m_version = i;}

	Sys::I_BinaryStream& m_stream;
	U16 m_version;
};

///////////////////////////////////////////////////////////////////////

// deserialize a binary stream and switch endian-ness
template<typename ENDIANSWITCH>
class BinaryInputArchive_t
{
private:
	FRIEND_CLASS BinaryInputArchive;

	BinaryInputArchive_t(Sys::I_BinaryStream& in) : m_stream(in), m_failed(false) {}

	void Open()
	{
		char arch[4];
		Read(&arch, 4);
		assert(strncmp(arch, "ARCH", 4) == 0);
		S32 version;
		*this >> version;
		m_version = (U16) version;
		assert(m_version == 0);
	}

	void Close() {}

public:
	/// Read unformatted data from the archive.
	inline void* Read(void* pData, U32 nBytes)
	{
		assert(!m_stream.IsWriting());
		if (pData)
		{
			m_stream.Read(pData, nBytes);
			return pData;
		}
		else
		{
			void* p = m_stream.m_pCurBuffer;
			m_stream.Read(0, nBytes);
			return p;
		}
	}

	inline void Align(U32 bytes)
	{
		U64 size = m_stream.GetPosition();
		U64 aligned = Btl::AlignUp(size, (U64)bytes);
		Read(0, U32(aligned - size));
	}

#define __BINARY_INPUT_STREAM_MACRO(TYPE, STYPE) \
	FORCE_INLINE BinaryInputArchive_t& operator >> (TYPE& t) {SerialiseAtom<TYPE, STYPE>(t); return *this;} \
	FORCE_INLINE BinaryInputArchive_t& operator >> (const TYPE& t) {SerialiseAtom<TYPE, STYPE>(t); return *this;}
	
	__BINARY_INPUT_STREAM_MACRO(U8,  U8)
	__BINARY_INPUT_STREAM_MACRO(S8,  U8)
	__BINARY_INPUT_STREAM_MACRO(char,U8)
	__BINARY_INPUT_STREAM_MACRO(bool,U8)
	__BINARY_INPUT_STREAM_MACRO(U16, U16)
	__BINARY_INPUT_STREAM_MACRO(S16, U16)
	__BINARY_INPUT_STREAM_MACRO(U32, U32)
	__BINARY_INPUT_STREAM_MACRO(S32, U32)
	__BINARY_INPUT_STREAM_MACRO(F32, U32)
	__BINARY_INPUT_STREAM_MACRO(F64, U64)
	__BINARY_INPUT_STREAM_MACRO(U64, U64)
	__BINARY_INPUT_STREAM_MACRO(S64, U64)

#undef __BINARY_INPUT_STREAM_MACRO

	/// Read an object from the archive.
	template<typename T>
	BinaryInputArchive_t& operator >> (T& obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;

		U16 version = 0;

		assert(!m_failed);

		if (SerialisationInfo<T>::GetSerialisationType() != Arch::SERIALISATION_TYPE_PRIMITIVE)
		{
			U16 test; *this >> test; 
			assert(test == 'AR');
			*this >> version;
			U16 maxVersion = Arch::SerialisationInfo<T_NC>::GetVersion();
			assert(version <= maxVersion);
		}

		U32 currentVersion = GetVersion();
		SetVersion(version);

		Deserialise(*this, const_cast<T_NC&>(obj));

		SetVersion(currentVersion);
		return *this;
	}

	/// Read an object through a pointer.
	/// Derived classes may be created, if the types were registered as polymorphic.
	/// If the object is not present, the pointer will be initialised to zero.
	template<typename T>
	FORCE_INLINE BinaryInputArchive_t& operator >> (T*& obj)
	{
		Deserialise(*this, obj);
		return *this;
	}

	/// Read a string of characters from the archive.
	/// Space allocated with operator new [].
	/// For an empty string, the pointer will be set to zero.
	BinaryInputArchive_t& operator >> (char*& p)
	{
		U32 n;
		*this >> n;
		p = new char[n + 1];
		Read(p, n);
		p[n] = 0;
		return *this;
	}

	FORCE_INLINE BinaryInputArchive_t& operator >> (SerialisationType& t)
	{
		U8 temp;
		*this >> temp;
		t = (SerialisationType) temp;
		return *this;
	}

	/// Read a name-value pair.
	/// For a binary archive, the name is ignored.
	template<typename T>
	FORCE_INLINE BinaryInputArchive_t& operator >> (Nvp<T>& obj)
	{
		*this >> obj.value;
		return *this;
	}

	/// Equivalent to >> operator.
	template<typename T>
	FORCE_INLINE BinaryInputArchive_t& operator & (T& obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;
		*this >> const_cast<T_NC&>(obj);
		return *this;
	}

	/// Equivalent to >> operator.
	template<typename T>
	FORCE_INLINE BinaryInputArchive_t& operator & (T*& obj)
	{
		*this >> obj;
		return *this;
	}

	template<typename T>
	FORCE_INLINE BinaryInputArchive_t& operator & (Nvp<T> obj)
	{
		*this >> obj.value;
		return *this;
	}

	/// The version number of the object currently being read.
	FORCE_INLINE U16 GetVersion() const {return m_version;}

	FORCE_INLINE const Sys::I_BinaryStream& Stream() const { return m_stream; }
	FORCE_INLINE Sys::I_BinaryStream& Stream() { return m_stream; }

	FORCE_INLINE bool GetEndianSwap() const
	{
		return ENDIANSWITCH::swap;
	}

	void SetFailed()
	{
		m_failed = true;
	}

	bool GetFailed()
	{
		return m_failed;
	}

private:
	// deserialisation functions for all basic core types, checks buffer underflow once per type
	template <typename TYPE, typename STYPE>
	FORCE_INLINE void SerialiseAtom(TYPE& t)
	{
		U32 SIZE = sizeof(TYPE);
		STYPE* p = (STYPE*)&t;
		if ((m_stream.m_pBufferEnd-m_stream.m_pCurBuffer) < (int)SIZE)
			Read((U8*)p, SIZE);
		else
		{
			//*p = *((STYPE*)m_stream.m_pCurBuffer);
			memcpy(p, m_stream.m_pCurBuffer, sizeof(STYPE));
			m_stream.m_pCurBuffer += SIZE;
		}
		ENDIANSWITCH::EndianSwap(*p);
	}

	void SetVersion(U16 i) {m_version = i;}

	Sys::I_BinaryStream& m_stream;
	U16 m_version;
	bool m_failed;
};

//////////////////////////////////////////////////////////////////////////////

class BinaryOutputArchive
{
public:
	BinaryOutputArchive(Sys::I_BinaryStream& out, bool endianSwap = false) :	m_endianSwapArchive(out), 
																				m_noEndianSwapArchive(out), 
																				m_endianSwap(endianSwap)
	{
		{
			out.Write("ARCH", 4);
			U32 i = 1;
			out.Write(&i, 4);
			U32 swap = endianSwap ? 0xffffffff : 0;
			out.Write(&swap, 4);
		}

		if (m_endianSwap) m_endianSwapArchive.Open();
		else m_noEndianSwapArchive.Open();
	}

	~BinaryOutputArchive()
	{
		if (m_endianSwap) m_endianSwapArchive.Close();
		else m_noEndianSwapArchive.Close();
	}

	template<typename T>
	BinaryOutputArchive& operator << (T& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	template<typename T>
	BinaryOutputArchive& operator & (T& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	template<typename T>
	BinaryOutputArchive& operator << (T* t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;		
	}

	template<typename T>
	BinaryOutputArchive& operator & (T* t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	bool GetEndianSwap() const
	{
		return m_endianSwap;
	}

private:
	BinaryOutputArchive_t<Ut::DoEndianSwap> m_endianSwapArchive;
	BinaryOutputArchive_t<Ut::NoEndianSwap> m_noEndianSwapArchive;
	bool m_endianSwap;
};

////////////////////////////////////////////////////////////////////

class BinaryInputArchive
{
public:
	BinaryInputArchive(Sys::I_BinaryStream& out) :	m_endianSwapArchive(out), 
													m_noEndianSwapArchive(out)
																	
	{
		{
			char arch[4];
			out.Read(&arch, 4);
			assert(strncmp(arch, "ARCH", 4) == 0);
			U32 i;
			out.Read(&i, 4);
			m_endianSwap = !(i == 1);
			U32 swap;
			out.Read(&swap, 4);
			if (swap) m_endianSwap = !m_endianSwap;
		}

		if (m_endianSwap) m_endianSwapArchive.Open();
		else m_noEndianSwapArchive.Open();
	}

	~BinaryInputArchive()
	{
		if (m_endianSwap) m_endianSwapArchive.Close();
		else m_noEndianSwapArchive.Close();
	}

	inline void* Read(void *pData, U32 nBytes)
	{
        if (m_endianSwap) return m_endianSwapArchive.Read(pData, nBytes);
		else return m_noEndianSwapArchive.Read(pData, nBytes);
	}

	template<typename T>
	BinaryInputArchive& operator >> (T& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	template<typename T>
	BinaryInputArchive& operator & (T& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	template<typename T>
	BinaryInputArchive& operator & (Nvp<T> obj)
	{
		*this >> obj.value;
		return *this;
	}

	template<typename T>
	BinaryInputArchive& operator >> (T*& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;		
	}

	template<typename T>
	BinaryInputArchive& operator & (T*& t)
	{
		if (m_endianSwap) m_endianSwapArchive & t;
		else m_noEndianSwapArchive & t;
		return *this;
	}

	const Sys::I_BinaryStream& Stream() const 
	{
		if (m_endianSwap) return m_endianSwapArchive.Stream(); 
		else return m_noEndianSwapArchive.Stream();
	}

	Sys::I_BinaryStream& Stream() 
	{ 
		if (m_endianSwap) return m_endianSwapArchive.Stream(); 
		else return m_noEndianSwapArchive.Stream();
	}

	bool GetEndianSwap() const
	{
		return m_endianSwap;
	}

private:
	BinaryInputArchive_t<Ut::DoEndianSwap> m_endianSwapArchive;
	BinaryInputArchive_t<Ut::NoEndianSwap> m_noEndianSwapArchive;
	bool m_endianSwap;
};

////////////////////////////////////////////////////////////////////////////////////

}

#endif

