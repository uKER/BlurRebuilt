#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <core\threads.h>

#include "..\common\thread.h"

#if defined(PLATFORM_PC)
#include "al.h"
#include "alc.h"
#include "efx.h"
#include "xram.h"
#include "efx-creative.h"
#include "EFX-Util.h"
#endif // USE_OPENAL


#if defined (PLATFORM_PS3)
#include <sysutil/sysutil_sysparam.h> // for CellAudioOutConfiguration
#include <cell/audio.h> // for CellAudioPortParam, CellAudioPortConfig
#include "memcpyinfo.h"
#endif

#include "..\BatchFrameAdd.h"


#define AUDIO_MAX_NUM_REVERBS 8

#define MAX_FRAME_UPDATE_INFO 16

namespace Audio
{
    namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
		class Renderer : public Thread
		{
		public:
			enum Mode
			{
				eOverwrite,
				eAdd,
				eMix
			};

			enum State
			{
				eActive = 0,
				eShutdownPending,
				eShutdownPhase2Pending,
				eShutdownComplete
			};

		public:
#if defined(PLATFORM_PC)
			static void* operator new(size_t sz);
			static void operator delete(void* ptr);
#endif
 		public:
			Renderer(U32 uNumBuffers);
			audio_virtual ~Renderer();

			//	Thread functionality
			audio_virtual void Execute(void*);

			bool Init(void* pContext = NULL, RenderCallback cbFrameStart = NULL, RenderCallback cbFrameEnd = NULL, U32 uNumReverbs = 1, bool bStereoReverb = false);

			/// signal worker thread to do some work
			void SignalEvent();

			inline void Start()
			{
				Thread::Start(NULL);
				m_bStarted = true;
			}
            inline void Stop() { m_bStarted = false; }

			// [Blur2PC] T144: returns false when the next frame is not rendered yet (retail PC 0x020c7299); the PC
			// output engine waits for it. The other callers ignore the result.
			bool GetNextFrame(F32** pfSurroundLFE, F32** pfSurround, F32** pfStereo, F32** pfMono);
			void GetNextReverbFrame(F32** pfReverb, U32 uIndex);
			//void GetNextReverbFrame2(F32** pfReverb);

			static void FoldDown(F32* pSamplesOut, U32 uChOut, F32* pSamplesIn, U32 uChIn, Mode eMode = eMix, F32 fLevel = 1.f);
			static Audio::BatchFrame::cBatch m_BatchProcess;

			inline Core::Input* MonoInput() { return m_pMonoInput; }
			inline Core::Input* StereoInput() { return m_pStereoInput; }
			inline Core::Input* SurroundInput() { return m_pSurroundInput; }
			inline Core::Input* SurroundLFEInput() { return m_pSurroundLFEInput; }

			inline Core::Input* MonoReverbInput(U32 uIndex) { return m_pMonoReverbInput[uIndex]; }
			//inline Core::Input* MonoReverbInput2() { return m_pMonoReverbInput2; }

			U32 GetFramesDropped(){return m_uDropped;}

			void MuteOutput() { m_bOutputMuted = true; }
			void UnmuteOutput() { m_bOutputMuted = false; }
			bool IsOutputMuted() { return m_bOutputMuted; }
			inline F32 GetLevel( U32 uCh )
			{
				float val = m_fPeak[ uCh ];
				m_fPeak[ uCh ] = 0.f;
				return val;
			}

#if defined(PLATFORM_PC)
			void SetReverb(EAXREVERBPROPERTIES* pSettings, U32 uIndex);
#endif	//	PLATFORM_PC

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#ifdef AUDIO_USE_OLD_REVERB
			void SetReverb(CellMSFXReverbParams* pSettings, U32 uIndex);
#else
			void SetReverb(CellMSFXI3DL2Params* pSettings, U32 uIndex);
#endif	//	AUDIO_USE_OLD_REVERB
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3


		private:
			void ClearFrame(S32 sFrameNum);
			void ClearReverbFrame(S32 sFrameNum, U32 uIndex);
			//void ClearReverb2Frame(S32 sFrameNum);
			F32* MonoFrameData(S32 sFrameNum);
			F32* StereoFrameData(S32 sFrameNum);
			F32* SurroundFrameData(S32 sFrameNum);
			F32* SurroundLFEFrameData(S32 sFrameNum);
			F32* ReverbFrameData(S32 sFrameNum, U32 uIndex);

