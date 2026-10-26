#ifndef _CORE_H_
#define _CORE_H_

//#define AUDIO_OPTIMIZE_OFF
//#define AUDIO_DSP_DEBUG

//#pragma warning( disable: 4996 )

// audio spu code is bigger in release builds than in speeds and we're sometimes running out of spu memory..
// so for now, disable dsp in release builds on spu
#if defined(PLATFORM_SPU) && defined(CFG_RELEASE)
#define AUDIO_DISABLE_DSP
#endif

#define MAX_NUM_SURROUND_SPEAKERS 7

#define MAX_STREAMER_FILE_PATH_LENGTH	64

#ifdef PLATFORM_SPU
#define audio_virtual
#else
#define audio_virtual	virtual
#endif

#ifdef PLATFORM_PS3
#include "vec_types.h"
#endif

#include "../common/types.h"
#include "../common/SampleTypes.h"

//#define AUDIO_CHECK_SAMPLES

// with this defined a stringhash will be created for each component.
// components can then be looked up by stringhash instead of by string.
#ifndef PLATFORM_SPU
#define AUDIO_STRING_HASH_COMPONENTS
#ifndef CFG_GOLD
#define AUDIO_STRING_HASH_OBJECT_NAMES
#define DUMP_PROCESSING_STATS
#endif
#endif


#define AUDIO_CORE_ALIGN_16(x) ((x + 0xf) & ~0xf)

#define AUDIO_MATHS_VECTOR_OPTIMISE_DECODER_XMA
#define AUDIO_MATHS_VECTOR_OPTIMISE_FRAME
#define AUDIO_MATHS_VECTOR_OPTIMISE_SATURATOR
#if ! defined (PLATFORM_SPU) && ! defined (PLATFORM_PS3)
#define AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1
#define AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1
#endif
#define AUDIO_MATHS_VECTOR_OPTIMISE_FILL_F32_BUFFER_FROM_S16
#define AUDIO_MATHS_VECTOR_OPTIMISE_RESAMPLE

#if defined( PLATFORM_PC )
#define AUDIO_REMOVE_DENORMALS
#endif

#if !defined(PLATFORM_SPU) && !defined(PLATFORM_PS3)
#define VF_DECODERS
#endif

#if defined(PLATFORM_PC)
#pragma warning(disable:4996)
#endif

#define AUDIO_MAX_INSTANCE_BUFFERS (1000)


#define NUM_AUDIO_OVERLAYS 2

#define NO_MERCURY_ASSERT
#include <System/Perf/Profiler.h>
#include <System/perf.h>

// jkc- currently saves 2.5k on spu code on release builds
#if defined(PLATFORM_SPU) && defined(CFG_RELEASE)
#define SPU_STRIP_PERF_FROM_AUDIO_RELEASE
#endif

#if defined(CFG_GOLD)
#define AUDIO_PERF_EVENT(Tag)
#define AUDIO_PERF_MICRO_EVENT(Tag)
#define AUDIO_PERF_BEGIN_EVENT(Tag)
#define AUDIO_PERF_END_EVENT()
#else // !CFG_GOLD
#if defined(CFG_SPEED) || defined(SPU_STRIP_PERF_FROM_AUDIO_RELEASE)
#define AUDIO_PERF_EVENT(Tag) PERF_EVENT(Tag)
#define AUDIO_PERF_MICRO_EVENT(Tag)
#define AUDIO_PERF_BEGIN_EVENT(Tag) Perf::BeginNamedEvent(Tag)
#define AUDIO_PERF_END_EVENT() Perf::EndNamedEvent()
#else // !CFG_SPEED
#define AUDIO_PERF_EVENT(Tag) PERF_EVENT(Tag)
#define AUDIO_PERF_MICRO_EVENT(Tag) PERF_EVENT(Tag)
#define AUDIO_PERF_BEGIN_EVENT(Tag) Perf::BeginNamedEvent(Tag)
#define AUDIO_PERF_END_EVENT() Perf::EndNamedEvent()
#endif // CFG_SPEED
#endif // CFG_GOLD

#define BUFFER_STACK_MAX_SIZE 16

#define FS_STREAM_HANDLE				Audio::PagedArrayHandle

namespace Sys
{
	class I_File;
}

