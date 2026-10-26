#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
//#include <System\SPU\AutoPointer.h>
#endif
#include <System/Serialise/I_FileSystem.h>
#if !defined (PLATFORM_SPU) && !defined (PLATFORM_PS3)
#include <memory>
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#if !defined (PLATFORM_SPU) && !defined (PLATFORM_PS3)
////////////////////////////////////////////////////////////////////////////////
//	WaveWriter
////////////////////////////////////////////////////////////////////////////////
Audio::Core::WaveWriter::WaveWriter( const char* pchName, U32 uNumChannels, bool bFoldDown )
{
	m_bFoldDown = bFoldDown;
	m_bError = true;
	m_uNumChannels = uNumChannels;
	m_uBytesWritten = 0;

	for( U32 uCh = 0; uCh < m_uNumChannels; uCh++ )
	{
		if( !m_bFoldDown || uCh == 0 )
		{
			Sys::C_FilePath strFilePath( "\\audio\\" );
			strFilePath += pchName;
			if( !m_bFoldDown )
			{
				strFilePath += "_";
				strFilePath += Btl::String( '0' + uCh );
			}
			strFilePath += ".wav";

			m_apFile[ uCh ] = Sys::GetDefaultFileSystem()->Open( strFilePath, Sys::E_FileAccess_Write );

			if( m_apFile[ uCh ] == NULL )
			{
				if( uCh )
				{
					for( U32 uCount = 0; uCount < uCh; uCount++ )
					{
						delete m_apFile[ uCount ];
						m_apFile[ uCount ] = NULL;
					}
				}

				return;
			}

			StartWave( m_apFile[ uCh ] );
		}
	}

	m_bError = false;
}

