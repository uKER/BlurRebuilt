#include "AudioPch.h"
#include "coreheaders.h"
#include "../SystemHeaders.h"

#include <System/memory/MemoryTracker.h>

#include <system/Perf/PerfEvent.h>

//#define PS3_MP3_TEST

//#include "../common/file.h"

#if defined(PLATFORM_PC)
#include <winbase.h>
#endif // PLATFORM_PC



Debug::Bool g_bTestNoise("Audio.System.TestNoise", false);
Debug::Int g_iTestChannel("Audio.System.TestChannel", 1, Debug::Range(0, 5));

Debug::Bool g_bDisableReverb("Audio.System.DisableReverb", false);
Debug::Bool g_bEnableWet("Audio.System.Reverb.EnableWet", true);
Debug::Bool g_bEnableDry("Audio.System.Reverb.EnableDry", true);
Debug::Bool g_bClearDroppedFrames("Audio.System.ClearDroppedFrames", true);
#ifdef PLATFORM_PS3
Debug::Int	g_AudioFramesToRender("Audio.System.AudioFramesToRender", 1, Debug::Range(1, (_AMAX_CELL_AUDIO_BLOCK-1)));
#endif

#ifndef CFG_SPEED
Debug::Bool g_bTestTone("Audio.System.RendererTestTone", false);
#endif

Debug::Float g_MasterAttenuation("Audio.System.MasterEffectAttenuation", 1.0f, Debug::Range(0.f, 1.f));

bool g_bPrevReverbDisabled = false;


#if defined (PLATFORM_PS3)
#define USE_PS3_LIBAUDIO_EVENT
#include "sys/ppu_thread.h"
#ifdef PS3_MP3_TEST
#define NUM_MP3S_TO_TEST		128
#define MP3_NUM_CHANNELS		2
#define MP3_START_DELAY_FRAMES	5
//Audio::MP3DecodeManager m_MP3Manager;
Audio::MP3Handle hMP3;
U32 g_u32MP3Size = 0;
char* gMP3DataBufferMem = 0;
U32 u32NumMP3sActive = 0;
U32 u32NumMP3sToUpdate = NUM_MP3S_TO_TEST;
class MP3Instance
{

public:
	void Init(char* pBufferMem, U32 u32MP3Size, U32 u32NumChannels)
	{
		if (! ps16MP3DataLeft)
			ps16MP3DataLeft = new S16[1024];
		if (! ps16MP3DataRight)
			ps16MP3DataRight = new S16[1024];

		hMP3 = m_MP3Manager.CreateMP3Stream(pBufferMem, u32MP3Size, u32NumChannels);
		bActive = true;
	}
	void Release()
	{
		m_MP3Manager.ReleaseMP3Stream(hMP3);
		bActive = false;
	}

	Audio::MP3Handle hMP3;
	S16* ps16MP3DataLeft;
	S16* ps16MP3DataRight;
	bool bActive;
};
MP3Instance mp3Instance[NUM_MP3S_TO_TEST];
#endif
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#define MULTISTREAM_FRAME_SIZE	512
//#include "system/spu/spurs.h"
//extern CellSpurs Spurs::spurs __attribute__((aligned(128)));
#ifdef AUDIO_USE_OLD_REVERB
extern uint32_t _binary_mstream_dsp_reverb_pic_start;
extern uint32_t _binary_mstream_dsp_reverb_pic_end;
extern uint32_t _binary_mstream_dsp_reverb_pic_size;
#else
extern uint32_t _binary_mstream_dsp_i3dl2_pic_start;
extern uint32_t _binary_mstream_dsp_i3dl2_pic_end;
extern uint32_t _binary_mstream_dsp_i3dl2_pic_size;
#endif	//	AUDIO_USE_OLD_REVERB
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif


#if defined(PLATFORM_PC)
#define ENABLE_MONO_OUT
#define ENABLE_STEREO_OUT
#endif	//	PLATFORM_PC
//#define ENABLE_MONO_OUT
//#define ENABLE_STEREO_OUT
//#define ENABLE_SURROUND_OUT
#define ENABLE_SURROUNDLFE_OUT
//#define ENABLE_REVERB_OUT

#if defined(PLATFORM_PC)

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif

const char* Audio::Core::Renderer::m_pchName = "Renderer";

Audio::BatchFrame::cBatch Audio::Core::Renderer::m_BatchProcess;


#ifdef PLATFORM_PS3

void FrameUpdateFinishInfo_AddMemCpy(FrameUpdateFinishInfo *pInfo, void *pDest, void *pSrc, int cSize)
{
	MemCpyInfo *pMemCpy = &pInfo->cpyInfo[pInfo->nMemcpys++];
	pMemCpy->pDst = pDest;
	pMemCpy->pSrc = pSrc;
	pMemCpy->cSize = cSize;
}

#endif


#if defined(PLATFORM_PC)


