#pragma once

// [Blur2PC] T144: retail Blur PC's audio output engine (Blur.exe 1.2, read with Ghidra; addresses below and in
// docs/analysis.md, "Audio output (PC)"). It replaces the snapshot's OpenAL engine and its unshipped
// DirectSound-notification engine (CPrimarySound), neither of which retail has: retail imports DSOUND.dll and
// ships no OpenAL. One looping DirectSound secondary buffer is fed by a writer thread that takes each
// 128-sample renderer frame, mixes it down to the device's speaker layout and copies it in as soon as the
// play cursor has freed room. The buffer holds "Audio.PC.Engine.NumberOfBuffers" blocks of 256 frames, set
// to output rate / 1500 unless "Audio.PC.Engine.NumberOfBuffersOverride" is on. The output rate is the
// audio quality option (24000, 36000 or 48000 Hz; C_SystemSubsystem, Audio::ISystem::SetOutputSampleRate).
// Class, member and function names are ours [Blur2PC name]; retail keeps no symbols for them.

#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>

namespace Audio
{
	namespace Core
	{
		class Renderer;

		enum Error
		{
			eSuccess = 0,
			eUninitialized,
			eFailedToCreateDirectSound,
			eFailedToSetCoopLevel,
			eFailedToCreatePrimaryBuffer,
			eFailedToCreateDirectSoundBuffer,
			eFailedToSetPrimaryBufferFormat,
			eFailedToLockPrimaryBuffer,
			eQueryInterfaceFailed,
			eSetNotificationPositionsFailed,
			eRendererExists,
			eVoiceInputMixExists,
			eUnknownEffect,
			eOutOfMemory,

			eNumErrors
		};

		//! [Blur2PC name] Retail's 200-byte output engine object (constructor 0x00546f10), created on first use by
		//! Instance() (0x0054b100, inlined at every call site in retail).
		class Engine
		{
		public:
			//! [Blur2PC name] One DirectSoundEnumerate entry (retail: 0x34-byte records filled by 0x00546fb0).
			struct Device
			{
				GUID		guid;
				Btl::String	description;
				Btl::String	module;
				void*		pContext;
			};

		public:
			Engine();
			~Engine();

			static Engine& Instance(void);

			//! 0x00538150: remembers the renderer, enumerates the output devices and starts the first one.
			bool Init(Renderer* pRenderer);

			//! 0x020c5fb8 (DRM-relocated): restarts the output at a new sample rate (the audio quality option).
			void SetOutputSampleRate(U32 uSampleRate);

			//! 0x00547870, the "Audio.PC.Engine.ChangeFormat" debug command: restarts the output with the
			//! current debug values.
			static void ChangeFormat(void* pContext);

			//! 0x00544060: stops the writer thread, the buffer and the device. Always returns true.
			bool Shutdown();

		protected:
			//! 0x00547710: creates the device, its buffer and the writer thread.
			bool StartDevice(U32 uDeviceIndex);

			//! 0x00546a90: creates the looping buffer in the device's speaker layout.
			HRESULT CreateBuffer(IDirectSound8* pDS, IDirectSoundBuffer8** ppBuffer, DWORD dwChannelMask);

			//! 0x00546d00: waits until the play cursor has freed uSize bytes, then copies them in.
			bool WriteToBuffer(const void* pData, U32 uSize);

			//! 0x00547250: one renderer frame mixed down to m_uChannels interleaved floats, clamped to [-1, 1].
			void RenderFrameFloat(F32* pfOut);
			//! 0x00547500
			void RenderFrameS16(S16* psOut);
			//! 0x005475a0
			void RenderFrameS32(S32* psOut);

			//! 0x00544130
			void MixMono(const F32* pfIn, F32* pfOut, U32 uChannels);
			//! 0x00544400
			void MixStereo(const F32* pfIn, F32* pfOut, U32 uChannels);
			//! 0x005458c0
			void MixSurroundLFE(const F32* pfIn, F32* pfOut, U32 uChannels);

			//! 0x00547630
			static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
			//! 0x00546fb0
			static BOOL CALLBACK EnumerateCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);

			inline Renderer* GetRenderer(void)
			{
				return m_pRenderer;
			}

		protected:
			Renderer*				m_pRenderer;		// +0x00
			IDirectSound8*			m_pDS;				// +0x04
			IDirectSoundBuffer8*	m_pBuffer;			// +0x08
			Btl::Vector<Device>		m_aDevices;			// +0x0c
			DSCAPS					m_Caps;				// +0x18
			DSBUFFERDESC			m_BufferDesc;		// +0x78
			U32						m_uWritePos;		// +0x9c
			HANDLE					m_hThread;			// +0xa0
			HANDLE					m_hThreadDone;		// +0xa8
			HANDLE					m_hThreadStop;		// +0xac
			U32						m_uDeviceIndex;		// +0xb0
			U32						m_uSampleRate;		// +0xb4
			U32						m_uBitsPerSample;	// +0xb8
			bool					m_bUseFloats;		// +0xbc
			U32						m_uChannels;		// +0xc0
			DWORD					m_dwSpeakerConfig;	// +0xc4
			bool					m_bTimerPeriodSet;	// [Blur2PC] T255: timeBeginPeriod(1) held while the engine runs

			static Engine*			s_pInstance;		// 0x0113f7e4
		};
	}
}
