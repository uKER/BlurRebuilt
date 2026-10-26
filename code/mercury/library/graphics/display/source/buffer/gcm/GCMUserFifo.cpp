#include "stdafx.h"
#include "Display/Buffer/GCM/GCMUserFifo.h"
#include "Display/Manager/GCM/GCMManager.h"

static int32_t userCallback(struct CellGcmContextData* con, uint32_t size);

// User Command buffer (double buffer)
static uint32_t* sUserCommandBufferAddress[2];
static uint32_t  sUserCommandBufferOffset[2];
static uint32_t  sUserCommandBufferSize;
static uint8_t   sUserCommandBufferIndex = 0;

#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)

static const uint32_t LABEL_INDEX = 64;
static uint32_t sLabelVal = 0;

void GCMDebugFinish(struct CellGcmContextData* con)
{
	// need to disable & save debug finish (because we are in it!)
	void (*save_debug_finish)(struct CellGcmContextData*) = gCellGcmDebugCallback;
	gCellGcmDebugCallback = NULL;

	// write label command into command buffer
	//
	cellGcmSetWriteBackEndLabelUnsafeInline(con, LABEL_INDEX, sLabelVal);

	// flush command buffer
#ifdef USE_UNSAFE_GCM
	cellGcmFlushUnsafeInline(con);
#else
	cellGcmFlush(con);
#endif

	// wait for label value to sLabelVal
	volatile uint32_t label_value = *(cellGcmGetLabelAddress(LABEL_INDEX));
	while(label_value != sLabelVal) {
		sys_timer_usleep(100);	
		label_value = *(cellGcmGetLabelAddress(LABEL_INDEX));
	}

	sLabelVal += 1;

	// restore debug finish
	gCellGcmDebugCallback = save_debug_finish;
}

#endif

static int32_t GCMUserCallback(struct CellGcmContextData* con, uint32_t size)
{
	volatile CellGcmControl* control;
	(void) size;

	sUserCommandBufferIndex = (sUserCommandBufferIndex + 1) % 2;

	// flush first
	cellGcmFlushUnsafeInline(con);

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = NULL;
	#endif

	// put jump command to beginning of next buffer
	cellGcmSetJumpCommandUnsafeInline(con, sUserCommandBufferOffset[sUserCommandBufferIndex]);

	// set current command buffer to next buffer
	cellGcmSetCurrentBuffer(sUserCommandBufferAddress[sUserCommandBufferIndex], sUserCommandBufferSize);

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = GCMDebugFinish;
	#endif

	// get put/get/ref register address
	control = cellGcmGetControlRegister();

	// wait for RSX to finish all command in next buffer 
	volatile uint32_t get = (volatile uint32_t)control->get;
	while( ( get >=  sUserCommandBufferOffset[sUserCommandBufferIndex] && 
	         get <  (sUserCommandBufferOffset[sUserCommandBufferIndex] + sUserCommandBufferSize))
		 || (get <  ( 0x1000 ) ) )
	{
		sys_timer_usleep( 100 );
		get = (volatile uint32_t)control->get;
	}

	return CELL_OK;
}

void GCMInitUserFifo( const void* host_address, const uint32_t cb_size )
{
	sUserCommandBufferIndex = 0;

	// double buffering
	//
	sUserCommandBufferSize = cb_size / 2;

	// User command buffer starts right after default command buffer
	// and 4KB reserved area.
	//
	sUserCommandBufferAddress[0] = (uint32_t*)((uint32_t)host_address);
	sUserCommandBufferAddress[1] = (uint32_t*)((uint32_t)host_address+sUserCommandBufferSize);

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	// if debug version is used, additioanl "WriteBackEndLabel" command (4 WORDS command)
	// is put into fifo at every command API.
	// End of command buffer should be set 4 WORDS command less, just in case we are
	// putting "WriteBackEndLabel" command at end of command buffer.
	// 
	sUserCommandBufferSize -= sizeof(uint32_t) * 4;
	#endif

	// Get offset
	cellGcmAddressToOffset(sUserCommandBufferAddress[0], &sUserCommandBufferOffset[0]);
	cellGcmAddressToOffset(sUserCommandBufferAddress[1], &sUserCommandBufferOffset[1]);

	assert( (sUserCommandBufferOffset[0] & 0x3) == 0 );
	assert( (sUserCommandBufferOffset[1] & 0x3) == 0 );

	// Jump to user user command buffer
	cellGcmSetJumpCommand(sUserCommandBufferOffset[sUserCommandBufferIndex]);

	// Changed default command buffer to user command buffer
	cellGcmSetCurrentBuffer(sUserCommandBufferAddress[sUserCommandBufferIndex], sUserCommandBufferSize);

	// Set user callbacks
	cellGcmSetUserCallback(GCMUserCallback);

	// Flush current command buffer
	cellGcmFlush();
}