////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::Core::Renderer::operator new(size_t sz)
{
	return System::Alloc(sz);
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::operator delete(void* ptr)
{
	System::Free(ptr);
}

#endif

////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
#if defined (PLATFORM_PC)
Audio::Core::Renderer::Renderer(U32 uNumBuffers) : Thread(64, eHighest), m_uNumBuffers(uNumBuffers)
#else
Audio::Core::Renderer::Renderer(U32 uNumBuffers) : Thread(64, eNormal), m_uNumBuffers(uNumBuffers)
#endif	//	PLATFORM_XENON
{
	m_bInited = false;
    m_bStarted = false;
	m_uNumReverbs = 0;
	m_sNextWrite = 0;
	m_sCurrRead = -1;
	m_uDropped = 0;
    m_cbFrameStart = NULL;
	m_pfMonoBufferData = NULL;
	m_pfStereoBufferData = NULL;
	m_pfSurroundBufferData = NULL;
	m_pfSurroundLFEBufferData = NULL;
	m_pMonoInput = NULL;
	m_pStereoInput = NULL;
	m_pSurroundInput = NULL;
	m_pSurroundLFEInput = NULL;
	for (S32 i=0; i<AUDIO_MAX_NUM_REVERBS; i++)
	{
		m_sNextReverbWrite[i] = 0;
		m_sCurrReverbRead[i] = -1;
		m_pfMonoReverbBufferData[i] = NULL;
		m_pMonoReverbInput[i] = NULL;
	}
	//m_pMonoReverbInput2 = NULL;
    m_pContext = NULL;
    m_cbFrameStart = NULL;
#if defined (PLATFORM_PS3)
#ifdef PS3_OUTPUT_TO_MULTISTREAM
	m_bSurround = true;

	m_pMonoMultiStreamDryPrimaryBuffer = NULL;
	m_pStereoMultiStreamDryPrimaryBuffer = NULL;
	m_pSurroundMultiStreamDryPrimaryBuffer = NULL;
	m_pMonoMultiStreamDrySecondaryBuffer = NULL;
	m_pStereoMultiStreamDrySecondaryBuffer = NULL;
	m_pSurroundMultiStreamDrySecondaryBuffer = NULL;
	m_pMonoMultiStreamWetPrimaryBuffer = NULL;
	m_pMonoMultiStreamWetSecondaryBuffer = NULL;

	m_u32MultistreamPacketSubmitSize = MULTISTREAM_FRAME_SIZE;
#else
	m_bSurround = true;
#endif
    m_hAudioThread = NULL;
    m_uPort = (S32)-1;
    m_pReadIndex = NULL;
	m_pReverbParams = 0;
	m_pReverbDsp = NULL;
	m_pReverbBuffer = NULL;

#endif // _PS3

	m_bOutputMuted = false;

	m_bWriteFile = false;
	m_bRecording = false;

	m_eState = eActive;
	m_BatchProcess.Init( 384 );
}

////////////////////////////////////////////////////////////////////////////////
//	~Renderer
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Renderer::~Renderer()
{
	// signal the renderer threads to shutdown and wait til it has
	m_eState = eShutdownPending;
#ifdef PLATFORM_PS3
	uint64_t exitStatus;
	sys_ppu_thread_join(m_hAudioThread, &exitStatus);
#endif
	WaitUntilTerminated();

	if (m_pfMonoBufferData)
	{
		System::Free(m_pfMonoBufferData);
		m_pfMonoBufferData = NULL;
	}

	if (m_pfStereoBufferData)
	{
		System::Free(m_pfStereoBufferData);
		m_pfStereoBufferData = NULL;
	}

	if (m_pfSurroundBufferData)
	{
		System::Free(m_pfSurroundBufferData);
		m_pfSurroundBufferData = NULL;
	}

	if (m_pfSurroundLFEBufferData)
	{
		System::Free(m_pfSurroundLFEBufferData);
		m_pfSurroundLFEBufferData = NULL;
	}

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
	if (m_pMonoMultiStreamDryPrimaryBuffer)
	{
		delete m_pMonoMultiStreamDryPrimaryBuffer;
		m_pMonoMultiStreamDryPrimaryBuffer = NULL;
	}

	if (m_pStereoMultiStreamDryPrimaryBuffer)
	{
		delete m_pStereoMultiStreamDryPrimaryBuffer;
		m_pStereoMultiStreamDryPrimaryBuffer = NULL;
	}

	if (m_pSurroundMultiStreamDryPrimaryBuffer)
	{
		delete m_pSurroundMultiStreamDryPrimaryBuffer;
		m_pSurroundMultiStreamDryPrimaryBuffer = NULL;
	}

	if (m_pMonoMultiStreamDrySecondaryBuffer)
	{
		delete m_pMonoMultiStreamDrySecondaryBuffer;
		m_pMonoMultiStreamDrySecondaryBuffer = NULL;
	}

	if (m_pStereoMultiStreamDrySecondaryBuffer)
	{
		delete m_pStereoMultiStreamDrySecondaryBuffer;
		m_pStereoMultiStreamDrySecondaryBuffer = NULL;
	}

	if (m_pSurroundMultiStreamDrySecondaryBuffer)
	{
		delete m_pSurroundMultiStreamDrySecondaryBuffer;
		m_pSurroundMultiStreamDrySecondaryBuffer = NULL;
	}

	if (m_pMonoMultiStreamWetPrimaryBuffer)
	{
		delete m_pMonoMultiStreamWetPrimaryBuffer;
		m_pMonoMultiStreamWetPrimaryBuffer = NULL;
	}

	if (m_pMonoMultiStreamWetSecondaryBuffer)
	{
		delete m_pMonoMultiStreamWetSecondaryBuffer;
		m_pMonoMultiStreamWetSecondaryBuffer = NULL;
	}

#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3

	for (S32 i=0; i<AUDIO_MAX_NUM_REVERBS; i++)
	{
		if (m_pfMonoReverbBufferData[i])
		{
			System::Free(m_pfMonoReverbBufferData[i]);
			m_pfMonoReverbBufferData[i] = NULL;
		}
	}

	//if (m_pfMonoReverbBufferData2)
	//{
	//	System::Free(m_pfMonoReverbBufferData2);
	//	m_pfMonoReverbBufferData2 = NULL;
	//}
}

/// signal worker thread to do some work
void Audio::Core::Renderer::SignalEvent()
{
	m_Event.Signal();
}

#ifdef PLATFORM_PC

#include "engine_pc.h"

bool Audio::Core::Renderer::Init(void* pContext, RenderCallback cbFrameStart, RenderCallback cbFrameEndL, U32 uNumReverbs, bool bStereoReverb )
{
    if (m_bInited == false)
    {
		m_uNumReverbs = uNumReverbs;

        m_pContext = pContext;
        m_cbFrameStart = cbFrameStart;
        m_cbFrameEnd = cbFrameEndL;

		Audio::Core::Engine::Instance().Init(this);

		U32 uSize;
#ifdef ENABLE_MONO_OUT
		U8* puMonoInput = new U8[ Core::Input::InputSize( 1, 0 ) ];
		Input* pMonoInput = new(puMonoInput) Core::Input( m_pchName, 1, 0 );
        uSize = 1 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfMonoBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pMonoInput->SetAltBuffer(m_pfMonoBufferData);
		memset(m_pfMonoBufferData, 0, uSize);
		m_pMonoInput = pMonoInput;
#endif // ENABLE_MONO_OUT

#ifdef ENABLE_STEREO_OUT
		U8* puStereoInput = new U8[ Core::Input::InputSize( 2, 0 ) ];
		Input* pStereoInput = new(puStereoInput) Core::Input( m_pchName, 2, 0 );
        uSize = 2 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfStereoBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pStereoInput->SetAltBuffer(m_pfStereoBufferData);
        memset(m_pfStereoBufferData, 0, uSize);
		m_pStereoInput = pStereoInput;
#endif // ENABLE_STEREO_OUT

#ifdef ENABLE_SURROUND_OUT
		U8* puSurroundInput = new U8[ Core::Input::InputSize( 5, 0 ) ];
		Input* pSurroundInput = new(puSurroundInput) Core::Input( m_pchName, 5, 0 );
        uSize = 5 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfSurroundBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pSurroundInput->SetAltBuffer(m_pfSurroundBufferData);
        memset(m_pfSurroundBufferData, 0, uSize);
		m_pSurroundInput = pSurroundInput;
#endif // ENABLE_SURROUND_OUT

#ifdef ENABLE_SURROUNDLFE_OUT
		U8* puSurroundLFEInput = new U8[ Core::Input::InputSize( 6, 0 ) ];
		Input* pSurroundLFEInput = new(puSurroundLFEInput) Core::Input( m_pchName, 6, 0 );
        uSize = 6 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfSurroundLFEBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pSurroundLFEInput->SetAltBuffer(m_pfSurroundLFEBufferData);
        memset(m_pfSurroundLFEBufferData, 0, uSize);
		m_pSurroundLFEInput = pSurroundLFEInput;
#endif // ENABLE_SURROUNDLFE_OUT

		//	reverb
#ifdef	ENABLE_REVERB_OUT
		for (U32 i=0; i<m_uNumReverbs; i++)
		{
			U8* pMonoReverbMem = new U8[ Core::Input::InputSize( 1, 0 ) ];
			Input* pMonoReverbInput = new(pMonoReverbMem) Core::Input( m_pchName, 1, 0 );
			uSize = (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
			m_pfMonoReverbBufferData[i] = (F32*)System::Alloc(uSize + uFrameLength, 128);
			pMonoReverbInput->SetAltBuffer(m_pfMonoReverbBufferData[i]);
			memset(m_pfMonoReverbBufferData[i], 0, uSize);
			m_pMonoReverbInput[i] = pMonoReverbInput;
		}
#endif // ENABLE_REVERB_OUT
	}

	return true;
}

#endif


#ifdef PLATFORM_PC
void Audio::Core::Renderer::SetReverb(EAXREVERBPROPERTIES* efxReverb, U32 uIndex)
{
#ifdef USE_EAX_REVERB	
	ALuint uiEffect = Audio::Core::Engine::Instance().GetEffect();
	ALuint uiEffectSlot = Audio::Core::Engine::Instance().GetEffectSlot();

	EFXEAXREVERBPROPERTIES pEFXEAXReverb;
	
	ConvertReverbParameters(efxReverb, &pEFXEAXReverb);

	pEFXEAXReverb.flDensity = 0.15f; ///HARD CODED FOR EMMA

	//if (pEFXEAXReverb)
	{
		// Clear AL Error code
		alGetError();

		if (!Audio::Core::Engine::SetEFXEAXReverbProperties(&pEFXEAXReverb, uiEffect))
			AUDIO_CORE_DEBUG("Failed to set Reverb Parameters\n");

		if (alGetError()!= AL_NO_ERROR)
			AUDIO_CORE_DEBUG("Error setting reverb params");

		//// Load Effect into Auxiliary Effect Slot
		alAuxiliaryEffectSloti(uiEffectSlot, AL_EFFECTSLOT_EFFECT, uiEffect);
	}
#endif


}
#endif	//	PLATFORM_PC

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#ifdef AUDIO_USE_OLD_REVERB
void Audio::Core::Renderer::SetReverb(CellMSFXReverbParams* pSettings, U32 uIndex)
{
	cellMSFXReverbSetParams(m_pReverbParams, m_pReverbBuffer, pSettings, -1);
}
#else
void Audio::Core::Renderer::SetReverb(CellMSFXI3DL2Params* pSettings, U32 uIndex)
{
	for (int i=0; i<8; i++)
		cellMSFXI3DL2SetParams(m_pReverbParams, m_pReverbBuffer, pSettings, i);
}
#endif	//	AUDIO_USE_OLD_REVERB
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3


#if defined (PLATFORM_PS3)
////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Renderer::Init(void* pContext, RenderCallback cbFrameStart, RenderCallback cbFrameEndL, U32 uNumReverbs, bool bStereoReverb)
{
	bool bSetupOk;
	//extern Audio::Core::ADPCMBufferAllocator g_AdpcmBufferAllocator;
	//g_AdpcmBufferAllocator.Init();

    if (m_bInited == false)
    {
		m_uNumReverbs = uNumReverbs;

		U32 uSize;
#ifdef ENABLE_MONO_OUT
		U8* puMonoInput = new U8[ Core::Input::InputSize( 1, 0 ) ];
		Input* pMonoInput = new(puMonoInput) Core::Input( m_pchName, 1, 0 );
        uSize = 1 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfMonoBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pMonoInput->SetAltBuffer(m_pfMonoBufferData);
		memset(m_pfMonoBufferData, 0, uSize);
		m_pMonoInput = pMonoInput;
#ifdef PS3_OUTPUT_TO_MULTISTREAM
		m_pMonoMultiStreamDryPrimaryBuffer = (F32*)System::Alloc(uSize);
		m_pMonoMultiStreamDrySecondaryBuffer = (F32*)System::Alloc(uSize);
        memset(m_pMonoMultiStreamDryPrimaryBuffer, 0, uSize);
        memset(m_pMonoMultiStreamDrySecondaryBuffer, 0, uSize);
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif // ENABLE_MONO_OUT

#ifdef ENABLE_STEREO_OUT
		U8* puStereoInput = new U8[ Core::Input::InputSize( 2, 0 ) ];
		Input* pStereoInput = new(puStereoInput) Core::Input( m_pchName, 2, 0 );
        uSize = 2 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfStereoBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pStereoInput->SetAltBuffer(m_pfStereoBufferData);
        memset(m_pfStereoBufferData, 0, uSize);
		m_pStereoInput = pStereoInput;
#ifdef PS3_OUTPUT_TO_MULTISTREAM
		m_pStereoMultiStreamDryPrimaryBuffer = (F32*)System::Alloc(uSize);
		m_pStereoMultiStreamDrySecondaryBuffer = (F32*)System::Alloc(uSize);
        memset(m_pStereoMultiStreamDryPrimaryBuffer, 0, uSize);
        memset(m_pStereoMultiStreamDrySecondaryBuffer, 0, uSize);
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif // ENABLE_STEREO_OUT

#ifdef ENABLE_SURROUND_OUT
		U8* puSurroundInput = new U8[ Core::Input::InputSize( 5, 0 ) ];
		Input* pSurroundInput = new(puSurroundInput) Core::Input( m_pchName, 5, 0 );
        uSize = 5 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfSurroundBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pSurroundInput->SetAltBuffer(m_pfSurroundBufferData);
        memset(m_pfSurroundBufferData, 0, uSize);
		m_pSurroundInput = pSurroundInput;
#endif // ENABLE_SURROUND_OUT

#ifdef ENABLE_SURROUNDLFE_OUT
		U8* puSurroundLFEInput = new U8[ Core::Input::InputSize( 6, 0 ) ];
		Input* pSurroundLFEInput = new(puSurroundLFEInput) Core::Input( m_pchName, 6, 0 );
        uSize = 6 * (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
		m_pfSurroundLFEBufferData = (F32*)System::Alloc(uSize + uFrameLength, 128);
		pSurroundLFEInput->SetAltBuffer(m_pfSurroundLFEBufferData);
        memset(m_pfSurroundLFEBufferData, 0, uSize);
		m_pSurroundLFEInput = pSurroundLFEInput;
#endif // ENABLE_SURROUNDLFE_OUT

#ifdef PS3_OUTPUT_TO_MULTISTREAM
		m_u32NumMultiStreamChannels = 6;
		U32 u32MultiStreamBufferSize = m_u32NumMultiStreamChannels * m_u32MultistreamPacketSubmitSize * sizeof(F32);
		U32 u32MultiStreamWetBufferSize = m_u32MultistreamPacketSubmitSize * sizeof(F32);
		
		m_pSurroundMultiStreamDryPrimaryBuffer = (F32*)System::Alloc(u32MultiStreamBufferSize, 128);
		m_pSurroundMultiStreamDrySecondaryBuffer = (F32*)System::Alloc(u32MultiStreamBufferSize, 128);
		
        memset(m_pSurroundMultiStreamDryPrimaryBuffer, 0, u32MultiStreamBufferSize);
        memset(m_pSurroundMultiStreamDrySecondaryBuffer, 0, u32MultiStreamBufferSize);
        
		m_pMonoMultiStreamWetPrimaryBuffer = (F32*)System::Alloc(u32MultiStreamWetBufferSize, 128);
		m_pMonoMultiStreamWetSecondaryBuffer = (F32*)System::Alloc(u32MultiStreamWetBufferSize, 128);
		
        memset(m_pMonoMultiStreamWetPrimaryBuffer, 0, u32MultiStreamWetBufferSize);
        memset(m_pMonoMultiStreamWetSecondaryBuffer, 0, u32MultiStreamWetBufferSize);
#endif
#ifdef PS3_MP3_TEST
		DebugBreak();
		m_MP3Manager.Init();
		gMP3DataBufferMem = new char[1024 * 1024 * 6];	// 6 meg!!
		File pMP3File;
		bool bYes = pMP3File.Open("/app_home/test.mp3");
		assert(bYes);
		g_u32MP3Size = pMP3File.Size();
		pMP3File.Read(gMP3DataBufferMem, g_u32MP3Size);
#endif

		//	reverb
		for (S32 i=0; i<m_uNumReverbs; i++)
		{
			U8* pMonoReverbMem = new U8[ Core::Input::InputSize( 1, 0 ) ];
			Input* pMonoReverbInput = new(pMonoReverbMem) Core::Input(m_pchName, 1, 0);
			uSize = (m_uNumBuffers + 1) * uFrameLength * sizeof(F32);
			m_pfMonoReverbBufferData[i] = (F32*)System::Alloc(uSize + uFrameLength, 128);
			pMonoReverbInput->SetAltBuffer(m_pfMonoReverbBufferData[i]);
			memset(m_pfMonoReverbBufferData[i], 0, uSize);
			m_pMonoReverbInput[i] = pMonoReverbInput;
		}

        m_pContext = pContext;
		m_cbFrameStart = cbFrameStart;
        m_cbFrameEnd = cbFrameEndL;

#ifdef PS3_OUTPUT_TO_MULTISTREAM
		U32 u32retSysUtil = cellMSSystemConfigureSysUtilEx(CELL_MS_AUDIOMODESELECT_SUPPORTSLPCM | CELL_MS_AUDIOMODESELECT_SUPPORTSDOLBY | CELL_MS_AUDIOMODESELECT_SUPPORTSDTS | CELL_MS_AUDIOMODESELECT_PREFERDTS);
		U32 u32numChans = (u32retSysUtil & 0x0000000F);
		U32 u32dolbyOn = (u32retSysUtil & 0x00000010);

		if (u32dolbyOn)
			AUDIO_SYS_DEBUG("Dolby On\n");
		else
			AUDIO_SYS_DEBUG("Dolby Off\n");

		AUDIO_SYS_DEBUG("Channels %d\n", u32numChans);

#else	// libAudio

		CellAudioOutState state;
		cellAudioOutGetState(1, CELL_AUDIO_OUT_PRIMARY, &state);

		int LPCMuseablechannels = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM, CELL_AUDIO_OUT_FS_48KHZ, 0);
		int AC3useablechannels = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3, CELL_AUDIO_OUT_FS_48KHZ, 0);
		int DTSuseablechannels = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS, CELL_AUDIO_OUT_FS_48KHZ, 0);

	    (void)memset(&m_Config, 0, sizeof(CellAudioOutConfiguration));

		if (LPCMuseablechannels > 2)		// if surround linear PCM is available, default to that...
		{
			m_Config.channel = Maths::Min(6, LPCMuseablechannels);
			m_Config.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
			m_Config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B;	// downmix to 6 channels
		}
		else if (AC3useablechannels > 0)
		{
			m_Config.channel = AC3useablechannels;
			m_Config.encoder   = CELL_AUDIO_OUT_CODING_TYPE_AC3;
			m_Config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B;	// downmix to 6 channels
		}
		else if (DTSuseablechannels > 0)
		{
			m_Config.channel = DTSuseablechannels;
			m_Config.encoder   = CELL_AUDIO_OUT_CODING_TYPE_DTS;
			m_Config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B;	// downmix to 6 channels
		}
		else	// only stereo PCM available...
		{
			m_Config.channel = LPCMuseablechannels;
			m_Config.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
			m_Config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_A;	// downmix to 2 channels
		}

		if (cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &m_Config, NULL, 0) != CELL_OK)
        {
            AUDIO_CORE_ERROR("cellAudioOutConfigure() failed");
		    return false;
        }
#endif	//	PS3_OUTPUT_TO_MULTISTREAM

	    //	init audio system
	    if (cellAudioInit() != CELL_OK)
        {
            AUDIO_CORE_ERROR("cellAudioInit() failed");
		    return false;
        }
    	
	    //  open audio port
		m_Param.nChannel = CELL_AUDIO_PORT_8CH;

	    m_Param.nBlock = _AMAX_CELL_AUDIO_BLOCK;
		m_Param.attr = 0;

	    if (cellAudioPortOpen(&m_Param, &m_uPort) != CELL_OK)
        {
		    cellAudioQuit();
            AUDIO_CORE_ERROR("cellAudioPortOpen() failed");
		    return false;
	    }
    	
	    //  get port config
	    if (cellAudioGetPortConfig(m_uPort, &m_PortConfig) != CELL_OK)
        {
		    cellAudioQuit();
            AUDIO_CORE_ERROR("cellAudioGetPortConfig() failed");
		    return false;
	    }
    	
        m_pPort = m_PortConfig.portAddr;
        m_BatchProcess.SetAudioPort( m_pPort );
        m_pReadIndex = m_PortConfig.readIndexAddr;

#ifdef PS3_OUTPUT_TO_MULTISTREAM
		bSetupOk = (cellMSSystemConfigureLibAudio(&m_Param, &m_PortConfig) != -1);

		CellMSSystemConfig msConfig;
    	
		float fBusVols[64] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

		float fBusVolsMute[64] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

		msConfig.channelCount = 7;
		msConfig.subCount = 1;
		msConfig.dspPageCount = 1;

		msConfig.flags = /*CELL_MS_DISABLE_SPU_PRINTF_SERVER | */CELL_MS_ROUTABLE_STREAMS_FLAG;

		S32 s32MemReq = cellMSSystemGetNeededMemorySize(&msConfig);

		U8* pSystemMemory = (U8*)(System::Alloc(s32MemReq, 128));

		if (! pSystemMemory)
			return false;

// this setup method used SPURS, which we don't use anymore..
		//U8 au8SpursTaskPriority[8] = {1, 1, 1, 1, 1, 1, 1, 1};

		//if (bSetupOk)
		//	bSetupOk = (cellMSSystemInitSPURS((void*)pSystemMemory, &msConfig, &Spurs::spurs, au8SpursTaskPriority) == 0);

		bSetupOk = (cellMSSystemInitSPUThread((void*)pSystemMemory, &msConfig, 100) == 0);

		if (bSetupOk)
		{
			// Setup the sub bus we will use
			cellMSCoreSetVolume64(CELL_MS_SUBBUS_1, CELL_MS_DRY, fBusVolsMute);
			cellMSCoreSetVolume64(CELL_MS_SUBBUS_1, CELL_MS_WET, fBusVols);

			// Setup the final output channel volumes
			cellMSCoreSetVolume64(CELL_MS_MASTER_BUS, CELL_MS_DRY, fBusVols);

			cellMSCoreRoutingInit(2);
		}

		bool bLoadReverb = true;

		if (bSetupOk && bLoadReverb)
		{
			bSetupOk = false;
#ifdef AUDIO_USE_OLD_REVERB
			U32 reverbPicSize = (U32)(void *)&_binary_mstream_dsp_reverb_pic_end - (U32)(void *)&_binary_mstream_dsp_reverb_pic_start;
#else
			U32 reverbPicSize = (U32)(void *)&_binary_mstream_dsp_i3dl2_pic_end - (U32)(void *)&_binary_mstream_dsp_i3dl2_pic_start;
#endif
			U8* u8TempReverbExeBuffer = (U8 *)System::Alloc(reverbPicSize, 256);

			if (! u8TempReverbExeBuffer)
				return false;

#ifdef AUDIO_USE_OLD_REVERB
			memcpy(u8TempReverbExeBuffer, (void *)&_binary_mstream_dsp_reverb_pic_start, reverbPicSize);
#else
			memcpy(u8TempReverbExeBuffer, (void *)&_binary_mstream_dsp_i3dl2_pic_start, reverbPicSize);
#endif
			m_hDsp = cellMSDSPLoadDSPFromMemory(u8TempReverbExeBuffer, &m_DSPInfo, CELL_MS_ALLOC_DSPHANDLE,CELL_MS_ALLOC_PAGE);
//			System::Free(u8TempReverbExeBuffer);

			if (m_hDsp != -1)
				bSetupOk = true;

			//File picFile;

			//if (picFile.Open("/app_home/Image/audio/mstream_dsp_reverb.pic"))
			//{
			//	U32 u32Size = picFile.Size();

			//	if (u32Size)
			//	{
			//		m_pReverbDsp = System::Alloc(u32Size, 128);

			//		if (m_pReverbDsp)
			//		{
			//			U32 u32BytesRead = 0;
			//			picFile.Read(m_pReverbDsp, u32Size, &u32BytesRead);

			//			if (u32BytesRead == u32Size)
			//			{
			//				m_hDsp = cellMSDSPLoadDSPFromMemory(m_pReverbDsp, &m_DSPInfo, CELL_MS_ALLOC_DSPHANDLE,CELL_MS_ALLOC_PAGE);

			//				if (m_hDsp != -1)
			//					bSetupOk = true;
			//			}
			//		}
			//	}
			//}
		}

		if (bSetupOk)
		{
			m_s32DryStreamIndex = cellMSStreamOpen();

			if (m_s32DryStreamIndex != -1)
				bSetupOk = (cellMSCoreInit(m_s32DryStreamIndex) != -1);
			else
				bSetupOk = false;
		}

		if (bSetupOk)
		{
			m_s32WetStreamIndex = cellMSStreamOpen();

			if (m_s32WetStreamIndex != -1)
				bSetupOk = (cellMSCoreInit(m_s32WetStreamIndex) != -1);
			else
				bSetupOk = false;
		}

		if (! bSetupOk)
		{
			AUDIO_CORE_ERROR("Multistream setup failed");
			return false;
		}

		// let's set up some reverb

		m_s32WetSubBuss = 1 | CELL_MS_BUS_FLAG;

		if (bLoadReverb)
		{
			// Set DSP effect to slot 0 of stream
			cellMSCoreSetDSP(m_s32WetSubBuss, CELL_MS_DSP_SLOT_0, m_hDsp);
			m_pReverbParams = (char*)cellMSCoreGetDSPParamAddr(m_s32WetSubBuss, CELL_MS_DSP_SLOT_0);

			// Get size required for 1 active reverb for a required slot.
			
			// TODO: if we only specify 1 channel, we get memory crashes in wavebank code! look into this
#ifdef AUDIO_USE_OLD_REVERB
			U32 u32ReverbSize = cellMSFXReverbGetNeededMemorySize(8);
#else
			U32 u32ReverbSize = cellMSFXI3DL2GetNeededMemorySize(8);
#endif

			m_pReverbBuffer = System::Alloc(u32ReverbSize, 128);
			memset(m_pReverbBuffer, 0, u32ReverbSize);
			
			//	Initialise the Reverb DSP.
			//	Set the initial pitch to 0.0f (original pitch). This automatically sets pitches for all multi-channel dsp effects
			//	Set to use to process three audio channels		

#ifdef AUDIO_USE_OLD_REVERB
			CellMSFXReverbParams msReverbParams;

			msReverbParams.fWetVolume = 1.0f;
			msReverbParams.fDryVolume = 0.0f;
			msReverbParams.fPreDelayMs = 60.0f;
			msReverbParams.fLowPassHz = 10000.0f;
			msReverbParams.fHighPassHz = 1000.0f;
			msReverbParams.fRoomSize = 0.4f;
			msReverbParams.fDamping = 0.0f;
			msReverbParams.fDiffusion = 1.0f;
			msReverbParams.fEarlyLate = 0.1f;
			msReverbParams.fLeftRight = 0.5f;
			msReverbParams.fFrontBack = 0.5f;

			if (cellMSFXReverbInit(m_pReverbParams, m_pReverbBuffer, &msReverbParams, 7/*1*/) < 0)
			{
				AUDIO_CORE_ERROR("reverb setup failed.");
				int lastError = cellMSSystemGetLastError();
				return false;
			}
#else
			CellMSFXI3DL2Params i3dl2params;
			i3dl2params.Room = 0.0f;
			i3dl2params.Room_HF = -100.0f; 
			i3dl2params.Decay_time = 8.402f;
			i3dl2params.Decay_HF_ratio = 0.6f;
			i3dl2params.Reflections = -600.0f;
			i3dl2params.Reflections_delay = 0.102f;
			i3dl2params.Reverb = -600.0f;
			i3dl2params.Reverb_delay = 0.0602f;
			i3dl2params.Diffusion = 98.0f;
			i3dl2params.Density = 97.0f;
			i3dl2params.HF_reference = 5000.0f;


			// extended parameters ( not in standard )
			i3dl2params.EarlyReflectionPattern = CELL_MSFX_ROOM1_LEFT;  // currently set either 0 or 1, this modifys the signals slightly to create 'stereo' effects
			i3dl2params.LateReverbPattern = CELL_MSFX_ROOM1_LATE_LEFT;
			i3dl2params.EarlyReflectionScaler = 0.05f;
			i3dl2params.MixChannel = -1;           // channel index to crossfade / mix signal with for stereo -1 == off, 0 to 7 == mix chan
			i3dl2params.MixAmount = 20.0f;         // percentage to cross-mix with MixChannel              0 to 100 %
			i3dl2params.LF_reference = 100.0f;     // reference low frequency                              20 to 20000 Hertz
			i3dl2params.Room_LF = 0.0f;

			if (cellMSFXI3DL2Init(m_pReverbParams, m_pReverbBuffer, &i3dl2params, 8 ) < 0)
			{
				AUDIO_CORE_ERROR("reverb setup failed.");
				int lastError = cellMSSystemGetLastError();
				return false;
			}
#endif

			for (U32 i = 0; i < 8; i++)
			{
				if( i%2 == 0 )
					i3dl2params.MixChannel = i+1;
				else
					i3dl2params.MixChannel = i-1;

				bSetupOk = cellMSFXI3DL2SetParams( m_pReverbParams, m_pReverbBuffer, &i3dl2params, i );

				cellMSCoreSetMask(m_s32WetSubBuss, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, i, 0);	// Allow input from none
				cellMSCoreSetMask(m_s32WetSubBuss, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, i, 0);	// Allow input from none
			}
			for (U32 i = 0; i < 8; i++)
			{
				cellMSCoreSetMask(m_s32WetSubBuss, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, i, 1<<i);	// Allow input from speaker n
				cellMSCoreSetMask(m_s32WetSubBuss, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, i, 1<<i);	// Allow output to speaker n
			}

		// Set the DSP slot (containing the Reverb DSP) to process twice
		//	cellMSCoreSetMask(PLAYSUB, CELL_MS_INMASK, CELL_MS_DSP_SLOT_0, 0, 0xff);	// Allow input from all
		//	cellMSCoreSetMask(PLAYSUB, CELL_MS_OUTMASK, CELL_MS_DSP_SLOT_0, 0, 0xff);	// Output to all

		// Turn DSP effect on for slot 0
			cellMSCoreBypassDSP(m_s32WetSubBuss, CELL_MS_DSP_SLOT_0, CELL_MS_NOTBYPASSED);
		}

		//	set up voices

		CellMSInfo msInfo;

		msInfo.SubBusGroup = CELL_MS_MASTER_BUS;
		msInfo.FirstBuffer = (void*)m_pSurroundMultiStreamDryPrimaryBuffer;
		msInfo.FirstBufferSize = u32MultiStreamBufferSize;
		msInfo.SecondBuffer = (void*)m_pSurroundMultiStreamDrySecondaryBuffer;
		msInfo.SecondBufferSize = u32MultiStreamBufferSize;
		msInfo.Pitch = 48000;
		msInfo.numChannels = m_u32NumMultiStreamChannels;
		msInfo.initialOffset = 0;
		msInfo.inputType = CELL_MS_32BIT_FLOAT;
		msInfo.flags = CELL_MS_STREAM_NOFLAGS;

		bSetupOk = (cellMSStreamSetInfo(m_s32DryStreamIndex, &msInfo) != -1);

		if (! bSetupOk)
		{
			AUDIO_CORE_ERROR("multistream voice setup failed.");
			int lastError = cellMSSystemGetLastError();
		#ifndef CFG_GOLD
			printf("%d", lastError);
		#endif
			return false;
		}

		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FL, CELL_MS_CHANNEL_0, 1.0f);
		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FR, CELL_MS_CHANNEL_1, 1.0f);
		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FC, CELL_MS_CHANNEL_2, 1.0f);
		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_LFE, CELL_MS_CHANNEL_3, 1.0f);
		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_RL, CELL_MS_CHANNEL_4, 1.0f);
		cellMSCoreSetVolume1(m_s32DryStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_RR, CELL_MS_CHANNEL_5, 1.0f);

		cellMSStreamPlay(m_s32DryStreamIndex);

		// now the wet (reverb) stream

		msInfo.SubBusGroup = m_s32WetSubBuss;
		msInfo.FirstBuffer = (void*)m_pMonoMultiStreamWetPrimaryBuffer;
		msInfo.FirstBufferSize = u32MultiStreamWetBufferSize;
		msInfo.SecondBuffer = (void*)m_pMonoMultiStreamWetSecondaryBuffer;
		msInfo.SecondBufferSize = u32MultiStreamWetBufferSize;
		msInfo.Pitch = 48000;
		msInfo.numChannels = 1;
		msInfo.initialOffset = 0;
		msInfo.inputType = CELL_MS_32BIT_FLOAT;
		msInfo.flags = CELL_MS_STREAM_NOFLAGS;

		cellMSStreamSetInfo(m_s32WetStreamIndex, &msInfo);

		cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_FL, CELL_MS_CHANNEL_0, 1.0f);
		cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_FR, CELL_MS_CHANNEL_0, 1.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_FC, CELL_MS_CHANNEL_0, 1.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_LFE, CELL_MS_CHANNEL_0, 0.0f);
		cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_RL, CELL_MS_CHANNEL_0, 1.0f);
		cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_WET, CELL_MS_SPEAKER_RR, CELL_MS_CHANNEL_0, 1.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FL, CELL_MS_CHANNEL_0, 0.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FR, CELL_MS_CHANNEL_0, 0.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_FC, CELL_MS_CHANNEL_0, 0.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_LFE, CELL_MS_CHANNEL_0, 0.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_RL, CELL_MS_CHANNEL_0, 0.0f);
		//cellMSCoreSetVolume1(m_s32WetStreamIndex, CELL_MS_DRY, CELL_MS_SPEAKER_RR, CELL_MS_CHANNEL_0, 0.0f);

		cellMSStreamPlay(m_s32WetStreamIndex);

		if (! bSetupOk)
		{
			AUDIO_CORE_ERROR("multistream voice setup failed.");
			return false;
		}