namespace Audio
{
	enum SPUAudioTag
	{
		eScheduler,
		eAudioTask,
		ePhysicsFastJoint,
		ePhysicsGetContacts,
		ePhysicsRayTrace,
		ePhysicsStaticCache,
		ePhysicsSolve,
		eLoadOverlayTag,
		eObjectSetupDMAsTag,
		eObjectSetupTag,
		eObjectUpdateTag,
		eObjectGenerateTag,
		eObjectShutdownTag,
		ePCMDecoderTag,
		eXMADecoderTag,
		eADPCMDecoderTag,
		eWaveTag,
		eMixerTag,
		eMemCpyTag,
		eFirstOverlayTag,
		eNextTag = eFirstOverlayTag + NUM_AUDIO_OVERLAYS,
	};

	extern bool g_bUseAltConnectMode;

	// forward decl
	class File;

////////////////////////////////////////////////////////////////////////////////
//	Core
////////////////////////////////////////////////////////////////////////////////
	namespace Core
	{
#if defined(PLATFORM_PS3)
        const U32 uSampleRate = 48000;
#elif defined(PLATFORM_PC)
		// [Blur2PC] T144: retail Blur PC mixes at 24000 Hz in 128-sample frames, whatever the output rate:
		// its sample-rate globals (0x0113f7ac/b0/b4/b8) are only ever set from 24000 (0x0053f660, 0x0053f6d0),
		// its frame-length globals hold 128 and 1/128 (0x01028c40/44), and every renderer buffer is 128
		// samples per channel (0x020c7299, 0x00560000). The output rate reaches the mix only through
		// fOutputRateRatio below.
        const U32 uSampleRate = 24000;
#else
        const U32 uSampleRate = 48000;
#endif
#if defined(PLATFORM_PC)
        const U32 uFrameLength = 128;	// [Blur2PC] T144: retail PC (was 512)
#else
        const U32 uFrameLength = 256;
#endif //PLATFORM_PC

        extern const F32 fSampleRate;
        extern const F32 fNyquist;
        extern const F32 fFrameLength;

		extern const F32 fRcpFrameLength;

		extern const F32 fRcpSampleRate;

#if defined(PLATFORM_PC)
		// [Blur2PC] T144: retail's uSampleRate / output sample rate (0x01028c3c). The decoders scale each voice's
		// pitch by it (retail 0x00541f40, 0x00542190, 0x00542300, 0x005425a0, 0x00542a20, 0x0054337d,
		// 0x00543520, 0x005439a0), so voices keep their pitch at any output rate.
		extern F32 fOutputRateRatio;
		//! [Blur2PC name] 0x0053f660: back to 1 (called when the output engine is created).
		void ResetOutputSampleRate();
		//! [Blur2PC name] 0x0053f6d0
		void SetOutputSampleRate(U32 uOutputSampleRate);
#endif

        extern U32 uFrameNum;

		typedef F32 Sample;

////////////////////////////////////////////////////////////////////////////////
//	wave server enumerated types
////////////////////////////////////////////////////////////////////////////////
		enum WaveStreamType
		{
			eUnknownType = 0,
			eFileStream,
			eMP3,

			NumWaveStreamTypes
		};


		enum StreamSourceType
		{
			eUnknownSource = 0,

			eFile
		};


		enum WaveServerMessageType
		{
			eMessageTypeMP3 = 0,
			eMessageTypeFileStream,
			eMessageTypeDeallocateResource
		};

////////////////////////////////////////////////////////////////////////////////
//	FileStreamFileWrapper
////////////////////////////////////////////////////////////////////////////////
        class FileStreamFileWrapper
        {

			friend class FileStreamManager;
			friend class WaveStreamLoadingThread;

        private:
			FileStreamFileWrapper();
			~FileStreamFileWrapper();

			bool				Load(const char* strFilename);
			bool				Open();
			bool				Close();
			bool				GetHeader(U32 u32HeaderSize, void* pDest);
			bool				RewindToStart();
			Sys::I_File*		GetFilePointer();
			inline void			FlagForRelease() { m_bFlaggedForRelease = true; }
			inline bool			IsReleasing() { return m_bFlaggedForRelease; }
			inline bool			IsNotInUse() { return (m_u32StreamRefCount == 0); }
			inline void			IncRefCount() { m_u32StreamRefCount++; }
			inline void			DecRefCount() { m_u32StreamRefCount--; }
			inline bool			IsOpen() { return (m_File != 0); }

