
#include "AudioPch.h"

#include "engine_pc.h"

#include "coreheaders.h"
#include "../SystemHeaders.h"

#include <mmsystem.h>	// [Blur2PC] T255: timeBeginPeriod

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

// [Blur2PC] T144: retail Blur PC's output engine; see engine_pc.h and docs/analysis.md "Audio output (PC)".
// Everything below follows Blur.exe 1.2 (addresses per function); names are ours [Blur2PC name].

////////////////////////////////////////////////////////////////////////////////
//	Debug values (retail static initialisers 0x00e128d0..0x00e12b70)
////////////////////////////////////////////////////////////////////////////////
enum E_PCEngineSampleRate
{
	E_PCEngineSampleRate_12000 = 12000,
	E_PCEngineSampleRate_16000 = 16000,
	E_PCEngineSampleRate_18000 = 18000,
	E_PCEngineSampleRate_22050 = 22050,
	E_PCEngineSampleRate_24000 = 24000,
	E_PCEngineSampleRate_28000 = 28000,
	E_PCEngineSampleRate_32000 = 32000,
	E_PCEngineSampleRate_36000 = 36000,
	E_PCEngineSampleRate_44100 = 44100,
	E_PCEngineSampleRate_48000 = 48000,
};

enum E_PCEngineBitsPerSample
{
	E_PCEngineBitsPerSample_16 = 16,
	E_PCEngineBitsPerSample_32 = 32,
};

// options tables 0x01028c48 and 0x01028ca0
DEBUG_ENUM(E_PCEngineSampleRate)
DE_OPTION(E_PCEngineSampleRate_12000, "12000 Hz")
DE_OPTION(E_PCEngineSampleRate_16000, "16000 Hz")
DE_OPTION(E_PCEngineSampleRate_18000, "18000 Hz")
DE_OPTION(E_PCEngineSampleRate_22050, "22050 Hz")
DE_OPTION(E_PCEngineSampleRate_24000, "24000 Hz")
DE_OPTION(E_PCEngineSampleRate_28000, "28000 Hz")
DE_OPTION(E_PCEngineSampleRate_32000, "32000 Hz")
DE_OPTION(E_PCEngineSampleRate_36000, "36000 Hz")
DE_OPTION(E_PCEngineSampleRate_44100, "44100 Hz")
DE_OPTION(E_PCEngineSampleRate_48000, "48000 Hz")
END_DEBUG_ENUM

DEBUG_ENUM(E_PCEngineBitsPerSample)
DE_OPTION(E_PCEngineBitsPerSample_16, "16")
DE_OPTION(E_PCEngineBitsPerSample_32, "32")
END_DEBUG_ENUM

static Debug::Enum<E_PCEngineSampleRate>	g_PCEngineSampleRate("Audio.PC.Engine.SampleRate", (E_PCEngineSampleRate)Audio::Core::uSampleRate);
static Debug::Enum<E_PCEngineBitsPerSample>	g_PCEngineBitsPerSample("Audio.PC.Engine.BitsPerSample", E_PCEngineBitsPerSample_16);
static Debug::Bool	g_PCEngineUseFloats("Audio.PC.Engine.UseFloats", false);
static Debug::Int	g_PCEngineSoundChannels("Audio.PC.Engine.SoundChannels", 2, Debug::Range(2, 8));
static Debug::Int	g_PCEngineNumberOfBuffers("Audio.PC.Engine.NumberOfBuffers", 16, Debug::Range(2, 256));
static Debug::Bool	g_PCEngineNumberOfBuffersOverride("Audio.PC.Engine.NumberOfBuffersOverride", false);
static Debug::Function	g_PCEngineChangeFormat("Audio.PC.Engine.ChangeFormat", Audio::Core::Engine::ChangeFormat, 0);
static Debug::Bool	g_PCEngineInjectLatency("Audio.PC.Engine.InjectLatency.UseAtOwnRisk.AddNoiseToSignal", false);
// registered by retail but read nowhere
static Debug::Bool	g_PCEngineLeftTest("Audio.PC.Engine.LeftTest", false);
static Debug::Bool	g_PCEngineCentreTest("Audio.PC.Engine.CentreTest", false);
static Debug::Bool	g_PCEngineRightTest("Audio.PC.Engine.RightTest", false);
static Debug::Bool	g_PCEngineLFETest("Audio.PC.Engine.LFETest", false);
static Debug::Bool	g_PCEngineRearLeftTest("Audio.PC.Engine.RearLeftTest", false);
static Debug::Bool	g_PCEngineRearRightTest("Audio.PC.Engine.RearRightTest", false);
// milliseconds slept after each write, and between polls of the play cursor (0: yield)
static Debug::Int	g_PCEngineDelayTest("Audio.PC.Engine.DelayTest", 0, Debug::Range(0, 10));
static Debug::Int	g_PCEngineAudioSleep("Audio.PC.Engine.AudioSleep", 5, Debug::Range(0, 100));

