#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

//using namespace Audio;
//using namespace Audio::Core;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#if defined (PLATFORM_PS3)
//#define AUDIO_UPSAMPLE_6KTO48K
#endif

#ifdef AUDIO_NEW_DECODERS
////////////////////////////////////////////////////////////////////////////////
//	Resample
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::SampleRateConverterLinear::Resample( F32* pfSamplesOut, U32 uBufferSize )
{
	//	copy in sample history from previous resample
	PushHistory();

	if ( m_uInputLength != uBufferSize )
	{
		//	shift required
#if 0
		for (U32 uCount = 0; uCount < uBufferSize; uCount++)
		{
			F32 fSubSample = (F32)uCount * (F32)m_uInputLength * fRcpFrameLength;

			F32 fIndex = Audio::Maths::Floor( fSubSample );

			F32 fFraction = fSubSample - (F32)fIndex;
			S32 sIndex = (S32)fIndex;

			F32 afSamples[2];
			afSamples[ 0 ] = m_afResampleBuffer[ sIndex ];
			afSamples[ 1 ] = m_afResampleBuffer[ sIndex + 1 ];

			F32 fDiff = afSamples[ 1 ] - afSamples[ 0 ];
			F32 fDelta = fDiff * fRatio;
			pfSamplesOut[ uCount ] = afSamples[ 0 ] + fDelta;
		}
#else
        using namespace Audio::Maths;

		Assert( uBufferSize % 8 == 0 );
        
        Vector4 vIndexIncrement1 = MakeVector4( m_uInputLength * 1 ) * MakeVector4( fRcpFrameLength );
        Vector4 vIndexIncrement8 = MakeVector4( m_uInputLength * 8 ) * MakeVector4( fRcpFrameLength );
        
        Vector4 vIndex_0123 = MakeVector4( g_VectorFloat_00_01_02_03 ) * vIndexIncrement1;
        Vector4 vIndex_4567 = MakeVector4( g_VectorFloat_04_05_06_07 ) * vIndexIncrement1;
        
        U32 iIndex_x_uFrameLength = 0;
		for ( U32 uCount = 0; uCount < uBufferSize; uCount+=8 )
        {
            const U32 iIndex0 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex1 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex2 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex3 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex4 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex5 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex6 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex7 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            
            const Vector4 vSampleA_0123 = MakeVector4( m_afResampleBuffer[iIndex0+0], m_afResampleBuffer[iIndex1+0], m_afResampleBuffer[iIndex2+0], m_afResampleBuffer[iIndex3+0] );
            const Vector4 vSampleA_4567 = MakeVector4( m_afResampleBuffer[iIndex4+0], m_afResampleBuffer[iIndex5+0], m_afResampleBuffer[iIndex6+0], m_afResampleBuffer[iIndex7+0] );
            const Vector4 vSampleB_0123 = MakeVector4( m_afResampleBuffer[iIndex0+1], m_afResampleBuffer[iIndex1+1], m_afResampleBuffer[iIndex2+1], m_afResampleBuffer[iIndex3+1] );
            const Vector4 vSampleB_4567 = MakeVector4( m_afResampleBuffer[iIndex4+1], m_afResampleBuffer[iIndex5+1], m_afResampleBuffer[iIndex6+1], m_afResampleBuffer[iIndex7+1] );
            
            const Vector4 vFraction_0123 = vIndex_0123 - RoundToIntZero( vIndex_0123 );
            const Vector4 vFraction_4567 = vIndex_4567 - RoundToIntZero( vIndex_4567 );
            
            const Vector4 vDiff_0123 = vSampleB_0123 - vSampleA_0123;
            const Vector4 vDiff_4567 = vSampleB_4567 - vSampleA_4567;
            
            vIndex_0123 += vIndexIncrement8;
            vIndex_4567 += vIndexIncrement8;
            
            const Vector4 result_0123 = MulAdd( vFraction_0123, vDiff_0123, vSampleA_0123 );
            const Vector4 result_4567 = MulAdd( vFraction_4567, vDiff_4567, vSampleA_4567 );
            
            ReadWriteBarrier();
            
            StoreAligned( result_0123, pfSamplesOut + uCount + 0 );
            StoreAligned( result_4567, pfSamplesOut + uCount + 4 );
        }
#endif
	}
	else
	{
		//	no shift, just copy
		CopySamples( pfSamplesOut, uBufferSize );
	}

	//	copy out sample history for next resample
	PopHistory();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	Resample
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::SampleRateConverterCubic::Resample( F32* pfSamplesOut, U32 uBufferSize )
{
	//	copy in sample history from previous resample
	PushHistory();

	if ( m_uInputLength != uBufferSize )
	{
		//	shift required
#if 0
		for (U32 uCount = 0; uCount < uBufferSize; uCount++)
		{
			F32 fSubSample = (F32)uCount * (F32)m_uInputLength * fRcpFrameLength;

			F32 fIndex = Audio::Maths::Floor( fSubSample );

			F32 fFraction = fSubSample - (F32)fIndex;
			S32 sIndex = (S32)fIndex;

			F32 fSA = m_afResampleBuffer[ sIndex ];
			F32 fSB = m_afResampleBuffer[ sIndex + 1 ];
			F32 fSC = m_afResampleBuffer[ sIndex + 2 ];
			F32 fSD = m_afResampleBuffer[ sIndex + 3 ];

			F32 fFrac2 = fFrac * fFrac;
			F32 fFrac3 = fFrac2 * fFrac;

			pfSamplesOut[ uCount ] = ( ( fSB * 2.f ) + ( -fSA + fSC ) * fFrac +
				( fSA * 2.f - fSB * 5.f + fSC * 4.f - fSD ) * fFrac2 +
				( -fSA + fSB * 3.f - fSC * 3.f + fSD ) * fFrac3 ) * 0.5f;
		}
#else
        /*
		F32 fTemp0 = 1.f * fSC - fSA;
		F32 fTemp1 = fSB * 5.f + 0.f;
		F32 fTemp2 = fSC * 4.f - fSD;
		F32 fTemp3 = fSB * 3.f - fSA;
		F32 fTemp4 = fSA * 2.f - fTemp1;
		F32 fTemp5 = fSC * 3.f - fSD;
		F32 fTemp6 = 1.f * fTemp4 + fTemp2;
		F32 fTemp7 = fTemp0 * fFrac + 0.f;
		F32 fTemp8 = fTemp6 * fFrac2 + 0.f;
		F32 fTemp9 = fSB * 2.f + fTemp7;
		F32 fTemp10 = 1.f * fTemp3 - fTemp5;
		F32 fTemp11 = 1.f * fTemp9 + fTemp8;
		F32 fTemp12 = fTemp10 * fFrac3 + fTemp11;

		pfSamplesOut[ uCount ] = fTemp12 * 0.5f + 0.f;
		*/

		using namespace Audio::Maths;

		Assert( uBufferSize % 8 == 0 );
        
        Vector4 vIndexIncrement1 = MakeVector4( m_uInputLength * 1 ) * MakeVector4( fRcpFrameLength );
        Vector4 vIndexIncrement8 = MakeVector4( m_uInputLength * 8 ) * MakeVector4( fRcpFrameLength );
        
        Vector4 vIndex_0123 = MakeVector4( g_VectorFloat_00_01_02_03 ) * vIndexIncrement1;
        Vector4 vIndex_4567 = MakeVector4( g_VectorFloat_04_05_06_07 ) * vIndexIncrement1;
        
        const Vector4 vZero =  MakeVector4( g_VectorZero );
        const Vector4 vHalf =  MakeVector4( g_VectorHalf );
        const Vector4 vOne =   MakeVector4( g_VectorOne );
        const Vector4 vTwo =   MakeVector4( g_VectorTwo );
        const Vector4 vThree = MakeVector4( g_VectorThree );
        const Vector4 vFour =  MakeVector4( g_VectorFour );
        const Vector4 vFive =  MakeVector4( g_VectorFive );
        
        U32 iIndex_x_uFrameLength = 0;
		for ( U32 uCount = 0; uCount < uBufferSize; uCount+=8 )
        {
            const U32 iIndex0 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex1 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex2 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex3 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex4 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex5 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex6 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            const U32 iIndex7 = iIndex_x_uFrameLength / uFrameLength; iIndex_x_uFrameLength += m_uInputLength;
            
            const Vector4 vSA_0123 = MakeVector4( m_afResampleBuffer[iIndex0+0], m_afResampleBuffer[iIndex1+0], m_afResampleBuffer[iIndex2+0], m_afResampleBuffer[iIndex3+0] );
            const Vector4 vSA_4567 = MakeVector4( m_afResampleBuffer[iIndex4+0], m_afResampleBuffer[iIndex5+0], m_afResampleBuffer[iIndex6+0], m_afResampleBuffer[iIndex7+0] );
            const Vector4 vSB_0123 = MakeVector4( m_afResampleBuffer[iIndex0+1], m_afResampleBuffer[iIndex1+1], m_afResampleBuffer[iIndex2+1], m_afResampleBuffer[iIndex3+1] );
            const Vector4 vSB_4567 = MakeVector4( m_afResampleBuffer[iIndex4+1], m_afResampleBuffer[iIndex5+1], m_afResampleBuffer[iIndex6+1], m_afResampleBuffer[iIndex7+1] );
            const Vector4 vSC_0123 = MakeVector4( m_afResampleBuffer[iIndex0+2], m_afResampleBuffer[iIndex1+2], m_afResampleBuffer[iIndex2+2], m_afResampleBuffer[iIndex3+2] );
            const Vector4 vSC_4567 = MakeVector4( m_afResampleBuffer[iIndex4+2], m_afResampleBuffer[iIndex5+2], m_afResampleBuffer[iIndex6+2], m_afResampleBuffer[iIndex7+2] );
            const Vector4 vSD_0123 = MakeVector4( m_afResampleBuffer[iIndex0+3], m_afResampleBuffer[iIndex1+3], m_afResampleBuffer[iIndex2+3], m_afResampleBuffer[iIndex3+3] );
            const Vector4 vSD_4567 = MakeVector4( m_afResampleBuffer[iIndex4+3], m_afResampleBuffer[iIndex5+3], m_afResampleBuffer[iIndex6+3], m_afResampleBuffer[iIndex7+3] );
            
            const Vector4 vFrac_0123 = vIndex_0123 - RoundToIntZero( vIndex_0123 );
            const Vector4 vFrac_4567 = vIndex_4567 - RoundToIntZero( vIndex_4567 );
            
            const Vector4 vFrac2_0123 = MulAdd( vFrac_0123, vFrac_0123, vZero );
            const Vector4 vFrac2_4567 = MulAdd( vFrac_4567, vFrac_4567, vZero );
            
            const Vector4 vFrac3_0123 = MulAdd( vFrac2_0123, vFrac2_0123, vZero );
            const Vector4 vFrac3_4567 = MulAdd( vFrac2_4567, vFrac2_4567, vZero );

			const Vector4 vTemp0_0123 = MulSub( vOne, vSC_0123, vSA_0123 );
            const Vector4 vTemp0_4567 = MulSub( vOne, vSC_4567, vSA_4567 );

            const Vector4 vTemp1_0123 = MulAdd( vSB_0123, vFive, vZero );
            const Vector4 vTemp1_4567 = MulAdd( vSB_4567, vFive, vZero );

            const Vector4 vTemp2_0123 = MulSub( vSC_0123, vFour, vSD_0123 );
            const Vector4 vTemp2_4567 = MulSub( vSC_4567, vFour, vSD_4567 );

            const Vector4 vTemp3_0123 = MulSub( vSB_0123, vThree, vSA_0123 );
            const Vector4 vTemp3_4567 = MulSub( vSB_4567, vThree, vSA_4567 );

            const Vector4 vTemp4_0123 = MulSub( vSA_0123, vTwo, vTemp1_0123 );
            const Vector4 vTemp4_4567 = MulSub( vSA_4567, vTwo, vTemp1_4567 );

            const Vector4 vTemp5_0123 = MulSub( vSC_0123, vThree, vSD_0123 );
            const Vector4 vTemp5_4567 = MulSub( vSC_4567, vThree, vSD_4567 );

            const Vector4 vTemp6_0123 = MulAdd( vOne, vTemp4_0123, vTemp2_0123 );
            const Vector4 vTemp6_4567 = MulAdd( vOne, vTemp4_4567, vTemp2_4567 );

            const Vector4 vTemp7_0123 = MulAdd( vTemp0_0123, vFrac_0123, vZero );
            const Vector4 vTemp7_4567 = MulAdd( vTemp0_4567, vFrac_4567, vZero );

            const Vector4 vTemp8_0123 = MulAdd( vTemp6_0123, vFrac2_0123, vZero );
            const Vector4 vTemp8_4567 = MulAdd( vTemp6_4567, vFrac2_4567, vZero );

            const Vector4 vTemp9_0123 = MulAdd( vSB_0123, vTwo, vTemp7_0123 );
            const Vector4 vTemp9_4567 = MulAdd( vSB_4567, vTwo, vTemp7_4567 );

            const Vector4 vTemp10_0123 = MulSub( vOne, vTemp3_0123, vTemp5_0123 );
            const Vector4 vTemp10_4567 = MulSub( vOne, vTemp3_4567, vTemp5_4567 );

            const Vector4 vTemp11_0123 = MulAdd( vOne, vTemp9_0123, vTemp8_0123 );
            const Vector4 vTemp11_4567 = MulAdd( vOne, vTemp9_4567, vTemp8_4567 );

            const Vector4 vTemp12_0123 = MulAdd( vTemp10_0123, vFrac3_0123, vTemp11_0123 );
            const Vector4 vTemp12_4567 = MulAdd( vTemp10_4567, vFrac3_4567, vTemp11_4567 );
 
			vIndex_0123 += vIndexIncrement8;
            vIndex_4567 += vIndexIncrement8;

            const Vector4 result_0123 = MulAdd( vTemp12_0123, vHalf, vZero );
            const Vector4 result_4567 = MulAdd( vTemp12_4567, vHalf, vZero );
            
            ReadWriteBarrier();
            
            StoreAligned( result_0123, pfSamplesOut + uCount + 0 );
            StoreAligned( result_4567, pfSamplesOut + uCount + 4 );
        }
#endif
	}
	else
	{
		//	no shift, just copy
		CopySamples( pfSamplesOut, uBufferSize );
	}

	//	copy out sample history for next resample
	PopHistory();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	GetDecodedData
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DecoderMonoSaw::GetDecodedData( F32* pfBuffer, U32 uLength )
{
	if ( pfBuffer )
	{
		static F32 fVal = 0.f;
		static F32 fInc = 0.01f;
		for ( U32 uSample = 0; uSample < uLength; uSample++ )
		{
			pfBuffer[ uSample ] = fVal;
			fVal += fInc;
			if ( fVal >= 1.f )
				fVal = -1.f;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	GetDecodedData
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DecoderMonoNoise::GetDecodedData( F32* pfBuffer, U32 uLength )
{
	if ( pfBuffer )
	{
		for ( U32 uSample = 0; uSample < uLength; uSample++ )
			pfBuffer[ uSample ] = -0.5f + (F32)rand() / (F32)RAND_MAX;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	GetDecodedData
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DecoderMonoPCMS16::GetDecodedData( F32* pfBuffer, U32 uLength )
{
	Core::PCMS16Decoder::WaveData waveData;
	S16* pDataIn;
	F32* pDataOut = pfBuffer;

	/*	TODO:
	U32 uBytesReq = uLength * sizeof(S16);
	while ( uBytesReq != 0 )
	{
		U32 uBytesRead;
		IWaveServer::Result res = m_pWaveServer->GetWaveData( m_hWave, m_uEncPos, &pDataIn, uBytesReq, &uBytesRead );

		if ( res == eSuccess )
		{
			pfBuffer = Decode( pfBuffer, pDataIn, uBytesRead );
			uBytesReq -= uBytesRead;
			m_uEncPos += uBytesRead;
		}
		else if ( res == eEndOfData )
		{
			pfBuffer = Decode( pfBuffer, pDataIn, uBytesRead );
			uBytesReq = 0;
			m_uEncPos += uBytesRead;
		}
		else if ( res == eDataNotReady )
		{
			//	underrun?
			uBytesReq = 0;
		}
	}

	xma
	//	get req num of samples from decoded buffer
	//	copy from xma buffer to decoded buffer
	//	submit more encoded data, if req
	*/
	U32 uBytesReq = uLength * sizeof(S16);
	Audio::Result res = m_pWaveServer->GetWave( m_hWave, &waveData );
	if ( Success == m_pWaveServer->GetWave( m_hWave, &waveData ) )
	{
		//	wave exists
		U8* pData = (U8*)waveData.psData;

		//	remaining encoded samples
		U32 uRemSamples = ( waveData.uLength - m_uEncPos ) / sizeof(S16);

		//	decode to buffer
		if ( uRemSamples > uLength )
		{
			//	all samples available, decode full buffer
			if ( pfBuffer )
				ConvertS16ToF32( pfBuffer, (S16*)&pData[ m_uEncPos ], uLength );

			m_uEncPos += uLength * sizeof(S16);
		}
		else
		{
			//	will need to wrap around, decode 1st half from end of encoded data
			if ( pfBuffer )
				ConvertS16ToF32( pfBuffer, (S16*)&pData[ m_uEncPos ], uRemSamples );

			//	decoded samples required for 2nd half
			U32 u2ndHalfSizeInSamples = uLength - uRemSamples;

			if ( 1 )	//	loop
			{
				//	set encoded data offset back to start
				m_uEncPos = u2ndHalfSizeInSamples * sizeof(S16);

				//	decode 2nd half from start of encoded data
				if ( pfBuffer )
					ConvertS16ToF32( &pfBuffer[ uRemSamples ], (S16*)&pData[ 0 ], u2ndHalfSizeInSamples );
			}
			else
			{
				//	set encoded data offset to end
				m_uEncPos = waveData.uLength;

				//	zero remainder of buffer
				memset( &pfBuffer[ uRemSamples ], 0, u2ndHalfSizeInSamples * sizeof(F32) );

				//	stop decoder
				Stop();
			}
		}
	}

	return true;
}
#endif // AUDIO_NEW_DECODERS

////////////////////////////////////////////////////////////////////////////////
//	PreInit
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::PreInit()
{
}

////////////////////////////////////////////////////////////////////////////////
//	PostShutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::PostShutdown()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::Init(const DecoderDataInfo& info)
{
	m_baseState.m_fB = Maths::ArcTan(Maths::fPi * 0.005f);
	m_baseState.m_fB = -(m_baseState.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_baseState.m_fB);
	m_baseState.m_fA = 0.5f * (1.0f - m_baseState.m_fB);

	if (! IsPlaying(info))
	{
		SetPlaying(info, true);
		m_uActivePCM++;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::Shutdown(const DecoderDataInfo& info)
{
	if (IsPlaying(info))
	{
		SetPlaying(info, false);
		m_uActivePCM--;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SetCallbackId
////////////////////////////////////////////////////////////////////////////////
//void Audio::Core::PCMS16Decoder::SetCallbackId(U32 u32StateOffset, U32 u32CallbackId)
//{
//    m_baseState.u32CallbackId = u32CallbackId;
//    m_baseState.u32StateOffset = u32StateOffset;
//}

////////////////////////////////////////////////////////////////////////////////
//	Position
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Core::PCMS16Decoder::Position(const DecoderDataInfo& info)
{
	if (m_waveData.psData)
	{
		if (m_waveData.uChannels)
		{
			S32 sNumSamples = (S32)m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));

			return (F32)GetWaveIndex(info) / (F32)sNumSamples;
		}
		else
			return 0.f;
	}

	Audio::Core::WaveStreamData * pData = GetWaveData(info);
	if (pData && pData->GetSourceDataPosition())
	{
		if (pData->GetSourceDataLength() > 0)	// to prevent divide by zero
			return F32((pData->GetSourceDataPosition() % pData->GetSourceDataLength()) / F32(pData->GetSourceDataLength()));
		else
			return 0.f;
	}
	else
		return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::Reset(const DecoderDataInfo& info, U32 uOffset)
{
	if (m_waveData.psData)
	    SetWaveIndex(uOffset);
	else if (m_waveData.hWaveServerIndex.IsValid())
		GetWaveData(info)->SetSourceDataPosition(0, (U32*)(info.pWaveServerData));

	m_baseState.fPitch = m_baseState.fNewPitch;
	m_baseState.fPrevPitch = m_baseState.fPitch;

	#ifdef CACHE_STREAMS_UPFRONT
	if (m_waveData.hWaveData.IsValid())
	{
		U32 u32NewFilePos = uOffset * m_waveData.uChannels * 2;	// * 2 converts samples to bytes

		// if file requested position is different to file pointer,
		// TODO: starting streaming wavs from an offset
		if (u32NewFilePos != Audio::System::Get()->GetStreamManager()->GetPrevSeekPos(m_waveData.hStream)
			|| ( (! Audio::System::Get()->GetStreamManager()->IsStreamPrepared(m_waveData.hStream))
				&& (! Audio::System::Get()->GetStreamManager()->IsStreamPreparing(m_waveData.hStream)) )
				)
		{
			Audio::System::Get()->GetStreamManager()->SetFilePosition(m_waveData.hStream, u32NewFilePos);
			bool bSuccess = Audio::System::Get()->GetStreamManager()->PrepareStream(m_waveData.hStream);
			Assert(bSuccess, "Could not prepare stream");
		}	
	}
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetPitch
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::SetPitch(F32 fPitch)
{
	F32 fRescaled = fPitch * (F32)m_waveData.uSampleRate * fRcpSampleRate;

    //  max shift up +2 oct above sample rate
    if (fRescaled > 4.f)
        m_baseState.fNewPitch = 4.f;

    //  max shift down -8 oct above sample rate
    else if (fRescaled < 0.00390625f)
        m_baseState.fNewPitch = 0.00390625f;

    else
        m_baseState.fNewPitch = fRescaled;
}

////////////////////////////////////////////////////////////////////////////////
//	Finished
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::PCMS16Decoder::Finished(const DecoderDataInfo& info)
{
	//if (m_waveData.hWaveData.IsValid())
	//	return Audio::System::Get()->GetStreamManager()->IsStreamFinished(m_waveData.hStream);

	if (m_waveData.psData)
	{
		S32 sNumSamples = (S32)m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));

		if (GetWaveIndex(info) >= sNumSamples)
			return true;
		else
			return false;
	}

	return GetWaveData(info)->IsFinished();
}

////////////////////////////////////////////////////////////////////////////////
//	SetPosition
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::SetPosition(U32 uPosition)
{
	if (m_waveData.psData)
	{
		S32 sNumSamples = (S32)m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));

		if (uPosition < (U32)sNumSamples)
			SetWaveIndex(uPosition);
	}
	else
	{
		// TODO: this
	}
}

////////////////////////////////////////////////////////////////////////////////
//	GetLoopCount
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::PCMS16Decoder::GetLoopCount(const DecoderDataInfo& info, bool bReset)
{
	U32 u32RetValue = 0;
	if (m_waveData.hWaveServerIndex.IsValid())
	{
		u32RetValue = GetWaveData(info)->GetLoopCount();

		if (bReset)
			GetWaveData(info)->ResetLoopCount();
	}
	return u32RetValue;
}

////////////////////////////////////////////////////////////////////////////////
//	UpdateFilterCoeffs
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::UpdateFilterCoeffs(F32 fPitch, const ProcessInfo& info)
{
	//	only update filter if using SRC and LP
	if (m_baseState.bResample)
	{
		//	pitch changing across subframes or new subframe set, recalc filter coeffs
		if ((m_baseState.fPitch != m_baseState.fPrevPitch) ||
			(info.uFrameMod == 0))
		{
			F32 fCutoff;
			if (fPitch > 0.5f)
				fCutoff = 0.5f / fPitch;
			else
				fCutoff = 1.f;

			m_baseState.m_fB = Maths::ArcTan(Maths::fPi * fCutoff);
			m_baseState.m_fB = -(m_baseState.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_baseState.m_fB);
			m_baseState.m_fA = 0.5f * (1.0f - m_baseState.m_fB);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	GetNumSamples
////////////////////////////////////////////////////////////////////////////////
S32 Audio::Core::PCMS16Decoder::GetNumSamples()
{
	return (S32)m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));
}

////////////////////////////////////////////////////////////////////////////////
//	PostUpdatePitch
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Core::PCMS16Decoder::GetSubFramePitch(const ProcessInfo& info)
{
	//  scale pitch by sample rate ratio
    F32 fPitch = m_baseState.fPrevPitch;

    //  interpolate between audio subframes
	fPitch += info.fFraction * (m_baseState.fPitch - m_baseState.fPrevPitch);

	return fPitch;
}

////////////////////////////////////////////////////////////////////////////////
//	PostUpdatePitch
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::PostUpdatePitch(const ProcessInfo& info)
{
	//	 if last sub frame, shift along pitch values
	if (info.uFrameMod == info.uFrameInt - 1)
	{
		m_baseState.fPrevPitch = m_baseState.fPitch;
		m_baseState.fPitch = m_baseState.fNewPitch;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CrossingBoundary
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::CrossingBoundary(const ProcessInfo& info, S32 sNumSamples)
{
	//  fire callback

	//if (m_baseState.u32CallbackId)
	//	Audio::System::TriggerDecoderCallback(m_baseState.u32CallbackId, m_baseState.u32StateOffset);

	if (m_baseState.bTriggerCallback)
		m_bCallbackFired = true;

	// don't change wave index if we're streaming- should always be 0
//	if (! m_waveData.hWaveData.IsValid())
	if (m_waveData.psData)
		SetWaveIndex(sNumSamples);

	if (! m_baseState.bLoop)
		Shutdown(info.decoderDataInfo);
}

#ifdef PLATFORM_SPU
U32 u32min(U32 a, U32 b) { return (a < b) ? a : b; }
#endif
////////////////////////////////////////////////////////////////////////////////
//	GetData
//  Note: if streamer is not ready GetData will return NULL
////////////////////////////////////////////////////////////////////////////////
S16* Audio::Core::PCMS16Decoder::GetData(const DecoderDataInfo& info, U32 u32NumSamples, U32* pu32SamplesProcessed)
{
//	if (! m_waveData.hWaveData.IsValid())
//	{
//#ifndef PLATFORM_SPU
		// for the time being, check if the old psData pointer is set- if so assume it's the source data
		// ultimately psData won't exist and we'll always use m_waveData.psData
		if (m_waveData.psData)
		{
			if (pu32SamplesProcessed)
				*(pu32SamplesProcessed) += u32NumSamples;

#ifdef PLATFORM_SPU
			return m_waveData.psData;
#else
			return m_waveData.psData;
#endif
		}
		else if (m_waveData.hWaveServerIndex.IsValid())
		{
			if (GetWaveData(info)->IsPrimed())
			{
#ifdef PLATFORM_SPU
				U8* pScratchBuffer = Audio::System::Get()->GetDmaScratchBuffer();
				U32 u32WaveServerBytesConsumed = GetWaveData(info)->u32BytesConsumed;
				S16* ps16Data = (S16*)pScratchBuffer + u32WaveServerBytesConsumed;
				U32 u32BytesConsumed = u32min(u32NumSamples * 2 * m_waveData.uChannels, GetWaveData(info)->u32BytesAvailable);

				if (pu32SamplesProcessed)
					*(pu32SamplesProcessed) += u32BytesConsumed / 2 / m_waveData.uChannels;

				GetWaveData(info)->OverrideBytesConsumed(u32BytesConsumed);
				return ps16Data;
#else
				return (S16*)(GetWaveData(info)->GetData(u32NumSamples * 2 * m_waveData.uChannels, (U32*)(info.pWaveServerData)));
#endif
			}
			else
			{
				GetWaveData(info)->Prime((U32*)(info.pWaveServerData));
				return 0;
			}
		}

		return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	IsDataInAvailable
////////////////////////////////////////////////////////////////////////////////
//bool Audio::Core::PCMS16Decoder::IsDataInAvailable()
//{
//	if (! m_waveData.hWaveData.IsValid())
//		return true;
//
//	return Audio::System::Get()->GetStreamManager()->IsStreamDataReady(m_waveData.hStream);
//}

////////////////////////////////////////////////////////////////////////////////
//	destructor
////////////////////////////////////////////////////////////////////////////////
Audio::Core::PCMS16Decoder::~PCMS16Decoder()
{
    m_uTotalPCM--;
}

#ifdef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	FetchData
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::FetchData(U8* pu8PpuAddress, U8* pu8SpuAddress, U32 u32SizeInBytes)
{
	// take into account that the dma must be 16 byte aligned
	u32DataOffset = (U32)pu8PpuAddress % 16;
	Spu::Fetch(pu8SpuAddress, (void*)(pu8PpuAddress - u32DataOffset), ((u32SizeInBytes + u32DataOffset) + 15) & 0xfffffff0);
}

////////////////////////////////////////////////////////////////////////////////
//	PrepareDmaTransfer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::PCMS16Decoder::PrepareDmaTransfer(const DecoderDataInfo& info)
{
	//if (m_waveData.hWaveData.IsValid())
	//	return;
	// if sourcing from the wave server...
	if (! m_waveData.psData)
	{
		WaveStreamData* pWaveStreamData = GetWaveData(info);
		U32 u32BytesAvailable = pWaveStreamData->GetBytesAvailable();

		// TODO: handle not receiving enough data
		//if (u32BytesAvailable > Audio::System::Get()->GetDmaScratchBufferSize())
		//	DebugBreak();

		if (pWaveStreamData->pData == 0 && u32BytesAvailable > 0)
			DebugBreak();

		U32 u32BytesToFetch = u32min(u32BytesAvailable, Audio::System::Get()->GetDmaScratchBufferSize());

		FetchData((U8*)(pWaveStreamData->GetData(u32BytesToFetch, info.pWaveServerData)), Audio::System::Get()->GetDmaScratchBuffer(), u32BytesToFetch);
		pWaveStreamData->OverrideBytesConsumed(0);
	}
	else
		FetchData((U8*)(m_waveData.psData + m_baseState.sWaveIndex), Audio::System::Get()->GetDmaScratchBuffer(), Audio::System::Get()->GetDmaScratchBufferSize());
}

#endif

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::MonoPCMS16Decoder::Init(const DecoderDataInfo& info)
{
	m_state.fPrevSample = 0.f;

	m_state.fPrevIn = 0.f;
	m_state.fPrevOut = 0.f;

	PCMS16Decoder::Init(info);
}

////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::MonoPCMS16Decoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.MonoPCMS16Decoder.FillBuffer");

	bool bActive = true;

	Frame::Zero(afData);

	//  num samples
	S32 sNumSamples = PCMS16Decoder::GetNumSamples();

	//  get subframe pitch
	F32 fPitch = PCMS16Decoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
	fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

#ifdef PLATFORM_SPU
	PrepareDmaTransfer(info.decoderDataInfo);
#endif
	
	//	recalc filter coeffs, if req
	PCMS16Decoder::UpdateFilterCoeffs(fPitch, info);

	bool bIsInRange = true;
	if (m_waveData.psData)
		bIsInRange = (GetWaveIndex(info.decoderDataInfo) < sNumSamples);
	else
		bIsInRange = true;//GetWaveData(info.decoderDataInfo)->u32SourceDataPosition < GetWaveData(info.decoderDataInfo)->u32SourceDataLength);

	//  only process if index is within sample length
	if (bIsInRange)
	{
		U32 u32OrigWaveIndex = GetWaveIndex(info.decoderDataInfo);

		//	using SRC and LP filter
		if (UsingResampler())
		{
			F32* AUDIO_RESTRICT afDataOut = afData;

			//	temp buffer
			F32 afShiftBuffer[(uFrameLength + 1) * 4];	//	1 extra sample, mono, upto 2 oct up

			//	copy required samples to temporary buffer
			U32 uSamplesRequired = (U32)((fPitch * fFrameLength) + 0.5f);

			U32 u32LocalWaveIndex = GetWaveIndex(info.decoderDataInfo);
			//if (m_waveData.hWaveData.IsValid())
			//	u32LocalWaveIndex = 0;

			U32 u32SamplesProcessed = 0;
#ifdef PLATFORM_SPU
			S16* asDataIn = GetData(info.decoderDataInfo, uSamplesRequired, &u32SamplesProcessed);
#else
			S16 const* AUDIO_RESTRICT asDataIn = GetData(info.decoderDataInfo, uSamplesRequired, &u32SamplesProcessed);
#endif
			//if (u32SamplesProcessed < uSamplesRequired)
			//	DebugBreak();

			// if no data in available, return a buffer full of 0's
			if ((! asDataIn) || GetBytesAvailable(info.decoderDataInfo) == 0)
			{
				for (U32 i = 0; i < uFrameLength; i++)
					afData[i] = 0.f;

				if ((! m_waveData.psData) && GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->IsFinished())
					PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());

				return true;
			}

			//	previous samples
			afShiftBuffer[0] = m_state.fPrevSample;
			F32* afShifted = &afShiftBuffer[1];

			bool bInRange = true;

			if (m_waveData.psData)
				bInRange = (GetWaveIndex(info.decoderDataInfo) + (S32)uSamplesRequired < sNumSamples);
			else
				// we don't need to loop or zero-pad if sourced from wave server...
				bInRange = true;//(! GetWaveData(info.decoderDataInfo)->bFinished);

			//  not crossing sample boundary, or streaming
			if (bInRange)
			{
				// Copy samples
#ifdef PLATFORM_SPU
				// don't take the wave index into account on the ps3, as we've already used it
				// to dma up the data we need
				FillF32BufferFromS16(uSamplesRequired, asDataIn, afShifted);
#else
				FillF32BufferFromS16(uSamplesRequired, asDataIn + u32LocalWaveIndex, afShifted);
#endif

				//for (U32 uCount = 0; uCount < uSamplesRequired; uCount++)
				//	afShifted[uCount] = (F32)asDataIn[m_baseState.sWaveIndex + uCount] * Maths::fInv2pow15;
			}
			//	crossing sample boundary
			else
			{
				U32 uLength1st = sNumSamples - GetWaveIndex(info.decoderDataInfo);
				U32 uLength2nd = uSamplesRequired - uLength1st;

				//	1st half
				for (U32 uCount = 0; uCount < uLength1st; uCount++)
#ifdef PLATFORM_SPU
					afShifted[uCount] = (F32)asDataIn[uCount] * Maths::fInv2pow15;
#else
					afShifted[uCount] = (F32)asDataIn[GetWaveIndex(info.decoderDataInfo) + uCount] * Maths::fInv2pow15;
#endif

				F32* afShifted2 = &afShifted[uLength1st];

				//	2nd half of looping
				if (m_baseState.bLoop)
				{
#ifdef PLATFORM_SPU
					SetWaveIndex(uLength1st);
					asDataIn = GetData(info.decoderDataInfo, uLength2nd, &u32SamplesProcessed);
#endif

					//	wraparound to start
					for (U32 uCount = 0; uCount < uLength2nd; uCount++)
						afShifted2[uCount] = (F32)asDataIn[uCount] * Maths::fInv2pow15;
				}
				//	2nd half of one shot
				else
				{
					//	pad with zeros
					for (U32 uCount = 0; uCount < uLength2nd; uCount++)
						afShifted2[uCount] = 0.f;
				}
			}

			LowPassFilter(uSamplesRequired, m_state, afShiftBuffer, m_baseState.m_fA, m_baseState.m_fB);

			Resample(fPitch, uSamplesRequired, afShiftBuffer, afDataOut);

			//	inc wave ptr
//			m_baseState.sWaveIndex += u32SamplesProcessed;
			SetWaveIndex(u32OrigWaveIndex + u32SamplesProcessed);
		}

		//	no SRC or LP - sample will be played back at Core::uSampleRate
		else
		{
			U32 u32SamplesProcessed = 0;

			U32 u32LocalWaveIndex = GetWaveIndex(info.decoderDataInfo);
			//if (m_waveData.hWaveData.IsValid())
			//	u32LocalWaveIndex = 0;

#ifdef PLATFORM_SPU
			// this pointer is changed in a wave wraparound situation, so don't make const
			S16* asDataIn = GetData(info.decoderDataInfo, uFrameLength, &u32SamplesProcessed);
#else
			S16 const* AUDIO_RESTRICT asDataIn = GetData(info.decoderDataInfo, uFrameLength, &u32SamplesProcessed);
#endif

			// if no data in available, return a buffer full of 0's
			if ((! asDataIn) || GetBytesAvailable(info.decoderDataInfo) == 0)
			{
				for (U32 i = 0; i < uFrameLength; i++)
					afData[i] = 0.f;

				return true;
			}

			afData[0] = m_state.fPrevSample;

			F32* AUDIO_RESTRICT afDataOut = &afData[1];
			//Frame::Zero(afDataOut);

#ifdef PLATFORM_SPU
				// don't take the wave index into account on the ps3, as we've already used it
				// to dma up the data we need
			S16 const* AUDIO_RESTRICT asIn = asDataIn;
#else
			S16 const* AUDIO_RESTRICT asIn = &asDataIn[u32LocalWaveIndex];
#endif

			//  not crossing sample boundary, or streaming
			if ((GetWaveIndex(info.decoderDataInfo) + (S32)uFrameLength - 1 < sNumSamples) || m_waveData.hWaveServerIndex.IsValid())
			{
				//	copy samples
				for (U32 uCount = 0; uCount < uFrameLength - 1; uCount++)
					afDataOut[uCount] = (F32)asIn[uCount] * Maths::fInv2pow15;

				//m_state.fPrevSample = afData[uFrameLength - 1];
				m_state.fPrevSample = (F32)asIn[uFrameLength - 1] * Maths::fInv2pow15;
			}
			else
			{
				U32 uLength1st = sNumSamples - GetWaveIndex(info.decoderDataInfo);
				U32 uLength2nd = uFrameLength - uLength1st - 1;

#ifdef PLATFORM_SPU
//				Spu::MemSyncDMA wrapAroundWaveDataTransfer;

//				wrapAroundWaveDataTransfer.Set((U32)(m_waveData.psData), uLength2nd * sizeof(S16));
//				if (wrapAroundWaveDataTransfer.GetRequiredBufferSize() > Audio::System::Get()->GetDmaScratchBufferSize())
//					DebugBreak();

//				wrapAroundWaveDataTransfer.PrepareBuffer(Audio::System::Get()->GetDmaScratchBuffer());

//				wrapAroundWaveDataTransfer.Lock();
//				wrapAroundWaveDataTransfer.Read();

				FetchData((U8*)m_waveData.psData, Audio::System::Get()->GetDmaScratchBuffer(), uLength2nd * sizeof(S16));
#endif

				//	1st half
				for (U32 uCount = 0; uCount < uLength1st; uCount++)
					afDataOut[uCount] = (F32)asIn[uCount] * Maths::fInv2pow15;

				F32* AUDIO_RESTRICT afOut2 = &afData[uLength1st + 1];

#ifdef PLATFORM_SPU
//				asDataIn = (S16*)(wrapAroundWaveDataTransfer.GetSPUAddress());
				asDataIn = (S16*)(Audio::System::Get()->GetDmaScratchBuffer());
#endif

				//	2nd half of looping
				if (m_baseState.bLoop)
				{
					//	wraparound to start
					for (U32 uCount = 0; uCount < uLength2nd; uCount++)
						afOut2[uCount] = (F32)asDataIn[uCount] * Maths::fInv2pow15;

					//m_state.fPrevSample = afData[uFrameLength - 1];
					m_state.fPrevSample = (F32)asDataIn[uLength2nd - 1] * Maths::fInv2pow15;
				}
				//	2nd half of one shot
				else
				{
					//	pad with zeros
					for (U32 uCount = 0; uCount < uLength2nd; uCount++)
						afOut2[uCount] = 0.f;

					m_state.fPrevSample = 0.f;
				}
#ifdef PLATFORM_SPU
//				wrapAroundWaveDataTransfer.Release();
#endif
			}

			//	inc wave ptr
			SetWaveIndex(u32SamplesProcessed);
		}
		//	crossed sample boundary

		if (m_waveData.psData)
		{
			if (GetWaveIndex(info.decoderDataInfo) >= sNumSamples)
				PCMS16Decoder::CrossingBoundary(info, sNumSamples);
		}
		else if (GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->IsFinished())
			PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());
		//else if (m_waveData.hWaveData.IsValid() && Audio::System::Get()->GetStreamManager()->IsStreamFinished(m_waveData.hStream))
		//	PCMS16Decoder::CrossingBoundary(info, sNumSamples);
	}
	else
	{
		//  past end of sample so zero buffer
		//if (sNumSamples == (S32)uFrameLength)
		//    Frame::Zero(afData);

		bActive = false;
	}

	PCMS16Decoder::PostUpdatePitch(info);

//#ifdef PLATFORM_SPU
//	ReleaseDmaTransfer();
//#endif

	return bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SkipBuffer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::MonoPCMS16Decoder::SkipBuffer(const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.MonoPCMS16Decoder.SkipBuffer");

#ifdef PLATFORM_SPU
	PrepareDmaTransfer(info.decoderDataInfo);
#endif
    //  num samples
    S32 sNumSamples = PCMS16Decoder::GetNumSamples();

    //  get subframe pitch
    F32 fPitch = PCMS16Decoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

	//	recalc filter coeffs, if req
	PCMS16Decoder::UpdateFilterCoeffs(fPitch, info);

	//  only process if index is within sample length
    if (GetWaveIndex(info.decoderDataInfo) < sNumSamples)
    {
		U32 uSamplesRequired = (U32)((fPitch * fFrameLength) + .5f);

		U32 u32SamplesProcessed = 0;
	    S16 const* AUDIO_RESTRICT asDataIn = GetData(info.decoderDataInfo, uSamplesRequired, &u32SamplesProcessed);

		if (asDataIn)
		{
			//	inc wave ptr
			SetWaveIndex(u32SamplesProcessed);

			//	crossed sample boundary
			if (GetWaveIndex(info.decoderDataInfo) >= sNumSamples)
				PCMS16Decoder::CrossingBoundary(info, sNumSamples);
			else if (GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->GetSourceDataPosition() >= GetWaveData(info.decoderDataInfo)->GetSourceDataLength())
				PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());
			//else if (m_waveData.hWaveData.IsValid() && Audio::System::Get()->GetStreamManager()->IsStreamFinished(m_waveData.hStream))
			//	PCMS16Decoder::CrossingBoundary(info, sNumSamples);

			//	store last sample for next iteration
			if (! m_waveData.hWaveServerIndex.IsValid())
			{
				if (GetWaveIndex(info.decoderDataInfo) != 0)
					m_state.fPrevSample = (F32)asDataIn[GetWaveIndex(info.decoderDataInfo) - 1] * Maths::fInv2pow15;
				else
					m_state.fPrevSample = (F32)asDataIn[sNumSamples - 1] * Maths::fInv2pow15;
			}
			else
				m_state.fPrevSample = (F32)asDataIn[uSamplesRequired - 1] * Maths::fInv2pow15;
		}
	}

	PCMS16Decoder::PostUpdatePitch(info);

//#ifdef PLATFORM_SPU
//	ReleaseDmaTransfer();
//#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::StereoPCMS16Decoder::Init(const DecoderDataInfo& info)
{
	m_state.fPrevSampleL = 0.f;
	m_state.fPrevSampleR = 0.f;

	m_state.fPrevInL = 0.f;
	m_state.fPrevInR = 0.f;
	m_state.fPrevOutL = 0.f;
	m_state.fPrevOutR = 0.f;

	PCMS16Decoder::Init(info);
}

////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::StereoPCMS16Decoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.StereoPCMS16Decoder.FillBuffer");

    bool bActive = true;

#ifdef PLATFORM_SPU
	PrepareDmaTransfer(info.decoderDataInfo);
#endif

	Frame::Zero(afData);
	Frame::Zero(&afData[uFrameLength]);

	//  num samples
	S32 sNumSamples = PCMS16Decoder::GetNumSamples();

	//  get subframe pitch
	F32 fPitch = PCMS16Decoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
	fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

	//	recalc filter coeffs, if req
	PCMS16Decoder::UpdateFilterCoeffs(fPitch, info);

	bool bIsInRange = true;
	if (m_waveData.psData)
		bIsInRange = (GetWaveIndex(info.decoderDataInfo) < sNumSamples);
	else
		bIsInRange = true;//GetWaveData(info.decoderDataInfo)->u32SourceDataPosition < GetWaveData(info.decoderDataInfo)->u32SourceDataLength);

	//  only process if index is within sample length
	if (bIsInRange)
	{
		//	temp buffers used when resampling enabled
		F32 afShiftBufferL[(uFrameLength + 1) * 4];		//	1 extra sample, stereo, 2 oct up
		F32 afShiftBufferR[(uFrameLength + 1) * 4];		//	1 extra sample, stereo, 2 oct up

		// The resampler used SRC and LP filter - if not on then we play back at Core::uSampleRate
		const bool usingResampler = UsingResampler();

		//	copy required samples to temporary buffer
		U32 uSamplesRequired = (usingResampler) ? (U32)((fPitch * fFrameLength) + .5f) : uFrameLength;
		U32 u32BytesAvailable = GetBytesAvailable(info.decoderDataInfo);

		U32 u32LocalWaveIndex = GetWaveIndex(info.decoderDataInfo);
		//if (m_waveData.hWaveData.IsValid())
		//	u32LocalWaveIndex = 0;

		U32 u32SamplesProcessed = GetWaveIndex(info.decoderDataInfo);
		S16 const* AUDIO_RESTRICT asDataIn = GetData(info.decoderDataInfo, uSamplesRequired, &u32SamplesProcessed);

		// if no data in available, return a buffer full of 0's
		if ((! asDataIn) || u32BytesAvailable == 0)
		{
			for (U32 i = 0; i < uFrameLength * 2; i++)
				afData[i] = 0.f;

			if ((! m_waveData.psData) && GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->IsFinished())
				PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());

			return true;
		}

		F32* AUDIO_RESTRICT afFillOutL;
		F32* AUDIO_RESTRICT afFillOutR;

		if (usingResampler)
		{
			//	previous samples
			afShiftBufferL[0] = m_state.fPrevSampleL;
			afShiftBufferR[0] = m_state.fPrevSampleR;

			afFillOutL = &afShiftBufferL[1];
			afFillOutR = &afShiftBufferR[1];
		}
		else
		{
			afFillOutL = &afData[0];
			afFillOutR = &afData[uFrameLength];
		}

		if (m_waveData.psData)
			bIsInRange = (GetWaveIndex(info.decoderDataInfo) + (S32)uSamplesRequired < sNumSamples);
		else
			// we don't need to loop or zero-pad if sourced from wave server...
			bIsInRange = true;//(! GetWaveData(info.decoderDataInfo)->bFinished);

		//  not crossing sample boundary, or streaming
		if (bIsInRange)
		{
			// Copy samples
#ifdef PLATFORM_SPU
				// don't take the wave index into account on the ps3, as we've already used it
				// to dma up the data we need
				FillF32BufferFromS16(uSamplesRequired, asDataIn, afFillOutL, afFillOutR);
#else
			FillF32BufferFromS16(uSamplesRequired, asDataIn + u32LocalWaveIndex*2, afFillOutL, afFillOutR);
#endif
		}
		else // Crossing sample boundary
		{
			// TODO: patch up on ps3

			U32 uLength1st = sNumSamples - GetWaveIndex(info.decoderDataInfo);
			U32 uLength2nd = uSamplesRequired - uLength1st;

			// 1st half
			FillF32BufferFromS16(uLength1st, asDataIn + GetWaveIndex(info.decoderDataInfo) * 2, afFillOutL, afFillOutR);

			// 2nd half of looping
			if (m_baseState.bLoop)
			{
				//	wraparound to start
				FillF32BufferFromS16(uLength2nd, asDataIn, afFillOutL + uLength1st, afFillOutR + uLength1st);
			}
			//	2nd half of one shot
			else
			{
				//	pad with zeros
				for (U32 uCount=0; uCount < uLength2nd; uCount++)
				{
					afFillOutL[uLength1st + uCount] = 0.f;
					afFillOutR[uLength1st + uCount] = 0.f;
				}
			}
		}

		if (usingResampler)
		{
			LowPassFilter(uSamplesRequired, m_state, afShiftBufferL, afShiftBufferR, m_baseState.m_fA, m_baseState.m_fB);

			F32* AUDIO_RESTRICT afDataOut = afData;
			Resample(fPitch, uSamplesRequired, afShiftBufferL, afShiftBufferR, afDataOut, afDataOut + uFrameLength);
		}

		//	inc wave ptr
		SetWaveIndex(u32SamplesProcessed);

		//	crossed sample boundary
		if (m_waveData.psData)
		{
			if (GetWaveIndex(info.decoderDataInfo) >= sNumSamples)
				PCMS16Decoder::CrossingBoundary(info, sNumSamples);
		}
		else if (GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->IsFinished())
			PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());
		//else if (m_waveData.hWaveData.IsValid() && Audio::System::Get()->GetStreamManager()->IsStreamFinished(m_waveData.hStream))
		//	PCMS16Decoder::CrossingBoundary(info, sNumSamples);
	}
	else
	{
		//  past end of sample so zero buffer
		//if (sNumSamples == (S32)uFrameLength)
		//{
		//	Frame::Zero(afData);
		//	Frame::Zero(&afData[uFrameLength]);
		//}

		bActive = false;
	}

	PCMS16Decoder::PostUpdatePitch(info);

//#ifdef PLATFORM_SPU
//	ReleaseDmaTransfer();
//#endif

	return bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SkipBuffer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::StereoPCMS16Decoder::SkipBuffer(const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.StereoPCMS16Decoder.SkipBuffer");

#ifdef PLATFORM_SPU
	PrepareDmaTransfer(info.decoderDataInfo);
#endif
    //  num samples
    S32 sNumSamples = PCMS16Decoder::GetNumSamples();

    //  get subframe pitch
    F32 fPitch = PCMS16Decoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

	//	recalc filter coeffs, if req
	PCMS16Decoder::UpdateFilterCoeffs(fPitch, info);

	//  only process if index is within sample length
    if (GetWaveIndex(info.decoderDataInfo) < sNumSamples)
    {
		U32 uSamplesRequired = (U32)((fPitch * fFrameLength) + .5f);

		U32 u32SamplesProcessed = 0;
	    S16 const* AUDIO_RESTRICT asDataIn = GetData(info.decoderDataInfo, uSamplesRequired, &u32SamplesProcessed);

		//	[nick] bug 25063 - fix for when GetData returns NULL
		if (asDataIn)
		{
			//	inc wave ptr
			SetWaveIndex(u32SamplesProcessed);

			//	crossed sample boundary
			if (m_waveData.psData)
			{
				if (GetWaveIndex(info.decoderDataInfo) >= sNumSamples)
					PCMS16Decoder::CrossingBoundary(info, sNumSamples);
			}
			else if (GetWaveData(info.decoderDataInfo) && GetWaveData(info.decoderDataInfo)->IsFinished())
				PCMS16Decoder::CrossingBoundary(info, GetWaveData(info.decoderDataInfo)->GetSourceDataLength());
			//else if (m_waveData.hWaveData.IsValid() && Audio::System::Get()->GetStreamManager()->IsStreamFinished(m_waveData.hStream))
			//	PCMS16Decoder::CrossingBoundary(info, sNumSamples);

			//	store last sample for next iteration (TODO: deal with loops/oneshots)
			if (! m_waveData.hWaveServerIndex.IsValid())
			{
				if (GetWaveIndex(info.decoderDataInfo) != 0)
				{
					m_state.fPrevSampleL = (F32)asDataIn[2 * GetWaveIndex(info.decoderDataInfo) - 2] * Maths::fInv2pow15;
					m_state.fPrevSampleR = (F32)asDataIn[2 * GetWaveIndex(info.decoderDataInfo) - 1] * Maths::fInv2pow15;
				}
				else
				{
					m_state.fPrevSampleL = (F32)asDataIn[2 * sNumSamples - 2] * Maths::fInv2pow15;
					m_state.fPrevSampleR = (F32)asDataIn[2 * sNumSamples - 1] * Maths::fInv2pow15;
				}
			}
			else
			{
				m_state.fPrevSampleL = (F32)asDataIn[2 * uSamplesRequired - 2] * Maths::fInv2pow15;
				m_state.fPrevSampleR = (F32)asDataIn[2 * uSamplesRequired - 1] * Maths::fInv2pow15;
			}
		}
	}

	PCMS16Decoder::PostUpdatePitch(info);

//#ifdef PLATFORM_SPU
//	ReleaseDmaTransfer();
//#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