void GCMSwapUserFifo()
{
	// Swap to the next buffer.
	sUserCommandBufferIndex = (sUserCommandBufferIndex + 1) % 2;

	// Flush the previous buffer.
	cellGcmFlushUnsafeInline(gCellGcmCurrentContext);

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = NULL;
	#endif

	// Put jump command to beginning of next buffer.
	cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, sUserCommandBufferOffset[sUserCommandBufferIndex]);

	// Set current command buffer to next buffer.
	cellGcmSetCurrentBuffer(sUserCommandBufferAddress[sUserCommandBufferIndex], sUserCommandBufferSize);

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = GCMDebugFinish;
	#endif

	// Get put/get/ref register address.
	volatile CellGcmControl* control = cellGcmGetControlRegister();

	// Wait for RSX to finish all command in next buffer.
	volatile uint32_t get = (volatile uint32_t)control->get;
	while((get >=  sUserCommandBufferOffset[sUserCommandBufferIndex] && get < (sUserCommandBufferOffset[sUserCommandBufferIndex] + sUserCommandBufferSize)) ||(get <  ( 0x1000 ) ) )
	{
		sys_timer_usleep( 100 );
		get = (volatile uint32_t)control->get;
	}
}

void GCMGetCurrentCommandBuffer(uint8_t& index, CellGcmControl** ppgcmControl)
{
	index = sUserCommandBufferIndex;
	*ppgcmControl = cellGcmGetControlRegister();
}

CellGcmContextData GCMCreateContextData(U32* const pBegin, const U32 size)
{
	CellGcmContextData gcmContext;
	gcmContext.begin	= pBegin;
	gcmContext.end		= pBegin + (size >> 2);
	gcmContext.current	= pBegin;
	gcmContext.callback	= 0;
	return gcmContext;
}

CellGcmContextData GCMCreateContextData(U32* const pBegin, U32* const pEnd)
{
	CellGcmContextData gcmContext;
	gcmContext.begin	= pBegin;
	gcmContext.end		= pEnd;
	gcmContext.current	= pBegin;
	gcmContext.callback	= 0;
	return gcmContext;
}

U32 GCMCreateCommandBufferHole(U32 &sizeInBytes)
{
#if 1

	sizeInBytes = (sizeInBytes + 0xF) & ~0xF;

	BIZ_ASSERT(sizeInBytes < 128);

	while(((U32)gCellGcmCurrentContext->current & 0xF) != 0)
	{
		GCM_NAMESPACE::cellGcmSetNopCommand(gCellGcmCurrentContext, 1);
	}

	GCM_NAMESPACE::cellGcmReserveMethodSize(gCellGcmCurrentContext, sizeInBytes / 4);

	U32 holeStart = (U32)gCellGcmCurrentContext->current;

	U32 holeEnd = holeStart + sizeInBytes;

	U32 jumpOffset;

	cellGcmAddressToOffset(gCellGcmCurrentContext->current, &jumpOffset);

	cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, jumpOffset);

	U32 nextJ2S = ((U32)gCellGcmCurrentContext->current + 0x80) & ~0x7F;

	while((U32)gCellGcmCurrentContext->current < holeEnd)
	{
		if((U32)gCellGcmCurrentContext->current == nextJ2S)
		{
			cellGcmAddressToOffset(gCellGcmCurrentContext->current, &jumpOffset);

			cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, jumpOffset);

			nextJ2S = ((U32)gCellGcmCurrentContext->current + 0x80) & ~0x7F;
		}
		else
		{
			cellGcmSetNopCommandUnsafeInline(gCellGcmCurrentContext, 1);
		}
	}

	BIZ_ASSERT((U32)gCellGcmCurrentContext->current == holeEnd);

	return holeStart;

