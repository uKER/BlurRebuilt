#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Saturator::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Saturator
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::Saturator::Saturator( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
    using namespace Audio::Maths;
    
	m_eType = Core::DSP::Effect::eSaturator;

	m_fDrive = 1.f;
    m_fClip = 1.f;
    m_fLevel = 1.f;

	m_fPrevDrive = 0.f;
    m_fPrevClip = 0.f;
    m_fPrevLevel = 0.f;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Saturator::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Saturator.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

    using namespace Audio::Maths;
    
    #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_SATURATOR ) // JASOND
    
    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
    
    const F32 fPrevDrive = m_fPrevDrive;
    const F32 fPrevClip  = m_fPrevClip;
    const F32 fPrevLevel = m_fPrevLevel;
    
	const F32 fDriveDelta = ( m_fDrive - m_fPrevDrive ) * Core::fRcpFrameLength;
	const F32 fClipDelta  = ( m_fClip  - m_fPrevClip  ) * Core::fRcpFrameLength;
	const F32 fLevelDelta = ( m_fLevel - m_fPrevLevel ) * Core::fRcpFrameLength;
    
    const F32 fOne = 1.0f;
    const F32 fMinusOne = -1.0f;
    
    // each channel
    U32 uSampleIndex = 0;
    for ( U32 uChannelIndex = 0; uChannelIndex < m_uChannels; uChannelIndex++ )
    {
        F32 fFrameIndex = 0.0f;
		for ( U32 uFrameIndex = 0; uFrameIndex < uFrameLength; uFrameIndex++ )
		{
            const F32 fSample = aSample[ uSampleIndex ];
            
			const F32 fDrive = fPrevDrive + fFrameIndex * fDriveDelta;
			const F32 fClip  = fPrevClip  + fFrameIndex * fClipDelta;
			const F32 fLevel = fPrevLevel + fFrameIndex * fLevelDelta;
			fFrameIndex += fOne;
            
			const F32 fIn  = fDrive * Maths::Abs( fSample );
			const F32 fSgn = Select( fSample, fOne, fMinusOne );
			const F32 fOut = Select( fClip - fIn, fIn, fIn * RcpEst( fOne + fIn - fClip ) );
            
            aSample[uSampleIndex++] = fLevel * fSgn * fOut;
		}
    }
    
    #else
    
    Vector4* __restrict aSample = (Vector4*)pFrameData;
    
    const Vector4 vRecipFrameLength = MakeVector4( Core::fRcpFrameLength );
    
    const Vector4 vPrevDrive = MakeVector4( m_fPrevDrive );
    const Vector4 vPrevClip  = MakeVector4( m_fPrevClip  );
    const Vector4 vPrevLevel = MakeVector4( m_fPrevLevel );
    
	const Vector4 vDriveDelta = ( MakeVector4( m_fDrive ) - vPrevDrive ) * vRecipFrameLength;
	const Vector4 vClipDelta  = ( MakeVector4( m_fClip  ) - vPrevClip  ) * vRecipFrameLength;
	const Vector4 vLevelDelta = ( MakeVector4( m_fLevel ) - vPrevLevel ) * vRecipFrameLength;
    
    const Vector4 vOne = MakeVector4( g_VectorOne );
	
	const Vector4 vSixteen = MakeVector4( g_VectorSixteen );
    
    // each channel
    U32 uQuadSampleIndex = 0;
    for ( U32 uChannelIndex = 0; uChannelIndex < m_uChannels; uChannelIndex++ )
    {
        //F32 fFrameIndex = 0.0f;
        Vector4 vFrameIndex_00_01_02_03 = MakeVector4( g_VectorFloat_00_01_02_03 );
        Vector4 vFrameIndex_04_05_06_07 = MakeVector4( g_VectorFloat_04_05_06_07 );
        Vector4 vFrameIndex_08_09_0A_0B = MakeVector4( g_VectorFloat_08_09_0A_0B );
        Vector4 vFrameIndex_0C_0D_0E_0F = MakeVector4( g_VectorFloat_0C_0D_0E_0F );
        
		for ( U32 uFrameIndex = 0; uFrameIndex < uFrameLength; uFrameIndex+=16 )
		{
            //const F32 fSample = aSample[ uSampleIndex ];
            const Vector4 vSample_00_01_02_03 = aSample[ uQuadSampleIndex + 0 ];
            const Vector4 vSample_04_05_06_07 = aSample[ uQuadSampleIndex + 1 ];
            const Vector4 vSample_08_09_0A_0B = aSample[ uQuadSampleIndex + 2 ];
            const Vector4 vSample_0C_0D_0E_0F = aSample[ uQuadSampleIndex + 3 ];
            
            
			const Vector4 vAbsSample_00_01_02_03 = Abs( vSample_00_01_02_03 );
			const Vector4 vAbsSample_04_05_06_07 = Abs( vSample_04_05_06_07 );
			const Vector4 vAbsSample_08_09_0A_0B = Abs( vSample_08_09_0A_0B );
			const Vector4 vAbsSample_0C_0D_0E_0F = Abs( vSample_0C_0D_0E_0F );
			
			//const F32 fDrive = fPrevDrive + fFrameIndex * fDriveDelta;
			const Vector4 vDrive_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vDriveDelta, vPrevDrive );
			const Vector4 vDrive_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vDriveDelta, vPrevDrive );
			const Vector4 vDrive_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vDriveDelta, vPrevDrive );
			const Vector4 vDrive_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vDriveDelta, vPrevDrive );
			
			//const F32 fClip = fPrevClip + fFrameIndex * fClipDelta;
			const Vector4 vClip_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vClipDelta, vPrevClip );
			const Vector4 vClip_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vClipDelta, vPrevClip );
			const Vector4 vClip_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vClipDelta, vPrevClip );
			const Vector4 vClip_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vClipDelta, vPrevClip );
			
			//const F32 fLevel = fPrevLevel + fFrameIndex * fLevelDelta;
			const Vector4 vLevel_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vLevelDelta, vPrevLevel );
			const Vector4 vLevel_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vLevelDelta, vPrevLevel );
			const Vector4 vLevel_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vLevelDelta, vPrevLevel );
			const Vector4 vLevel_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vLevelDelta, vPrevLevel );
			
			//fFrameIndex += fOne;
            vFrameIndex_00_01_02_03 += vSixteen;
            vFrameIndex_04_05_06_07 += vSixteen;
            vFrameIndex_08_09_0A_0B += vSixteen;
            vFrameIndex_0C_0D_0E_0F += vSixteen;
            
			//const F32 fIn = fDrive * Abs( fSample );
			const Vector4 vIn_00_01_02_03 = vDrive_00_01_02_03 * vAbsSample_00_01_02_03;
			const Vector4 vIn_04_05_06_07 = vDrive_04_05_06_07 * vAbsSample_04_05_06_07;
			const Vector4 vIn_08_09_0A_0B = vDrive_08_09_0A_0B * vAbsSample_08_09_0A_0B;
			const Vector4 vIn_0C_0D_0E_0F = vDrive_0C_0D_0E_0F * vAbsSample_0C_0D_0E_0F;
			
            const Vector4 vOneMinusClip_00_01_02_03 = vOne - vClip_00_01_02_03;
            const Vector4 vOneMinusClip_04_05_06_07 = vOne - vClip_04_05_06_07;
            const Vector4 vOneMinusClip_08_09_0A_0B = vOne - vClip_08_09_0A_0B;
            const Vector4 vOneMinusClip_0C_0D_0E_0F = vOne - vClip_0C_0D_0E_0F;
			
			//const F32 fSgn = Select( fSample, fOne, fMinusOne );
			const Vector4 vSignSample_00_01_02_03 = Sign( vSample_00_01_02_03 );
			const Vector4 vSignSample_04_05_06_07 = Sign( vSample_04_05_06_07 );
			const Vector4 vSignSample_08_09_0A_0B = Sign( vSample_08_09_0A_0B );
			const Vector4 vSignSample_0C_0D_0E_0F = Sign( vSample_0C_0D_0E_0F );
			
			// RcpEst( fIn + ( fOne - fClip ) )
			//const Vector4 vSatRatio_00_01_02_03 = RcpEst( vIn_00_01_02_03 + vOneMinusClip_00_01_02_03 );
			//const Vector4 vSatRatio_04_05_06_07 = RcpEst( vIn_04_05_06_07 + vOneMinusClip_04_05_06_07 );
			//const Vector4 vSatRatio_08_09_0A_0B = RcpEst( vIn_08_09_0A_0B + vOneMinusClip_08_09_0A_0B );
			//const Vector4 vSatRatio_0C_0D_0E_0F = RcpEst( vIn_0C_0D_0E_0F + vOneMinusClip_0C_0D_0E_0F );
			const Vector4 vTemp_00_01_02_03 = MulAdd( vDrive_00_01_02_03, vAbsSample_00_01_02_03, vOneMinusClip_00_01_02_03 );
			const Vector4 vTemp_04_05_06_07 = MulAdd( vDrive_04_05_06_07, vAbsSample_04_05_06_07, vOneMinusClip_04_05_06_07 );
			const Vector4 vTemp_08_09_0A_0B = MulAdd( vDrive_08_09_0A_0B, vAbsSample_08_09_0A_0B, vOneMinusClip_08_09_0A_0B );
			const Vector4 vTemp_0C_0D_0E_0F = MulAdd( vDrive_0C_0D_0E_0F, vAbsSample_0C_0D_0E_0F, vOneMinusClip_0C_0D_0E_0F );
			const Vector4 vSatRatio_00_01_02_03 = RcpEst( vTemp_00_01_02_03 );
			const Vector4 vSatRatio_04_05_06_07 = RcpEst( vTemp_04_05_06_07 );
			const Vector4 vSatRatio_08_09_0A_0B = RcpEst( vTemp_08_09_0A_0B );
			const Vector4 vSatRatio_0C_0D_0E_0F = RcpEst( vTemp_0C_0D_0E_0F );
			
			const Vector4 vSaturated_00_01_02_03 = vIn_00_01_02_03 * vSatRatio_00_01_02_03;
			const Vector4 vSaturated_04_05_06_07 = vIn_04_05_06_07 * vSatRatio_04_05_06_07;
			const Vector4 vSaturated_08_09_0A_0B = vIn_08_09_0A_0B * vSatRatio_08_09_0A_0B;
			const Vector4 vSaturated_0C_0D_0E_0F = vIn_0C_0D_0E_0F * vSatRatio_0C_0D_0E_0F;
			
			const Vector4 vLevel_x_Sign_00_01_02_03 = vLevel_00_01_02_03 * vSignSample_00_01_02_03;
			const Vector4 vLevel_x_Sign_04_05_06_07 = vLevel_04_05_06_07 * vSignSample_04_05_06_07;
			const Vector4 vLevel_x_Sign_08_09_0A_0B = vLevel_08_09_0A_0B * vSignSample_08_09_0A_0B;
			const Vector4 vLevel_x_Sign_0C_0D_0E_0F = vLevel_0C_0D_0E_0F * vSignSample_0C_0D_0E_0F;
			
			//const F32 fOut = Select( fClip - fIn, fIn, fIn * RcpEst( fOne + fIn - fClip ) );
			const Vector4 vOut_00_01_02_03 = Select_GE( vClip_00_01_02_03, vIn_00_01_02_03, vIn_00_01_02_03, vSaturated_00_01_02_03 );
			const Vector4 vOut_04_05_06_07 = Select_GE( vClip_04_05_06_07, vIn_04_05_06_07, vIn_04_05_06_07, vSaturated_04_05_06_07 );
			const Vector4 vOut_08_09_0A_0B = Select_GE( vClip_08_09_0A_0B, vIn_08_09_0A_0B, vIn_08_09_0A_0B, vSaturated_08_09_0A_0B );
			const Vector4 vOut_0C_0D_0E_0F = Select_GE( vClip_0C_0D_0E_0F, vIn_0C_0D_0E_0F, vIn_0C_0D_0E_0F, vSaturated_0C_0D_0E_0F );
            
            //aSample[uSampleIndex++] = fLevel * fSgn * fOut;
            aSample[ uQuadSampleIndex + 0 ] = vLevel_x_Sign_00_01_02_03 * vOut_00_01_02_03;
            aSample[ uQuadSampleIndex + 1 ] = vLevel_x_Sign_04_05_06_07 * vOut_04_05_06_07;
            aSample[ uQuadSampleIndex + 2 ] = vLevel_x_Sign_08_09_0A_0B * vOut_08_09_0A_0B;
            aSample[ uQuadSampleIndex + 3 ] = vLevel_x_Sign_0C_0D_0E_0F * vOut_0C_0D_0E_0F;
            
            uQuadSampleIndex += 4;
        }
    }
    
    #endif
    
	m_fPrevDrive = m_fDrive;
	m_fPrevClip  = m_fClip;
	m_fPrevLevel = m_fLevel;
	
    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
