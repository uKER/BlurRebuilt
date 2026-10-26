#ifndef _SYSTEM_HEADERS_SPU_H_
#define _SYSTEM_HEADERS_SPU_H_

#if defined (USER_JONC) && defined (CFG_DEBUG)
#define AUDIO_SPU_MINIMAL
#endif

#ifdef OVERRIDE_SPU_ALLOCS
void* operator new(size_t size);
void operator delete(void* ptr);
void* operator new[](size_t size);
void operator delete[](void* ptr);

//void CheckMemory();
//void ResetHeap();
//void StoreHeapPosition();
//void ResetHeapToStoredPosition();
#endif

/* common headers */
#include <stdint.h>
#include <stdio.h>
#include <spu_intrinsics.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

/* Lv2 OS headers */
#include <sys/spu_thread.h>

/* spu_printf */
#include <spu_printf.h>

/* system dma */
#include <cell/dma.h>


// audio specific stuff
#include "common/Util.h"
#include "../include/Audio.h"
#include "core/audio_core.h"
#include "Common/3dMaths.h"
#include "Definition.h"
#include "Common/Array.h"
#include "Common/BitLimitedPagedArray.h"
#include "Common\Allocator.h"
#include "core/coreheaders.h"
#include "../source/SystemPS3shared.h"


namespace Audio { namespace Core { class QueueItem { }; } };

#include "InstanceUpdate.h"
#include "Instance.h"
#include "ComponentHeaders.h"
#include "ComponentContainer.h"
#include "Object.h"

#include "SystemCommon.h"

//	random number seeds
#define AUDIO_BIG_PRIME_A 99923
#define AUDIO_BIG_PRIME_B 72649


class AudioDataTransfer
{

public:
	AudioDataTransfer() { m_pSpuAddr = 0; m_pPpuAddr = 0; m_u32Size = 0; }

	inline void Set(void* pPpuAddr, U32 u32Size)	{ m_pPpuAddr = pPpuAddr; m_u32Size = u32Size; }
	inline void Lock()								{ m_pSpuAddr = new char[m_u32Size]; }
	inline void Read()								{ Spu::Fetch(m_pSpuAddr, m_pPpuAddr, m_u32Size); }
	inline void ReadAsync()							{ Spu::FetchAsync(m_pSpuAddr, m_pPpuAddr, m_u32Size, 0); }
	inline void Write()								{ Spu::Store(m_pPpuAddr, m_pSpuAddr, m_u32Size); }
	inline void WriteAsync()						{ Spu::StoreAsync(m_pPpuAddr, m_pSpuAddr, m_u32Size, 0); }
	inline void* GetSPUAddress()					{ return m_pSpuAddr; }
	inline void PrepareBuffer(void* pBuffer)		{ m_pSpuAddr = pBuffer; }

private:
	void* m_pSpuAddr;
	void* m_pPpuAddr;
	U32 m_u32Size;
};


namespace Audio
{
/*
	class SpuFileStreamManager
	{
	public:

		bool						IsStreamFinishedReading(FS_STREAM_HANDLE hStream)		{ return QueryStateFlag(hStream, STREAM_FINISHED_READING); }
		bool						IsStreamFinishedWriting(FS_STREAM_HANDLE hStream)		{ return QueryStateFlag(hStream, STREAM_FINISHED_WRITING); }
		bool						IsStreamPrepared(FS_STREAM_HANDLE hStream)				{ return QueryStateFlag(hStream, STREAM_ACTIVE); }
		bool						IsStreamPreparing(FS_STREAM_HANDLE hStream)				{ return QueryStateFlag(hStream, STREAM_PREPARING); }
		bool						IsStreamReleasing(FS_STREAM_HANDLE hStream)				{ return QueryStateFlag(hStream, STREAM_RELEASING); }
		bool						AreStreamBuffersAllocated(FS_STREAM_HANDLE hStream)		{ return QueryStateFlag(hStream, STREAM_BUFFERS_ALLOCATED); }
		bool						IsStreamDataReady(FS_STREAM_HANDLE hStream)				{ return (! QueryStateFlag(hStream, STREAM_DATA_NOT_READY)); }
		bool						AllocStreamBuffers(FS_STREAM_HANDLE hStream)			{ SetStateFlagToTrue(hStream, STREAM_BUFFERS_REQUIRED); return true; }

		bool						PrepareStream(FS_STREAM_HANDLE hStream)					{ SetStateFlagToTrue(hStream, STREAM_PREPARATION_REQUIRED); return true; }
		bool						ReleaseStream(FS_STREAM_HANDLE hStream)					{ SetStateFlagToTrue(hStream, STREAM_RELEASE_REQUIRED); return true; }
		void*						GetData(FS_STREAM_HANDLE hStream, U32 u32NumBytes);
		bool						SetFilePosition(FS_STREAM_HANDLE hStream, U32 u32PositionInBytes);

//		void						OverrideBytesConsumed(FS_STREAM_HANDLE hStream, U32 u32NumBytes) { m_aStreamInfo[hStream].u16BytesReadOnSpu = u32NumBytes; }

		// spu-side specific stuff
//		void						SetArrayPointer(void* pArray)							{ m_aStreamInfo = (Core::FileStreamInfoPS3*)pArray; }
//		void						ReleaseResources()										{ m_DmaTransfer.Release(); }

	private:
		//bool						QueryStateFlag(FS_STREAM_HANDLE hStream, U16 u16Flag)		{ return ((u16Flag & m_aStreamInfo[hStream].u16StateFlags) > 0); }
		//void						SetStateFlagToTrue(FS_STREAM_HANDLE hStream, U16 u16Flag)	{ m_aStreamInfo[hStream].u16StateFlags |= u16Flag; }
		//void						SetStateFlagToFalse(FS_STREAM_HANDLE hStream, U16 u16Flag)	{ m_aStreamInfo[hStream].u16StateFlags &= ~u16Flag; }

//		Core::FileStreamInfoPS3*	m_aStreamInfo;
		AudioDataTransfer			m_DmaTransfer;
	};
*/
	class System
	{

