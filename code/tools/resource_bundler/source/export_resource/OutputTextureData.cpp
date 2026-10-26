#include "stdafx.h"

#include "OutputTextureData.h"

using namespace ResourceBundler;


C_OutputTextureData::C_OutputTextureData()
{
	m_pBuf = NULL;
	m_Size = 0;

	m_NumImages = 0;
	m_numMipLevels = 0;

	m_pBufPtr = NULL;
	m_BufSize = NULL;
}

C_OutputTextureData::~C_OutputTextureData()
{
	free(m_pBuf);

	free(m_pBufPtr);
	free(m_BufSize);
}

const U32 C_OutputTextureData::GetDataSize() const
{
	return m_Size;
}

const U8* C_OutputTextureData::GetDataPtr() const
{
	return m_pBuf;
}

const U32 C_OutputTextureData::GetMipSize(S32 level/*=0*/, S32 image/*=0*/) const
{
	return m_BufSize[image * m_numMipLevels + level];
}

const U8* C_OutputTextureData::GetMipPtr(S32 level/*=0*/, S32 image/*=0*/) const
{
	return m_pBufPtr[image * m_numMipLevels + level];
}

bool C_OutputTextureData::AllocateStorage(S32 numImages, S32 numLevels)
{
	m_NumImages = numImages;
	m_numMipLevels = numLevels;

	m_pBufPtr = (U8**)malloc(sizeof(U8*) * numImages*numLevels);
	m_BufSize = (U32*)malloc(sizeof(U32) * numImages*numLevels);

	memset(m_pBufPtr, 0, sizeof(U8*) * numImages*numLevels);

	return true;
}

U8* C_OutputTextureData::ReallocateStorage(U32 newSize)
{
	if (m_pBuf == NULL)
	{
		m_pBuf = (U8*) malloc(newSize);
		assert(m_pBuf);
	}
	else
	{
		m_pBuf = (U8*) realloc(m_pBuf, newSize);
		assert(m_pBuf);
	}

	m_Size = newSize;

	return m_pBuf;
}

U8* C_OutputTextureData::IncreaseStorage(U32 size)
{
	U32 oldSize = m_Size;
	assert(oldSize == 0);
	U8 *pPtr = ReallocateStorage(oldSize + size);

	if (!pPtr)
		return 0;

	return pPtr + oldSize;
}


bool C_OutputTextureData::SetMipInfo(S32 image, S32 level, U8 *pPtr, U32 size)
{
	m_pBufPtr[image * m_numMipLevels + level] = pPtr;
	m_BufSize[image * m_numMipLevels + level] = size;

	return true;
}

