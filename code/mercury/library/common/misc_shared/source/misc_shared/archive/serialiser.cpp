#include "MiscSharedPch.h"
#include "misc_shared/archive/serialiser.h"

//#pragma optimize("", off)

namespace Arch
{

Serialiser::Serialiser(bool m_endianSwap, U32 alignment, CompressionType compression, S32 maxDecompressionSize) :
		m_endianSwap(m_endianSwap),
		m_alignment(alignment),
		m_compressionType(compression),
		m_maxSize(maxDecompressionSize),
		m_memoryStream(1024 * 1024),
		m_endianSwapStream(m_memoryStream), 
		m_noEndianSwapStream(m_memoryStream) {}

U32 Serialiser::Compress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
	if (m_compressionType == ZLIB_COMPRESSION)
	{
		U32 outSize;
		Compression::C_Deflate deflate;
		deflate.Create();
		deflate.SetPolicy(0);
		deflate.Start(pInData, nInSize, pOutData, nOutSize);
		deflate.Finish(outSize);
		deflate.Destroy();
		return outSize;
	}
	else if (m_compressionType == XMEM_COMPRESSION)
	{
		return Compression::XMemCompress(pInData, nInSize, pOutData, nOutSize);
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////

Deserialiser::Deserialiser(bool endianSwap) :
		m_endianSwap(endianSwap),
		m_memoryStream(),
		m_endianSwapStream(m_memoryStream), 
		m_noEndianSwapStream(m_memoryStream),
		m_pDst(0),
		m_group(-1),
		m_newGroup(-1),
		m_fragmentCount(0)
		{}

Deserialiser::~Deserialiser()
{
	bool getData = (m_pDstBuffer != 0);
	if ((m_fragmentCount == 1 && m_fragment.pSrcBuffer != 0)
		|| (m_fragments.Size() > 1 && m_fragments[0].pSrcBuffer != 0))
		getData = true;

	if (getData) GetData();
	if (m_newGroup != -1) Scheduler::DestroyGroup(m_newGroup);
}


void Deserialiser::BeginDecompression(Scheduler::PolicyHandle hPolicy)
{
	if (m_compressionType != NO_COMPRESSION)
	{
		Allocate(m_pDstBuffer, m_size);
		m_pDst = Pointer(m_pDstBuffer);

		m_newGroup = Scheduler::CreateGroup();
		m_group = m_newGroup;

		m_memoryStream.SetBuffer(m_pDst, m_size);
	}

	DecompressFragments(hPolicy);
}

void Deserialiser::BeginDecompression(U8* pDst, U32 size, Scheduler::PolicyHandle hPolicy, Scheduler::GroupHandle hGroup)
{
	if (!pDst)
		return;

	if (m_compressionType != NO_COMPRESSION)
	{
		assert(size >= m_size);

		if (hGroup == -1)
		{
			m_newGroup = Scheduler::CreateGroup();
			m_group = m_newGroup;
		}
		else
		{
			m_group = hGroup;
		}

		m_pDst = pDst;
		m_memoryStream.SetBuffer(pDst, m_size);
	}

	DecompressFragments(hPolicy);
}

void Deserialiser::DecompressFragments(Scheduler::PolicyHandle hPolicy)
{
	U8* pDst = m_pDst;

	for (int i = 0; i < m_fragmentCount; i++)
	{
		Fragment* pFragment = (m_fragmentCount == 1) ? &m_fragment : &m_fragments[i];

		if (m_compressionType == ZLIB_COMPRESSION)
		{
			Compression::C_Inflate inflate;
			inflate.Create(m_group);
			inflate.SetPolicy(hPolicy);
			inflate.Start(pFragment->pSrc, pFragment->compressedSize, pDst, pFragment->originalSize);
			//inflate.Finish();
		}
		else if (m_compressionType == XMEM_COMPRESSION)
		{
			Compression::XMemDecompress_Asynch(pFragment->pSrc, pFragment->compressedSize, pDst, pFragment->originalSize, m_group, hPolicy);
		}

		pDst += pFragment->originalSize;
	}
}

void Deserialiser::GetData()
{
	if (m_pDst == 0) 
		BeginDecompression();

	if (m_group != -1 && m_compressionType != NO_COMPRESSION)
		Scheduler::Flush(m_group);
}

}
