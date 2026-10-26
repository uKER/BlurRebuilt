#include "stdafx.h"

#include "Dynamic/DynamicVertexBuffer.h"
#include "Display/Device/Device.h"
#include "Display/Context/Context.h"
#include "Display/DisplayToDisplay2.h"

namespace DynamicEffect
{

C_DynamicVertexBuffer::C_DynamicVertexBuffer() :
	m_VertexBufferSize(0),
	m_VertexBufferCount(0),
	m_IndexBufferSize(0),
	m_IndexBufferCount(0),
	m_BlockCount(0),
	m_BeginBlocks(0),
	m_CurrentVertexOffset(0),
	m_CurrentVertexBuffer(0),
	m_CurrentIndexBuffer(0),
	m_BeginIndexBuffer(0),
	m_CurrentFrame(0),
	m_CurrentBlock(0),
	m_BlocksGrow(false),
	m_Created(false)
{
}

C_DynamicVertexBuffer::~C_DynamicVertexBuffer()
{
	
}

bool C_DynamicVertexBuffer::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("VertexBuffer");

	m_Created = true;

	MEMORY_TRACKER_PUSH("graphics.dynamic.vertexbuffers");
	for (U32 vertexBufferIndex = 0; vertexBufferIndex < m_VertexBufferCount; vertexBufferIndex++)
	{
		if (!Display::Device()->CreateVertexBuffer(m_VertexBufferSize, Display::E_Usage_Video | Display::E_Usage_Dynamic, &m_VertexBuffers[vertexBufferIndex]))
		{
			MEMORY_TRACKER_POP("graphics.dynamic.vertexbuffers");
			LOGFATAL(Display::g_Log, "DynamicVertexBuffer : VertexBuffer : Failed to create vertex buffer %i\n", m_VertexBufferSize);
			return false;
		}

		m_VertexBuffers[vertexBufferIndex].pVertices = NULL;
		m_VertexBuffers[vertexBufferIndex].locks = 0;
	}
	MEMORY_TRACKER_POP("graphics.dynamic.vertexbuffers");

	MEMORY_TRACKER_PUSH("graphics.dynamic.indexbuffers");
	for (U32 indexBufferIndex = 0; indexBufferIndex < m_IndexBufferCount; indexBufferIndex++)
	{
		if (!Display::Device()->CreateIndexBuffer(m_IndexBufferSize, Display::E_Usage_Video | Display::E_Usage_Dynamic, Display::E_Format_Index16, &m_IndexBuffers[indexBufferIndex]))
		{
			MEMORY_TRACKER_POP("graphics.dynamic.indexbuffers");
			LOGFATAL(Display::g_Log, "DynamicVertexBuffer : IndexBuffer : Failed to create index buffer %i\n", m_IndexBufferSize);
			return false;
		}

		m_IndexBuffers[indexBufferIndex].pIndices = NULL;
		m_IndexBuffers[indexBufferIndex].locks = 0;
	}
	MEMORY_TRACKER_POP("graphics.dynamic.indexbuffers");

	return true;
}

void C_DynamicVertexBuffer::OnDestroyVolatile()
{
	RESOURCE_MEMORY_ID("VertexBuffer");

	m_Created = false;

	for (U32 vertexBufferIndex = 0; vertexBufferIndex < m_VertexBufferCount; vertexBufferIndex++)
	{
		Display::Device()->DestroyVertexBuffer(&m_VertexBuffers[vertexBufferIndex]);
	}

	for (U32 indexBufferIndex = 0; indexBufferIndex < m_IndexBufferCount; indexBufferIndex++)
	{
		Display::Device()->DestroyIndexBuffer(&m_IndexBuffers[indexBufferIndex]);
	}
}

void C_DynamicVertexBuffer::SwitchVertexBuffer()
{
	// increment buffer index
	m_VertexBuffers[m_CurrentVertexBuffer].frame = m_CurrentFrame;
	++m_CurrentVertexBuffer;
	if(m_CurrentVertexBuffer == m_VertexBufferCount)
		m_CurrentVertexBuffer = 0;

	// reset offset
	m_CurrentVertexOffset = 0;
}

