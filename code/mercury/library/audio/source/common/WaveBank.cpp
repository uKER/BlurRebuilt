#include "AudioPch.h"
#ifdef AUDIO_TOOLS
// TODO: make audio tools use AudioPch.h
#include "stdafx.h"
#endif
#include <string.h>
#include <stdio.h>
#include "WaveBank.h"
#include "../SystemHeaders.h"

#include <System/memory/MemoryTracker.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

namespace Audio
{
 
// default mem functions
WaveBank::AllocFuncPtr WaveBank::AllocPtr = WaveBank::DefaultAlloc;
WaveBank::FreeFuncPtr WaveBank::FreePtr = WaveBank::DefaultFree;
	
//-----------------------------------------------------------------------------
// Name: DefaultAlloc()
// Desc: Default memory allocation function.  
//-----------------------------------------------------------------------------
void* WaveBank::DefaultAlloc(size_t sz, U32 uAlignment, AllocType eType)
{
	#ifndef AUDIO_TOOLS
	void* ptr;
	System::Get()->PushCurrentMemoryAllocationId(1);
	MEMORY_TRACKER_PUSH("Audio.WaveBanks");
	
	ptr = Audio::System::Alloc(sz, uAlignment, (Audio::System::AllocType)eType);
	
	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.WaveBanks");
	return ptr;
	#else
	return Audio::System::Alloc(sz, uAlignment, (Audio::System::AllocType)eType);
	#endif
}

//-----------------------------------------------------------------------------
// Name: DefaultFree()
// Desc: Default memory freeing function.  
//-----------------------------------------------------------------------------
void WaveBank::DefaultFree(void* ptr, AllocType eType)
{
	#ifndef AUDIO_TOOLS
	System::Get()->PushCurrentMemoryAllocationId(1);
	MEMORY_TRACKER_PUSH("Audio.WaveBanks");

	Audio::System::Free(ptr, (Audio::System::AllocType)eType);
	
	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.WaveBanks");
	#else
	Audio::System::Free(ptr, (Audio::System::AllocType)eType);
	#endif
}

//-----------------------------------------------------------------------------
// Name: Constructor
// Desc: 
//-----------------------------------------------------------------------------
WaveBank::WaveBank(const char* pchFname)
{ 
	Init(); 
	SetFilename(pchFname); 
}

//-----------------------------------------------------------------------------
// Name: Destructor
// Desc: 
//-----------------------------------------------------------------------------
WaveBank::~WaveBank()
{
	//printf("deleting bank %s, %x\n", m_BankHeader.pchName, this);

	for (int i=0; i<m_vpWaves.Size(); i++)
	{
		if (m_vpWaves[i])
		{
			if (m_vpWaves[i]->m_pExtraHeader)
				Free(m_vpWaves[i]->m_pExtraHeader);

			if (m_vpWaves[i]->m_pvWaveData)
			{
				{
					//printf("free wav %d, %x\n", i, m_vpWaves[i]->m_pvWaveData);
					Free(m_vpWaves[i]->m_pvWaveData);
				}
			}
			
			//printf("deleting wav %d, %x\n", i, m_vpWaves[i]);
			delete m_vpWaves[i];
			//m_vpWaves.Erase(i);
		}
	}


	if (m_pchFilename)
		Free(m_pchFilename);
}

//-----------------------------------------------------------------------------
// Name: Init
// Desc: Sets up WaveBank for first use
//-----------------------------------------------------------------------------
void WaveBank::Init() 
{

	m_pchFilename=NULL; 
	m_MemoryFootprint = 0;
	InitBankHeader();
}

//-----------------------------------------------------------------------------
// Name: InitBankHeader
// Desc: 
//-----------------------------------------------------------------------------
void WaveBank::InitBankHeader() 
{
	memset(&m_BankHeader, 0, sizeof(BankHeader));
	m_BankHeader.uTag=*((U32*)g_pchWbTag_Bank);
	m_BankHeader.uSize=sizeof(BankHeader);
	m_BankHeader.uVersion=g_WbVersion;
	m_BankHeader.uNumWaves=0;
	m_BankHeader.chEndian=eLittleEndian;
	m_BankHeader.chStreaming=false;
}

//-----------------------------------------------------------------------------
// Name: SetFilename
// Desc: Sets the filename of the bank.
//-----------------------------------------------------------------------------
void WaveBank::SetFilename(const char* pchFname)
{
	if (pchFname==NULL)
		return;

	// put the extension on if not already
	bool bHasExtension = (stricmp(pchFname + strlen(pchFname) - strlen(g_pchWbFileExt), g_pchWbFileExt) == 0);	// it hasnt got the extension

	size_t bufferLength = strlen(pchFname)+(bHasExtension?0:strlen(g_pchWbFileExt))+1;

	m_pchFilename = (char*)Alloc(bufferLength);
    strcpy_s(m_pchFilename, bufferLength, pchFname);
	if (!bHasExtension)
		strcat_s(m_pchFilename, bufferLength, g_pchWbFileExt);
}

//-----------------------------------------------------------------------------
// Name: BankFormat
// Desc: Return the bank's format. Debug.
//		 If this bank's waves are not all the same format then returns -1
//-----------------------------------------------------------------------------
S32	WaveBank::BankFormat()
{
	S32 sFormat = -1;

#ifndef CFG_GOLD
	for (int i=0; i<m_vpWaves.Size(); i++)
	{
		if (m_vpWaves[i])
		{
			if (sFormat == -1)
				sFormat = m_vpWaves[i]->m_baseHeader.uFormat;
			else
			if (sFormat != m_vpWaves[i]->m_baseHeader.uFormat)
				return -1;
		}
	}
#endif
	return sFormat;
}

//-----------------------------------------------------------------------------
// Name: ContainsFormat
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBank::ContainsFormat(U32 uFormat)
{
	for (int i=0; i<m_vpWaves.Size(); i++)
	{
		if (m_vpWaves[i])
		{
			if (uFormat == m_vpWaves[i]->m_baseHeader.uFormat)
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Name: Wave::Init
// Desc: setup the base wave
//-----------------------------------------------------------------------------
void WaveBank::Wave::Init()
{
	m_pExtraHeader = NULL;
	m_pvWaveData = NULL;

	memset(&m_baseHeader, 0, sizeof(WaveBaseHeader));
	m_baseHeader.uTag=*((U32*)g_pchWbTag_Wave);
	m_baseHeader.uSize=sizeof(WaveBaseHeader);
	m_baseHeader.uFormat=0xffffffff;			
	m_baseHeader.uDataStart=0;
	m_baseHeader.uDataLength=0;
	m_baseHeader.fDefaultLevel=1.0f;
	m_baseHeader.uFlags = 0;
}
//-----------------------------------------------------------------------------
// Name: BitsPerSample
// Desc: Query how many bits per sample a wave has got
//-----------------------------------------------------------------------------
U32 WaveBank::Wave::BitsPerSample()
{
	switch (m_baseHeader.uFormat)
	{
		case Core::uPCM32BitFloat: case Core::uPCM16BitSignedInt: return PcmWave()->uBitsPerSample;
		case Core::uXMA16Bit: case Core::uXMA216Bit: return XmaWave()->uBitsPerSample;
		case Core::uADPCM4Bit: return AdpcmWave()->uBitsPerSample;
		case Core::uMP3: return Mp3Wave()->uBitsPerSample;
		default: BIZ_ASSERT_MSG(0, "format %d not dealt with", m_baseHeader.uFormat); 
			return 0;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: NumChannels
// Desc: Query how many channels a wave has got
//-----------------------------------------------------------------------------
U32 WaveBank::Wave::NumChannels()
{
	switch (m_baseHeader.uFormat)
	{
		case Core::uPCM32BitFloat: case Core::uPCM16BitSignedInt: return PcmWave()->uNumChannels;
		case Core::uXMA16Bit: case Core::uXMA216Bit: return XmaWave()->uNumChannels;
		case Core::uADPCM4Bit: return AdpcmWave()->uNumChannels;
		case Core::uMP3: return Mp3Wave()->uNumChannels;
		default: BIZ_ASSERT_MSG(0, "format %d not dealt with", m_baseHeader.uFormat); 
			return 0;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: SampleRate
// Desc: Query a wave's sample rate
//-----------------------------------------------------------------------------
U32 WaveBank::Wave::SampleRate()
{
	switch (m_baseHeader.uFormat)
	{
		case Core::uPCM32BitFloat: case Core::uPCM16BitSignedInt: return PcmWave()->uSampleRate;
		case Core::uXMA16Bit: case Core::uXMA216Bit: return XmaWave()->uSampleRate;
		case Core::uADPCM4Bit: return AdpcmWave()->uSampleRate;
		case Core::uMP3: return Mp3Wave()->uSampleRate;
		default: BIZ_ASSERT_MSG(0, "format %d not dealt with", m_baseHeader.uFormat); 
			return 0;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: SampleRate
// Desc: Query if a wave is a loop
//-----------------------------------------------------------------------------
U32 WaveBank::Wave::Loop()
{
	switch (m_baseHeader.uFormat)
	{
		case Core::uPCM32BitFloat: case Core::uPCM16BitSignedInt: return PcmWave()->uNumLoops;
		case Core::uXMA16Bit: case Core::uXMA216Bit: return XmaWave()->uNumLoops;
		case Core::uADPCM4Bit: return AdpcmWave()->uLoop;
		case Core::uMP3: return Mp3Wave()->uLoop;
		default: BIZ_ASSERT_MSG(0, "format %d not dealt with", m_baseHeader.uFormat); 
			return 0;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: SampleRate
// Desc: Calculate a sample's length in secs
//-----------------------------------------------------------------------------
F32 WaveBank::Wave::LengthInSecs()
{ 
	if (!SampleRate() || !DataLength() || !BitsPerSample() || !NumChannels())
		return 0;
	return (F32)(DataLength() / ((F32)BitsPerSample() / 8.f) / SampleRate() / NumChannels()); }
}
#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
