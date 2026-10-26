#include "stdafx.h"
#include "Display/Buffer/DynamicVertexBuffer.h"
#include "Display/Device/Device.h"

#ifdef PLATFORM_PS3
//#define USE_SINGLE_BUFFER
#endif

#ifdef USE_SINGLE_BUFFER
S32 g_FrameUsage = 0;
S32 g_FrameUsages[3] = {0,0,0};
#endif

#ifndef CFG_GOLD
Debug::Bool g_debugDisplayShowDynamicVertexBufferUsage("Graphics.Display.ShowDynamicVertexBufferUsage", false);
#endif

namespace Display
{

C_DynamicVertexBuffer::C_DynamicVertexBuffer() :
	m_Locked(false),
	m_Size(0),
	m_Offset(0),
	m_VertexSize(0),
	m_VertexCount(0),
	m_VertexStride(0),
	m_VertexBuffer(0),
	m_PrimitiveType(E_PrimitiveType_Unknown),
	m_pVertexDeclaration(NULL)
{
}

C_DynamicVertexBuffer::~C_DynamicVertexBuffer()
{
	assert(!IsCreated() && "The dynamic vertex buffer has not been destroyed");
}

bool C_DynamicVertexBuffer::Create(const U32 size)
{
	assert(size > 0);
	m_Size = size;

	U32 flags = E_Usage_Dynamic;

	#if defined(PLATFORM_PC)
	flags |= E_Usage_Video;
	#elif defined(PLATFORM_PS3)
	flags |= E_Usage_System;
	#endif
	
	if(!Device()->CreateVertexBuffer(m_Size, flags, &m_VertexBuffers[0]))
		return false;
#ifndef USE_SINGLE_BUFFER
	if(!Device()->CreateVertexBuffer(m_Size, flags, &m_VertexBuffers[1]))
		return false;
#endif
	
	return true;
}

void C_DynamicVertexBuffer::Destroy()
{
	Device()->DestroyVertexBuffer(&m_VertexBuffers[0]);
#ifndef USE_SINGLE_BUFFER
	Device()->DestroyVertexBuffer(&m_VertexBuffers[1]);
#endif
}

void* C_DynamicVertexBuffer::Begin(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration)
{
	assert(!m_Locked);
	m_Locked = true;
	
	C_VertexBuffer* pVertexBuffer = GetVertexBuffer();
	assert(pVertexBuffer->IsCreated());

	assert(vertexCount);
	m_VertexCount = vertexCount;

	assert(vertexStride);
	m_VertexStride = vertexStride;

	assert(pVertexDeclaration);
	m_pVertexDeclaration = pVertexDeclaration;

	m_VertexSize = m_VertexCount * m_VertexStride;

	m_PrimitiveType = primitiveType;

	#ifdef PLATFORM_PC
	//m_Offset = 0;
	#endif

#ifdef USE_SINGLE_BUFFER
	U32 totalSize = pVertexBuffer->GetSize(); // This is the total size of the vertex buffer.
	S32 freeSize = totalSize-(g_FrameUsages[0]+g_FrameUsages[1]+g_FrameUsages[2]); // This is how much memory is currently free.
	
	if((S32)m_VertexSize > freeSize) // We want more than is free.
	{
		m_Locked = false;
		return NULL;
	}

	U32 wastedSize = 0;
	U32 newOffset = m_Offset;
	
	if(newOffset+m_VertexSize > totalSize) // We've gone over the end.
	{
		wastedSize = totalSize-newOffset; // How much wasted space is there.
		freeSize -= wastedSize; // We've lost this space.
		newOffset = 0; // Lets start again from the beginning.
	}

	if((S32)m_VertexSize > freeSize) // We want more than is free.
	{
		m_Locked = false;
		return NULL;
	}

	U8* pVertex = (U8*)pVertexBuffer->Lock(newOffset, m_VertexSize);
	if(pVertex)
	{
		m_Offset = newOffset; // This is our new offset.
		g_FrameUsage += wastedSize+m_VertexSize; // How much memory did this call use.
		return pVertex;
	}
	else
	{
		m_Locked = false;
		return NULL;
	}
#else
	if(m_Offset+m_VertexSize <= pVertexBuffer->GetSize())
	{
		#ifdef PLATFORM_PC
		U32 flags = D3DLOCK_NOOVERWRITE;
		if (m_Offset == 0)
			flags = D3DLOCK_DISCARD;
		return pVertexBuffer->Lock(m_Offset, m_VertexSize, flags);
		#else
		return pVertexBuffer->Lock(m_Offset, m_VertexSize);
		#endif
	}
	else
	{
		m_Locked = false;
		return NULL;
	}
#endif
}

void C_DynamicVertexBuffer::End()
{
	assert(m_Locked);
	m_Locked = false;
	
	C_VertexBuffer* pVertexBuffer = GetVertexBuffer();
	assert(pVertexBuffer->IsCreated());

	pVertexBuffer->Unlock();

	m_Offset += m_VertexSize;
}

void C_DynamicVertexBuffer::Swap()
{
#ifdef USE_SINGLE_BUFFER
	g_FrameUsages[2] = g_FrameUsages[1];
	g_FrameUsages[1] = g_FrameUsages[0];
	g_FrameUsages[0] = g_FrameUsage;
	g_FrameUsage = 0;
#else
	#ifndef CFG_GOLD
	if(g_debugDisplayShowDynamicVertexBufferUsage)
	{
		LOGINFO(g_Log, "*** Dynamic vertex buffer %i usage %i out of  %i\n", m_VertexBuffer, m_Offset, m_Size);
	}
	#endif
	m_Offset = 0;
	m_VertexBuffer = m_VertexBuffer == 0 ? 1 : 0;
#endif
}

}; // namespace Display