void C_DynamicVertexBuffer::SwitchIndexBuffer()
{
	// increment buffer index
	++m_CurrentIndexBuffer;
	if(m_CurrentIndexBuffer == m_IndexBufferCount)
		m_CurrentIndexBuffer = 0;

	// reset offset
	m_CurrentIndexOffset = 0;
}

void C_DynamicVertexBuffer::Begin()
{
	SwitchVertexBuffer();
	SwitchIndexBuffer();
	++m_CurrentFrame;

	Display::Device()->SetVertexBuffer(0, NULL, 0, 0);

	//assert(m_BeginBlocks == 0);
	m_CurrentBlock = 0;
	m_BufferWarning = false;

	// see if we need to grow the block vector
	if(m_BlocksGrow)
	{
		m_BlockCount += 32;
		m_Blocks.Resize(m_BlockCount);
		m_BlocksGrow = false;

		LOGINFO(Display::g_Log, "DynamicVertexBuffer : Growing Blocks to %i\n", m_BlockCount);
	}

	// make sure theres no outstanding locks going on
	for(S32 i = 0; i < m_VertexBuffers.Size(); ++i)
	{
		if(m_VertexBuffers[i].locks != 0)
		{
			LOGWARN(Display::g_Log, "DynamicVertexBuffer : VertexBuffer %d was left Locked! This will cause GPU sync problems\n");
			
			m_VertexBuffers[i].Unlock();
			m_VertexBuffers[i].pVertices = NULL;
			m_VertexBuffers[i].locks = 0;
		}
	}

	for(S32 i = 0; i < m_IndexBuffers.Size(); ++i)
	{
		if(m_IndexBuffers[i].locks != 0)
		{
			LOGWARN(Display::g_Log, "DynamicVertexBuffer : IndexBuffer %d was left Locked! This will cause GPU sync problems\n");
			
			m_IndexBuffers[i].Unlock();
			m_IndexBuffers[i].pIndices = NULL;
			m_IndexBuffers[i].locks = 0;
		}
	}
}