#endif	//	PS3_OUTPUT_TO_MULTISTREAM

	    // create sound thread
	    if (sys_ppu_thread_create(&m_hAudioThread, Core::Renderer::AudioThreadProc, (uint64_t)this, 86, 0x10000, 0, "Audio Renderer") != CELL_OK)
        {
//		    cellAudioQuit();
            AUDIO_CORE_ERROR("sys_ppu_thread_create() failed");
		    return false;
	    }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioThreadProc
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::AudioThreadProc(uint64_t uParam)
{
    Core::Renderer* pRenderer = (Core::Renderer*)uParam;
	pRenderer->MainLoop();
	sys_ppu_thread_exit(0);
}

////////////////////////////////////////////////////////////////////////////////
//	MainLoop
////////////////////////////////////////////////////////////////////////////////
F32* BigFatBuffer = 0;
U32 u32BigFatBufferOffset = 0;
#define BIG_FAT_BUFFER_SIZE	512 * 1024
bool bWriteToFatBuffer = false;
#ifdef PS3_OUTPUT_TO_MULTISTREAM
bool bSignallingMultiStream = false;
void Audio::Core::Renderer::MainLoop()
{
	//  start port
	if (cellAudioPortStart(m_uPort) != CELL_OK)
    {
        AUDIO_CORE_ERROR("cellAudioPortStart() failed");
		return;
	}

	if (SurroundInput())
	{
		U32 u32PrevDryCallbackRequestAddress = 0;
		U32 u32PrevWetCallbackRequestAddress = 0;

		//  infinite loop
		for(;;)
		{
			if ((! BigFatBuffer) && bWriteToFatBuffer)
				BigFatBuffer = new F32[BIG_FAT_BUFFER_SIZE];

			bool bDryDataRequired = true;
			bool bWetDataRequired = true;

			if (bDryDataRequired | bWetDataRequired)
			{
				bSignallingMultiStream = true;
				cellMSSystemSignalSPU();
				bSignallingMultiStream = false;
			}

			char* u8Array = cellMSSystemGetCallbacksNeeded();
			bDryDataRequired = (u8Array[m_s32DryStreamIndex] == CELL_MS_CALLBACK_MOREDATA);
			bWetDataRequired = (u8Array[m_s32WetStreamIndex] == CELL_MS_CALLBACK_MOREDATA);
			cellMSSystemFreeCallbacksNeeded();

			//printf("MS tick d:%d w:%d\n", bDryDataRequired?1:0, bWetDataRequired?1:0);

			volatile static U32 u32SleepTime = 500;

			if (bDryDataRequired)
			{
				CellMSInfo streamInfo;

				cellMSStreamGetInfo(m_s32DryStreamIndex, &streamInfo);

				//if ((U32)streamInfo.SecondBuffer == u32PrevDryCallbackRequestAddress)
				//	bDryDataRequired = false;
				//else
				{
					u32PrevDryCallbackRequestAddress = (U32)streamInfo.SecondBuffer;

					U32 u32CallbackRequestAddress = (U32)streamInfo.SecondBuffer;
					U32 u32CallbackRequestSize = (U32)streamInfo.SecondBufferSize;

					if (m_bSurround)
					{
						//	5.1 port
						F32* afDataOut = (F32*)u32CallbackRequestAddress;

						for (U32 i = 0; i < m_u32MultistreamPacketSubmitSize / uFrameLength; i++)
						{
							F32* pfMono = NULL;
							F32* pfStereo = NULL;
							F32* pfSurround = NULL;
							F32* pfSurroundLFE = NULL;

							//	get render buffers
							GetNextFrame(&pfSurroundLFE, &pfSurround, &pfStereo, &pfMono);

							//printf("%f, %f, %f\n", pfSurroundLFE[0], pfSurroundLFE[512], pfSurroundLFE[256]);

							if (g_bEnableDry)
							{
								F32 fTemp = -1.f;
								F32 fStep = 0.018f;
								for (U32 uCount = 0; uCount < uFrameLength; uCount++)
								{
									//afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//afDataOut[(uCount * m_u32NumMultiStreamChannels + 2) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//afDataOut[(uCount * m_u32NumMultiStreamChannels + 4) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//afDataOut[(uCount * m_u32NumMultiStreamChannels + 5) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//afDataOut[(uCount * m_u32NumMultiStreamChannels + 3) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.25f * fTemp;
									//fTemp += fStep;
									//if (fTemp >= 1.f)
									//	fTemp -= 2.f;
									afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount];
									afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount + uFrameLength];
									afDataOut[(uCount * m_u32NumMultiStreamChannels + 2) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount + 2 * uFrameLength];
									afDataOut[(uCount * m_u32NumMultiStreamChannels + 4) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount + 4 * uFrameLength];
									afDataOut[(uCount * m_u32NumMultiStreamChannels + 5) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount + 5 * uFrameLength];
									afDataOut[(uCount * m_u32NumMultiStreamChannels + 3) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = 0.5f * pfSurroundLFE[uCount + 3 * uFrameLength];
								}
							}
						}


						// temp debug code..
						if (bWriteToFatBuffer && BigFatBuffer)
						{
							for (U32 i = 0; (i < m_u32MultistreamPacketSubmitSize / uFrameLength) && bWriteToFatBuffer; i++)
							{
								for (U32 uCount = 0; (uCount < uFrameLength) && bWriteToFatBuffer; uCount++)
								{
									BigFatBuffer[u32BigFatBufferOffset++] = afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)];
									BigFatBuffer[u32BigFatBufferOffset++] = afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)];

									if (u32BigFatBufferOffset >= BIG_FAT_BUFFER_SIZE)
										bWriteToFatBuffer = false;
								}
							}
						}

					}
					else
					{
						//	stereo port
						F32* afDataOut = (F32*)u32CallbackRequestAddress;
						for (U32 i = 0; i < m_u32MultistreamPacketSubmitSize / uFrameLength; i++)
						{
							F32* pfMono = NULL;
							F32* pfStereo = NULL;
							F32* pfSurround = NULL;
							F32* pfSurroundLFE = NULL;

							//	get render buffers
							GetNextFrame(&pfSurroundLFE, &pfSurround, &pfStereo, &pfMono);

							for (U32 uCount = 0; uCount < uFrameLength; uCount++)
							{
								//afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = pfStereo[uCount];
								//afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = pfStereo[uCount + uFrameLength];

								afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = pfSurround[uCount];
								afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] += 0.707f * pfSurround[uCount + 2 * uFrameLength];
								afDataOut[(uCount * m_u32NumMultiStreamChannels) + i * (uFrameLength * m_u32NumMultiStreamChannels)] += 0.707f * pfSurround[uCount + 4 * uFrameLength];
								afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] = pfSurround[uCount + uFrameLength];
								afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] += 0.707f * pfSurround[uCount + 2 * uFrameLength];
								afDataOut[(uCount * m_u32NumMultiStreamChannels + 1) + i * (uFrameLength * m_u32NumMultiStreamChannels)] += 0.707f * pfSurround[uCount + 5 * uFrameLength];
							}

							// temp debug code..
							if (bWriteToFatBuffer && BigFatBuffer)
							{
								memcpy(BigFatBuffer + u32BigFatBufferOffset, afDataOut, uFrameLength * m_u32NumMultiStreamChannels * sizeof(F32));
								u32BigFatBufferOffset += uFrameLength * m_u32NumMultiStreamChannels;
								if (u32BigFatBufferOffset + (uFrameLength * m_u32NumMultiStreamChannels) > BIG_FAT_BUFFER_SIZE)
									bWriteToFatBuffer = false;
							}

						}
					}
				}
			}
			if (bWetDataRequired)
			{
				CellMSInfo streamInfo;

				cellMSStreamGetInfo(m_s32WetStreamIndex, &streamInfo);

				if ((U32)streamInfo.SecondBuffer == u32PrevWetCallbackRequestAddress)
					bWetDataRequired = false;
				else
				{
					u32PrevWetCallbackRequestAddress = (U32)streamInfo.SecondBuffer;

					U32 u32CallbackRequestAddress = (U32)streamInfo.SecondBuffer;
					U32 u32CallbackRequestSize = (U32)streamInfo.SecondBufferSize;

					//	5.1 port
					F32* afDataOut = (F32*)u32CallbackRequestAddress;

					for (U32 i = 0; i < m_u32MultistreamPacketSubmitSize / uFrameLength; i++)
					{
						//  write next frame
						F32* afDataIn = 0;
						GetNextReverbFrame(&afDataIn, 0);

						if (g_bEnableWet)
							memcpy((void*)(afDataOut + i * uFrameLength), (void*)afDataIn, uFrameLength * sizeof(F32));

						//for (U32 uCount = 0; uCount < uFrameLength; uCount++)
						//	afDataOut[(uCount) + i * (uFrameLength)] = afDataIn[uCount];
					}
				}
			}


			//if (!bDryDataRequired && !bWetDataRequired)
			//	sys_timer_usleep(u32SleepTime);

			// if we've processed more data, signal spu again