			Sys::I_File*		m_File;
			char				m_strFilename[MAX_STREAMER_FILE_PATH_LENGTH];
			U32					m_u32HeaderSize;
			U32					m_u32StreamRefCount;
			bool				m_bLoaded;
			bool				m_bFlaggedForRelease;
		};

////////////////////////////////////////////////////////////////////////////////
//	WaveStreamReadRequest
////////////////////////////////////////////////////////////////////////////////
		struct WaveStreamReadRequest
		{
			FileStreamFileWrapper*	m_pFileWrapper;
			U32						m_u32BytesToRead;
			U32						m_u32SeekPosition;
			void*					m_pDestBuffer;
#ifdef PLATFORM_PS3
			void*					m_pDestBufferR;
#endif
			WaveServerHandle		m_hStream;
			U32						m_u32RequestId;
			bool					m_bLoop;
			bool					m_bTempAmaxNullFileReadHack;
			bool					m_bDeallocateResources;
			U8						m_u8ReadRequestType;
		};

////////////////////////////////////////////////////////////////////////////////
//	WaveStreamData
////////////////////////////////////////////////////////////////////////////////
		struct WaveStreamData
		{
			WaveStreamData() { Init(); }

			bool Destroy();
			void Init()
			{
				u32BytesConsumed = 0;
				bPlaying = false;
				bFinished = false;
				bResourcesAllocated = false;
				u32SourceDataPosition = 0;
				u32SourceDataLength = 0;
				u32LoopCount = 0;
				u32BytesAvailable = 0;
				hWaveData.SetInvalidHandle();
				bRequiresUpdate = false;
				u32LocalOffset = 0;
			}
			U32 GetBytesAvailable() { return u32BytesAvailable; }
			U8* GetData(U32 u32NumBytes, U32* pu32StreamsToUpdate)
			{
				SetToUpdate(pu32StreamsToUpdate);

				if (! pData)
					return 0;

				U32 u32BytesToConsume = (u32NumBytes < u32BytesAvailable ? u32NumBytes : u32BytesAvailable);
				u32BytesConsumed += u32BytesToConsume;
				u32BytesAvailable -= u32BytesToConsume;
				U8* pu8RetAddr = &(pData[u32LocalOffset]);
				u32LocalOffset += u32BytesToConsume;
				return pu8RetAddr;
			}
			bool IsFinished() { return bFinished; }
			bool IsPlaying() { return bPlaying; }
			void SetPlaying(bool bParamPlaying, U32* pu32StreamsToUpdate) { bPlaying = bParamPlaying; SetToUpdate(pu32StreamsToUpdate); }
			bool IsPrimed() { return bResourcesAllocated; }
			void Prime(U32* pu32StreamsToUpdate) { SetPlaying(true, pu32StreamsToUpdate); }
			U32 GetSourceDataPosition() { return u32SourceDataPosition; }
			void SetSourceDataPosition(U32 u32Pos, U32* pu32StreamsToUpdate) { u32SourceDataPosition = u32Pos; SetToUpdate(pu32StreamsToUpdate); }
			U32 GetSourceDataLength() { return u32SourceDataLength; }
//			U32 GetSourceTotalBytesRemainingInStream() { return u32SourceDataLength - u32SourceDataPosition; }	// TODO: need to actually set u32SourceDataLength somewhere before this will work
			U32 GetLoopCount() { return u32LoopCount; }
			void ResetLoopCount() { u32LoopCount = 0; }
			void SetToUpdate(U32* pu32StreamsToUpdate) { Assert (hWaveData.IsValid()); if (! bRequiresUpdate) (*(pu32StreamsToUpdate))++; bRequiresUpdate = true; }
			U32 GetBytesConsumed() { return u32BytesConsumed; }
			void OverrideBytesConsumed(U32 u32OverrideValue)
			{
				S32 s32Diff = (S32)u32BytesConsumed - (S32)u32OverrideValue;
				u32LocalOffset -= s32Diff;
				u32BytesAvailable += s32Diff;
				u32BytesConsumed = u32OverrideValue;
			}