C_DynamicVertexBufferBlock	C_DynamicVertexBuffer::Begin(const Util::C_String128& debugName, const U32 vertexSize, const U32 vertexCount, const U32 indexCount)
{
	if(m_CurrentBlock == m_BlockCount)
	{
#ifndef CFG_GOLD
		if(!m_BufferWarning)
		{
			LOGINFO(Display::g_Log, "DynamicVertexBuffer : No more blocks available this frame %s\n", debugName.Chars());
			m_BufferWarning = true;
		}
#endif

		m_BlocksGrow = true;
		return C_DynamicVertexBufferBlock();
	}

	if(vertexCount == 0)
	{
#ifndef CFG_GOLD
		if(!m_BufferWarning)
		{
			LOGWARN(Display::g_Log, "DynamicVertexBuffer : Trying to render zero vertices\n");
			m_BufferWarning = true;
		}
#endif
		return C_DynamicVertexBufferBlock();
	}
	else if(m_CurrentVertexOffset + vertexCount * vertexSize > m_VertexBufferSize)
	{
		if(vertexCount * vertexSize > m_VertexBufferSize)
		{
#ifndef CFG_GOLD
			if(!m_BufferWarning)
			{
				LOGWARN(Display::g_Log, "DynamicVertexBuffer : Trying to render too many vertices\n");
				m_BufferWarning = true;
			}
#endif
			return C_DynamicVertexBufferBlock();
		}
		else
		{
			SwitchVertexBuffer();
		}
	}

	if(m_CurrentFrame - m_VertexBuffers[m_CurrentVertexBuffer].frame < 2)
	{
#ifndef CFG_GOLD
		if(!m_BufferWarning)
		{
			LOGWARN(Display::g_Log, "DynamicVertexBuffer : No more vertex buffers available this frame %s\n", debugName.Chars());
			m_BufferWarning = true;
		}
#endif
		return C_DynamicVertexBufferBlock();
	}

	// if we are using dynamic indices then check to see if we need to lock
	if(indexCount != 0xFFFFFFFF)
	{	
		if(indexCount == 0)
		{
			LOGWARN(Display::g_Log, "DynamicVertexBuffer : Trying to render zero indices\n");
			return C_DynamicVertexBufferBlock();
		}
		else if (m_CurrentIndexOffset + indexCount > m_IndexBufferSize)
		{
			if (indexCount > m_IndexBufferSize)
			{
				LOGWARN(Display::g_Log, "DynamicVertexBuffer : Trying to render too many indices\n");
				return C_DynamicVertexBufferBlock();
			}
			else
			{
				SwitchIndexBuffer();
			}
		}

		if(m_IndexBuffers[m_CurrentIndexBuffer].locks == 0)
		{
			assert(m_IndexBuffers[m_CurrentIndexBuffer].pIndices == NULL);
			m_IndexBuffers[m_CurrentIndexBuffer].pIndices = m_IndexBuffers[m_CurrentIndexBuffer].Lock(0, 0);
		}
		++m_IndexBuffers[m_CurrentIndexBuffer].locks;
	}

	// lock buffer if required
	if(m_VertexBuffers[m_CurrentVertexBuffer].locks == 0)
	{
		assert(m_VertexBuffers[m_CurrentVertexBuffer].pVertices == NULL);
		// using Lock on Xenon kills pix
	#ifdef PLATFORM_PC
		m_VertexBuffers[m_CurrentVertexBuffer].pVertices = m_VertexBuffers[m_CurrentVertexBuffer].Lock(0, 0);
	#else
		m_VertexBuffers[m_CurrentVertexBuffer].pVertices = m_VertexBuffers[m_CurrentVertexBuffer].GetAddress();
	#endif
	}
	// increment lock count
	++m_VertexBuffers[m_CurrentVertexBuffer].locks;


	assert(m_CurrentBlock < m_BlockCount);
#ifndef CFG_SPEED
	m_Blocks[m_CurrentBlock].debugName = debugName;	
#endif
	m_Blocks[m_CurrentBlock].bInUse = true;
	m_Blocks[m_CurrentBlock].vertexBufferOffset = m_CurrentVertexOffset;
	m_Blocks[m_CurrentBlock].vertexBufferIndex = m_CurrentVertexBuffer;
	m_Blocks[m_CurrentBlock].vertexSize = vertexSize;
	m_Blocks[m_CurrentBlock].vertexCount = vertexCount;

	// return a buffer block to the client for filling
	C_DynamicVertexBufferBlock block;
	block.pVertices = (void*)((INTPTR)m_VertexBuffers[m_CurrentVertexBuffer].pVertices + m_CurrentVertexOffset);	// [Blur2PC][x64]
	m_CurrentVertexOffset += vertexSize * vertexCount;

	// if we're using index buffers then store relevant data here
	if(indexCount != 0xFFFFFFFF)
	{
		m_Blocks[m_CurrentBlock].indexBufferOffset = m_CurrentIndexOffset;
		m_Blocks[m_CurrentBlock].indexBufferIndex = m_CurrentIndexBuffer;
		m_Blocks[m_CurrentBlock].indexCount = indexCount;

		block.pIndices = (void*)((INTPTR)m_IndexBuffers[m_CurrentIndexBuffer].pIndices + m_CurrentIndexOffset);	// [Blur2PC][x64]
		m_CurrentIndexOffset += sizeof(U16) * indexCount;
	}
	else
	{
		m_Blocks[m_CurrentBlock].indexBufferOffset = 0xFFFFFFFF;
		m_Blocks[m_CurrentBlock].indexBufferIndex = 0xFFFFFFFF;
		m_Blocks[m_CurrentBlock].indexCount = 0xFFFFFFFF;
		block.pIndices = NULL;
	}
	

	block.blockIndex = m_CurrentBlock;
	
	// increment block and offset
	assert(m_CurrentBlock < m_BlockCount);
	++m_CurrentBlock;
	++m_BeginBlocks;

	return block;
}