//			if (u8Array[m_s32DryStreamIndex] == CELL_MS_CALLBACK_MOREDATA || u8Array[m_s32WetStreamIndex] == CELL_MS_CALLBACK_MOREDATA)
			if (Audio::g_bQuitAllTasks) break;
		}
	}
	// Close LibAudio
	cellAudioRemoveNotifyEventQueue( m_queueKey );
	
	cellAudioQuit();
	
}
#else
#define AUDIO_MASTER_ATTENUATION (1.f)
bool bSineTest = false;
U32 u32PacketSubmitMultiple = 1;
void Audio::Core::Renderer::MainLoop()
{


    U32 uFrameCount = 0;
	U32 uReadFrame = 0;
	U32 uWriteFrame = 0;
	U32 uFrameSize;
	if (m_bSurround)
		uFrameSize = CELL_AUDIO_PORT_8CH * sizeof(float) * CELL_AUDIO_BLOCK_SAMPLES;
	else
		uFrameSize = CELL_AUDIO_PORT_2CH * sizeof(float) * CELL_AUDIO_BLOCK_SAMPLES;
	
	//  start port
	if (cellAudioPortStart(m_uPort) != CELL_OK)
    {
        AUDIO_CORE_ERROR("cellAudioPortStart() failed");
		return;
	}
#ifdef USE_PS3_LIBAUDIO_EVENT
	int ret = cellAudioCreateNotifyEventQueue(&m_queue, &m_queueKey);
	if (ret != CELL_OK)
	{
		return;
	}

	/* register event queue to libaudio */
	ret = cellAudioSetNotifyEventQueue(m_queueKey);
	if (ret < 0)
	{
		return;
	}
#endif
#ifdef PLATFORM_PS3
	return;
#endif

	F32* pDataOut = (F32*) BizAlignedMalloc( 16 * sizeof(F32) * 6 * uFrameLength, 128 );

	if (SurroundInput())
	{
		//  infinite loop
		for(;;)
		{
			if (m_eState != eActive)
				break;

			if ((! BigFatBuffer) && bWriteToFatBuffer)
				BigFatBuffer = new F32[BIG_FAT_BUFFER_SIZE];

			uReadFrame = *(uint64_t*)m_pReadIndex;

			//  wait until ready
			if (uReadFrame == uWriteFrame)
				sys_timer_usleep(30);
			else
			{

				//  write next frame
//				F32* afDataIn = GetNextFrame();
				F32* pfMono = NULL;
				F32* pfStereo = NULL;
				F32* pfSurround = NULL;
				F32* pfSurroundLFE = NULL;


//				static F32 pDataOut[6 * uFrameLength];
				memset(pDataOut, 0, sizeof(F32) * 6 * uFrameLength);
				m_BatchProcess.Clear();
				//	get render buffers
				GetNextFrame(&pfSurroundLFE, &pfSurround, &pfStereo, &pfMono);

				if (pfMono)
					Core::Renderer::FoldDown(pDataOut,
						6,
						pfMono,
						1,
						Core::Renderer::eMix,
						g_MasterAttenuation);

				if (pfStereo)
					Core::Renderer::FoldDown(pDataOut,
						6,
						pfStereo,
						2,
						Core::Renderer::eMix,
						g_MasterAttenuation);

				if (pfSurround)
					Core::Renderer::FoldDown(pDataOut,
						6,
						pfSurround,
						5,
						Core::Renderer::eMix,
						g_MasterAttenuation);

				if (pfSurroundLFE)
					Core::Renderer::FoldDown(pDataOut,
						6,
						pfSurroundLFE,
						6,
						Core::Renderer::eMix,
						g_MasterAttenuation);

//				static int countit = 0;
//				printf("STA: nr %d/%d count %d\n",m_BatchProcess.mFrameAddIndex, m_BatchProcess.mpQueueSize,countit++ );
				m_BatchProcess.Process();
				m_BatchProcess.Wait();
//				printf("END: nr %d/%d\n",m_BatchProcess.mFrameAddIndex, m_BatchProcess.mpQueueSize );
	
				if (m_bSurround)
				{
					//	5.1 port
					F32* afDataOut = (F32*)(m_pPort + (uWriteFrame * uFrameSize));
					if (! bSineTest)
					{
						for (U32 uCount = 0; uCount < CELL_AUDIO_BLOCK_SAMPLES; uCount++)
						{
#ifndef PS3_MP3_TEST
							afDataOut[uCount * CELL_AUDIO_PORT_8CH] = pDataOut[uCount];
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 1] = pDataOut[uCount + uFrameLength];
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 2] = pDataOut[uCount + 2 * uFrameLength];
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 3] = pDataOut[uCount + 3 * uFrameLength];
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 4] = pDataOut[uCount + 4 * uFrameLength];
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 5] = pDataOut[uCount + 5 * uFrameLength];
#else
							afDataOut[uCount * CELL_AUDIO_PORT_8CH] = 0.f;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 1] = 0.f;

							if (MP3_NUM_CHANNELS == 2)
							{
								for (U32 i = 0; i < u32NumMP3s; i++)
								{
									if (mp3Instance[i].bActive)
									{
										afDataOut[uCount * CELL_AUDIO_PORT_8CH] += ((float)(mp3Instance[i].ps16MP3DataLeft[uCount]) / 50000.f) / (i + 1);
										afDataOut[uCount * CELL_AUDIO_PORT_8CH + 1] += ((float)(mp3Instance[i].ps16MP3DataRight[uCount]) / 50000.f) / (i + 1);
									}
								}
							}
							else
							{
								for (U32 i = 0; i < u32NumMP3s; i++)
								{
									if (mp3Instance[i].bActive)
									{
										afDataOut[uCount * CELL_AUDIO_PORT_8CH] += ((float)(mp3Instance[i].ps16MP3DataLeft[uCount]) / 50000.f) / (i + 1);
										afDataOut[uCount * CELL_AUDIO_PORT_8CH + 1] += ((float)(mp3Instance[i].ps16MP3DataLeft[uCount]) / 50000.f) / (i + 1);
									}
								}
							}
#endif
						}

						// temp debug code..
						if (bWriteToFatBuffer && BigFatBuffer)
						{
							U8 bigend[4];
							U8 littleend[4];
							for (U32 uCount = 0; uCount < CELL_AUDIO_BLOCK_SAMPLES; uCount++)
							{
								*((F32*)bigend) = afDataOut[uCount * CELL_AUDIO_PORT_8CH/* + 2*/];
								littleend[0] = bigend[3];
								littleend[1] = bigend[2];
								littleend[2] = bigend[1];
								littleend[3] = bigend[0];
								BigFatBuffer[u32BigFatBufferOffset + uCount] = *((F32*)littleend);
							}
							u32BigFatBufferOffset += uFrameLength;
							if (u32BigFatBufferOffset + uFrameLength > BIG_FAT_BUFFER_SIZE)
								bWriteToFatBuffer = false;
						}
					}
					else
					{
						static U32 u32RendererFrameCount = 0;
						static F32 f32RendererWaveRate = 20.f;
						static bool bOutputOnlyCentre = true;
						u32RendererFrameCount++;
						for (U32 uCount = 0; uCount < CELL_AUDIO_BLOCK_SAMPLES; uCount++)
						{
							F32 f32RendererSineWave = sin(F32(u32RendererFrameCount * CELL_AUDIO_BLOCK_SAMPLES + uCount) / f32RendererWaveRate);

							afDataOut[uCount * CELL_AUDIO_PORT_8CH] = (bOutputOnlyCentre) ? 0.f : f32RendererSineWave;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 1] = (bOutputOnlyCentre) ? 0.f : f32RendererSineWave;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 2] = f32RendererSineWave;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 3] = (bOutputOnlyCentre) ? 0.f : f32RendererSineWave;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 4] = (bOutputOnlyCentre) ? 0.f : f32RendererSineWave;
							afDataOut[uCount * CELL_AUDIO_PORT_8CH + 5] = (bOutputOnlyCentre) ? 0.f : f32RendererSineWave;
						}
					}
				}
				else
				{
					//	stereo port
					F32* afDataOut = (F32*)(m_pPort + (uWriteFrame * uFrameSize));
					for (U32 uCount = 0; uCount < CELL_AUDIO_BLOCK_SAMPLES; uCount++)
					{
						afDataOut[uCount * CELL_AUDIO_PORT_2CH] = pDataOut[uCount];
						afDataOut[uCount * CELL_AUDIO_PORT_2CH] += 0.707f * pDataOut[uCount + 2 * uFrameLength];
						afDataOut[uCount * CELL_AUDIO_PORT_2CH] += 0.707f * pDataOut[uCount + 4 * uFrameLength];
						afDataOut[uCount * CELL_AUDIO_PORT_2CH + 1] = pDataOut[uCount + uFrameLength];
						afDataOut[uCount * CELL_AUDIO_PORT_2CH + 1] += 0.707f * pDataOut[uCount + 2 * uFrameLength];
						afDataOut[uCount * CELL_AUDIO_PORT_2CH + 1] += 0.707f * pDataOut[uCount + 5 * uFrameLength];
					}
				}
			}

			uWriteFrame = uReadFrame;
			if (Audio::g_bQuitAllTasks) break;
			//sys_timer_usleep(4800);
		}
	}
	BizAlignedFree( pDataOut );
}
#endif	// ndef PS3_OUTPUT_TO_MULTISTREAM
#endif // _PS3