	public:
		System()
		{
			m_pScratchDmaBuffer = 0;
			m_u32ScratchDmaBufferSize = 0;
			m_bAsyncDmaCommandIssued = false;
		}

		static System* Create();


		static System* Get() { return m_pSystem; }

        inline const U32 GetFrameNum() { return m_SystemInfo.state.m_uFrameNum; }

		inline MixerComponent::PanningMode GetPanningMode() { return m_SystemInfo.state.m_ePanningMode; }
		inline U32 GetNumSpeakers() { return m_SystemInfo.state.m_uNumSpeakers; }
		inline F32* GetSpeakerAngles() { return m_SystemInfo.state.m_afAngles; }
        inline F32 GetSpeakerDistance() { return m_SystemInfo.state.m_fSpeakerDistance; }
        inline bool HasCentre() { return m_SystemInfo.state.m_bHasCentre; }
		inline bool	AnglesChanged(){ return m_SystemInfo.state.m_SpeakerAngleHasChanged; }

		inline void AsyncDmaCommandIssued() { m_bAsyncDmaCommandIssued = true; }
		inline void WaitForAsyncDmas() { if (m_bAsyncDmaCommandIssued) cellDmaWaitTagStatusAll(1 << 0); m_bAsyncDmaCommandIssued = false; }

		void LoadCodeOverlayForComponent(U32 u32CompType);
		U32 LookupCodeOverlayIdForComponent(U32 u32CompType);
		bool LoadCodeOverlay(U32 u32OverlayId);
		U32 GetCurrentCodeOverlayId() { return g_u32CurrentCodeOverlayId; }

//		SpuFileStreamManager* StreamManager() { return &m_FileStreamManager; }

		static void* Alloc(size_t uSize, U32 uAlignment = 1)
		{
			// TODO: SPU_TODO: alignment
			if (uAlignment != 1)
				DebugBreak();

			return (void*)(new U8[uSize]);
		}

		static void Free(void* pAddress)
		{
//			DebugBreak();
//			delete [] (U8*)pAddress;
		}

        inline F32 GetRandom()
        {
            m_uSeed = (m_uSeed + AUDIO_BIG_PRIME_A) * AUDIO_BIG_PRIME_B;
            return (F32)m_uSeed / 4294967296.f;
        }
		inline void SeedRand(U32 u32Seed) { m_uSeed = u32Seed; }

		inline void* GetStateBuffer(const U32 u32Size) { return (void*)(new U8[u32Size]); }
		inline bool FreeStateBuffer(const void* pBuffer) { return true; }

		SystemSpuInfo m_SystemInfo;
        U32 m_uSeed;


		//inline U32 GetComponentNumMessages(U32 u32Type) { return m_aComponentInfo[u32Type].u32NumMessages; }

//		static void TriggerDecoderCallback(U32 u32CallbackId, U32 u32StateOffset);

//		void SetOverlayTablePointer(void* pTable) { g_pOverlayTable = pTable; }

//		inline SpuFileStreamManager* GetStreamManager() { return &m_FileStreamManager; }

		void SetDmaScratchBuffer(U32 u32Size) { 
		m_pScratchDmaBuffer = new U8[u32Size];
		m_u32ScratchDmaBufferSize = u32Size; }
		inline U8* GetDmaScratchBuffer() { return m_pScratchDmaBuffer; }
		inline U32 GetDmaScratchBufferSize() { return m_u32ScratchDmaBufferSize; }

//		ComponentInfo m_aComponentInfo[Component::NumComponents];

#ifdef PLATFORM_PS3
		// cycle spu task to give other tasks some spu time
		void CycleSpuTask();

//		CellSpursEventFlag	*m_pFrameFinishedEvent;
#endif

    private:
        static System*			m_pSystem;
		//U32						m_u32CurrentCodeOverlayId;
		//void*					m_pOverlayTable;
//		SpuFileStreamManager	m_FileStreamManager;
		U8*						m_pScratchDmaBuffer;
		U32						m_u32ScratchDmaBufferSize;
		bool					m_bAsyncDmaCommandIssued;
	};
};


#endif	//	_SYSTEM_HEADERS_SPU_H_
