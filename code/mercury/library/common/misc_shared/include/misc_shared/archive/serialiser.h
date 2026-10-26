#ifndef ARCHIVE_SERIALISER_H
#define ARCHIVE_SERIALISER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/utility/endian.h"
#include "btl/utility/types.h"
#include "system/serialise/i_binarystream.h"
#include "system/serialise/C_MemoryStream.h"
#include "system/serialise/BinarySerialise.h"
#include "serialisationinfo.h"
#include "misc_shared/compression/deflate.h"
#include "misc_shared/compression/inflate.h"
#include "btl/utility/autopointer.h"
#include "misc_shared/compression/xmemcompress.h"

namespace Arch
{

enum CompressionType
{
	NO_COMPRESSION,
	ZLIB_COMPRESSION,
	XMEM_COMPRESSION,
};

// Serialiser is a way to write low-level data to a stream, which can then be written to an archive. 
// It does not support versioning or polymorphic types and is always a binary stream in memory.
// The data can be aligned and compressed within the archive.
class Serialiser
{
public:
	Serialiser(bool m_endianSwap = false, U32 alignment = 0, CompressionType compression = NO_COMPRESSION, S32 maxDecompressionSize = 32 * 1024);

	template<typename T>
	Serialiser& operator << (T& t)
	{
		if (m_endianSwap) m_endianSwapStream << t;
		else m_noEndianSwapStream << t;
		return *this;
	}

	inline bool Write(const void *pData, U32 bytes)
	{
		assert(m_memoryStream.IsWriting());
        return m_memoryStream.Write(pData, bytes);
	}

	Sys::I_BinaryStream& Stream() {return m_memoryStream;}
	U64 GetBufferSize() const {return m_memoryStream.GetBufferSize();}

	template <typename Archive>
	friend void Serialise(Archive& ar, Serialiser& t);

private:
	U32 Compress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize);

	Sys::C_MemoryStream m_memoryStream;
	Sys::BinarySerialiser<Ut::DoEndianSwap> m_endianSwapStream;
	Sys::BinarySerialiser<Ut::NoEndianSwap> m_noEndianSwapStream;
	U32 m_alignment;
	CompressionType m_compressionType;
	S32 m_maxSize;
	bool m_endianSwap;
};

template <typename Archive>
void Serialise(Archive& ar, Serialiser& t)
{
	U32 size = (U32) t.m_memoryStream.GetBufferSize();
	ar & size;
	ar & t.m_alignment;
	ar & (U32&) t.m_compressionType;
	ar & t.m_endianSwap;

	switch (t.m_compressionType)
	{
		case NO_COMPRESSION:
		{
			ar.Align(t.m_alignment);
			ar.Write(t.m_memoryStream.GetBufferPtr(), size);
			break;
		}
		case ZLIB_COMPRESSION:
		case XMEM_COMPRESSION:
		{
			S32 maxSize = t.m_maxSize;
			S32 count = Btl::AlignUp(size, maxSize) / maxSize;
			ar << count;

			for (int i = 0; i < count; i++)
			{
				U32 bufferSize = maxSize + maxSize / 50 + 128;
				U8* pOutData = new U8[bufferSize];
				U8* pInData = ((U8*)t.m_memoryStream.GetBufferPtr()) + i * maxSize;
				U32 originalSize = (i == (count - 1)) ? (size - (count - 1) * maxSize) : maxSize;
				U32 compressedSize = t.Compress(pInData, originalSize, pOutData, bufferSize);

				ar << originalSize;
				ar << compressedSize;
				ar.Align(t.m_alignment);
				ar.Write(pOutData, compressedSize);

				delete pOutData;
			}

			break;
		}
		default:
			assert(0);
	}
}

///////////////////////////////////////////////////////////////////////////

class Deserialiser
{
public:
	Deserialiser(bool endianSwap = false);
	~Deserialiser();

	template<typename T>
	Serialiser& operator >> (T& t)
	{
		GetData();
		if (m_endianSwap) m_endianSwapStream >> t;
		else m_noEndianSwapStream >> t;
		return *this;
	}