#if defined(PLATFORM_PC)

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif

static volatile int MovieCapture = 0; // 1 if we are capturing sound
static volatile F32 MovieWant = 0.f; // Amount of audio data we want to capture (seconds)
static Audio::CaptureFunc MovieCallback = 0;

namespace Audio
{

// This is called by the movie capture code to capture audio data:
void CaptureFrames(CaptureFunc callback, F32 seconds)
{
	MovieCapture = 1;
	MovieCallback = callback;
	MovieWant += seconds;
}

// This is called to stop capturing audio
void CaptureStop()
{
	MovieCapture = 0;
}

}

#ifndef PLATFORM_PS3
////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::Execute(void*)
{
#ifndef PLATFORM_PS3
	// disabled on ps3 cos it started crashing when I reduced the audio update thread priority from highest to normal.. jkc
	MEMORY_TRACKER_PUSH("audio.renderer");
#endif

#ifdef PLATFORM_PC
	// [Blur2PC] T144: retail PC's renderer thread (0x00560000) raises its own priority to time-critical and
	// leaves the process priority class alone
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif

	for (;;)
	{
		//printf("writing locked frame %d (%d)\n", m_sNextWrite % m_uNumBuffers, m_sCurrRead % m_uNumBuffers);


		if (m_bStarted)
		{
			ClearFrame(0);
			for (U32 i=0; i<m_uNumReverbs; i++)
				ClearReverbFrame(0, i);
//#ifdef PLATFORM_XENON
//			ClearReverb2Frame(0);
//#endif

			//  start frame callback
			if (m_pContext && m_cbFrameStart)
				m_cbFrameStart(m_pContext);

			//memcpy(m_pfSurroundLFEBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 6 * uFrameLength, m_pfSurroundLFEBufferData, 6 * uFrameLength * sizeof(F32));
			//memcpy(m_pfSurroundBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 5 * uFrameLength, m_pfSurroundBufferData, 5 * uFrameLength * sizeof(F32));

#ifndef CFG_SPEED
			if (g_bTestTone)
			{
				F32 f32Multiple = .25f / (F32)uFrameLength;

				for (U32 i = 0; i < uFrameLength; i++)
				{
					m_pfMonoBufferData[i]								= (f32Multiple * i);

					m_pfStereoBufferData[i]								= (f32Multiple * i);
					m_pfStereoBufferData[i + uFrameLength]				= (f32Multiple * i);

					m_pfSurroundBufferData[i]							= (f32Multiple * i);
					m_pfSurroundBufferData[i + uFrameLength]			= (f32Multiple * i);
					m_pfSurroundBufferData[i + uFrameLength * 2]		= (f32Multiple * i);
					m_pfSurroundBufferData[i + uFrameLength * 3]		= (f32Multiple * i);
					m_pfSurroundBufferData[i + uFrameLength * 4]		= (f32Multiple * i);

					m_pfSurroundLFEBufferData[i]						= (f32Multiple * i);
					m_pfSurroundLFEBufferData[i + uFrameLength]			= (f32Multiple * i);
					m_pfSurroundLFEBufferData[i + uFrameLength * 2]		= (f32Multiple * i);
					m_pfSurroundLFEBufferData[i + uFrameLength * 3]		= (f32Multiple * i);
					m_pfSurroundLFEBufferData[i + uFrameLength * 4]		= (f32Multiple * i);
					m_pfSurroundLFEBufferData[i + uFrameLength * 5]		= (f32Multiple * i);
				}
			}
#endif
            
			#if defined( PLATFORM_PS3 ) && defined(SPU_MEMCPY)

			// perform the copy on the SPU
			FrameUpdateFinishInfo info;
			info.nMemcpys = 0;

			FrameUpdateFinishInfo_AddMemCpy(&info, m_pfSurroundLFEBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 6 * uFrameLength, m_pfSurroundLFEBufferData, 6 * uFrameLength * sizeof(F32));
			FrameUpdateFinishInfo_AddMemCpy(&info, m_pfSurroundBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 5 * uFrameLength, m_pfSurroundBufferData, 5 * uFrameLength * sizeof(F32));
			FrameUpdateFinishInfo_AddMemCpy(&info, m_pfStereoBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 2 * uFrameLength, m_pfStereoBufferData, 2 * uFrameLength * sizeof(F32));
			FrameUpdateFinishInfo_AddMemCpy(&info, m_pfMonoBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * uFrameLength, m_pfMonoBufferData, uFrameLength * sizeof(F32));

			for (U32 i=0; i<m_uNumReverbs; i++)
				FrameUpdateFinishInfo_AddMemCpy(&info, m_pfMonoReverbBufferData[i] + ((m_sNextReverbWrite[i] % m_uNumBuffers) + 1) * uFrameLength, m_pfMonoReverbBufferData[i], uFrameLength * sizeof(F32));

			Audio::System::Get()->RunFrameFinishMarker(&info);

			#else

			// [Blur2PC] only copy the output buffers that Init allocated: on PC ENABLE_SURROUND_OUT is off,
			// so m_pfSurroundBufferData is NULL and the unconditional copy read address 0.
#ifdef ENABLE_SURROUNDLFE_OUT
			memcpy(m_pfSurroundLFEBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 6 * uFrameLength, m_pfSurroundLFEBufferData, 6 * uFrameLength * sizeof(F32));
#endif
#ifdef ENABLE_SURROUND_OUT
			memcpy(m_pfSurroundBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 5 * uFrameLength, m_pfSurroundBufferData, 5 * uFrameLength * sizeof(F32));
#endif
#ifdef ENABLE_STEREO_OUT
			memcpy(m_pfStereoBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * 2 * uFrameLength, m_pfStereoBufferData, 2 * uFrameLength * sizeof(F32));
#endif
#ifdef ENABLE_MONO_OUT
			memcpy(m_pfMonoBufferData + ((m_sNextWrite % m_uNumBuffers) + 1) * uFrameLength, m_pfMonoBufferData, uFrameLength * sizeof(F32));
#endif

			// temp ifdef'ed out on pc.. needs to go back in again!
//#ifndef PLATFORM_PC
#ifdef USE_EAX_REVERB	
			for (U32 i=0; i<m_uNumReverbs; i++)
				memcpy(m_pfMonoReverbBufferData[i] + ((m_sNextReverbWrite[i] % m_uNumBuffers) + 1) * uFrameLength, m_pfMonoReverbBufferData[i], uFrameLength * sizeof(F32));
#endif

			//#endif
            
            #endif

			//	temp
			//static File m_File;
			//static U8 asData[2048];

			//static bool m_bWriteFile = false;
			//static bool m_bRecording = false;

			//if (m_bWriteFile)
			//{
			//	if (!m_bRecording)
			//	{
			//	#if defined(PLATFORM_XENON)
			//		m_File.OpenWrite("GAME:\\test.raw");
			//	#else
			//		m_File.OpenWrite("C:\\Documents and Settings\\nickb\\Desktop\\test.raw");
			//	#endif
			//		m_bRecording = true;
			//	}
			//}
			//else
			//{
			//	if (m_bRecording)
			//	{
			//		m_File.Close();
			//		m_bRecording = false;
			//	}
			//}

			//if (m_bRecording)
			//{
			//	for (U32 uCount = 0; uCount < uFrameLength; uCount++)
			//	{
			//		F32 fVal = m_pfSurroundBufferData[uCount];								//	FL
			//		fVal += m_pfSurroundBufferData[uFrameLength + uCount];					//	FR
			//		fVal += 0.707f * m_pfSurroundBufferData[2 * uFrameLength + uCount];	//	C
			//		fVal += 0.707f * m_pfSurroundBufferData[4 * uFrameLength + uCount];	//	SL
			//		fVal += 0.707f * m_pfSurroundBufferData[4 * uFrameLength + uCount];	//	SR
			//		fVal *= 0.707f;	//	st->m
			//		fVal = Maths::Clamp(fVal, -1.f, 1.f);
			//		S16 sVal = (S16)(fVal * 32767.f);

			//		asData[uCount*2] = sVal & 0xff;
			//		asData[uCount*2+1] = (sVal >> 8) & 0xff;
			//	}
			//	
			//	m_File.Write(asData, uFrameLength * sizeof(S16));
			//}
            

			//if (m_pfMonoReverbBufferData2)
			//	memcpy(m_pfMonoReverbBufferData2 + ((m_sNextReverb2Write % m_uNumBuffers) + 1) * uFrameLength, m_pfMonoReverbBufferData2, uFrameLength * sizeof(F32));

			//	inc write ptrs
			Increment(&m_sNextWrite);
			//printf("[inc write]read=%d, write=%d\n", m_sCurrRead, m_sNextWrite);
			for (U32 i=0; i<m_uNumReverbs; i++)
				Increment(&m_sNextReverbWrite[i]);
//#ifdef PLATFORM_XENON
//			Increment(&m_sNextReverb2Write);
//#endif

			// temporarily yield the audio spurs task so the other audio spurs tasks can get some time in
			// jkc- changed to false by default, was causing big glitches in music when video frames were dropped
			static Debug::Bool g_bAudioEvent("Audio.System.SyncAudioWithGame", false);

			int movieLoop = MovieCapture || MovieWant > 0.f;
            if (movieLoop)
            {
				if (MovieWant > 0.f)
				{
					MovieCallback(m_pfSurroundLFEBufferData, 6, uFrameLength);
					MovieWant -= (F32)uFrameLength / (F32)uSampleRate;
				}
				
				// If we are waiting for MovieCapture and haven't captured all the sound we needed then
				// sleep until we want more movie data
				while (MovieCapture && MovieWant <= 0.f)
 				{
 #ifndef PLATFORM_PS3
 					Sleep(1);
 #else
					if (g_bAudioEvent)
						m_Event.Wait();
					else
						sys_timer_usleep(1000);
					if (Audio::g_bQuitAllTasks) break;
 #endif
 				}
			}
			else
			{
				//	deal with buffer over-run: if all buffers are filled, wait
				if (g_bDisableReverb)
				{
					while (m_sNextWrite % m_uNumBuffers == (m_sCurrRead - 1) % m_uNumBuffers)
					{
						//printf("sleeping\n");
#ifndef PLATFORM_PS3
						Sleep(1);
#ifdef PLATFORM_PC
						if (Audio::g_bQuitAllTasks) break;	// [Blur2PC] T144: retail PC 0x00560000
#endif
#else
						if (g_bAudioEvent)
							m_Event.Wait();
						else
							sys_timer_usleep(1000);
						if (Audio::g_bQuitAllTasks) break;
#endif
					}
				}
				else
				{
					while (m_sNextWrite % m_uNumBuffers == m_sCurrRead % m_uNumBuffers
//#ifndef PLATFORM_PC
#ifdef USE_EAX_REVERB
						|| m_sNextReverbWrite[0] % m_uNumBuffers == m_sCurrReverbRead[0] % m_uNumBuffers
#endif
//#endif
						)
					{
						//printf("[sleep]    read=%d, write=%d\n", m_sCurrRead, m_sNextWrite);
#if defined(PLATFORM_PC)
						// [Blur2PC] T144: retail PC 0x00560000 yields and stops waiting on quit
						SwitchToThread();
						if (Audio::g_bQuitAllTasks) break;
#elif !defined(PLATFORM_PS3)
						Sleep(1);
#else
						if (g_bAudioEvent)
							m_Event.Wait();
						else
							sys_timer_usleep(1000);
						
						if (Audio::g_bQuitAllTasks) break;
#endif
					}
				}
			}
			
			//  end frame callback
			if (m_pContext && m_cbFrameEnd)
				m_cbFrameEnd(m_pContext);

			uFrameNum++;
		}
		if (Audio::g_bQuitAllTasks) break;

		if (m_eState != eActive)
		{
			m_eState = eShutdownComplete;
			Abort();
			return;
		}

	}

#ifndef PLATFORM_PS3
	MEMORY_TRACKER_POP("audio.renderer");
#endif
}
#else // ndef PLATFORM_PS3
////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
static uint64_t peaktime = 0;
static U32 peaknrfames = 0;
void Audio::Core::Renderer::Execute(void*)
{
    uint64_t tb_freq = sys_time_get_timebase_frequency();
	uint64_t tb_start;
	uint64_t tb_end;
	
	uint64_t tb_start_render;
	uint64_t tb_end_render;
	uint64_t tb_start_folddown;
	uint64_t tb_end_folddown;
	uint64_t tb_start_foldownwait;
	uint64_t tb_end_foldownwait;

	U32 uReadFrame = 0;
	U32 uWriteFrame = 0;
	U32 uStartWriteFrame = 0;
	U32 uStartReadFrame = 0;
	F32* pDataOut = (F32*) BizAlignedMalloc( 16 * sizeof(F32) * 6 * uFrameLength, 128 );
	U32 uFrameSize;
	if (m_bSurround)
		uFrameSize = CELL_AUDIO_PORT_8CH * sizeof(float) * CELL_AUDIO_BLOCK_SAMPLES;
	else
		uFrameSize = CELL_AUDIO_PORT_2CH * sizeof(float) * CELL_AUDIO_BLOCK_SAMPLES;
	SetPriority(  45 );

	U32 nrFramesRequired = 8;
	uReadFrame = *(uint64_t*)m_pReadIndex;
	uWriteFrame = uReadFrame + 1;
	uWriteFrame &= (_AMAX_CELL_AUDIO_BLOCK-1);
	nrFramesRequired = (uReadFrame - uWriteFrame) & (_AMAX_CELL_AUDIO_BLOCK-1);
	for (;;)
	{
		if (!m_bStarted) 
		{
			sys_timer_usleep( 1000 );
			continue;
		}

		SYS_TIMEBASE_GET( tb_start );
		uReadFrame = *(uint64_t*)m_pReadIndex;
		// Nr frames reqd. 
{
		SYS_TIMEBASE_GET(tb_start_render);
		for (U32 frame = 0 ; frame < nrFramesRequired ; frame++)
		{
			ClearFrame(0);
			for (U32 i=0; i<m_uNumReverbs; i++)
				ClearReverbFrame(0, i);

			//  start frame callback
			if (m_pContext && m_cbFrameStart)
				m_cbFrameStart(m_pContext);
        
			int frameNr = frame + 1;
			memcpy(m_pfSurroundLFEBufferData	+ frameNr * 6 * uFrameLength,	m_pfSurroundLFEBufferData,	6 * uFrameLength * sizeof(F32));
			//  end frame callback
			if (m_pContext && m_cbFrameEnd)
				m_cbFrameEnd(m_pContext);

			uFrameNum++;
		}	
		SYS_TIMEBASE_GET(tb_end_render);
}
		// Fold down and copy into hardware

{
		SYS_TIMEBASE_GET(tb_start_folddown);
		memset(pDataOut, 0, 16 * sizeof(F32) * 6 * uFrameLength);
		m_BatchProcess.Clear();
		for (U32 frameNr = 0 ; frameNr < nrFramesRequired ; frameNr++)
		{
			//	get render buffers
			U32 framePlusOne = frameNr + 1;
			if (m_pfSurroundLFEBufferData)
			{
				Core::Renderer::FoldDown(pDataOut + frameNr * 6 * uFrameLength,
					6,
					m_pfSurroundLFEBufferData + framePlusOne * 6 * uFrameLength,
					6,
					Core::Renderer::eMix,
					g_MasterAttenuation);

				if (g_bTestNoise)
				{
					for(int i=0; i<uFrameLength; i++)
						pDataOut[ frameNr * 6 * uFrameLength + g_iTestChannel * uFrameLength + i ] = Audio::System::Get()->GetRandom() -0.5f;
				}
			}
		}
		SYS_TIMEBASE_GET(tb_end_folddown);
	
}		
		
	
		// Add the interleave task
//		F32* afDataOut = (F32*)(m_pPort + (uStartWriteFrame * uFrameSize));
//		printf("writeframe %d uWriteFrame %d nrFramesRequired %d\n",writeFrame,uWriteFrame,nrFramesRequired);
//		printf("writeFrame %d pDataOut %08x afDataOut %08x m_bSurround %4.4f\n",(U32)(F32*)writeFrame, (U32)pDataOut, (U32)afDataOut, (F32)(m_bSurround ? nrFramesRequired : -nrFramesRequired));
		SYS_TIMEBASE_GET(tb_start_foldownwait);
#define USE_SPU_INTERLEAVE
#ifdef USE_SPU_INTERLEAVE
		m_BatchProcess.Add( (F32*)uWriteFrame, pDataOut, (F32*)m_pPort, nrFramesRequired );
		m_BatchProcess.Process();
		m_BatchProcess.Wait();
		uWriteFrame+= nrFramesRequired;
		uWriteFrame&=(_AMAX_CELL_AUDIO_BLOCK-1);
#else
		m_BatchProcess.Process();
		m_BatchProcess.Wait();

		// Interleave data into Hardware
		U32 nrBuffers = nrFramesRequired;
		F32* pPPUDest = (F32*)m_pPort;//(F32*)addList[x].mpFromB;
		F32* pSource = pDataOut;
//		printf("luWriteFrame");
		for (U32 uCount = 0 ; uCount < nrBuffers ; uCount++)
		{
			F32* portPtr = (F32*)(m_pPort);
			pPPUDest = portPtr + ( uFrameLength *  8 * uWriteFrame );
			for (U32 uIndex = 0; uIndex < uFrameLength; uIndex++)
			{
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH]	   = pSource[uIndex];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 1] = pSource[uIndex + uFrameLength];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 2] = pSource[uIndex + 2 * uFrameLength];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 3] = pSource[uIndex + 3 * uFrameLength];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 4] = pSource[uIndex + 4 * uFrameLength];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 5] = pSource[uIndex + 5 * uFrameLength];
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 6] = 0;
				pPPUDest[uIndex * CELL_AUDIO_PORT_8CH + 7] = 0;
			}
			pSource += (6 * uFrameLength);
