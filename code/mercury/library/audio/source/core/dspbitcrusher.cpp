#include "AudioPch.h"
#include "coreheaders.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::BitCrusher::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	BitCrusher
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::BitCrusher::BitCrusher( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
    using namespace Audio::Maths;
    
	m_eType = Core::DSP::Effect::eBitCrusher;

    m_fQuantize = 32768.f;
    m_fLevel = 1.f;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::BitCrusher::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.BitCrusher.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

//#define USE_SPU_OPTIMISED_BITCRUSHER
#if defined( PLATFORM_SPU ) && defined ( USE_SPU_OPTIMISED_BITCRUSHER )
    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

//	if ((U32)aSample & 0xf) DebugBreak();


	vector float* vaSample = (vector float*)aSample;
	vector float vfQuantize = spu_splats( m_fQuantize );
	vector float fRcpQuantize = spu_re( vfQuantize );
	vector float vfLevel = spu_splats( m_fLevel );
	U32 uSampleIndex = 0;
	
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		for( U32 uSample = 0; uSample < uFrameLength; uSample+=4 )
		{
			vector float vSample		= vaSample[ uSampleIndex ];
			vector float vfOut			= spu_mul( vSample, vfQuantize );
			vector signed int viOut		= spu_convts( vfOut, 0 );
			vfOut						= spu_convtf( viOut, 0 );
			vector float vfOut2			= spu_mul( vfOut,  fRcpQuantize );
			vaSample[ uSampleIndex++ ]	= spu_mul( vfOut2, vfLevel );
		}
	}
#else

#if !defined( AUDIO_OPTIMIZE_BITCRUSHER )

	using namespace Audio::Maths;
    
    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
    
    const F32 fOne = 1.0f;
    const F32 fMinusOne = -1.0f;
    
    const F64 fQuantize = (F64)m_fQuantize;
    const F64 fRcpQuantize = 1.0f / fQuantize;
    const F32 fLevel = m_fLevel;

	U32 uSampleIndex = 0;
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		for( U32 uSample = 0; uSample < uFrameLength; uSample++ )
		{
			const F32 fSample = aSample[ uSampleIndex ];
   			const F32 fSgn = Select( fSample, fOne, fMinusOne );
			const F64 fIn = (F64)Maths::Abs( fSample );
			const F32 fOut = (F32)( fRcpQuantize * Floor( fIn * fQuantize ) );

			aSample[ uSampleIndex++ ] = fLevel * fSgn * fOut;
		}
	}

#else

	using namespace Audio::Maths;
    
	Vector4* pBuffer = (Vector4*)pFrameData;

    const Vector4 vOne = MakeVector4( g_VectorOne );
    const Vector4 vZero = MakeVector4( g_VectorZero );
    const Vector4 vMinusOne = MakeVector4( g_VectorMinusOne );

    const Scalar vQuantize = _Scalar( m_fQuantize );
	const Scalar vRcpQuantize = _Scalar( 1.f / m_fQuantize );
    const Scalar vLevel = _Scalar( m_fLevel );

	U32 uQuadSampleIndex = 0;
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		for( U32 uSample = 0; uSample < uFrameLength; uSample += 16 )
		{
			Vector4 vSample0 = pBuffer[ uQuadSampleIndex + 0 ];
			Vector4 vSample1 = pBuffer[ uQuadSampleIndex + 1 ];
			Vector4 vSample2 = pBuffer[ uQuadSampleIndex + 2 ];
			Vector4 vSample3 = pBuffer[ uQuadSampleIndex + 3 ];

			Vector4 vSgn0 = Select_GT( vSample0, vZero, vOne, vMinusOne );
			Vector4 vSgn1 = Select_GT( vSample1, vZero, vOne, vMinusOne );
			Vector4 vSgn2 = Select_GT( vSample2, vZero, vOne, vMinusOne );
			Vector4 vSgn3 = Select_GT( vSample3, vZero, vOne, vMinusOne );

			Vector4 vIn0 = Abs( vSample0 );
			Vector4 vIn1 = Abs( vSample1 );
			Vector4 vIn2 = Abs( vSample2 );
			Vector4 vIn3 = Abs( vSample3 );

			Vector4 vOut0 = vRcpQuantize * RoundToIntZero( vIn0 * vQuantize );
			Vector4 vOut1 = vRcpQuantize * RoundToIntZero( vIn1 * vQuantize );
			Vector4 vOut2 = vRcpQuantize * RoundToIntZero( vIn2 * vQuantize );
			Vector4 vOut3 = vRcpQuantize * RoundToIntZero( vIn3 * vQuantize );

            pBuffer[ uQuadSampleIndex + 0 ] = vLevel * vSgn0 * vOut0;
            pBuffer[ uQuadSampleIndex + 1 ] = vLevel * vSgn1 * vOut1;
            pBuffer[ uQuadSampleIndex + 2 ] = vLevel * vSgn2 * vOut2;
            pBuffer[ uQuadSampleIndex + 3 ] = vLevel * vSgn3 * vOut3;

			uQuadSampleIndex += 4;
		}
	}
#endif

#endif

	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
