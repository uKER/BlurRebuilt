#include <core/core.h>
#include "common\3dmaths.h"
#include <system/scheduler.h>
#include <btl/containers/vector.h>
#include "BatchFrameAdd.h"

#ifdef PLATFORM_SPU
#define MINIMAL_SPU_FRAME_FUNCTIONS
namespace Audio
{
	namespace Core
	{
		const U32 uFrameLength = 256;
	}
}
#define AUDIO_USE_NEW_MATHS
#endif

#include "core/frame.h"


#ifndef PLATFORM_SPU
Debug::Bool gPS3AudioBatchEnable("Playstation3.AudioBatchAdd.Enable", true, Debug::Persist());
#endif


namespace Audio
{
	namespace BatchFrame
	{
#ifndef PLATFORM_SPU
		cBatch						gMoveAudioBatch;
		bool						gTaskInitialised = false;
		Scheduler::PolicyHandle		gPolicy;
#endif

		struct TaskData
		{
			cFrameAdd*		gpFrameAdd;
			U32				gFrameAddIndex;
			U32				gNrEntries;
			U32				gmpPort;
		};

#ifdef PLATFORM_SPU
#ifndef CFG_SPEED
		#define FIXED_MEM_BOUNDS_CHECK 
#endif
		#define FIXED_MEM_HEAP_SIZE (1024 * 128)
		char fixedmem[ FIXED_MEM_HEAP_SIZE ] __attribute__((aligned(128)));
		int fixedmemPos;

		void* allocatefixed( int size )
		{
			size += 127;
			size &= ~127;
		#ifdef FIXED_MEM_BOUNDS_CHECK
			if ( fixedmemPos + size > FIXED_MEM_HEAP_SIZE )
			{
				DebugBreak();
			}
		#endif
			char* p = fixedmem;
			p += fixedmemPos;
			fixedmemPos += size;
			return p;
		}
#endif

		#define ABS(x) ((x < 0) ? -x : x)

		void UpdateTask( Scheduler::Task* pTask )
		{
#ifdef PLATFORM_SPU
//			return;
			fixedmemPos = 0;
			
			TaskData *pData = (TaskData*)pTask->ParamData;
			// DMA the list of FrameAdd Jobs
			U32 dmaSize = ((sizeof( cFrameAdd ) * pData->gFrameAddIndex) + 15) & ~15;
			cFrameAdd* addList = (cFrameAdd*)allocatefixed( dmaSize );
			Spu::Fetch( addList, pData->gpFrameAdd, dmaSize );

			F32* dest = (F32*)allocatefixed( 256 * sizeof(F32) );
			F32* srcA = (F32*)allocatefixed( 256 * sizeof(F32) );
			F32* srcB = (F32*)allocatefixed( 256 * sizeof(F32) );

			// For Each Job, Fetch Data, Perform add, Store Data			
			for (U32 x = 0 ; x < pData->gFrameAddIndex ; x++)
			{
//				DebugBreak();
				U32 toPtr = (U32)addList[x].mpTo;
				if ((toPtr & ~0xf) == 0)
				{
					// Interleave data into Hardware
					U32 nrBuffers = (U32)(( addList[x].mfLevel ));
					U32	 uWriteFrame = (((U32)addList[x].mpTo));
					F32* pSource = (F32*)allocatefixed( 6 * 256 * sizeof(F32) );
					F32* pDest   = (F32*)allocatefixed( 8 * 256 * sizeof(F32) );
					F32* pPPUDest = (F32*)pData->gmpPort;//(F32*)addList[x].mpFromB;
					U32 uFrameLength = 256;
					static const U32 CELL_AUDIO_PORT_8CH = 8;
					static const U32 CELL_AUDIO_PORT_2CH = 2;
					F32* pPPUSourceData = addList[x].mpFromA;
					for (U32 uCount = 0 ; uCount < nrBuffers ; uCount++)
					{
						Spu::Fetch( pSource, pPPUSourceData,  256 * sizeof(F32) * 6 );
						F32* portPtr = (F32*)(pData->gmpPort);
						pPPUDest = portPtr + ( uFrameLength *  8 * uWriteFrame );
						//	5.1 port
						for (U32 uIndex = 0; uIndex < uFrameLength; uIndex++)
						{
							pDest[uIndex * CELL_AUDIO_PORT_8CH]		= pSource[uIndex];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 1] = pSource[uIndex + uFrameLength];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 2] = pSource[uIndex + 2 * uFrameLength];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 3] = pSource[uIndex + 3 * uFrameLength];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 4] = pSource[uIndex + 4 * uFrameLength];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 5] = pSource[uIndex + 5 * uFrameLength];
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 6] = 0;
							pDest[uIndex * CELL_AUDIO_PORT_8CH + 7] = 0;
						}
						Spu::Store( pPPUDest, pDest, uFrameLength * sizeof(F32) * 8 );
						pPPUSourceData += (6 * uFrameLength);
						
						uWriteFrame++;
						uWriteFrame &= (_AMAX_CELL_AUDIO_BLOCK-1);
					}
				} 
				else if ( addList[x].mfLevel )
				{
					Spu::Fetch( dest, addList[x].mpTo,		256 * sizeof(F32) );
					Spu::Fetch( srcA, addList[x].mpFromA,	256 * sizeof(F32) );
					Spu::Fetch( srcB, addList[x].mpFromB,	256 * sizeof(F32) );
					Frame::AddGain( dest, srcA, srcB, addList[x].mfLevel );
					Spu::Store( addList[x].mpTo,	dest, 256 * sizeof(F32) );
				}
				else if (addList[x].mpFromB )
				{
					Spu::Fetch( dest, addList[x].mpTo,		256 * sizeof(F32) );
					Spu::Fetch( srcA, addList[x].mpFromA,	256 * sizeof(F32) );
					Spu::Fetch( srcB, addList[x].mpFromB,	256 * sizeof(F32) );
					Frame::DoAdd( dest, srcA, srcB );
					Spu::Store( addList[x].mpTo, dest, 256 * sizeof(F32) );
				}
				else if ( addList[x].mpFromA )
				{
					Spu::Fetch( srcA, addList[x].mpFromA,	256 * sizeof(F32) );
					Spu::Store( addList[x].mpTo, dest,		256 * sizeof(F32) );
				} 
				else
				{
					vector float	vzero = { 0.0f, 0.0f, 0.0f, 0.0f };
					vector float*	vdest = (vector float*)dest;
					for (U32 z = 0; z < 256/4; z++)
					{
						vdest[z] = vzero;
					}
					Spu::Store( addList[x].mpTo, dest,		256 * sizeof(F32) );
				}

			}
			