//			printf("[%d]",uWriteFrame);
			uWriteFrame++;
			uWriteFrame &= (_AMAX_CELL_AUDIO_BLOCK-1);

		}
//		printf("\n");

#endif
//		static int countit = 0;
//		printf("STA: nr %d/%d count %d\n",m_BatchProcess.mFrameAddIndex, m_BatchProcess.mpQueueSize,countit++ );
//		printf("END: nr %d/%d\n",m_BatchProcess.mFrameAddIndex, m_BatchProcess.mpQueueSize );

//		if (uReadFrame != (U32)*(uint64_t*)m_pReadIndex) printf("*");
//		else printf("-");
		
//		printf("uReadFrame[%d] h/wRead[%d] uStartReadFrame[%d] uStartWriteFrame[%d] uWriteFrame[%d] nrFramesRequired[%d]\n",uReadFrame,(U32)*(uint64_t*)m_pReadIndex,uStartReadFrame,uStartWriteFrame,uWriteFrame,nrFramesRequired);
			
		SYS_TIMEBASE_GET(tb_end_foldownwait);
		SYS_TIMEBASE_GET( tb_end );
	
		uint64_t time = ((tb_end-tb_start)* 1000000) / tb_freq ;
		U32 targetFrame = (uReadFrame + nrFramesRequired) & (_AMAX_CELL_AUDIO_BLOCK-1);
		
		if (time > peaktime) 
		{
			peaknrfames = nrFramesRequired;
			peaktime = time;
		}

		S64 sleepTime = 5333-time;