	inline bool Read(void *pData, U32 bytes)
	{
		GetData();
		assert(!m_memoryStream.IsWriting());
        return m_memoryStream.Read(pData, bytes);
	}

	inline U32 GetSize() const {return m_size;}

	Sys::I_BinaryStream& Stream() {GetData(); return m_memoryStream;}
 	CompressionType GetCompressionType() const {return m_compressionType;}

	void BeginDecompression(Scheduler::PolicyHandle hPolicy = 0);
	void BeginDecompression(U8* pDst, U32 size, Scheduler::PolicyHandle hPolicy, Scheduler::GroupHandle hGroup);

	template <typename Archive>
	friend void Deserialise(Archive& ar, Deserialiser& t);

private:
	void GetData();
	void DecompressFragments(Scheduler::PolicyHandle hPolicy);

	Sys::C_MemoryStream m_memoryStream;
	Sys::BinaryDeserialiser<Ut::DoEndianSwap> m_endianSwapStream;
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> m_noEndianSwapStream;

	// read from archive
	U32 m_size;

	struct Fragment
	{
		U32 originalSize;
		U32 compressedSize;
		U8* pSrc;
		Btl::AutoPointer<U8, Btl::ArrayAllocator<U8> > pSrcBuffer;
	};

	Btl::Vector<Fragment> m_fragments;
	Fragment m_fragment;
	int m_fragmentCount;
	
	CompressionType m_compressionType;
	bool m_endianSwap;

	// temp
	Btl::AutoPointer<U8, Btl::ArrayAllocator<U8> > m_pDstBuffer;
	
	U8* m_pDst;
	Scheduler::GroupHandle m_group;
	Scheduler::GroupHandle m_newGroup;
};

template <typename Archive>
void Deserialise(Archive& ar, Deserialiser& t)
{
	U32 alignment;

	ar & t.m_size;
	ar & alignment;
	ar & (U32&) t.m_compressionType;
	ar & t.m_endianSwap;

	switch (t.m_compressionType)
	{
		case NO_COMPRESSION:
		{
			ar.Align(alignment);

			if (ar.Stream().Name() == "MemoryStream")
			{
				void* p = ar.Read(0, t.m_size);
				t.m_memoryStream.SetBuffer((U8*)p, t.m_size);
				t.m_pDst = (U8*)p;
			}
			else
			{
				Allocate(t.m_pDstBuffer, t.m_size);
				ar.Read(Pointer(t.m_pDstBuffer), t.m_size);
				t.m_memoryStream.SetBuffer(Pointer(t.m_pDstBuffer), t.m_size);
				t.m_pDst = Pointer(t.m_pDstBuffer);
			}

			break;
		}
		case ZLIB_COMPRESSION:
		case XMEM_COMPRESSION:
		{
			ar >> t.m_fragmentCount;

			if (t.m_fragmentCount > 1)
				t.m_fragments.Resize(t.m_fragmentCount);

			for (int i = 0; i < t.m_fragmentCount; i++)
			{
				Deserialiser::Fragment* pFragment = (t.m_fragmentCount == 1) ? &t.m_fragment : &t.m_fragments[i];

				ar >> pFragment->originalSize;
				ar >> pFragment->compressedSize;
				assert(pFragment->compressedSize);
				ar.Align(alignment);

				if (ar.Stream().Name() == "MemoryStream")
				{
					pFragment->pSrc = (U8*) ar.Read(0, pFragment->compressedSize);
				}
				else
				{
					Allocate(pFragment->pSrcBuffer, pFragment->compressedSize);
					ar.Read(Pointer(pFragment->pSrcBuffer), pFragment->compressedSize);
					pFragment->pSrc = Pointer(pFragment->pSrcBuffer);
				}
			}

			break;
		}
		default:
			assert(0);
	}
}

}

SERIALISATION_VERSION(Arch::Serialiser, 0, "Arch::Serialiser")

#endif