////////////////////////////////////////////////////////////////////////////////
//	WaveWriter
////////////////////////////////////////////////////////////////////////////////
Audio::Core::WaveWriter::~WaveWriter()
{
	for( U32 uCh = 0; uCh < m_uNumChannels; uCh++ )
	{
		if( !m_bFoldDown || uCh == 0 )
			EndWave( m_apFile[ uCh ] );
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Write
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::WaveWriter::Write( F32* pFrameData, F32 fLevel )
{
	bool bReturn = false;

	if( !m_bError )
	{
		bReturn = true;

		U8* asData = (U8*)alloca( uFrameLength * sizeof(S16) );

		if( !m_bFoldDown )
		{
			//	write out as many wavs as there are channels
			for( U32 uCh = 0; uCh < m_uNumChannels; uCh++ )
			{
				if( m_apFile[ uCh ] )
				{
					F32* afSamples = &pFrameData[ uCh * uFrameLength ];

					for( U32 uCount = 0; uCount < uFrameLength; uCount++ )
					{
						F32 fVal = fLevel * afSamples[ uCount ];
						S16 sVal = (S16)( fVal * 32767.f );

						asData[ uCount * 2 ] = sVal & 0xff;
						asData[ uCount * 2 + 1 ] = ( sVal >> 8 ) & 0xff;
					}

					m_apFile[ uCh ]->Write( asData, uFrameLength * sizeof(S16) );

					if( m_uBytesWritten % ( 128 * 1024 ) == 0 )
						m_apFile[ uCh ]->Flush();
				}
				else
					bReturn = false;
			}
		}
		else
		{
			//	fold down all channels to mono
			if( m_apFile[ 0 ] )
			{
				if( m_uNumChannels == 1 )
				{
					for( U32 uCount = 0; uCount < uFrameLength; uCount++ )
					{
						F32 fVal = fLevel * pFrameData[ uCount ];
						S16 sVal = (S16)( fVal * 32767.f );

						asData[ uCount * 2 ] = sVal & 0xff;
						asData[ uCount * 2 + 1 ] = ( sVal >> 8 ) & 0xff;
					}
				}
				else if( m_uNumChannels == 2 )
				{
					for( U32 uCount = 0; uCount < uFrameLength; uCount++ )
					{
						F32 fVal = fLevel * ( pFrameData[ uCount ] + pFrameData[ uFrameLength + uCount ] );
						S16 sVal = (S16)( fVal * 32767.f );

						asData[ uCount * 2 ] = sVal & 0xff;
						asData[ uCount * 2 + 1 ] = ( sVal >> 8 ) & 0xff;
					}
				}
				else if( m_uNumChannels == 5 )
				{
					for( U32 uCount = 0; uCount < uFrameLength; uCount++ )
					{
						F32 fL = pFrameData[ uCount ];
						F32 fR = pFrameData[ uFrameLength + uCount ];
						F32 fC = pFrameData[ 2 * uFrameLength + uCount ];
						F32 fSL = pFrameData[ 3 * uFrameLength + uCount ];
						F32 fSR = pFrameData[ 4 * uFrameLength + uCount ];
						F32 fVal = fLevel * ( fL + fR + fC + 0.5f * ( fSL + fSR ) );
						S16 sVal = (S16)( fVal * 32767.f );

						asData[ uCount * 2 ] = sVal & 0xff;
						asData[ uCount * 2 + 1 ] = ( sVal >> 8 ) & 0xff;
					}
				}
				else if( m_uNumChannels == 6 )
				{
					for( U32 uCount = 0; uCount < uFrameLength; uCount++ )
					{
						F32 fL = pFrameData[ uCount ];
						F32 fR = pFrameData[ uFrameLength + uCount ];
						F32 fC = pFrameData[ 2 * uFrameLength + uCount ];
						F32 fSL = pFrameData[ 4 * uFrameLength + uCount ];
						F32 fSR = pFrameData[ 5 * uFrameLength + uCount ];
						F32 fVal = fLevel * ( fL + fR + fC + 0.5f * ( fSL + fSR ) );
						S16 sVal = (S16)( fVal * 32767.f );

						asData[ uCount * 2 ] = sVal & 0xff;
						asData[ uCount * 2 + 1 ] = ( sVal >> 8 ) & 0xff;
					}
				}
				else
					Assert( 0 );

				m_apFile[ 0 ]->Write( asData, uFrameLength * sizeof(S16) );

				if( m_uBytesWritten % ( 128 * 1024 ) == 0 )
					m_apFile[ 0 ]->Flush();
			}
			else
				bReturn = false;
		}

		m_uBytesWritten += uFrameLength * sizeof(S16);
	}

	return bReturn;
}

////////////////////////////////////////////////////////////////////////////////
//	StartWave
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::WaveWriter::StartWave( Sys::I_File* pFile )
{
	U32 uNumChannels = 1;

	char chRiff[4] = { 'F', 'F', 'I', 'R' };
	WriteEndianBytes( pFile, &chRiff, 4 );
	U32 uSize = 0;	//	fill later
	WriteEndianBytes( pFile, &uSize, 4 );
	char chWave[4] = { 'E', 'V', 'A', 'W' };
	WriteEndianBytes( pFile, &chWave, 4 );

	char chFmt[4] = { ' ', 't', 'm', 'f' };
	WriteEndianBytes( pFile, &chFmt, 4 );
	uSize = 16;
	WriteEndianBytes( pFile, &uSize, 4 );
	U16 uCompression = 1;
	WriteEndianBytes( pFile, &uCompression, 2 );
	U16 uChannels = uNumChannels;
	WriteEndianBytes( pFile, &uChannels, 2 );
	U32 uSampleRate = 48000;
	WriteEndianBytes( pFile, &uSampleRate, 4 );
	U16 uBitsPerSamp = 16;
	U16 uBlockAlign = uNumChannels * ( uBitsPerSamp >> 3 );
	U32 uAvBytesPerSec = uSampleRate * uBlockAlign;
	WriteEndianBytes( pFile, &uAvBytesPerSec, 4 );
	WriteEndianBytes( pFile, &uBlockAlign, 2 );
	WriteEndianBytes( pFile, &uBitsPerSamp, 2 );
	//U16 uExtraBytes = 0;
	//WriteEndianBytes( &uExtraBytes, 2 );

	char chData[4] = { 'a', 't', 'a', 'd' };
	WriteEndianBytes( pFile, &chData, 4 );
	uSize = 0;	//	fill later
	WriteEndianBytes( pFile, &uSize, 4 );
}

////////////////////////////////////////////////////////////////////////////////
//	EndWave
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::WaveWriter::EndWave( Sys::I_File* pFile )
{
	if( pFile )
	{
		pFile->SetPosition( 4 );
		U32 uSize = m_uBytesWritten + 44 - 8;
		WriteEndianBytes( pFile, &uSize, 4 );
		pFile->SetPosition( 40 );
		uSize = m_uBytesWritten;
		WriteEndianBytes( pFile, &uSize, 4 );
		pFile->SetPosition( pFile->Length() );

		pFile->Flush();

		delete pFile;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	WriteEndianBytes
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::WaveWriter::WriteEndianBytes( Sys::I_File* pFile, void* pData, U32 uSize )
{
#if 0
	pFile->Write( pData, uSize );
#else
	U8* pBytes = (U8*)pData;
	U8* pSwap = (U8*)alloca( uSize );
	for( unsigned int i = 0; i < uSize; i++ )
		pSwap[ i ] = pBytes[ uSize - 1 - i ];

	pFile->Write( pSwap, uSize );
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	statics
////////////////////////////////////////////////////////////////////////////////
#ifndef CFG_GOLD
U32 Audio::Core::Generator::m_uTotalGen = 0;
U32 Audio::Core::Generator::m_uActiveGen = 0;
U32 Audio::Core::Generator::m_uTotalWave = 0;
U32 Audio::Core::Generator::m_uActiveWave = 0;
U32 Audio::Core::Generator::m_uTotalMix = 0;
U32 Audio::Core::Generator::m_uActiveMix = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
//	Generator
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Generator::Generator( const char* pchInfo, U32 u32NumChannels, bool bAlwaysOn )
{
	m_bProcess = true;
	m_bAlwaysOn = bAlwaysOn;

	m_uEffects = 0;

    m_bDynamicBuffMem = false;
    m_bDynamicBuffSizes = false;
    m_bStaticBuffMem = false;

	m_uTrailFrames = 0;

    m_bProcessing = false;

	m_uFrameCount = (U32)-3;

    //m_uTrailFrames = 0;

	m_u32NumChannels = u32NumChannels;

	m_uBuffer = (U32)-1;

    m_pchInfo = pchInfo;

#ifndef CFG_GOLD
	m_uTotalGen++;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	~Generator
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Generator::~Generator()
{
#ifndef CFG_GOLD
	m_uTotalGen--;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferDepths
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Core::Generator::DynamicBufferDepths( const ProcessInfo& info, BufferStack& buffers )
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	if( !m_bDynamicBuffMem )
    {
		m_uBuffer = buffers.GetDepth();	//	at this stage just an index

		AUDIO_CORE_DEBUG( "Generator::DynamicBufferDepths() framebuffers index: %d [%s]", m_uBuffer, m_pchInfo );

		//	start counting buffers required
		U32 uBuffersSize = Channels();

		//	do effects
	    for( U32 uEffect = 0; uEffect < m_uEffects; uEffect++ )
        {
			if( aEffects[ uEffect ].eType != DSP::Effect::eUndefined )
			{
				DSP::Effect* pDSP = (DSP::Effect*)&info.pCoreData[ aEffects[ uEffect ].uOffset ];

				switch( pDSP->GetType() )
				{
				case DSP::Effect::eSpectralReverb:
					uBuffersSize += ( (DSP::SpectralReverb*)pDSP )->DynamicBuffersRequired( info );
					break;

				case DSP::Effect::eFlanger:
					uBuffersSize += ( (DSP::Flanger*)pDSP )->DynamicBuffersRequired( info );
					break;

			//	case DSP::Effect::eSpectralEnvelope:
				//	( (DSP::SpectralEnvelope*)pDSP )->DynamicBufferDepths( info, buffers );
				//	break;

				default:
					break;
				}
			}
        }

		buffers.SetSize( uBuffersSize );

		m_bDynamicBuffMem = true;
    }
    else
		AUDIO_CORE_ERROR( "Generator::DynamicBufferDepths() input already configured: [%s]", m_pchInfo );
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferOffsets
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Core::Generator::DynamicBufferOffsets( const ProcessInfo& info, BufferStack& buffers )
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	if( !m_bDynamicBuffSizes )
    {
		//Assert(m_uBuffer < 1000);

		m_uBuffer = uFrameLength * buffers.GetOffset( m_uBuffer );

		AUDIO_CORE_DEBUG("Generator::DynamicBufferOffsets() framebuffers offset: %d [%s]", m_uBuffer, m_pchInfo);

		//	start of effects buffers
		U32 uBufferOffset = m_uBuffer + uFrameLength * Channels();

		//	do effects
	    for( U32 uEffect = 0; uEffect < m_uEffects; uEffect++ )
        {
			if( aEffects[ uEffect ].eType != DSP::Effect::eUndefined )
			{
				DSP::Effect* pDSP = (DSP::Effect*)&info.pCoreData[ aEffects[ uEffect ].uOffset ];

				switch( pDSP->GetType() )
				{
				case DSP::Effect::eSpectralReverb:
					( (DSP::SpectralReverb*)pDSP )->SetDynamicBuffer( info, uBufferOffset );
					uBufferOffset += uFrameLength * ( (DSP::SpectralReverb*)pDSP )->DynamicBuffersRequired( info );
					break;

				case DSP::Effect::eFlanger:
					( (DSP::Flanger*)pDSP )->SetDynamicBuffer( info, uBufferOffset );
					uBufferOffset += uFrameLength * ( (DSP::Flanger*)pDSP )->DynamicBuffersRequired( info );
					break;

				//case DSP::Effect::eSpectralEnvelope:
				//	( (DSP::SpectralEnvelope*)pDSP )->DynamicBufferOffsets( info, buffers );
				//	break;

				default:
					break;
				}
			}
        }

		m_bDynamicBuffSizes = true;
    }
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	StaticBufferOffsets
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Core::Generator::StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	U32 uNewSize = 0;

    if( !m_bStaticBuffMem )
    {
        uNewSize += uSize;

		//	add effects that use static memory
	    for (U32 uEffect = 0; uEffect < m_uEffects; uEffect++)
        {
			if( aEffects[ uEffect ].eType != DSP::Effect::eUndefined )
			{
				DSP::Effect* pDSP = (DSP::Effect*)&info.pCoreData[ aEffects[ uEffect ].uOffset ];

				switch (pDSP->GetType())
				{
				case DSP::Effect::eDelay:
					uNewSize += ((DSP::Delay*)pDSP)->StaticBufferOffsets(info, uNewSize);
					break;

				case DSP::Effect::eFlanger:
					uNewSize += ((DSP::Flanger*)pDSP)->StaticBufferOffsets(info, uNewSize);
					break;

				//case DSP::Effect::eFeedbackDelay:
				//	uNewSize += ((DSP::FeedbackDelay*)pDSP)->StaticBufferOffsets(info, uNewSize);
				//	break;

				case DSP::Effect::eSpectralReverb:
					uNewSize += ((DSP::SpectralReverb*)pDSP)->StaticBufferOffsets(info, uNewSize);
					break;

				//case DSP::Effect::eSpectralEnvelope:
				//	uNewSize += ((DSP::SpectralEnvelope*)pDSP)->StaticBufferOffsets(info, uNewSize);
				//	break;

				//case DSP::Effect::eLipSync:
				//	uNewSize += ((DSP::LipSync*)pDSP)->StaticBufferOffsets(info, uNewSize);
				//	break;

				default:
					break;
				}
			}
        }

        m_bStaticBuffMem = true;

        return uNewSize;
    }
    else
        return uSize;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Generator::Process( const ProcessInfo& info )
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	if (m_uFrameCount != info.uFrameNum)
	{
		if (m_uBuffer != (U32)-1)
		{
			F32* pFrameBuffer = (F32*)&info.pDynamicBuffers[m_uBuffer];

#ifdef AUDIO_CHECK_SAMPLES
			if (!Frame::IsValid((F32*)pFrameBuffer))
				AUDIO_CORE_FATAL("Output::Process() Invalid sample in '%s'", m_pchInfo);
#endif // AUDIO_CHECK_SAMPLES

			//	run effects on data
			for (U32 uEffect = 0; uEffect < m_uEffects; uEffect++)
			{
				if( aEffects[uEffect].eType != DSP::Effect::eUndefined )
				{
					DSP::Effect* pDSP = (DSP::Effect*)&info.pCoreData[ aEffects[ uEffect ].uOffset ];

#ifdef PLATFORM_SPU
					//spu_printf("%d / %d\n", aEffects[uEffect].eType, pDSP->GetType());
#endif

					bool bProcess = m_bProcessing;

					switch (aEffects[ uEffect ].eType)
					{
					//case DSP::Effect::eGain:
					//	((DSP::Gain*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					//case DSP::Effect::ePeakLevel:
					//	((DSP::PeakLevel*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eRMSLevel:
						((DSP::RMSLevel*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eOscilloscope:
					//	((DSP::Oscilloscope*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eIIRLowPass1stOrder:
						((DSP::IIRLowPass1stOrder*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::eIIRLowPass2ndOrder:
						((DSP::IIRLowPass2ndOrder*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eIIRLowPass4thOrder:
					//	((DSP::IIRLowPass4thOrder*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eIIRHighPass1stOrder:
						((DSP::IIRHighPass1stOrder*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eIIRHighPass2ndOrder:
					//	((DSP::IIRHighPass2ndOrder<uChannelsOut>*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eIIRPeakEQ:
						((DSP::IIRPeakEQ*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eIIRBandPass:
					//	((DSP::IIRBandPass<uChannelsOut>*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					//case DSP::Effect::eIIRLowShelf:
					//	((DSP::IIRLowShelf*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					//case DSP::Effect::eIIRHighShelf:
					//	((DSP::IIRHighShelf*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eDelay:
						((DSP::Delay*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eFeedbackDelay:
					//	((DSP::FeedbackDelay*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eSaturator:
						((DSP::Saturator*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::eBitCrusher:
						((DSP::BitCrusher*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::eRingModulator:
						((DSP::RingModulator*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::eCompressor:
						((DSP::Compressor*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;
	//
	//				//case DSP::Effect::eConvolution:
	//				//	((DSP::Convolution*)pDSP)->Process(pFrameBuffer, info, bProcess);
	//				//	break;
	//
	//#ifndef SPU_MEMORYSAVER
					//case DSP::Effect::eLipSync:
					//	((DSP::LipSync*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;
	//#endif
					//case DSP::Effect::ePhaser:
					//	((DSP::Phaser*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eFlanger:
						((DSP::Flanger*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::ePitchShift:
						((DSP::PitchShift*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					case DSP::Effect::eSpectralReverb:
						((DSP::SpectralReverb*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					//case DSP::Effect::eSpectralEnvelope:
					//	((DSP::SpectralEnvelope*)pDSP)->Process(pFrameBuffer, info, bProcess);
					//	break;

					case DSP::Effect::eSend:
						((DSP::Send*)pDSP)->Process(pFrameBuffer, info, bProcess);
						break;

					default:
#ifdef PLATFORM_SPU
						sLOGERROR(0, "Unrecognised DSP effect.");
#endif
						break;
					}
				}
			}

#ifdef AUDIO_CHECK_SAMPLES
			if (!Frame::IsValid((F32*)pFrameBuffer))
				AUDIO_CORE_FATAL("Output::Process() Invalid sample in '%s'", m_pchInfo);
#endif // AUDIO_CHECK_SAMPLES

			Frame::ZeroDenormals(pFrameBuffer);

			m_uFrameCount = info.uFrameNum;
		}
		else
			AUDIO_CORE_WARN("Generator '%s' not connected to audio graph", m_pchInfo);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetEffect
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
bool Audio::Core::Generator::SetEffect(U32 uIndex, U32 eType, U32 uOffset)
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	if (uIndex < m_uEffects)
	{
		aEffects[ uIndex ].eType = (Audio::Core::DSP::Effect::Type)eType;
		aEffects[ uIndex ].uOffset = uOffset;
		return true;
	}
	else
		return false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	SetTrailFrames
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Generator:: SetTrailFrames( const Audio::Core::ProcessInfo& info )
{
	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	m_uTrailFrames = 0;

    for( U32 uEffect = 0; uEffect < m_uEffects; uEffect++ )
    {
		if( aEffects[ uEffect ].eType != DSP::Effect::eUndefined )
		{
			U32 uTrailFrames = 0;
			DSP::Effect* pDSP = (DSP::Effect*)&info.pCoreData[ aEffects[ uEffect ].uOffset ];

			switch( pDSP->GetType( ) )
			{
			case DSP::Effect::eDelay:
				uTrailFrames = ( (DSP::Delay*)pDSP )->TrailFrames( info );
				break;

			//case DSP::Effect::eFeedbackDelay:
			//	uTrailFrames = ( (DSP::FeedbackDelay*)pDSP )->TrailFrames( info );
			//	break;

			case DSP::Effect::eSpectralReverb:
				uTrailFrames = ( (DSP::SpectralReverb*)pDSP )->TrailFrames( info );
				break;

			default:
				break;
			}

			m_uTrailFrames = uTrailFrames > m_uTrailFrames ? uTrailFrames : m_uTrailFrames;
		}
    }
}

#ifdef DUMP_PROCESSING_STATS
#define AUDIO_MAX_PROCESSING_DEPTH 64
////////////////////////////////////////////////////////////////////////////////
//	DumpProcessingStatus
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Generator::DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth )
{
	Assert( uDepth < AUDIO_MAX_PROCESSING_DEPTH );
	char pchIndent[ AUDIO_MAX_PROCESSING_DEPTH ];
	memset( pchIndent, ' ', AUDIO_MAX_PROCESSING_DEPTH );
	pchIndent[ uDepth ] = '\0';

	AUDIO_CORE_INFO( "%s%s:%s = %s", pchIndent, pchInstance, m_pchInfo, m_bProcessing ? "true" : "false" );
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