// 0x01028d18: the first write re-checks the write position against the buffer
static bool g_bPCEngineResyncWritePos = true;

// 0x01028cb8: DirectSound speaker configuration -> output channels
static const U32 s_aSpeakerConfigChannels[12][2] =
{
	{ 6, 6 }, { 9, 6 }, { 6, 6 }, { 7, 8 }, { 0, 2 }, { 1, 2 },
	{ 2, 2 }, { 3, 4 }, { 4, 2 }, { 5, 6 }, { 7, 8 }, { 8, 8 },
};

// 0x00e85a90 / 0x00e85aa0: KSDATAFORMAT_SUBTYPE_PCM and KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
static const GUID s_SubTypePCM =	{ 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
static const GUID s_SubTypeFloat =	{ 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

Audio::Core::Engine* Audio::Core::Engine::s_pInstance = NULL;

////////////////////////////////////////////////////////////////////////////////
//	Engine (0x00546f10)
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Engine::Engine() :
	m_pRenderer(NULL),
	m_pDS(NULL),
	m_pBuffer(NULL),
	m_uWritePos(0),
	m_hThread(NULL),
	m_uDeviceIndex(0),
	m_dwSpeakerConfig(0),
	m_bTimerPeriodSet(false)	// [Blur2PC] T255
{
	memset(&m_Caps, 0, sizeof(m_Caps));
	memset(&m_BufferDesc, 0, sizeof(m_BufferDesc));

	m_uBitsPerSample = g_PCEngineBitsPerSample;
	m_uChannels = g_PCEngineSoundChannels;
	m_uSampleRate = g_PCEngineSampleRate;
	m_bUseFloats = g_PCEngineUseFloats;

	m_hThreadDone = CreateEventA(NULL, TRUE, FALSE, NULL);
	m_hThreadStop = CreateEventA(NULL, TRUE, FALSE, NULL);
}

////////////////////////////////////////////////////////////////////////////////
//	~Engine (0x00546f90)
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Engine::~Engine()
{
	if (m_pDS)
		Shutdown();
}

////////////////////////////////////////////////////////////////////////////////
//	Instance (0x0054b100)
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Engine& Audio::Core::Engine::Instance(void)
{
	if (s_pInstance == NULL)
	{
		s_pInstance = new Engine;
		ResetOutputSampleRate();
	}

	return *s_pInstance;
}

////////////////////////////////////////////////////////////////////////////////
//	Init (0x00538150)
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Engine::Init(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	// retail calls a DRM-relocated helper on the device list here (0x00547af0 -> 0x0053b560); read as
	// emptying it before the enumeration refills it
	m_aDevices.Clear();

	if (FAILED(DirectSoundEnumerateA(EnumerateCallback, this)))
		return false;

	StartDevice(0);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	EnumerateCallback (0x00546fb0)
////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK Audio::Core::Engine::EnumerateCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
	Engine* pEngine = (Engine*)lpContext;

	Device device;
	if (lpGuid)
		device.guid = *lpGuid;
	else
		memset(&device.guid, 0, sizeof(device.guid));
	device.description = lpcstrDescription;
	device.module = lpcstrModule;
	device.pContext = lpContext;

	pEngine->m_aDevices.PushBack(device);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//	StartDevice (0x00547710)
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Engine::StartDevice(U32 uDeviceIndex)
{
	m_uDeviceIndex = uDeviceIndex;

	if (m_pDS)
		Shutdown();

	// retail indexes the device list directly; with no devices listed that pointer is NULL (the default device)
	const GUID* pGuid = (uDeviceIndex < (U32)m_aDevices.Size()) ? &m_aDevices[uDeviceIndex].guid : NULL;

	HRESULT hr = DirectSoundCreate8(pGuid, &m_pDS, NULL);
	if (FAILED(hr))
		return false;

	hr = m_pDS->SetCooperativeLevel(Audio::System::Get()->GetHWND(), DSSCL_PRIORITY);
	if (SUCCEEDED(hr))
	{
		m_Caps.dwSize = sizeof(DSCAPS);
		hr = m_pDS->GetCaps(&m_Caps);
		if (SUCCEEDED(hr))
		{
			hr = CreateBuffer(m_pDS, &m_pBuffer, 0);
			if (SUCCEEDED(hr))
			{
				hr = m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				if (SUCCEEDED(hr))
				{
					// [Blur2PC] T255: the writer polls the play cursor with Sleep(AudioSleep); at Windows' default
					// 15.6 ms timer it cannot keep a 27-32 ms ring fed (D32), so ask for 1 ms while the engine runs
					timeBeginPeriod(1);
					m_bTimerPeriodSet = true;

					ResetEvent(m_hThreadStop);
					ResetEvent(m_hThreadDone);
					m_hThread = CreateThread(NULL, 0x4000, ThreadProc, this, 0, NULL);
					SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);
					return true;
				}
			}
		}
	}

	Shutdown();
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown (0x00544060)
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Engine::Shutdown()
{
	if (m_hThread)
	{
		// retail does not close the thread handle
		SignalObjectAndWait(m_hThreadStop, m_hThreadDone, INFINITE, FALSE);
		m_hThread = NULL;
	}

	if (m_pBuffer)
	{
		m_pBuffer->Stop();
		m_pBuffer->Release();
		m_pBuffer = NULL;
	}

	if (m_pDS)
	{
		m_pDS->Release();
		m_pDS = NULL;
	}

	if (m_bTimerPeriodSet)	// [Blur2PC] T255
	{
		timeEndPeriod(1);
		m_bTimerPeriodSet = false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetOutputSampleRate (0x020c5fb8)
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Engine::SetOutputSampleRate(U32 uSampleRate)
{
	if (Shutdown())
	{
		g_PCEngineSampleRate = (E_PCEngineSampleRate)uSampleRate;

		m_uBitsPerSample = g_PCEngineBitsPerSample;
		m_uChannels = g_PCEngineSoundChannels;
		m_uSampleRate = g_PCEngineSampleRate;
		m_bUseFloats = g_PCEngineUseFloats;

		Audio::Core::SetOutputSampleRate(g_PCEngineSampleRate);

		StartDevice(m_uDeviceIndex);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ChangeFormat (0x00547870)
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Engine::ChangeFormat(void*)
{
	Engine& engine = Instance();

	if (engine.Shutdown())
	{
		engine.m_uBitsPerSample = g_PCEngineBitsPerSample;
		engine.m_uChannels = g_PCEngineSoundChannels;
		engine.m_uSampleRate = g_PCEngineSampleRate;
		engine.m_bUseFloats = g_PCEngineUseFloats;

		Audio::Core::SetOutputSampleRate(g_PCEngineSampleRate);

		engine.StartDevice(engine.m_uDeviceIndex);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CreateBuffer (0x00546a90)
////////////////////////////////////////////////////////////////////////////////
HRESULT Audio::Core::Engine::CreateBuffer(IDirectSound8* pDS, IDirectSoundBuffer8** ppBuffer, DWORD dwChannelMask)
{
	// the device's speaker layout decides the channel count
	if (pDS->GetSpeakerConfig(&m_dwSpeakerConfig) == DS_OK)
	{
		for (U32 i = 0; i < 12; i++)
		{
			if (s_aSpeakerConfigChannels[i][0] == (m_dwSpeakerConfig & 0xffff))
			{
				g_PCEngineSoundChannels = (int)s_aSpeakerConfigChannels[i][1];
				m_uChannels = g_PCEngineSoundChannels;
				break;
			}
		}
	}

	const bool bUseFloats = m_bUseFloats;

	WAVEFORMATEXTENSIBLE wfx;
	memset(&wfx, 0, sizeof(wfx));
	wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfx.Format.nChannels = (WORD)m_uChannels;
	wfx.Format.nSamplesPerSec = m_uSampleRate;
	wfx.Format.wBitsPerSample = bUseFloats ? 32 : (WORD)m_uBitsPerSample;
	wfx.Format.nBlockAlign = (WORD)(((S32)wfx.Format.wBitsPerSample * (S32)wfx.Format.nChannels) / 8);
	wfx.Format.nAvgBytesPerSec = wfx.Format.nBlockAlign * m_uSampleRate;
	wfx.Format.cbSize = 22;
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;

	if (dwChannelMask == 0)
	{
		switch (m_uChannels)
		{
			case 2: wfx.dwChannelMask = 0x3;	break;
			case 3: wfx.dwChannelMask = 0xb;	break;
			case 4: wfx.dwChannelMask = 0x33;	break;
			case 5: wfx.dwChannelMask = 0x37;	break;
			case 6: wfx.dwChannelMask = 0x3f;	break;
			case 8: wfx.dwChannelMask = 0x63f;	break;
			default:							break;
		}
	}
	else
	{
		wfx.dwChannelMask = dwChannelMask;
	}

	wfx.SubFormat = bUseFloats ? s_SubTypeFloat : s_SubTypePCM;

	// the buffer is NumberOfBuffers blocks of 256 frames; that count follows the output rate unless overridden
	// [Blur2PC] T255 (D32): retail uses rate / 1500 (32 blocks, 171 ms at 48 kHz). Default to the
	// community v1.3 patch's counts, tested by ear: 3 / 4 / 5 blocks at 24 / 36 / 48 kHz (32 / 28 / 27 ms);
	// 4 at 48 kHz crackled. Rates between take the next quality's count.
	if (!g_PCEngineNumberOfBuffersOverride)
		g_PCEngineNumberOfBuffers = (m_uSampleRate <= 24000) ? 3 : (m_uSampleRate <= 36000) ? 4 : 5;

	memset(&m_BufferDesc, 0, sizeof(m_BufferDesc));
	m_BufferDesc.dwSize = sizeof(DSBUFFERDESC);
	m_BufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	m_BufferDesc.dwBufferBytes = (wfx.Format.wBitsPerSample >> 3) * wfx.Format.nChannels * (U32)(int)g_PCEngineNumberOfBuffers * 256;
	m_BufferDesc.lpwfxFormat = &wfx.Format;

	IDirectSoundBuffer* pBuffer = NULL;
	HRESULT hr = pDS->CreateSoundBuffer(&m_BufferDesc, &pBuffer, NULL);
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)ppBuffer);
		pBuffer->Release();
	}

	m_uWritePos = 0;

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
//	WriteToBuffer (0x00546d00)
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Engine::WriteToBuffer(const void* pData, U32 uSize)
{
	if (m_pBuffer == NULL)
		return false;

	if (g_PCEngineInjectLatency)
	{
		for (U32 i = 0; i < uSize; i++)
			((U8*)pData)[i] |= 0x55;
	}

	if (g_bPCEngineResyncWritePos)
	{
		DWORD dwPlay, dwWrite;
		if (m_pBuffer->GetCurrentPosition(&dwPlay, &dwWrite) == DS_OK && (S32)m_uWritePos < 0)
			m_uWritePos += m_BufferDesc.dwBufferBytes;
		g_bPCEngineResyncWritePos = false;
	}

	if (uSize != 0)
	{
		for (;;)
		{
			// room between the write position and the play cursor
			DWORD dwPlay, dwWrite;
			U32 uFree = 0;
			if (m_pBuffer->GetCurrentPosition(&dwPlay, &dwWrite) == DS_OK)
				uFree = (m_uWritePos > dwPlay) ? (m_BufferDesc.dwBufferBytes - m_uWritePos + dwPlay) : (dwPlay - m_uWritePos);

			while (uFree < uSize)
			{
				if (g_PCEngineAudioSleep == 0)
					SwitchToThread();
				else
					Sleep(g_PCEngineAudioSleep);

				uFree = 0;
				if (m_pBuffer->GetCurrentPosition(&dwPlay, &dwWrite) == DS_OK)
					uFree = (m_uWritePos > dwPlay) ? (m_BufferDesc.dwBufferBytes - m_uWritePos + dwPlay) : (dwPlay - m_uWritePos);
			}

			void* pvPart1 = NULL;
			void* pvPart2 = NULL;
			DWORD dwPart1 = 0;
			DWORD dwPart2 = 0;
			const U32 uWritePos = m_uWritePos;
			HRESULT hr = m_pBuffer->Lock(uWritePos, uSize, &pvPart1, &dwPart1, &pvPart2, &dwPart2, 0);
			if (hr == DSERR_BUFFERLOST)
			{
				m_pBuffer->Restore();
				hr = m_pBuffer->Lock(uWritePos, uSize, &pvPart1, &dwPart1, &pvPart2, &dwPart2, 0);
			}
			if (FAILED(hr))
				continue;

			memcpy(pvPart1, pData, dwPart1);
			if (pvPart2)
				memcpy(pvPart2, (const U8*)pData + dwPart1, dwPart2);

			if (FAILED(m_pBuffer->Unlock(pvPart1, dwPart1, pvPart2, dwPart2)))
				continue;

			m_uWritePos = (m_uWritePos + uSize) % m_BufferDesc.dwBufferBytes;
			break;
		}
	}

	Sleep(g_PCEngineDelayTest);

	return false;
}

////////////////////////////////////////////////////////////////////////////////
//	Mixing (0x00544130, 0x00544400, 0x005458c0): renderer frames (planar, 128 samples per channel) added into
//	an interleaved frame of uChannels channels
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Engine::MixMono(const F32* pfIn, F32* pfOut, U32 uChannels)
{
	switch (uChannels)
	{
		case 2:
		case 3:
		case 4:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				pfOut[j * uChannels] += pfIn[j];
				pfOut[j * uChannels + 1] += pfIn[j];
			}
			break;

		case 5:
		case 6:
		case 8:
			for (U32 j = 0; j < uFrameLength; j++)
				pfOut[j * uChannels + 2] += pfIn[j];
			break;

		default:
			break;
	}
}

void Audio::Core::Engine::MixStereo(const F32* pfIn, F32* pfOut, U32 uChannels)
{
	// retail reads input j and j + 1 for the two sides of output frame j, as the snapshot's OpenAL mix did,
	// and uses this for the surround frame as well
	if (m_uChannels >= 2 && (m_uChannels <= 6 || m_uChannels == 8))
	{
		for (U32 j = 0; j < uFrameLength; j++)
		{
			pfOut[j * uChannels] += pfIn[j];
			pfOut[j * uChannels + 1] += pfIn[j + 1];
		}
	}
}

void Audio::Core::Engine::MixSurroundLFE(const F32* pfIn, F32* pfOut, U32 uChannels)
{
	// input planes: 0 front left, 1 front right, 2 centre, 3 LFE, 4 surround left, 5 surround right
	const F32* pfL = pfIn;
	const F32* pfR = pfIn + uFrameLength;
	const F32* pfC = pfIn + uFrameLength * 2;
	const F32* pfLFE = pfIn + uFrameLength * 3;
	const F32* pfSL = pfIn + uFrameLength * 4;
	const F32* pfSR = pfIn + uFrameLength * 5;

	switch (m_uChannels)
	{
		case 2:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[0] += pfC[j] * 0.5f;
				pf[1] += pfC[j] * 0.5f;
			}
			break;

		case 3:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[2] += pfLFE[j];
				pf[0] += pfSL[j] * 0.5f;
				pf[1] += pfSR[j] * 0.5f;
			}
			break;

		case 4:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[2] += pfSL[j];
				pf[3] += pfSR[j];
			}
			break;

		case 5:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[2] += pfC[j];
				pf[3] += pfSL[j];
				pf[4] += pfSR[j];
			}
			break;

		case 6:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[2] += pfC[j];
				pf[3] += pfLFE[j];
				pf[4] += pfSL[j];
				pf[5] += pfSR[j];
			}
			break;

		case 8:
			for (U32 j = 0; j < uFrameLength; j++)
			{
				F32* pf = pfOut + j * uChannels;
				pf[0] += pfL[j];
				pf[1] += pfR[j];
				pf[2] += pfC[j];
				pf[3] += pfLFE[j];
				pf[4] += pfSL[j];
				pf[5] += pfSR[j];
				pf[6] += pfSL[j] * 0.25f;
				pf[7] += pfSR[j] * 0.25f;
			}
			break;

		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	RenderFrameFloat (0x00547250)
////////////////////////////////////////////////////////////////////////////////
static inline F32 ClampOutput(F32 f)
{
	return (f < 1.f) ? ((f < -1.f) ? -1.f : f) : 1.f;
}

void Audio::Core::Engine::RenderFrameFloat(F32* pfOut)
{
	F32 afMix[8 * uFrameLength];
	const U32 uChannels = m_uChannels;

	F32* pfSurroundLFE = NULL;
	F32* pfSurround = NULL;
	F32* pfStereo = NULL;
	F32* pfMono = NULL;

	while (!Instance().GetRenderer()->GetNextFrame(&pfSurroundLFE, &pfSurround, &pfStereo, &pfMono))
		Sleep(1);

	if (!Instance().GetRenderer()->IsOutputMuted())
	{
		memset(afMix, 0, uChannels * uFrameLength * sizeof(F32));

		if (pfMono)
			MixMono(pfMono, afMix, uChannels);
		if (pfStereo)
			MixStereo(pfStereo, afMix, uChannels);
		if (pfSurround)
			MixStereo(pfSurround, afMix, uChannels);
		if (pfSurroundLFE)
			MixSurroundLFE(pfSurroundLFE, afMix, uChannels);
	}
	else
	{
		memset(afMix, 0, sizeof(afMix));
	}

	for (U32 i = 0; i < uChannels * uFrameLength; i++)
		pfOut[i] = ClampOutput(afMix[i]);
}

////////////////////////////////////////////////////////////////////////////////
//	RenderFrameS16 (0x00547500)
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Engine::RenderFrameS16(S16* psOut)
{
	F32 afFrame[8 * uFrameLength];
	const U32 uSamples = m_uChannels * uFrameLength;

	RenderFrameFloat(afFrame);

	for (U32 i = 0; i < uSamples; i++)
		psOut[i] = (S16)(S32)(ClampOutput(afFrame[i]) * 32767.f);
}

////////////////////////////////////////////////////////////////////////////////
//	RenderFrameS32 (0x005475a0)
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Engine::RenderFrameS32(S32* psOut)
{
	F32 afFrame[8 * uFrameLength];
	const U32 uSamples = m_uChannels * uFrameLength;

	RenderFrameFloat(afFrame);

	for (U32 i = 0; i < uSamples; i++)
		psOut[i] = (S32)(ClampOutput(afFrame[i]) * 2147483648.f);
}

////////////////////////////////////////////////////////////////////////////////
//	ThreadProc (0x00547630)
////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI Audio::Core::Engine::ThreadProc(LPVOID)
{
	Engine& engine = Instance();

	const U32 uChannels = engine.m_uChannels;
	const U32 uBytesPerSample = engine.m_uBitsPerSample >> 3;

	void* pBuffer = BizAlignedMalloc(uBytesPerSample << 10, 0x80);

	do
	{
		if (!engine.m_bUseFloats)
		{
			if (engine.m_uBitsPerSample == 16)
				engine.RenderFrameS16((S16*)pBuffer);
			else
				engine.RenderFrameS32((S32*)pBuffer);
		}
		else
		{
			// retail sizes the write below from BitsPerSample, not from the 32-bit floats
			engine.RenderFrameFloat((F32*)pBuffer);
		}

		engine.WriteToBuffer(pBuffer, uChannels * uBytesPerSample * uFrameLength);
	}
	while (WaitForSingleObject(engine.m_hThreadStop, 0) != WAIT_OBJECT_0 && !Audio::g_bQuitAllTasks);

	SetEvent(engine.m_hThreadDone);

	BizAlignedFree(pBuffer);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	ShutdownOutputEngine: retail's C_GameApp::OnShutdown (0x0042f620) calls Engine::Instance().Shutdown()
//	inline; a free function lets the game reach it without the audio library's private headers
////////////////////////////////////////////////////////////////////////////////
namespace Audio
{
	void ShutdownOutputEngine()
	{
		Audio::Core::Engine::Instance().Shutdown();
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