void C_DynamicVertexBuffer::End(U32 blockIndex)
{
	C_VertexBlock& vertexBlock = m_Blocks[blockIndex];

	assert(m_VertexBuffers[vertexBlock.vertexBufferIndex].locks > 0);
	assert(vertexBlock.bInUse == true);

	--m_BeginBlocks;
	vertexBlock.bInUse = false;

	if(vertexBlock.indexBufferIndex != 0xFFFFFFFF)
	{
		if(--m_IndexBuffers[vertexBlock.indexBufferIndex].locks == 0)
		{
			m_IndexBuffers[vertexBlock.indexBufferIndex].pIndices = NULL;
			m_IndexBuffers[vertexBlock.indexBufferIndex].Unlock();
		}
	}

	// if there are no outstanding locks on this buffer then unlock it
	if(--m_VertexBuffers[vertexBlock.vertexBufferIndex].locks == 0)
	{
		m_VertexBuffers[vertexBlock.vertexBufferIndex].pVertices = NULL;
	#ifdef PLATFORM_PC
		m_VertexBuffers[vertexBlock.vertexBufferIndex].Unlock();
	#endif
	}
}

void C_DynamicVertexBuffer::End(C_DynamicVertexBufferBlock block)
{
	End(block.blockIndex);
}

void C_DynamicVertexBuffer::DrawPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const C_DynamicVertexBufferBlock block) const
{
	DrawPrimitiveBlock(pContext, primitiveType, block.blockIndex);
}

void C_DynamicVertexBuffer::DrawPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const U32 blockIndex) const
{
	assert(blockIndex < m_BlockCount);
	assert(m_Blocks[blockIndex].vertexBufferIndex < m_VertexBufferCount);
	//assert(m_VertexBuffers[m_Blocks[blockIndex].vertexBufferIndex].locks == 0);

	pContext->SetVertexBuffer(0, &m_VertexBuffers[m_Blocks[blockIndex].vertexBufferIndex], m_Blocks[blockIndex].vertexBufferOffset, m_Blocks[blockIndex].vertexSize);

	assert(m_Blocks[blockIndex].vertexCount > 0);
	pContext->DrawPrimitive(ToPrimitiveType2(primitiveType), 0, m_Blocks[blockIndex].vertexCount); // This function takes the vertex count not index count.
}

void C_DynamicVertexBuffer::DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const C_DynamicVertexBufferBlock block, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount) const
{
	DrawIndexedPrimitiveBlock(pContext, primitiveType, block.blockIndex, pIndexBuffer, indexCount);
}


void C_DynamicVertexBuffer::DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const U32 blockIndex, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount) const
{
	assert(blockIndex < m_BlockCount);
	
	const C_VertexBlock& block = m_Blocks[blockIndex];
	assert(block.vertexBufferIndex < m_VertexBufferCount);

	const C_VertexBuffer& vertexBuffer = m_VertexBuffers[block.vertexBufferIndex];
	assert(vertexBuffer.locks == 0);

	pContext->SetVertexBuffer(0, &vertexBuffer, block.vertexBufferOffset, block.vertexSize);
	pContext->SetIndexBuffer(pIndexBuffer);

	assert(block.vertexCount > 0);
	pContext->DrawIndexedPrimitive(ToPrimitiveType2(primitiveType), 0, 0, block.vertexCount, 0, indexCount);

}

void C_DynamicVertexBuffer::SetVertexBufferSize(U32 bufferSize, U32 bufferCount)
{
	if(!m_Created)
	{
		m_VertexBufferSize = bufferSize;
		m_VertexBufferCount = bufferCount;

		m_VertexBuffers.Resize(m_VertexBufferCount);
	}
}

void C_DynamicVertexBuffer::SetIndexBufferSize(U32 bufferSize, U32 bufferCount)
{
	if(!m_Created)
	{
		m_IndexBufferSize = bufferSize;
		m_IndexBufferCount = bufferCount;

		m_IndexBuffers.Resize(m_IndexBufferCount);
	}
}

void C_DynamicVertexBuffer::ReserveBlocks(U32 blocks)
{
	m_BlockCount = blocks;
	m_Blocks.Resize(blocks);
	m_BlocksGrow = false;
}

}; // namespace Display