//		sys_event_queue_drain(m_queue);
		do
		{
#ifdef USE_PS3_LIBAUDIO_EVENT
			sys_event_t  libaudio_event;
			int ret = sys_event_queue_receive(m_queue, &libaudio_event, 4 * 1000 * 1000);
			if (ret == ETIMEDOUT)
			{
			#ifndef CFG_GOLD
				printf(" *** Something wrong, there is no event for four seconds ...\n");
			#endif
				continue;
			}
			
#else 			
			if (sleepTime > 0)
			{
				sys_timer_usleep( sleepTime  );
			}
			else
			{
				sleepTime += 5333;
			}
#endif
			uReadFrame = *(uint64_t*)m_pReadIndex;
			nrFramesRequired = (uReadFrame - uWriteFrame) & (_AMAX_CELL_AUDIO_BLOCK-1);

			//printf("nrFramesRequired %d\n",nrFramesRequired);

		} while(nrFramesRequired < g_AudioFramesToRender);
		

		#if 0

		static int avg_time = 0;
		static int every_n_counter = 0;
		avg_time += time;
		if (every_n_counter == 100)
		{
			U32 maxTime = (nrFramesRequired * 256 * 1000000 / 48000);
			U32 percentTime = (avg_time * 100 / maxTime);
		#ifndef CFG_GOLD
			printf("time %d (%d%%)frames %d (Peak:%d frames %d)\n",(int)(avg_time / 100),percentTime, (int)nrFramesRequired,(int)peaktime,(int)peaknrfames);
		#endif

			uint64_t render_time	= ((tb_end_render-tb_start_render)* 1000000) / tb_freq ;
			uint64_t fold_time		= ((tb_end_foldownwait-tb_start_foldownwait)* 1000000) / tb_freq ;
			uint64_t fold_wait_time = ((tb_end_folddown-tb_start_folddown)* 1000000) / tb_freq ;
				
		#ifndef CFG_GOLD
			printf("render %d fold %d wait %d\n",(int)render_time,(int)fold_time,(int)fold_wait_time);
		#endif
			peaktime = 0;
			every_n_counter = 0;
			avg_time = 0;
		}
		else
		{
			every_n_counter++;
		}
		#endif


		// temporarily yield the audio spurs task so the other audio spurs tasks can get some time in
		// jkc- changed to false by default, was causing big glitches in music when video frames were dropped
		static Debug::Bool g_bAudioEvent("Audio.System.SyncAudioWithGame", false);

		int movieLoop = MovieCapture || MovieWant > 0.f;
        if (movieLoop)
        {
			if (MovieWant > 0.f)
			{
				MovieCallback(m_pfSurroundLFEBufferData, 6, uFrameLength);
				MovieWant -= (F32)uFrameLength / (F32)uSampleRate;
			}
			
			// If we are waiting for MovieCapture and haven't captured all the sound we needed then
			// sleep until we want more movie data
			while (MovieCapture && MovieWant <= 0.f)
			{
				if (g_bAudioEvent)
					m_Event.Wait();
				else
					sys_timer_usleep(1000);
				if (Audio::g_bQuitAllTasks) break;
			}
		}

			
		if (Audio::g_bQuitAllTasks) break;
	}

	if (m_eState != eActive)
	{
		m_eState = eShutdownComplete;
		Abort();
		return;
	}

#ifdef PLATFORM_PS3
	BizAlignedFree( pDataOut );
#endif