#else

	// round size up to next 16 bytes
	sizeInBytes = (sizeInBytes + 0xF) & ~0xF;

	// get the current command buffer
	CellGcmContextData *pGCMContext = gCellGcmCurrentContext;

	// if there's not enough memory for our hole, allocate a bigger command buffer
	if (pGCMContext->current + sizeInBytes/4 + 3 > pGCMContext->end)
	{
		if ((*pGCMContext->callback)(pGCMContext, sizeInBytes/4 + 3) != CELL_OK)
			return 0;
	}
	// ensure the current pointer is aligned to 16 bytes
	while (((U32)pGCMContext->current & 0xF) != 0 )
		*pGCMContext->current++ = 0;

	// create the hole
	U32 holeEa = (U32) pGCMContext->current;
	U32 holeEnd = holeEa + sizeInBytes;
	U32 jumpOffset;
	cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
	cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
	U32 nextJ2S = ((U32) pGCMContext->current + 0x80) & ~0x7F;
	while(nextJ2S < holeEnd)
	{
		pGCMContext->current = (U32 *) nextJ2S;
		cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
		cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
		nextJ2S = ((U32) pGCMContext->current + 0x80) & ~0x7F;
	}
	pGCMContext->current = (U32 *) holeEnd;

	// return the address of the hole
	return holeEa;

#endif
}

#if 0
U32 GCMCreateCommandBufferHoleUnsafe(U32 &sizeInBytes)
{
	// round size up to next 16 bytes
	sizeInBytes = (sizeInBytes + 0xF) & ~0xF;
	
	// get the current command buffer
	CellGcmContextData *pGCMContext = gCellGcmCurrentContext;

	// ensure the current pointer is aligned to 16 bytes
	while (((U32)pGCMContext->current & 0xF) != 0 )
		*pGCMContext->current++ = 0;
	
	// create the hole
	U32 holeEa = (U32) pGCMContext->current;
	U32 holeEnd = holeEa + sizeInBytes;
	U32 jumpOffset;
	cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
	cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
	U32 nextJ2S = ((U32) pGCMContext->current + 0x80) & ~0x7F;
	while(nextJ2S < holeEnd)
	{
		pGCMContext->current = (U32 *) nextJ2S;
		cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
		cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
		nextJ2S = ((U32) pGCMContext->current + 0x80) & ~0x7F;
	}
	pGCMContext->current = (U32 *) holeEnd;

	// return the address of the hole
	return holeEa;
}
#endif

#if 0
// [StephenM] Currently seems to crash GCM...
U32 GCMCreateCommandBufferHole128BytesUnsafe()
{
	// get the current command buffer
	CellGcmContextData *pGCMContext = gCellGcmCurrentContext;

	// ensure the current pointer is aligned to 128 bytes
	while (((U32)pGCMContext->current & 0x7F) != 0 )
		*pGCMContext->current++ = 0;
	
	// create the hole
	U32 holeEa = (U32) pGCMContext->current;
	U32 jumpOffset;
	cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
	cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
	pGCMContext->current = (U32 *) (holeEa + 0x80);

	// return the address of the hole
	return holeEa;
}
#endif

U32 GCMCreateJumpToSelf()
{
#if 1
	while(((U32)gCellGcmCurrentContext->current & 0xF) != 0)
		GCM_NAMESPACE::cellGcmSetNopCommand(gCellGcmCurrentContext, 1);

	GCM_NAMESPACE::cellGcmReserveMethodSize(gCellGcmCurrentContext, 4);

	void* j2sAddress = gCellGcmCurrentContext->current;

	U32 j2sOffset = 0;
	S32 result = cellGcmAddressToOffset(j2sAddress, &j2sOffset);
	BIZ_ASSERT(result == CELL_OK);

	cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, j2sOffset);
	cellGcmSetNopCommandUnsafeInline(gCellGcmCurrentContext, 3);

	return (U32)j2sAddress;
