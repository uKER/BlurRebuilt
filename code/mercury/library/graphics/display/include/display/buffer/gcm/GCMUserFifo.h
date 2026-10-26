#ifndef DISPLAY_BUFFER_GCM_GCM_USER_FIFO_H
#define DISPLAY_BUFFER_GCM_GCM_USER_FIFO_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"

//#define USE_GCM_USER_FIFO

#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
void GCMDebugFinish(struct CellGcmContextData* con);
#endif

void GCMInitUserFifo(const void* host_address, const uint32_t cb_size);
void GCMSwapUserFifo();

void GCMGetCurrentCommandBuffer(uint8_t& index, CellGcmControl** ppgcmControl);

CellGcmContextData GCMCreateContextData(U32* const pBegin, const U32 size);
CellGcmContextData GCMCreateContextData(U32* const pBegin, U32* const pEnd);

U32 GCMCreateCommandBufferHole(U32 &sizeInBytes);
//U32 GCMCreateCommandBufferHoleUnsafe(U32 &sizeInBytes);
//U32 GCMCreateCommandBufferHole128BytesUnsafe();
U32 GCMCreateJumpToSelf();
U32 GCMCreateJumpToSelf128BytesAligned();

void GCMCpuSetLabel(const U32 index, const U32 value);
void GCMGpuSetLabelOnCommandEnd(const U32 index, const U32 value);
void GCMGpuSetLabelOnTextureEnd(const U32 index, const U32 value);
void GCMGpuSetLabelOnRasterEnd(const U32 index, const U32 value);
void GCMCpuWaitOnLabel(const U32 index, const U32 value);
void GCMGpuWaitOnLabel(const U32 index, const U32 value);

namespace Display
{
namespace GCM
{

extern const U32 g_BlockerMemorySize;

class C_BlockerPool
{
public:

	C_BlockerPool();
	~C_BlockerPool();

	void	Create(U32* const pBase, const U32 size);
	U32*	Allocate(const U32 count = 1);

private:

	U32*	m_pBase;
	U32*	m_pTop;
	U32*	m_pPosition;

};

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_BUFFER_GCM_GCM_USER_FIFO_H