			F32 m_fPeak[ 8 ];

//#ifdef PLATFORM_XENON
//			F32* ReverbFrameData2(S32 sFrameNum);
//#endif	//	PLATFORM_XENON

#if defined (PLATFORM_PS3)
            static void AudioThreadProc(uint64_t uParam);
            void MainLoop();
#endif // _PS3

#ifdef AUDIO_USE_XMP_CAPTURE
    // XMP Callback
    static void CaptureCallback( LPCXMP_CAPTURE_BUFFER pSampleBuffer, LPVOID pContext, BOOL fTitleRendering );
#endif
		private:
			static const char* m_pchName;
			const U32 m_uNumBuffers;

            bool m_bInited;
            bool m_bStarted;

			U32 m_uNumReverbs;

			volatile S32 m_sNextWrite;
			volatile S32 m_sCurrRead;

			volatile S32 m_sNextReverbWrite[AUDIO_MAX_NUM_REVERBS];
			volatile S32 m_sCurrReverbRead[AUDIO_MAX_NUM_REVERBS];

			//volatile S32 m_sNextReverb2Write;
			//volatile S32 m_sCurrReverb2Read;

			U32 m_uDropped;

            F32* m_pfMonoBufferData;
            F32* m_pfStereoBufferData;
            F32* m_pfSurroundBufferData;
            F32* m_pfSurroundLFEBufferData;
            F32* m_pfMonoReverbBufferData[AUDIO_MAX_NUM_REVERBS];
			//F32* m_pfMonoReverbBufferData2;

			Audio::Core::Input* m_pMonoInput;
			Audio::Core::Input* m_pStereoInput;
			Audio::Core::Input* m_pSurroundInput;
			Audio::Core::Input* m_pSurroundLFEInput;

			Audio::Core::Input* m_pMonoReverbInput[AUDIO_MAX_NUM_REVERBS];
			//Audio::Core::Input* m_pMonoReverbInput2;
	


			U32 m_u32MultistreamPacketSubmitSize;

            void* m_pContext;
            RenderCallback m_cbFrameStart;
            RenderCallback m_cbFrameEnd;

#if defined (PLATFORM_PS3)
			bool m_bSurround;

            sys_ppu_thread_t m_hAudioThread;

            CellAudioOutConfiguration m_Config;
            CellAudioPortParam m_Param;
            CellAudioPortConfig m_PortConfig;

#ifdef PS3_OUTPUT_TO_MULTISTREAM
			F32* m_pMonoMultiStreamDryPrimaryBuffer;
			F32* m_pStereoMultiStreamDryPrimaryBuffer;
			F32* m_pSurroundMultiStreamDryPrimaryBuffer;

			F32* m_pMonoMultiStreamDrySecondaryBuffer;
			F32* m_pStereoMultiStreamDrySecondaryBuffer;
			F32* m_pSurroundMultiStreamDrySecondaryBuffer;

			F32* m_pMonoMultiStreamWetPrimaryBuffer;

			F32* m_pMonoMultiStreamWetSecondaryBuffer;

			U32 m_u32NumMultiStreamChannels;
#endif

            sys_addr_t			m_pPort;
            uint32_t			m_uPort;
            sys_addr_t			m_pReadIndex;

			sys_event_queue_t	m_queue;
			uint64_t			m_queueKey;


			S32 m_s32DryStreamIndex;
			S32 m_s32WetStreamIndex;
			S32 m_s32WetSubBuss;

			char* m_pReverbParams;
			void* m_pReverbBuffer;
			void* m_pReverbDsp;
			S32 m_hDsp;
#ifdef PS3_OUTPUT_TO_MULTISTREAM
			CellMSDSP m_DSPInfo;
#endif	//	PS3_OUTPUT_TO_MULTISTREAM

#endif // _PS3

			volatile State m_eState;

			Sys::I_File* m_File;
			bool m_bWriteFile;
			bool m_bRecording;

			bool m_bOutputMuted;

			MT::Event m_Event; // event for signalling the audio thread to do some processing
		};
	};
};

#endif // _PROCESSOR_H_

