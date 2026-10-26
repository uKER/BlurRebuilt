
#include <stdafx.h>

#include "aftk/AftkBlobData.h"

using namespace Aftk;

C_BlobData::C_BlobData()
:
m_size(0), m_pData(0)
{
}


C_BlobData::C_BlobData(const C_BlobData &blob)
:
m_size(0), m_pData(0)
{
	*this = blob;
}


C_BlobData::~C_BlobData()
{
	if (m_pData)
	{
	#ifdef AFTK_USE_MEMORY_POOL
		C_ArrayAllocator<U8>::Free((U8*)m_pData, m_size);
	#else
		delete (U8*)m_pData;
	#endif
	}
}


const C_BlobData& C_BlobData::operator=(const C_BlobData &rhs)
{
	//delete data
	if (m_size && m_pData)
	{
	#ifdef AFTK_USE_MEMORY_POOL
		C_ArrayAllocator<U8>::Free((U8*)m_pData, m_size);
	#else
		delete (U8*)m_pData;
	#endif
		m_size = 0;
		m_pData = 0;
	}

	m_size = rhs.m_size;
	if (m_size)
	{
	#ifdef AFTK_USE_MEMORY_POOL
		m_pData = C_ArrayAllocator<U8>::Allocate(m_size);
	#else
		m_pData = new U8[m_size];
	#endif
	}
	else
		m_pData = 0;

	memcpy(m_pData, rhs.m_pData, m_size);

	return *this;
}


bool C_BlobData::operator== (const C_BlobData &rhs) const
{
	return ((m_size == rhs.m_size) &&
		((m_pData == rhs.m_pData) || (memcmp(m_pData, rhs.m_pData, m_size) == 0)));
}


bool C_BlobData::operator!= (const C_BlobData &rhs) const
{
	return !((*this) == rhs);
}


C_BlobData* C_BlobData::Clone() const
{
	C_BlobData *pBLOb = new C_BlobData(*this);
	return pBLOb;
}


Aftk::C_Status C_BlobData::SetData(U32 size, void* pData)
{
	//Early out if we already own the data
	if (m_pData == pData)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	if (m_size && m_pData)
	{
	#ifdef AFTK_USE_MEMORY_POOL
		C_ArrayAllocator<U8>::Free((U8*)m_pData, m_size);
	#else
		delete (U8*)m_pData;
	#endif
		m_size = 0;
		m_pData = 0;
	}
	m_size = size;

	if (m_size)
	{
	#ifdef AFTK_USE_MEMORY_POOL
		m_pData = C_ArrayAllocator<U8>::Allocate(m_size);
	#else
		m_pData = new U8[m_size];
	#endif
	}
	else
		m_pData = 0;

	if (m_pData)
		return Aftk::C_Status::E_Status_Success;
	else
		return Aftk::C_Status::E_Status_OutOfMemory;
}


Aftk::C_Status C_BlobData::Resize(U32 size)
{
	AFTK_ASSERT(size > m_size);
	if (size == m_size)
		return Aftk::C_Status::E_Status_Success;

	U32 oldSize = size;
	void *oldData = m_pData;

	m_size = size;

#ifdef AFTK_USE_MEMORY_POOL
	m_pData = C_ArrayAllocator<U8>::Allocate(m_size);
#else
	m_pData = new U8[m_size];
#endif

	if (m_pData == 0)
		return Aftk::C_Status::E_Status_OutOfMemory;

	memset(m_pData, 0, m_size);

	if(oldData)
	{
		memcpy(m_pData, oldData, oldSize);
	#ifdef AFTK_USE_MEMORY_POOL
		C_ArrayAllocator<U8>::Free((U8*)oldData, oldSize);
	#else
		delete (U8*)oldData;
	#endif

	}

	return Aftk::C_Status::E_Status_Success;
}


U32 C_BlobData::GetSize() const
{
	return m_size;
}


void* C_BlobData::GetDataPtr() const
{
	return m_pData;
}