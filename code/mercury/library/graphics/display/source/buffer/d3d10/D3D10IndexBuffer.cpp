#include "stdafx.h"
#include "Display/Buffer/D3D10/D3D10IndexBuffer.h"

namespace NewDisplay
{
namespace D3D10
{

C_IndexBuffer::C_IndexBuffer() : m_pd3dBuffer(NULL)
{
#ifdef DEVICE_DEBUG
	m_Mapped	= false;
	m_pHeader	= NULL;
#endif
}

C_IndexBuffer::~C_IndexBuffer()
{
	assert(!IsCreated() && "The d3d index buffer has not been destroyed");

#ifdef DEVICE_DEBUG
	SAFE_DELETE_PTR(m_pHeader);
#endif
}

void* C_IndexBuffer::Lock(const U32 offset, const U32 size)
{
	assert(IsCreated());
#ifdef DEVICE_DEBUG
	assert(!m_Mapped);
	m_Mapped = true;
#endif
	assert(offset + size < GetSize());
	
	U8* pData = NULL;
	const HRESULT hr = m_pd3dBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&pData); // TODO: Set correct map type.
	return &pData[offset];
}

void* C_IndexBuffer::Lock(const U32 size)
{
	return Lock(0, size);
}

void* C_IndexBuffer::Lock()
{
	return Lock(0, 0);
}

void C_IndexBuffer::Unlock()
{
	assert(IsCreated());
#ifdef DEVICE_DEBUG
	assert(m_Mapped);
	m_Mapped = false;
#endif

	m_pd3dBuffer->Unmap();
}

}; // namespace D3D10
}; // namespace NewDisplay
