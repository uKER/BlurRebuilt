#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
//#include <System\SPU\AutoPointer.h>
#endif
#include <System/Serialise/I_FileSystem.h>

#if defined (AUDIO_OUTPUT_RECORD)
Debug::Float g_dfAudioOutputRecordLevel("Audio.System.OutputRecordLevel", 1.f, Debug::Range(0.f, 10.0f), Debug::Step(0.01f));
Debug::Bool g_dbAudioEnableRecordOutputs("Audio.System.EnableRecordOutputs", false);
Debug::Bool g_dbAudioFoldDownOutRecordings("Audio.System.FoldDownOutputRecordings", true);
Debug::Bool g_dbAudioRecordOutputs("Audio.System.RecordOutputs", false);
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	OutputSize (static)
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::Output::OutputSize( U16 uChannelsIn, U16 uEffects )
{
	U32 uSize = sizeof( Output );

	//	effects
	uSize += sizeof( DSP::Element ) * uEffects;

	return AUDIO_CORE_ALIGN_16( uSize );
}

////////////////////////////////////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::Output::Output(const char* pchInfo, U32 u32NumChannels, U16 uEffects) : Generator( pchInfo, u32NumChannels, false )
{
	m_uClassSize = sizeof( Output );

	m_uEffects = uEffects;

	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	Generator::ResetEffects( aEffects, m_uEffects );

	Generator::m_eType = eOutput;

	m_Item.eType = eUndefined;
	m_Item.uOffset = 0;

	for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
		m_apInputs[ uCount ] = NULL;

	m_bMuted = false;
	m_bMakeData = true;

#if defined (AUDIO_OUTPUT_RECORD)
	m_pWriter = NULL;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferDepths
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::DynamicBufferDepths( const ProcessInfo& info, BufferStack& buffers )
{
	switch( m_Item.eType )
	{
	case eMixer:
		( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferDepths( info, buffers );
		break;
	case eWave:
		( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferDepths( info, buffers );
		break;
	case eInput:
		( (Input*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferDepths( info, buffers );
		break;
	case eReturn:
		( (Return*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferDepths( info, buffers );
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferOffsets
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::DynamicBufferOffsets( const ProcessInfo& info, BufferStack& buffers )
{
	switch( m_Item.eType )
	{
	case eMixer:
		( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferOffsets( info, buffers );
		break;
	case eWave:
		( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferOffsets( info, buffers );
		break;
	case eInput:
		( (Input*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferOffsets( info, buffers );
		break;
	case eReturn:
		( (Return*)&info.pCoreData[ m_Item.uOffset ] )->DynamicBufferOffsets( info, buffers );
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	StaticBufferOffsets
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::Output::StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
{
	m_uOutBuffer = uSize;
    uSize += m_u32NumChannels * uFrameLength;

	switch( m_Item.eType )
	{
	case eMixer:
		return ( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->StaticBufferOffsets( info, uSize );
	case eWave:
		return ( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->StaticBufferOffsets( info, uSize );
	case eOutput:
		return ( (Output*)&info.pCoreData[ m_Item.uOffset ] )->StaticBufferOffsets( info, uSize );
	case eInput:
		return ( (Input*)&info.pCoreData[ m_Item.uOffset ] )->StaticBufferOffsets( info, uSize );
	case eReturn:
		return ( (Return*)&info.pCoreData[ m_Item.uOffset ] )->StaticBufferOffsets( info, uSize );
	}

	return uSize;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	SetProcessFlag
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::SetProcessFlag( const ProcessInfo& info )
{
	switch ( m_Item.eType )
	{
	case eMixer:
		Generator::m_bProcessing = ( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->SetProcessFlag( info, m_bMakeData && !m_bMuted );
		break;
	case eWave:
		Generator::m_bProcessing = ( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->SetProcessFlag( info, m_bMakeData && !m_bMuted );
		break;
	case eInput:
		Generator::m_bProcessing = ( (Input*)&info.pCoreData[ m_Item.uOffset ] )->SetProcessFlag( info, m_bMakeData && !m_bMuted );
		break;
	case eReturn:
		Generator::m_bProcessing = ( (Return*)&info.pCoreData[ m_Item.uOffset ] )->SetProcessFlag( info, m_bMakeData && !m_bMuted );
		break;
	case eUndefined:
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	MoveAudio
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::MoveAudio( const ProcessInfo& info )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.Output.MoveAudio");

	if( Generator::m_bProcessing )
	{
		F32* pFromBuffer = (F32*)&info.pStaticBuffers[ m_uOutBuffer ];

		//	write into next object/renderer
		for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
		{
			if( m_apInputs[ uCount ] )
			{
				if( m_apInputs[ uCount ]->WantsData() )
				{
					F32* pDestBuffer = (F32*)m_apInputs[ uCount ]->GetAltBuffer();

					for( U32 uCh = 0; uCh < m_u32NumChannels; uCh++ )
						Frame::DoAdd( pDestBuffer + uCh * uFrameLength, pDestBuffer + uCh * uFrameLength, pFromBuffer + uCh * uFrameLength );

					//	let input know data has been written to it
					m_apInputs[ uCount ]->SetActive( true );
				}
			}
		}
	}

	//	reset make data flag
	m_bMakeData = false;

	//	query next objects/renderer to see if this output needs to generate data
	for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
	{
		if( m_apInputs[ uCount ] )
		{
			if( m_apInputs[ uCount ]->WantsData() )
				m_bMakeData = true;
		}
	}

#if defined (AUDIO_OUTPUT_RECORD)
	if( g_dbAudioEnableRecordOutputs )
		WriteWave( info );
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	MoveAudio
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::MoveAudioBatch( const ProcessInfo& info )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.Output.MoveAudio");

	if( Generator::m_bProcessing )
	{
		F32* pFromBuffer = (F32*)&info.pStaticBuffers[ m_uOutBuffer ];

		//	write into next object/renderer
		for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
		{
			if( m_apInputs[ uCount ] )
			{
				if( m_apInputs[ uCount ]->WantsData() )
				{
					F32* pDestBuffer = (F32*)m_apInputs[ uCount ]->GetAltBuffer();

					U32 offset = 0;
					for( U32 uCh = 0; uCh < m_u32NumChannels; uCh++ )
					{
						Frame::AddBatch( &Audio::BatchFrame::gMoveAudioBatch, pDestBuffer + offset, pDestBuffer + offset, pFromBuffer + offset );
						offset += uFrameLength;
					}

					//	let input know data has been written to it
					m_apInputs[ uCount ]->SetActive( true );
				}
			}
		}
	}

	//	reset make data flag
	m_bMakeData = false;

	//	query next objects/renderer to see if this output needs to generate data
	for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
	{
		if( m_apInputs[ uCount ] )
		{
			if( m_apInputs[ uCount ]->WantsData() )
				m_bMakeData = true;
		}
	}

#if defined (AUDIO_OUTPUT_RECORD)
	if( g_dbAudioEnableRecordOutputs )
		WriteWave( info );
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Audio::Core::Output::Process
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::Process( const ProcessInfo& info )
{
	AUDIO_PERF_BEGIN_EVENT("Audio.Generate.Output.Process");

	if( m_Item.eType != (U32)eUndefined )
	{
		switch ( m_Item.eType )
		{
		case eMixer:
			( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->Process( info );
			break;
		case eWave:
			( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->Process( info );
			break;
		case eInput:
			( (Input*)&info.pCoreData[ m_Item.uOffset ] )->Process( info );
			break;
		case eReturn:
			( (Return*)&info.pCoreData[ m_Item.uOffset ] )->Process( info );
			break;
		}

		F32* AUDIO_RESTRICT pData = NULL;
		switch( m_Item.eType )
		{
		case eMixer:
			pData = ( ( Mixer* )&info.pCoreData[ m_Item.uOffset ] )->OutputBuffer( info );
			break;
		case eWave:
			pData = ( ( Wave* )&info.pCoreData[ m_Item.uOffset ] )->OutputBuffer( info );
			break;
		case eInput:
			pData = ( ( Input* )&info.pCoreData[ m_Item.uOffset ] )->OutputBuffer( info );
			break;
		case eReturn:
			pData = ( ( Return* )&info.pCoreData[ m_Item.uOffset ] )->OutputBuffer( info );
			break;
		}

		//	write into out buffer
		F32* pDestBuffer = (F32*)&info.pStaticBuffers[ m_uOutBuffer ];

		for( U32 uCh = 0; uCh < m_u32NumChannels; uCh++ )
			Frame::Copy( pDestBuffer + uCh * uFrameLength, pData + uCh * uFrameLength );
	}

	AUDIO_PERF_END_EVENT();
}

#ifdef DUMP_PROCESSING_STATS
////////////////////////////////////////////////////////////////////////////////
//	DumpProcessingStatus
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth )
{
	Generator::DumpProcessingStatus( info, pchInstance, uDepth );

	if( m_Item.eType != (U32)eUndefined )
	{
		switch ( m_Item.eType )
		{
		case eMixer:
			( (Mixer*)&info.pCoreData[ m_Item.uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eWave:
			( (Wave*)&info.pCoreData[ m_Item.uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eInput:
			( (Input*)&info.pCoreData[ m_Item.uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eReturn:
			( (Return*)&info.pCoreData[ m_Item.uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eUndefined:
		default:
			AUDIO_CORE_ERROR( "Output::DumpProcessingStatus() - Unknown input type" );
			break;
		}
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////
//	AddInput
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::AddInput( Input* pInput )
{
	for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
	{
		if( m_apInputs[ uCount ] == NULL )
		{
			m_apInputs[ uCount ] = pInput;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	RemoveInput
//	TODO: remove holes in array and use total slots used instead of the #define
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::RemoveInput( Input* pInput )
{
	for( U32 uCount = 0; uCount < AUDIO_MAX_INPUTS_PER_OUTPUT; uCount++ )
	{
		if( m_apInputs[ uCount ] == pInput )
		{
			m_apInputs[ uCount ] = NULL;
			break;
		}
	}
}

#ifdef ENABLE_DEBUG_EXTENSIONS
////////////////////////////////////////////////////////////////////////////////
//	Audio::Core::Output::DumpStats
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals)
{
#ifndef PLATFORM_SPU
    if (Generator::m_pchInfo)
        AUDIO_SYS_DEBUG("%s[Output %dch] \"%s\" <%s>", pchPrefix, m_u32NumChannels, Generator::m_pchInfo, Generator::m_bActive ? "active" : "inactive");
    else
        AUDIO_SYS_DEBUG("%s[Output %dch] <%s>", pchPrefix, m_u32NumChannels, Generator::m_bActive ? "active" : "inactive");

	if( m_Item.eType != eUndefined )
	{
		char pch[256];
        sprintf(pch, "  |%s", pchPrefix);

		switch( m_Item.eType )
		{
		case eMixer:
			((Mixer*)&info.pCoreData[m_Item.uOffset])->DumpStats(info, pch, pInternals);
			break;
		case eWave:
			((Wave*)&info.pCoreData[m_Item.uOffset])->DumpStats(info, pch, pInternals);
			break;
		case eOutput:
			((Output*)&info.pCoreData[m_Item.uOffset])->DumpStats(info, pch, pInternals);
			break;
		case eInput:
			((Input*)&info.pCoreData[m_Item.uOffset])->DumpStats(info, pch, pInternals);
			break;
		case eReturn:
			((Return*)&info.pCoreData[m_Item.uOffset])->DumpStats(info, pch, pInternals);
			break;
		}
	}
#endif
}
#endif

#if defined (AUDIO_OUTPUT_RECORD)
////////////////////////////////////////////////////////////////////////////////
//	Audio::Core::Output::WriteWave
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Output::WriteWave( const ProcessInfo& info )
{
	if( strcmp( m_pchInfo, "MusicSurroundOutput" ) == 0 )
	{
		if( g_dbAudioRecordOutputs )
		{
			if( m_pWriter == NULL )
				m_pWriter = WaveWriter::CreateWriter( m_pchInfo, m_u32NumChannels );

			if( m_pWriter != NULL )
			{
				F32* pFrameBuffer = (F32*)&info.pStaticBuffers[ m_uOutBuffer ];
				m_pWriter->Write( pFrameBuffer, g_dfAudioOutputRecordLevel );
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