			U32 u32BytesConsumed;
			U32 u32BytesAvailable;
			U32 u32SourceDataPosition;
			U32 u32SourceDataLength;
			U32 u32LoopCount;
			U32 u32LocalOffset;
			U8* pData;
			StreamHandle hWaveData;
			bool bPlaying;
			bool bFinished;
			bool bResourcesAllocated;
			bool bRequiresUpdate;
		};

////////////////////////////////////////////////////////////////////////////////
//	WaveServerRegisterSourceParams
////////////////////////////////////////////////////////////////////////////////
		struct WaveServerRegisterSourceParams
		{
			WaveServerRegisterSourceParams()
			{
				m_eType = eUnknownSource;
				m_pSourceData = 0;
			}

			StreamSourceType		m_eType;
			void*					m_pSourceData;
		};

////////////////////////////////////////////////////////////////////////////////
//	WaveServerStreamCreateParams
////////////////////////////////////////////////////////////////////////////////
		struct WaveServerStreamCreateParams
		{
			StreamSourceHandle	m_hSourceHandle;	// will contain stream handle for file streamer
			void*				m_pMemoryResidentData;
			U32					m_u32StreamEncodedDataFormat;

			U32					m_u32DataProcessMaxChunkSize;	// size of return buffers. maximum number of bytes GetData() can return
			U32					m_u32FileReadChunkSize;			// number of bytes we read from disk in one file read
			U32					m_u32NumInitialWriteChunks;
			U32					m_u32BufferSizeInReadChunks;	// number of chunks of size m_u32FileReadChunkSize we've got for holding data
			U32					m_u32FileStartPosition;
			U32					m_u32FileEndPosition;
			U32					m_u32ReturnBufferAlignment;
			U32					m_u32HeaderSkipSize;
			U32					m_u32MaxReturnBufferSize;
			bool				m_bReturnBufferPhysicalMemory;
			bool				m_bLoop;
			bool				m_bPrecache;
			bool				m_bTempAmaxNullFileReadHack;
			U8					m_u8NumChannels;

			#ifndef CFG_SPEED
			Btl::String			m_strBankFilename;
			Btl::String			m_strWaveName;
			#endif

			WaveServerStreamCreateParams()
			{
				m_pMemoryResidentData = 0;
				m_u32StreamEncodedDataFormat = 0;
				m_u32DataProcessMaxChunkSize = 0;
				m_u32FileReadChunkSize = 0;
				m_u32NumInitialWriteChunks = 0;
				m_u32BufferSizeInReadChunks = 0;
				m_u32FileStartPosition = 0;
				m_u32FileEndPosition = 0;
				m_u32ReturnBufferAlignment = 0;
				m_u32HeaderSkipSize = 0;
				m_u32MaxReturnBufferSize = 0;
				m_bReturnBufferPhysicalMemory = false;
				m_bLoop = false;
				m_bPrecache = false;
				m_u8NumChannels = 1;
				m_bTempAmaxNullFileReadHack = false;
			}
		};

////////////////////////////////////////////////////////////////////////////////
//	AdpcmWaveInfo
////////////////////////////////////////////////////////////////////////////////

		enum AdpcmWaveBufferRequest
		{
			eNoRequest = 0,
			eAllocateBuffer,
			eReleaseBuffer
		};

		enum AdpcmWaveBufferState
		{
			eBufferNotAllocated = 0,
			eBufferAllocated
		};

		struct AdpcmWaveInfo
		{
			void* pPrevDecodedFrameBuffer;
			U16 u16BufferIndex;
			U8 u8BufferRequest;
			U8 u8BufferState;

			AdpcmWaveInfo() { Init(); }

			void Init()
			{
				pPrevDecodedFrameBuffer = 0;
				u8BufferRequest = eNoRequest;
				u8BufferState = eBufferNotAllocated;
				u16BufferIndex = 0;
			}
		};

////////////////////////////////////////////////////////////////////////////////
//	debug struct
////////////////////////////////////////////////////////////////////////////////
        struct Internals
        {
            Internals()
            {
                uTotalNumGenerators = 0;
                uTotalNumMixers = 0;
                uTotalNumWaves = 0;
                uNumGeneratorsActive = 0;
                uNumMixersActive = 0;
                uNumWavesActive = 0;
            };

