#include "stdafx.h"
#include "Display/Buffer/DynamicVB.h"
#include "Display/Device/Device.h"

namespace Display
{

C_DynamicVB::C_DynamicVB() : I_Renderer("DynamicVB"),
	m_BufferSize(256 * 1024),
	m_BufferCount(12),
	m_BlockCount(128),
	m_Created(false),
	m_BeginBlock(false),
	m_CurrentOffset(0),
	m_CurrentBuffer(0),
	m_CurrentBlock(0),
	m_pCurrentPointer(NULL),
	m_pStartPointer(NULL)
{
	m_Buffers.Resize(m_BufferCount);
	m_Blocks.Resize(m_BlockCount);
}

C_DynamicVB::~C_DynamicVB()
{
	DestroyRenderer();
}

bool C_DynamicVB::OnCreate()
{
	RESOURCE_MEMORY_ID("DynamicVB");

	return true;
}

bool C_DynamicVB::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("DynamicVB");

	m_Created = true;

	for (U32 bufferIndex = 0; bufferIndex < m_BufferCount; bufferIndex++)
	{
		if (!Device()->CreateVertexBuffer(m_BufferSize, E_Usage_Video | E_Usage_Dynamic, &m_Buffers[bufferIndex]))
		{
			LOGFATAL(g_Log, "DynamicVB : Failed to create vertex buffer %i\n", m_BufferSize);
			return false;
		}
	}

	return true;
}

void C_DynamicVB::OnDestroyVolatile()
{
	RESOURCE_MEMORY_ID("DynamicVB");

	m_Created = false;

	for (U32 bufferIndex = 0; bufferIndex < m_BufferCount; bufferIndex++)
	{
		Device()->DestroyVertexBuffer(&m_Buffers[bufferIndex]);
	}
}

void C_DynamicVB::OnDestroy()
{
	RESOURCE_MEMORY_ID("DynamicVB");
}

void C_DynamicVB::SwitchBuffer()
{
	++m_CurrentBuffer;
	if(m_CurrentBuffer == m_BufferCount)
		m_CurrentBuffer = 0;
}

void C_DynamicVB::Begin()
{
	assert(m_BeginBlock == false);
	m_CurrentOffset = 0;
	Device()->SetVertexBuffer(0, NULL, 0, 0);

	assert(m_CurrentBuffer < m_BufferCount);
	m_pCurrentPointer = m_Buffers[m_CurrentBuffer].Lock(0, 0);
	m_pStartPointer = m_pCurrentPointer;
}

void C_DynamicVB::End()
{
	assert(m_BeginBlock == false);
	assert(m_CurrentBuffer < m_BufferCount);
	m_Buffers[m_CurrentBuffer].Unlock();
}

void* C_DynamicVB::BeginBlock(const char* debugName, const U32 vertexSize, const U32 vertexCount)
{
	assert(m_BeginBlock == false);

	if (vertexCount == 0)
	{
		LOGWARN(g_Log, "DynamicVB : Trying to render zero vertices\n");
		return NULL;
	}
	else if (m_CurrentOffset + vertexCount * vertexSize > m_BufferSize)
	{
		if (vertexCount * vertexSize > m_BufferSize)
		{
			LOGWARN(g_Log, "DynamicVB : Trying to render too many vertices\n");
			return NULL;
		}
		else
		{
			End();
			SwitchBuffer();
			Begin();
		}
	}

	assert(m_CurrentBlock < m_BlockCount);
#ifndef CFG_SPEED
#ifdef PLATFORM_PS3
	strcpy(m_Blocks[m_CurrentBlock].m_DebugName, debugName);	
#else
	strcpy_s(m_Blocks[m_CurrentBlock].m_DebugName, debugName);	
#endif
#endif
	m_Blocks[m_CurrentBlock].m_BufferOffset = m_CurrentOffset;
	m_Blocks[m_CurrentBlock].m_BufferIndex= m_CurrentBuffer;
	m_Blocks[m_CurrentBlock].m_VertexSize = vertexSize;
	m_Blocks[m_CurrentBlock].m_VertexCount = vertexCount;

	m_BeginBlock = true;

	return m_pCurrentPointer;
}

U32 C_DynamicVB::EndBlock(void* pPointer)
{
	assert(m_BeginBlock);
	m_BeginBlock = false;

	if (!pPointer || pPointer == m_pCurrentPointer)
		return 0xFFFFFFFF;

	assert(m_CurrentBlock < m_BlockCount);


#ifndef CFG_SPEED
	U64 used = (U64)pPointer - (U64)m_pCurrentPointer;
	U64 reserved = (U64)m_Blocks[m_CurrentBlock].m_VertexCount * (U64)m_Blocks[m_CurrentBlock].m_VertexSize;
	if(used < reserved)
		LOGWARN(g_Log, "DynamicVB : Client reserved %i bytes, but only used %i bytes!\n", reserved, used);
	else if(used > reserved)
		LOGERROR(g_Log, "DynamicVB : Client requested %i bytes, then used %i bytes!\n", reserved, used);
#endif

	m_pCurrentPointer = pPointer;
	m_CurrentOffset += m_Blocks[m_CurrentBlock].m_VertexCount * m_Blocks[m_CurrentBlock].m_VertexSize;//(U32)pPointer - (U32)m_pStartPointer;


	U32 block = m_CurrentBlock;
	++m_CurrentBlock;
	if(m_CurrentBlock == m_BlockCount)
		m_CurrentBlock = 0;

	return block;
}

void C_DynamicVB::DrawBlock(const E_PrimitiveType primitiveType, const U32 blockIndex)
{
	assert(m_BeginBlock == false);
	assert(blockIndex < m_BlockCount);
	assert(m_Blocks[blockIndex].m_BufferIndex < m_BufferCount);
	Device()->SetVertexBuffer(0, &m_Buffers[m_Blocks[blockIndex].m_BufferIndex], m_Blocks[blockIndex].m_BufferOffset, m_Blocks[blockIndex].m_VertexSize);

	assert(m_Blocks[blockIndex].m_VertexCount > 0);
	Device()->DrawPrimitive(primitiveType, 0, m_Blocks[blockIndex].m_VertexCount); // This function takes the vertex count not index count.
}

void C_DynamicVB::DrawIndexedBlock(const E_PrimitiveType primitiveType, const U32 blockIndex, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount)
{
	assert(m_BeginBlock == false);
	assert(blockIndex < m_BlockCount);
	assert(m_Blocks[blockIndex].m_BufferIndex < m_BufferCount);
	Device()->SetVertexBuffer(0, &m_Buffers[m_Blocks[blockIndex].m_BufferIndex], m_Blocks[blockIndex].m_BufferOffset, m_Blocks[blockIndex].m_VertexSize);
	Device()->SetIndexBuffer(pIndexBuffer);

	assert(m_Blocks[blockIndex].m_VertexCount > 0);
	//Device()->DrawPrimitive(primitiveType, 0, m_Blocks[blockIndex].m_VertexCount); // This function takes the vertex count not index count.
	Device()->DrawIndexedPrimitive(primitiveType, 0, 0, m_Blocks[blockIndex].m_VertexCount, 0, indexCount);

}

void C_DynamicVB::SetBufferSize(U32 bufferSize, U32 bufferCount)
{
	if(!m_Created)
	{
		m_BufferSize = bufferSize;
		m_BufferCount = bufferCount;

		m_Buffers.Resize(m_BufferCount);
	}
}


C_DynamicVB* g_pDynamicVB = NULL;

void DynamicVB(C_DynamicVB* pDynamicVB)
{
	g_pDynamicVB = pDynamicVB;
}

}; // namespace Display