#else
	// get the current command buffer
	CellGcmContextData *pGCMContext = gCellGcmCurrentContext;

	// if there's not enough memory for our command, allocate a bigger command buffer
	if (pGCMContext->current + 4 > pGCMContext->end)
	{
		if ((*pGCMContext->callback)(pGCMContext, 4) != CELL_OK)
		{
			assert(0);
			return 0;
		}

		pGCMContext = gCellGcmCurrentContext;
	}
	// ensure the current pointer is aligned to 16 bytes
	while (((U32)pGCMContext->current & 0xF) != 0 )
	{
		BIZ_ASSERT(0);
		*pGCMContext->current++ = 0;
	}

	U32 j2sAddress = (U32) pGCMContext->current;

	// add the jump to self
	U32 jumpOffset;
	cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
	cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);
	cellGcmSetNopCommandUnsafeInline(pGCMContext, 3);

	// return the address of the J2S
	return j2sAddress;
#endif
}

U32 GCMCreateJumpToSelf128BytesAligned()
{
	// get the current command buffer
	CellGcmContextData *pGCMContext = gCellGcmCurrentContext;

	// if there's not enough memory for our command, allocate a bigger command buffer
	if (pGCMContext->current + 32 > pGCMContext->end)
	{
		if ((*pGCMContext->callback)(pGCMContext, 32) != CELL_OK)
		{
			assert(0);
			return 0;
		}

		pGCMContext = gCellGcmCurrentContext;
	}
	// ensure the current pointer is aligned to 128 bytes
	while (((U32)pGCMContext->current & 0x7F) != 0 )
		cellGcmSetNopCommandUnsafeInline(pGCMContext, 1);
	
	U32 j2sAddress = (U32) pGCMContext->current;

	// add the jump to self
	U32 jumpOffset;
	cellGcmAddressToOffset(pGCMContext->current, &jumpOffset);
	cellGcmSetJumpCommandUnsafeInline(pGCMContext, jumpOffset);

	// return the address of the J2S
	return j2sAddress;
}

void GCMCpuSetLabel(const U32 index, const U32 value)
{
	*(cellGcmGetLabelAddress(index)) = value;
}

void GCMGpuSetLabelOnCommandEnd(const U32 index, const U32 value)
{
	assert(index >= 64 && index <= 255);
	cellGcmSetWriteCommandLabel(index, value);
}

void GCMGpuSetLabelOnTextureEnd(const U32 index, const U32 value)
{
	assert(index >= 64 && index <= 255);
	cellGcmSetWriteTextureLabel(index, value);
}

void GCMGpuSetLabelOnRasterEnd(const U32 index, const U32 value)
{
	assert(index >= 64 && index <= 255);
	cellGcmSetWriteBackEndLabel(index, value);
}

void GCMCpuWaitOnLabel(const U32 index, const U32 value)
{
	volatile U32 labelValue = *(cellGcmGetLabelAddress(index));
	while (labelValue != value)
	{
		sys_timer_usleep(100);
		labelValue = *(cellGcmGetLabelAddress(index));
	}
}

void GCMGpuWaitOnLabel(const U32 index, const U32 value)
{
	assert(index >= 64 && index <= 255);
	cellGcmSetWaitLabel(index, value);
}

namespace Display
{
namespace GCM
{

const U32 g_BlockerMemorySize = 1 * 1024; // 1 kilobyte

C_BlockerPool::C_BlockerPool() : m_pBase(0), m_pTop(0), m_pPosition(0)
{
}

C_BlockerPool::~C_BlockerPool()
{
}

void C_BlockerPool::Create(U32* const pBase, const U32 size)
{
	m_pBase		= pBase;
	m_pPosition	= pBase;
	m_pTop		= pBase + (size >> 2);
}

U32* C_BlockerPool::Allocate(const U32 count)
{
	U32* pAllocBase	= m_pPosition;
	U32* pAllocTop	= m_pPosition + count + 1;
	if (pAllocTop > m_pTop)
	{
		pAllocBase	= m_pBase;
		pAllocTop	= m_pBase + count + 1;
	}
	m_pPosition = pAllocTop;

	U32 gcmOffset = 0;
	cellGcmAddressToOffset(pAllocBase, &gcmOffset);

	CellGcmContextData gcmContext = GCMCreateContextData(pAllocBase, pAllocTop);
	for (U32 i = 0; i < count; ++i)
	{
		cellGcmSetJumpCommandUnsafeInline(&gcmContext, gcmOffset);
	}
	cellGcmSetReturnCommandUnsafeInline(&gcmContext);

	return pAllocBase;
}

}; // namespace GCM
}; // namespace Display