#ifndef PLATFORM_PS3
	MEMORY_TRACKER_POP("audio.renderer");
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Renderer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::ClearFrame(S32 sFrameNum)
{
	// zero frame buffer memory
	if (m_pSurroundLFEInput)
	{
		F32* pfFrame = m_pfSurroundLFEBufferData + sFrameNum * 6 * uFrameLength;
		Frame::Zero(pfFrame, 6);
		//m_pSurroundLFEInput->Clear();
	}

	if (m_pSurroundInput)
	{
		F32* pfFrame = m_pfSurroundBufferData + sFrameNum * 5 * uFrameLength;
		Frame::Zero(pfFrame, 5);
		//m_pSurroundInput->Clear();
	}

	if (m_pStereoInput)
	{
		F32* pfFrame = m_pfStereoBufferData + sFrameNum * 2 * uFrameLength;
		Frame::Zero(pfFrame, 2);
		//m_pStereoInput->Clear();
	}

	if (m_pMonoInput)
	{
		F32* pfFrame = m_pfMonoBufferData + sFrameNum * uFrameLength;
		Frame::Zero(pfFrame);
		//m_pMonoInput->Clear();
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ClearReverbFrame
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::ClearReverbFrame(S32 sFrameNum, U32 uIndex)
{
	// zero frame buffer memory
	if (m_pMonoReverbInput[uIndex])
	{
		F32* pfFrame = m_pfMonoReverbBufferData[uIndex] + sFrameNum * uFrameLength;
		m_pMonoReverbInput[uIndex]->SetAltBuffer(pfFrame);
		Frame::Zero(pfFrame);
		//m_pMonoReverbInput[uIndex]->Clear();
	}

//#ifdef PLATFORM_PS3
//#ifdef PS3_AUDIO_USE_SPU
//	Audio::System::Get()->SubmitSpuTask(m_pMonoReverbInput->m_pfFrameData, uFrameLength * sizeof(F32));
//#endif
//#endif
}

////////////////////////////////////////////////////////////////////////////////
//	ClearReverb2Frame
////////////////////////////////////////////////////////////////////////////////
//#ifdef PLATFORM_XENON
//void Audio::Core::Renderer::ClearReverb2Frame(S32 sFrameNum)
//{
//	// zero frame buffer memory
//	if (m_pMonoReverbInput2)
//		m_pMonoReverbInput2->SetAltBuffer(m_pfMonoReverbBufferData2 + sFrameNum * uFrameLength);
//
//	if (m_pMonoReverbInput2)
//	{
//		m_pMonoReverbInput2->SetAltBuffer(m_pfMonoReverbBufferData2 + sFrameNum * uFrameLength);
//		m_pMonoReverbInput2->Clear();
//	}
//}
//#endif	//	PLATFORM_XENON

////////////////////////////////////////////////////////////////////////////////
//	MonoFrameData
////////////////////////////////////////////////////////////////////////////////
F32* Audio::Core::Renderer::MonoFrameData(S32 sFrameNum)
{
	if (m_pMonoInput)
		return m_pfMonoBufferData + sFrameNum * uFrameLength;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	StereoFrameData
////////////////////////////////////////////////////////////////////////////////
F32* Audio::Core::Renderer::StereoFrameData(S32 sFrameNum)
{
	if (m_pStereoInput)
		return m_pfStereoBufferData + sFrameNum * 2 * uFrameLength;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	SurroundFrameData
////////////////////////////////////////////////////////////////////////////////
F32* Audio::Core::Renderer::SurroundFrameData(S32 sFrameNum)
{
	if (m_pSurroundInput)
		return m_pfSurroundBufferData + sFrameNum * 5 * uFrameLength;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	SurroundLFEFrameData
////////////////////////////////////////////////////////////////////////////////
F32* Audio::Core::Renderer::SurroundLFEFrameData(S32 sFrameNum)
{
	if (m_pSurroundLFEInput)
		return m_pfSurroundLFEBufferData + sFrameNum * 6 * uFrameLength;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	ReverbFrameData
////////////////////////////////////////////////////////////////////////////////
F32* Audio::Core::Renderer::ReverbFrameData(S32 sFrameNum, U32 uIndex)
{
	if (m_pMonoReverbInput[uIndex])
		return m_pfMonoReverbBufferData[uIndex] + sFrameNum * uFrameLength;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	ReverbFrameData2
////////////////////////////////////////////////////////////////////////////////
//#ifdef PLATFORM_XENON
//F32* Audio::Core::Renderer::ReverbFrameData2(S32 sFrameNum)
//{
//	if (m_pMonoReverbInput2)
//		return m_pfMonoReverbBufferData2 + sFrameNum * uFrameLength;
//	else
//		return NULL;
//}
//#endif

////////////////////////////////////////////////////////////////////////////////
//	GetNextFrame
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Renderer::GetNextFrame(F32** pfSurroundLFE, F32** pfSurround, F32** pfStereo, F32** pfMono)
{
	//Perf::BeginNamedEvent("GetNextFrame");

	S32 sDiff = m_sNextWrite - m_sCurrRead;
	bool bReady = true;	// [Blur2PC] T144

	//	deal with buffer underrun: only inc read ptr if next frame is ready
	if ((m_sNextWrite - 1) % m_uNumBuffers == m_sCurrRead % m_uNumBuffers)
	{
		//printf("[dropout] (%d) read=%d, write=%d\n", sDiff, m_sCurrRead, m_sNextWrite);
		m_uDropped++;

		if (g_bClearDroppedFrames)
			ClearFrame( m_sCurrRead % m_uNumBuffers );

		bReady = false;	// [Blur2PC] T144
	}
	else
	{
		Increment(&m_sCurrRead);
		//printf("[inc read] (%d) read=%d, write=%d\n", sDiff, m_sCurrRead, m_sNextWrite);
	}

	U32 uCurrReadBuffer = (m_sCurrRead % m_uNumBuffers) + 1;

	*pfMono = MonoFrameData(uCurrReadBuffer);
	*pfStereo = StereoFrameData(uCurrReadBuffer);
	*pfSurround = SurroundFrameData(uCurrReadBuffer);
	*pfSurroundLFE = SurroundLFEFrameData(uCurrReadBuffer);

	//Perf::EndNamedEvent();

	return bReady;	// [Blur2PC] T144
}

////////////////////////////////////////////////////////////////////////////////
//	GetNextReverbFrame
////////////////////////////////////////////////////////////////////////////////
//#ifndef PLATFORM_PC
//#ifdef USE_EAX_REVERB
void Audio::Core::Renderer::GetNextReverbFrame(F32** pfReverb, U32 uIndex)
{
	//	deal with buffer underrun: only inc read ptr if next frame is ready
	if ((m_sNextReverbWrite[uIndex] - 1) % m_uNumBuffers != m_sCurrReverbRead[uIndex] % m_uNumBuffers)
		Increment(&m_sCurrReverbRead[uIndex]);

	U32 uCurrReadBuffer = (m_sCurrReverbRead[uIndex] % m_uNumBuffers) + 1;

	*pfReverb = ReverbFrameData(uCurrReadBuffer, uIndex);
}
//#else
//void Core::Renderer::GetNextReverbFrame(F32** pfReverb, U32 uIndex)
//{
//}
//#endif

#ifndef PLATFORM_PS3
//////////////////////////////////////////////////////////////////////////////////
////	GetNextReverb2Frame
//////////////////////////////////////////////////////////////////////////////////
//void Audio::Core::Renderer::GetNextReverbFrame2(F32** pfReverb)
//{
//	//	deal with buffer underrun: only inc read ptr if next frame is ready
//	if (m_sNextReverb2Write % m_uNumBuffers != m_sCurrReverb2Read % m_uNumBuffers)
//		Increment(&m_sCurrReverb2Read);
//
//	U32 uCurrReadBuffer = (m_sCurrReverb2Read % m_uNumBuffers) + 1;
//
//	*pfReverb = ReverbFrameData2(uCurrReadBuffer);
//}
#endif
//#endif

#ifdef PLATFORM_PS3
#define AUDIO_USE_BATCH_FOLD_DOWN
#endif

#ifdef AUDIO_USE_BATCH_FOLD_DOWN
#define AUDIO_FRAME_ZERO( a )			m_BatchProcess.Add( a )
#define AUDIO_FRAME_COPY( a, b )		m_BatchProcess.Add( a, b )
#define AUDIO_FRAME_ADD( a, b, c )		m_BatchProcess.Add( a, b, c )
#define AUDIO_FRAME_GAIN( a, b, c, d )	m_BatchProcess.Add( a, b, c, d )
#else
#define AUDIO_FRAME_ZERO( a )			Frame::Zero( a )
#define AUDIO_FRAME_COPY( a, b )		Frame::Copy( a, b )
#define AUDIO_FRAME_ADD( a, b, c )		Frame::DoAdd( a, b, c )
#define AUDIO_FRAME_GAIN( a, b, c, d )	Frame::AddGain( a, b, c, d )
#endif

////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Renderer::FoldDown(F32* pSamplesOut, U32 uChOut, F32* pSamplesIn, U32 uChIn, Mode eMode, F32 fLevel)
{

	if (uChOut == 6)
	{
		if (uChIn == 1)
		{
			switch (eMode)
			{
			case eOverwrite:
				AUDIO_FRAME_ZERO(pSamplesIn);
				AUDIO_FRAME_ZERO(&pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[2 * Core::uFrameLength], pSamplesIn);
				AUDIO_FRAME_ZERO(&pSamplesOut[3 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[4 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[5 * Core::uFrameLength]);
				break;

			case eAdd:
				AUDIO_FRAME_ADD(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], pSamplesIn);
				break;

			case eMix:
				AUDIO_FRAME_GAIN(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], pSamplesIn, fLevel);
				break;
			}
		}
		else if (uChIn == 2)
		{
			switch (eMode)
			{
			case eOverwrite:
				AUDIO_FRAME_COPY(pSamplesOut, pSamplesIn);
				AUDIO_FRAME_COPY(&pSamplesOut[1 * Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[2 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[3 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[4 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[5 * Core::uFrameLength]);
				break;

			case eAdd:
				AUDIO_FRAME_ADD(pSamplesOut, pSamplesOut, pSamplesIn);
				AUDIO_FRAME_ADD(&pSamplesOut[1 * Core::uFrameLength], &pSamplesOut[1 * Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				break;

			case eMix:
				AUDIO_FRAME_GAIN(pSamplesOut, pSamplesOut, pSamplesIn, fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[Core::uFrameLength], &pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength], fLevel);
				break;
			}
		}
		else if (uChIn == 5)
		{
			switch (eMode)
			{
			case eOverwrite:
				AUDIO_FRAME_COPY(pSamplesOut, pSamplesIn);
				AUDIO_FRAME_COPY(&pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength]);
				AUDIO_FRAME_ZERO(&pSamplesOut[3 * Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[5 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength]);
				break;

			case eAdd:
				AUDIO_FRAME_ADD(pSamplesOut, pSamplesOut, pSamplesIn);
				AUDIO_FRAME_ADD(&pSamplesOut[Core::uFrameLength], &pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[4 * Core::uFrameLength], &pSamplesOut[3 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[5 * Core::uFrameLength], &pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength]);
				break;

			case eMix:
				AUDIO_FRAME_GAIN(pSamplesOut, pSamplesOut, pSamplesIn, fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[Core::uFrameLength], &pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[4 * Core::uFrameLength], &pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[5 * Core::uFrameLength], &pSamplesOut[5 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength], fLevel);
				break;
			}
		}
		else if (uChIn == 6)
		{
			switch (eMode)
			{
			case eOverwrite:
				AUDIO_FRAME_COPY(pSamplesOut, pSamplesIn);
				AUDIO_FRAME_COPY(&pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[3 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength]);
				AUDIO_FRAME_COPY(&pSamplesOut[5 * Core::uFrameLength], &pSamplesIn[5 * Core::uFrameLength]);
				break;

			case eAdd:
				AUDIO_FRAME_ADD(pSamplesOut, pSamplesOut, pSamplesIn);
				AUDIO_FRAME_ADD(&pSamplesOut[Core::uFrameLength], &pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[3 * Core::uFrameLength], &pSamplesOut[3 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[4 * Core::uFrameLength], &pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength]);
				AUDIO_FRAME_ADD(&pSamplesOut[5 * Core::uFrameLength], &pSamplesOut[5 * Core::uFrameLength], &pSamplesIn[5 * Core::uFrameLength]);
				break;

			case eMix:
				AUDIO_FRAME_GAIN(pSamplesOut, pSamplesOut, pSamplesIn, fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[Core::uFrameLength], &pSamplesOut[Core::uFrameLength], &pSamplesIn[Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[2 * Core::uFrameLength], &pSamplesOut[2 * Core::uFrameLength], &pSamplesIn[2 * Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[3 * Core::uFrameLength], &pSamplesOut[3 * Core::uFrameLength], &pSamplesIn[3 * Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[4 * Core::uFrameLength], &pSamplesOut[4 * Core::uFrameLength], &pSamplesIn[4 * Core::uFrameLength], fLevel);
				AUDIO_FRAME_GAIN(&pSamplesOut[5 * Core::uFrameLength], &pSamplesOut[5 * Core::uFrameLength], &pSamplesIn[5 * Core::uFrameLength], fLevel);
				break;
			}
		}
	}
	else if (uChOut == 1)
	{
		if (uChIn == 1)
		{
			switch (eMode)
			{
			case eOverwrite:
				AUDIO_FRAME_COPY(pSamplesOut, pSamplesIn);
				break;

			case eAdd:
				AUDIO_FRAME_ADD(pSamplesOut, pSamplesOut, pSamplesIn);
				break;

			case eMix:
				AUDIO_FRAME_GAIN(pSamplesOut, pSamplesOut, pSamplesIn, fLevel);
				break;
			}
		}
	}


}



#if defined(PLATFORM_PC)

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif
