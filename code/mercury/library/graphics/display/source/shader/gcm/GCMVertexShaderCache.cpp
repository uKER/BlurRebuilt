#include "stdafx.h"
#include "Display/Shader/GCM/GCMVertexShaderCache.h"
#include "Display/Shader/GCM/GCMVertexShader.h"

namespace Display
{
namespace GCM
{

C_VertexShaderCache::C_VertexShaderCache()
{
}

C_VertexShaderCache::~C_VertexShaderCache()
{
	m_Blocks.Clear();
}

U32 C_VertexShaderCache::Allocate(const C_VertexShader * pVertexShader)
{
	const U32 instructionCount = pVertexShader->m_ProgramConfig.instructionCount;

	// if no blocks...
	if(m_Blocks.Size() == 0)
	{
		C_VertexShaderCommandBlock block;

		block.m_StartSlot = 0;
		block.m_EndSlot = instructionCount;
		block.m_pVertexShader = pVertexShader;
		m_Blocks.PushBack(block);

		return 0;
	}

	// search for shader in blocks
	for(S32 blockNo = 0; blockNo < m_Blocks.Size(); ++blockNo)
	{
		C_VertexShaderCommandBlock &block = m_Blocks[blockNo];
		if(block.m_pVertexShader == pVertexShader)
		{
			return block.m_StartSlot;
		}
	}

	// not in existing blocks, so find room
	
	C_VertexShaderCommandBlock &lastBlock = m_Blocks.Back();
	U32 freeSlot = lastBlock.m_EndSlot;
	U32 freeSize = 512 - freeSlot;
	if(instructionCount <= freeSize)
	{
		C_VertexShaderCommandBlock block;
		block.m_StartSlot = freeSlot;
		block.m_EndSlot = freeSlot + instructionCount;
		block.m_pVertexShader = pVertexShader;
		m_Blocks.PushBack(block);

		return freeSlot;
	}
	else
	{
		m_Blocks.Clear();

		C_VertexShaderCommandBlock block;
		block.m_StartSlot = 0;
		block.m_EndSlot = instructionCount;
		block.m_pVertexShader = pVertexShader;
		m_Blocks.PushBack(block);

		return 0;
	}
}

void C_VertexShaderCache::Free(const C_VertexShader * pVertexShader)
{
}

void C_VertexShaderCache::Clear()
{
	m_Blocks.Clear();
}

S32 C_VertexShaderCache::FindBlockOfSize(const U32 size)
{
	for (S32 blockNo = 0; blockNo < m_Blocks.Size(); ++blockNo)
	{
		C_VertexShaderCommandBlock &block = m_Blocks[blockNo];

		U32 slotSize = block.m_EndSlot - block.m_StartSlot;
		if (block.m_pVertexShader && size <= slotSize)
		{
			return blockNo;
		}
	}
	return -1;
}

}; // namespace GCM
}; // namespace Display