#endif  // PLATFORM_SPU
		
		

#ifdef PLATFORM_PC
		TaskData *pData = (TaskData*)pTask->ParamData;
		cFrameAdd* frameAdd = pData->gpFrameAdd;
		for (U32 x = 0 ; x < pData->gFrameAddIndex; x++)
		{
		
			F32* dest = frameAdd[x].mpTo;
			const F32* srcA = frameAdd[x].mpFromA;
			const F32* srcB = frameAdd[x].mpFromB;
			
			Frame::DoAdd( dest, srcA, srcB );
		}
#endif
		}

	}
}

#if defined( PLATFORM_SPU) || defined( PLATFORM_PS3 ) 
JQ_DECLARE_SPU_MODULE(AudioBatchFrameAddSPUTask, Audio::BatchFrame::UpdateTask, AudioBatchFrameAddSPU, "mercury.audio.system");
#else
JQ_DECLARE_PPU_MODULE(AudioBatchFrameAddSPUTask, Audio::BatchFrame::UpdateTask);
#endif


namespace Audio
{
	namespace BatchFrame
	{

		
#ifndef PLATFORM_SPU
		bool gLastEnabledState = gPS3AudioBatchEnable;
		
		bool cBatch::IsEnabled()
		{
			if (gLastEnabledState != gPS3AudioBatchEnable)
			{
				Clear();
			}
			gLastEnabledState = gPS3AudioBatchEnable;
			return gPS3AudioBatchEnable;
		}
	
		
		void cBatch::Clear()
		{
			mFrameAddIndex = 0;
		}
	
		void cBatch::Add( F32* pTo, const F32* pFromA, const F32* pFromB, const F32 fLevel )
		{
			if (IsEnabled())
			{
				mpQueue[mFrameAddIndex].mpTo = pTo;
				mpQueue[mFrameAddIndex].mpFromA = pFromA;
				mpQueue[mFrameAddIndex].mpFromB = pFromB;
				mpQueue[mFrameAddIndex].mfLevel = fLevel;
				mFrameAddIndex++;
				if (mFrameAddIndex == mpQueueSize)
				{
					Process();
					Clear();
				}
			}
			else
			{
				Frame::DoAdd( pTo, pFromA, pFromB );
			}
		}

		void cBatch::InitTask( Scheduler::PolicyHandle policy )
		{
			gPolicy = policy;
			gTaskInitialised = true;
		}
		
		cBatch* cBatch::Init( U32 queueSize )
		{
			mGroup	= Scheduler::CreateGroup( true );
			mpQueue = (cFrameAdd*)BizAlignedMalloc( sizeof(cFrameAdd) * queueSize, 128 );
			mpQueueSize = queueSize;
			return this;
		}
		
		void cBatch::Shutdown()
		{
			Wait();
			Scheduler::DestroyGroup( mGroup );
			BizAlignedFree( mpQueue );
		}
		
		void cBatch::Process()
		{

			if (IsEnabled())
			{
				TaskData data;	
				data.gpFrameAdd		= mpQueue;
				data.gFrameAddIndex = mFrameAddIndex;
				data.gNrEntries		= mpQueueSize;
#ifdef PLATFORM_PS3
				data.gmpPort		= m_pPort;
				Scheduler::AddTask(AudioBatchFrameAddSPUTask, 0, 0, mGroup, gPolicy, &data, sizeof(data));
#else
				Scheduler::Task Task;
				memcpy(Task.ParamData,&data,sizeof(data));
				UpdateTask( &Task );
#endif
			}
		}

		void cBatch::Wait()
		{
			Scheduler::Flush( mGroup, false );
		}
	
#endif
		
	}

}