            U32 uTotalNumGenerators;
            U32 uTotalNumMixers;
            U32 uTotalNumWaves;
            U32 uNumGeneratorsActive;
            U32 uNumMixersActive;
            U32 uNumWavesActive;
        };

////////////////////////////////////////////////////////////////////////////////
//	lightweight core update struct
////////////////////////////////////////////////////////////////////////////////
		struct DecoderDataInfo
		{
			void* pWaveServerData;
			void* pAdpcmBufferData;
		};

////////////////////////////////////////////////////////////////////////////////
//	core update struct
////////////////////////////////////////////////////////////////////////////////
        struct ProcessInfo
        {
			U32 uFrameNum;
            U16 uFrameInt;
            U16 uFrameMod;
			F32 fFrameInt;
			F32 fFrameMod;
			F32 fFraction;
			F32 fWidth;

			U8* pCoreData;

			F32* pDynamicBuffers;
			F32* pStaticBuffers;

			DecoderDataInfo decoderDataInfo;

			bool bMutePrev;
			bool bMuteNext;
			bool bSuspendPrev;
			bool bSuspendNext;	//32
        };

		// lightweight core struct

////////////////////////////////////////////////////////////////////////////////
//	DSP
////////////////////////////////////////////////////////////////////////////////
	    namespace DSP
	    {
            template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
            class FFT;

		    class Effect;

			void RealFFT_Split(F32 *data,long n);
			void IRealFFT_Split(F32 *data,long n);
        }

		enum Type
		{
			eUndefined = -1,
			eInput,
			eOscillator,
			eWave,
			eMixer,
			eOutput,
			eReturn,

			//	dsp
			//eFeedbackDelay,

			eNumTypes
		};

        class Decoder;

		struct Item
		{
			Type eType;
			U32 uOffset;
		};

		class Input;

		typedef void (*RenderCallback)(void* pContext);

		class Renderer;

		class BufferStack
		{
		public:
			BufferStack() : m_uCurrDepth( 0 )
			{
				for( U32 uCount = 0; uCount < BUFFER_STACK_MAX_SIZE; uCount++ )
					m_aSize[ uCount ] = 0;
			}

			inline void SetDepth( U32 uCurrDepth )
			{
				Assert( uCurrDepth < BUFFER_STACK_MAX_SIZE );
				m_uCurrDepth = uCurrDepth;
			}
			inline void SetSize( U32 uSize ) { m_aSize[ m_uCurrDepth ] = m_aSize[ m_uCurrDepth ] > uSize ? m_aSize[ m_uCurrDepth ] : uSize; }

			inline U32 GetDepth() { return m_uCurrDepth; }
			inline U32 GetSize() { return m_aSize[ m_uCurrDepth ]; }
			inline U32 GetSize( U32 uDepth )
			{
				Assert( uDepth < BUFFER_STACK_MAX_SIZE );
				return m_aSize[ uDepth ];
			}
			inline U32 GetOffset( U32 uDepth )
			{
				Assert( uDepth < BUFFER_STACK_MAX_SIZE );
				U32 uOffset = 0;
				for( U32 uCount = 0; uCount < uDepth; uCount++ )
					uOffset += m_aSize[ uCount ];

				return uOffset;
			}

			inline U32 GetFootprint()
			{
				U32 uOffset = 0;
				for( U32 uCount = 0; uCount < BUFFER_STACK_MAX_SIZE; uCount++ )
					uOffset += m_aSize[ uCount ];

				return uOffset;
			}

		private:
			U32 m_aSize[ BUFFER_STACK_MAX_SIZE ];
			U32 m_uCurrDepth;
		};

		WaveStreamData* GetWaveData(WaveServerLocalIndex hWaveData, const void* pWaveServerData);
		AdpcmWaveInfo* GetAdpcmBufferData(U32 uAdpcmWaveInfoIndex, const void* pAdpcmBufferData);
	};


////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
    Core::Renderer* CreateRenderer();

	extern bool g_bQuitAllTasks;
};

bool ProcessOutput(const Audio::Core::ProcessInfo& info, bool bProcess, U32 uInput, U32 uOffset);

#include "wavrenderer.h"

#define AUDIOMEMCPY( dest, source, size )			\
	{												\
		U32 nrQWords = size >> 4;					\
		qword* pSrc = (qword*)source;				\
		qword* pDest = (qword*)dest;				\
		for (U32 x = 0 ; x < nrQWords ; x++)		\
		{											\
			*pDest = *pSrc;							\
			pDest++;								\
			pSrc++;									\
		}											\
	}												\

//	Bring in ATG FFT

#endif // _CORE_H_

