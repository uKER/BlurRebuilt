#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
//#include <System\SPU\AutoPointer.h>
extern bool bTempDebugBreak;
#endif
#include <System/Debug/Debug.h>

#if defined (AUDIO_INPUT_RECORD)
Debug::Float g_dfAudioInputRecordLevel("Audio.System.InputRecordLevel", 1.f, Debug::Range(0.f, 10.0f), Debug::Step(0.01f));
Debug::Bool g_dbAudioEnableRecordInputs("Audio.System.EnableRecordInputs", false);
Debug::Bool g_dbAudioFoldDownRecordings("Audio.System.FoldDownInputRecordings", true);
Debug::Bool g_dbAudioRecordInputs("Audio.System.RecordInputs", false);
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	InputSize (static)
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Core::Input::InputSize( U16 uChannelsOut, U16 uEffects )
{
	U32 uSize = sizeof( Input );

	//	effects
	uSize += sizeof( DSP::Element ) * uEffects;

	return AUDIO_CORE_ALIGN_16( uSize );
}

////////////////////////////////////////////////////////////////////////////////
//	Input
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Input::Input( const char* pchInfo, U32 u32NumChannels, U16 uEffects ) : Generator( pchInfo, u32NumChannels, false )
{
	m_uClassSize = sizeof( Input );

	m_uEffects = uEffects;

	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	Generator::ResetEffects( aEffects, m_uEffects );

	Generator::m_eType = eInput;

	m_afAltBuffer = NULL;
	m_bMuted = false;
	m_bWantsData = true;

#if defined (AUDIO_INPUT_RECORD)
	m_pWriter = NULL;
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Input::SetProcessFlag( const ProcessInfo& info, bool bProcess )
{
	m_bWantsData = bProcess;

	return m_bProcessing;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Input::Process( const ProcessInfo& info )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.Input.Process");

	if (Generator::m_uBuffer != (U32)-1 )
	{
		F32* pFrameBuffer = (F32*)&info.pDynamicBuffers[ Generator::m_uBuffer ];

#if defined (PLATFORM_SPU)
		Spu::Fetch( Audio::System::Get()->GetDmaScratchBuffer(), m_afAltBuffer, uFrameLength * m_u32NumChannels * sizeof(F32) );
		F32* pIOBuffer = (F32*)Audio::System::Get()->GetDmaScratchBuffer();
#else
		F32* pIOBuffer = m_afAltBuffer;
#endif

		for( U32 uCh = 0; uCh < m_u32NumChannels; uCh++ )
		{
#ifdef AUDIO_CHECK_SAMPLES
			if( !Frame::IsValid( (F32*)( pFrameBuffer + uCh * uFrameLength ) ) )
				AUDIO_CORE_FATAL( "Input::Process() Invalid sample in pFrameBuffer frame in '%s'. chanIn = %d", m_pchInfo, uCh );
#endif // AUDIO_CHECK_SAMPLES

			if( Generator::m_bProcessing )
			{
				Frame::Copy( pFrameBuffer + uCh * uFrameLength, pIOBuffer + uCh * uFrameLength );
				Frame::Zero( pIOBuffer + uCh * uFrameLength );
			}
			else
				Frame::Zero( pFrameBuffer + uCh * uFrameLength );
		}

#if defined (PLATFORM_SPU)
		Spu::StoreAsync( m_afAltBuffer, Audio::System::Get()->GetDmaScratchBuffer(), uFrameLength * m_u32NumChannels * sizeof(F32), 0 );
		Audio::System::Get()->AsyncDmaCommandIssued();
#endif

		Generator::m_bProcessing = false;
	}
	else
		AUDIO_CORE_WARN( "Input '%s' not connected to audio graph", m_pchInfo );

	m_uFrameCount = info.uFrameNum;

#if defined (AUDIO_INPUT_RECORD)
	if( g_dbAudioEnableRecordInputs )
		WriteWave( info );
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DumpStats
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Input::DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals)
{
    if (pInternals)
    {
        pInternals->uTotalNumGenerators++;

        if (Generator::m_bProcessing)
            pInternals->uNumGeneratorsActive++;
    }

	F32* pFrameData = (F32*)&info.pDynamicBuffers[Generator::m_uBuffer];

    if (Generator::m_pchInfo)
        AUDIO_SYS_DEBUG("%s[Input %dch] \"%s\" <%s> F:0x%.8x (%.2f)", pchPrefix, m_u32NumChannels, Generator::m_pchInfo, Generator::m_bProcessing ? "active" : "inactive", pFrameData, *pFrameData);
    else
        AUDIO_SYS_DEBUG("%s[Input %dch] <%s> F:0x%.8x (%.2f)", pchPrefix, m_u32NumChannels, Generator::m_bProcessing ? "active" : "inactive", pFrameData, *pFrameData);
}

#if defined (AUDIO_INPUT_RECORD)
////////////////////////////////////////////////////////////////////////////////
//	WriteWave
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Input::WriteWave( const ProcessInfo& info )
{
	if( ( strcmp( m_pchInfo, "AICarSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "AICollisionsSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "AmbientSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "ReverbInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "PlayerCarSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "PlayerCollisionsSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "ListenerSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "MusicStereoInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "VOMonoInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "DOPOSurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "UISurroundInput" ) == 0 ) ||
		( strcmp( m_pchInfo, "PerkSurroundInput" ) == 0 ) )
	{
		if( g_dbAudioRecordInputs )
		{
			if( m_pWriter == NULL )
				m_pWriter = WaveWriter::CreateWriter( m_pchInfo, m_u32NumChannels, g_dbAudioFoldDownRecordings );

			if( m_pWriter != NULL )
			{
				F32* pFrameBuffer = (F32*)&info.pDynamicBuffers[ Generator::m_uBuffer ];
				m_pWriter->Write( pFrameBuffer, g_dfAudioInputRecordLevel );
			}
		}
		else
		{
			if( m_pWriter != NULL )
			{
				delete m_pWriter;
				m_pWriter = NULL;
			}
		}
	}
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
