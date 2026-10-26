#include "stdafx.h"

#include <System/Scheduler.h>

#include <Display2.h>
//#include <Display2/Mesh/Mesh2.h>
//#include <Display2/Mesh/Mesh2Args.h>
//#include <Display2/Mesh/Mesh2Render.h>

void MeshRenderTaskFunc(Scheduler::Task* _pTask);

JQ_DECLARE_SPU_MODULE(MeshRenderTask, MeshRenderTaskFunc, meshmodule, "graphics.g_Mesh")

//Display2::C_ScratchBuffer<256>		g_ScratchBuffer STOREALIGN16;
//Display2::C_ScratchBuffer<16>		g_ScratchBuffer16 STOREALIGN16;
//Display2::C_RingBuffer				g_RingBuffer STOREALIGN16;
//Display2::C_CommandChain			g_CommandChain STOREALIGN16;
//Render::C_Mesh2						g_Mesh STOREALIGN16;
//U32									g_CommandChainUsed = 0;

void CreateJumpToNext(const U32* localStore, const U32 jumpOffset, void* eaJumpToSelf)
{
#if 0
	Display2::C_ContextData contextData;
	cellGcmSetupContextData(&contextData, localStore, 16, NULL);

	Display2::C_ContextNoCheckNoTrackUnsafeInline context(&contextData);
	context.SetJumpCommand(jumpOffset);
	context.SetNopCommand(3);

	Spu::Store(eaJumpToSelf, localStore, 16);
#endif
}

int32_t FlushScratchBuffer(CellGcmContextData* pContextData)
{
#if 0
	// how much of the scratch buffer has been used
	const U32 scratchBufferUsed = (U8*)pContextData->current - (U8*)pContextData->begin;
	if(scratchBufferUsed == 0)
		return CELL_OK;

	// make sure the size is aligned to 16 bytes
	const U32 scratchBufferSize = ALIGN_SIZE(scratchBufferUsed, 16);

	// pad it out with nops if not aligned to 16 bytes
	const U32 scratchBufferDiff = scratchBufferSize - scratchBufferUsed;
	if(scratchBufferDiff > 0)
		cellGcmSetNopCommandUnsafeInline(pContextData, scratchBufferDiff / 4);

	// dma down the contents of the scratch buffer to main memory
	Spu::Store((void*)((U8*)g_CommandChain.m_pAddress + g_CommandChainUsed), pContextData->begin, scratchBufferSize);

	g_CommandChainUsed += scratchBufferSize;
#endif

	return CELL_OK;
}

int32_t MeshRenderCallback(CellGcmContextData* pContextData, uint32_t size)
{
#if 0
	const U32 scratchBufferUsed = (U32)pContextData->current - (U32)pContextData->begin;

	if((g_CommandChainUsed + scratchBufferUsed) > (g_CommandChain.m_Size - 16)) // Reserve 16 bytes so can dma an j2n if needs be
	{
		U32 newOffset = g_ScratchBuffer.AllocMemory(g_CommandChain.m_Size);
		U8* pNewAddress = g_RingBuffer.GetAddressFromOffset(newOffset);

		CreateJumpToNext(g_ScratchBuffer16.m_Data, newOffset, (void*)((U8*)g_CommandChain.m_pAddress + g_CommandChainUsed));

		g_CommandChain.m_Offset = newOffset;
		g_CommandChain.m_pAddress = (U32*)pNewAddress;

		g_CommandChainUsed = 0;
	}
	
	FlushScratchBuffer(pContextData);
	
	cellGcmSetupContextData(pContextData, g_ScratchBuffer.m_Data, g_ScratchBuffer.m_Size - 12, MeshRenderCallback);
#endif

	return CELL_OK;
}
	
void MeshRenderTaskFunc(Scheduler::Task* pTask)
{
#if 0
	//DebugBreak();

	// get the mesh param args
	Render::C_Mesh2Param* pParamArgs = (Render::C_Mesh2Param*)pTask->ParamData;

	// dma up the mesh
	Spu::Fetch(&g_Mesh, (const void*)pParamArgs->m_eaMeshPtr, sizeof(Render::C_Mesh2));

	// dma up the ring buffer
	Spu::Fetch(&g_RingBuffer, (const void*)pParamArgs->m_eaRingBufferPtr, sizeof(Display2::C_RingBuffer));

	// setup the scratch buffer
	g_ScratchBuffer.m_eaRingBufferCurOffsetPtr	= pParamArgs->m_eaRingBufferCurOffsetPtr;

	// setup the command buffer
	g_CommandChain.m_Size			= g_ScratchBuffer.m_Size;
	g_CommandChain.m_Offset		= g_ScratchBuffer.AllocMemory(g_CommandChain.m_Size);
	g_CommandChain.m_pAddress		= (U32*)g_RingBuffer.GetAddressFromOffset(g_CommandChain.m_Offset);
	
	U32 cbStartOffset = g_CommandChain.m_Offset;
	
	Display2::C_ContextData cbContextData;
	
	// setup the scratch buffer with the command buffer context data
	cellGcmSetupContextData(&cbContextData, g_ScratchBuffer.m_Data, g_ScratchBuffer.m_Size - 12, MeshRenderCallback);

	Display2::C_ContextNoCheckNoTrack cbContext(&cbContextData);

	// render the mesh to the command buffer context
	Render::RenderMesh2(&g_Mesh, &cbContext);

	// this is where we want to jump back too in the default command buffer
	cbContext.SetJumpCommand(pParamArgs->m_JumpToSelfReturnOffset);

	// dma down the scratch buffer to main memory
	MeshRenderCallback(&cbContextData, 0);

	CreateJumpToNext(g_ScratchBuffer16.m_Data, cbStartOffset, (void*)pParamArgs->m_eaJumpToSelfPtr);

	int g = 0;
#endif
}
