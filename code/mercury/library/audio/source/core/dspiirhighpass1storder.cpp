#include "AudioPch.h"
#include "coreheaders.h"



#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRHighPass1stOrder::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRHighPass1stOrder
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRHighPass1stOrder::IIRHighPass1stOrder( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
    using namespace Audio::Maths;
    
	m_eType = Core::DSP::Effect::eIIRHighPass1stOrder;

    m_fPrevCutoff = -1.f;
    m_fCutoff = fSampleRate * 0.5f;
    m_fGain = 1.f;
    
    #if defined( AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1 ) // JASOND
    const Vector4 v4_zero = MakeVector4( g_VectorZero );
    #endif
    
    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1 ) // JASOND
        m_fIn1[uCh] = 0.f;
        m_fOut1[uCh] = 0.f;
        #else
        m_vIn_00_01_02_03[uCh] = v4_zero;
        m_vOut_00_01_02_03[uCh] = v4_zero;
        #endif
    }
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRHighPass1stOrder::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.IIRHighPass1stOrder.Process");

#ifndef AUDIO_DISABLE_DSP

	//	limit freq slew rate
	F32 fCutoffDelta = (m_fTargetCutoff - m_fCutoff);
	F32 fStep = m_fCutoff * fFrameLength * 0.001953125f; // 1/512 - 1oct per 256 samp frame
	fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
	m_fCutoff += fCutoffDelta;

	if (m_fCutoff != m_fPrevCutoff)
    {
		m_fB = Maths::ArcTan(Maths::fPi * m_fCutoff * fRcpSampleRate);
		m_fB = -(m_fB - 1.f) * Maths::Rcp(1.f + m_fB);
        m_fA = 0.5f * (1.f + m_fB);

        m_fPrevCutoff = m_fCutoff;
    }

    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

	if( !m_bActive || bDisableAll )
	{
		for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
		{
			#if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1 )
			m_fIn1[ uCh ] = aSample[ uCh * uFrameLength + uFrameLength - 1 ];
			m_fOut1[ uCh ] = 0.f;
			#else
			m_vIn_00_01_02_03[ uCh ] = Maths::LoadAligned( &aSample[ uCh * uFrameLength + uFrameLength - 4 ] );
			m_vOut_00_01_02_03[ uCh ] = Maths::MakeVector4( Maths::g_VectorZero );
			#endif
		}

		return true;
	}

    #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1 ) // JASOND
    
    //  each channel
    for (U32 uCh = 0; uCh < m_uChannels; uCh++)
    {
        F32 fIn1 = m_fIn1[uCh];
        F32 fOut1 = m_fOut1[uCh];

        //  each sample
        for (U32 uSample = 0; uSample < uFrameLength; uSample++)
        {
            const S32 i = uCh * uFrameLength + uSample;
            
            const F32 fIn0  = aSample[i];
            
#ifdef PLATFORM_PC
			static const F32 fTinyVal = 1e-18;
            const F32 fOut0 = m_fA * ( fIn0 - fIn1 ) + m_fB * fOut1 + fTinyVal;
#else
            const F32 fOut0 = m_fA * ( fIn0 - fIn1 ) + m_fB * fOut1;
#endif
			aSample[i] = m_fGain * fOut0;
            
            fIn1  = fIn0;
            fOut1 = fOut0;
        }

        //  store previous samples
        m_fIn1[uCh] = fIn1;
        m_fOut1[uCh] = fOut1;
    }
    
    #else
    
    using namespace Audio::Maths;
    
    Vector4* pBuffer = (Vector4*)pFrameData;
    
    const Scalar vGain = _Scalar( m_fGain );
    
    const Scalar vB1 = _Scalar( m_fB );
    const Scalar vB2 = vB1 * vB1;
    const Scalar vB3 = vB1 * vB2;
    const Scalar vB4 = vB2 * vB2;
    
    const Scalar vA = _Scalar( m_fA );
    const Scalar vA_x_vB1 = vA * vB1;
    const Scalar vA_x_vB2 = vA * vB2;
    const Scalar vA_x_vB3 = vA * vB3;
    
    const Vector4 v4_zero = MakeVector4( g_VectorZero );
    
    Vector4 vIn_00_01_02_03; // = v4_zero;
    Vector4 vIn_01_02_03_04; // = v4_zero;
    Vector4 vIn_02_03_04_05; // = v4_zero;
    Vector4 vIn_03_04_05_06; // = v4_zero;
    Vector4 vIn_04_05_06_07; // = v4_zero;
    Vector4 vIn_05_06_07_08; // = v4_zero;
    Vector4 vIn_06_07_08_09; // = v4_zero;
    Vector4 vIn_07_08_09_0A; // = v4_zero;
    Vector4 vIn_08_09_0A_0B; // = v4_zero;
    Vector4 vIn_09_0A_0B_0C; // = v4_zero;
    Vector4 vIn_0A_0B_0C_0D; // = v4_zero;
    Vector4 vIn_0B_0C_0D_0E; // = v4_zero;
    Vector4 vIn_0C_0D_0E_0F; // = v4_zero;
    Vector4 vIn_0D_0E_0F_10; // = v4_zero;
    Vector4 vIn_0E_0F_10_11; // = v4_zero;
    Vector4 vIn_0F_10_11_12; // = v4_zero;
    Vector4 vIn_10_11_12_13; // = v4_zero;
    
    Vector4 vOut_00_01_02_03; // = v4_zero;
    Vector4 vOut_04_05_06_07; // = v4_zero;
    Vector4 vOut_08_09_0A_0B; // = v4_zero;
    Vector4 vOut_0C_0D_0E_0F; // = v4_zero;
    Vector4 vOut_10_11_12_13; // = v4_zero;
    
    #if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
    const Vector4 v_AB3_AB2_AB1_A = MakeVector4( vA_x_vB3, vA_x_vB2, vA_x_vB1, vA );
    #endif
    
    //  each channel
    U32 uQuadSampleIndex = 0;
    for ( U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
         vIn_10_11_12_13 =  m_vIn_00_01_02_03[uCh];
        vOut_10_11_12_13 = m_vOut_00_01_02_03[uCh];
        
        for ( U32 uSampleIndex=0; uSampleIndex < uFrameLength; uSampleIndex += 16 )
        {
             vIn_00_01_02_03 =  vIn_10_11_12_13;
            vOut_00_01_02_03 = vOut_10_11_12_13;
            
            vIn_04_05_06_07 = pBuffer[ uQuadSampleIndex + 0 ];
            vIn_08_09_0A_0B = pBuffer[ uQuadSampleIndex + 1 ];
            vIn_0C_0D_0E_0F = pBuffer[ uQuadSampleIndex + 2 ];
            vIn_10_11_12_13 = pBuffer[ uQuadSampleIndex + 3 ];
            
            vIn_01_02_03_04 = RotateLeft<1>( vIn_00_01_02_03, vIn_04_05_06_07 );
            vIn_02_03_04_05 = RotateLeft<2>( vIn_00_01_02_03, vIn_04_05_06_07 );
            vIn_03_04_05_06 = RotateLeft<3>( vIn_00_01_02_03, vIn_04_05_06_07 );
            
            vIn_05_06_07_08 = RotateLeft<1>( vIn_04_05_06_07, vIn_08_09_0A_0B );
            vIn_06_07_08_09 = RotateLeft<2>( vIn_04_05_06_07, vIn_08_09_0A_0B );
            vIn_07_08_09_0A = RotateLeft<3>( vIn_04_05_06_07, vIn_08_09_0A_0B );
            
            vIn_09_0A_0B_0C = RotateLeft<1>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
            vIn_0A_0B_0C_0D = RotateLeft<2>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
            vIn_0B_0C_0D_0E = RotateLeft<3>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
            
            vIn_0D_0E_0F_10 = RotateLeft<1>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
            vIn_0E_0F_10_11 = RotateLeft<2>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
            vIn_0F_10_11_12 = RotateLeft<3>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
            
            const Vector4 vDif_00_01_02_03 = vIn_00_01_02_03 - vIn_01_02_03_04;
            const Vector4 vDif_01_02_03_04 = vIn_01_02_03_04 - vIn_02_03_04_05;
            const Vector4 vDif_02_03_04_05 = vIn_02_03_04_05 - vIn_03_04_05_06;
            const Vector4 vDif_03_04_05_06 = vIn_03_04_05_06 - vIn_04_05_06_07;
            const Vector4 vDif_04_05_06_07 = vIn_04_05_06_07 - vIn_05_06_07_08;
            const Vector4 vDif_05_06_07_08 = vIn_05_06_07_08 - vIn_06_07_08_09;
            const Vector4 vDif_06_07_08_09 = vIn_06_07_08_09 - vIn_07_08_09_0A;
            const Vector4 vDif_07_08_09_0A = vIn_07_08_09_0A - vIn_08_09_0A_0B;
            const Vector4 vDif_08_09_0A_0B = vIn_08_09_0A_0B - vIn_09_0A_0B_0C;
            const Vector4 vDif_09_0A_0B_0C = vIn_09_0A_0B_0C - vIn_0A_0B_0C_0D;
            const Vector4 vDif_0A_0B_0C_0D = vIn_0A_0B_0C_0D - vIn_0B_0C_0D_0E;
            const Vector4 vDif_0B_0C_0D_0E = vIn_0B_0C_0D_0E - vIn_0C_0D_0E_0F;
            const Vector4 vDif_0C_0D_0E_0F = vIn_0C_0D_0E_0F - vIn_0D_0E_0F_10;
            const Vector4 vDif_0D_0E_0F_10 = vIn_0D_0E_0F_10 - vIn_0E_0F_10_11;
            const Vector4 vDif_0E_0F_10_11 = vIn_0E_0F_10_11 - vIn_0F_10_11_12;
            const Vector4 vDif_0F_10_11_12 = vIn_0F_10_11_12 - vIn_10_11_12_13;
            
            #if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
            
            vOut_04_05_06_07 = MakeVector4( v_AB3_AB2_AB1_A % vDif_00_01_02_03,
                                            v_AB3_AB2_AB1_A % vDif_01_02_03_04,
                                            v_AB3_AB2_AB1_A % vDif_02_03_04_05, 
                                            v_AB3_AB2_AB1_A % vDif_03_04_05_06 ) +
                               vB4 * vOut_00_01_02_03;
            
            vOut_08_09_0A_0B = MakeVector4( v_AB3_AB2_AB1_A % vDif_04_05_06_07,
                                            v_AB3_AB2_AB1_A % vDif_05_06_07_08,
                                            v_AB3_AB2_AB1_A % vDif_06_07_08_09,
                                            v_AB3_AB2_AB1_A % vDif_07_08_09_0A ) +
                               vB4 * vOut_04_05_06_07;
            
            vOut_0C_0D_0E_0F = MakeVector4( v_AB3_AB2_AB1_A % vDif_08_09_0A_0B,
                                            v_AB3_AB2_AB1_A % vDif_09_0A_0B_0C,
                                            v_AB3_AB2_AB1_A % vDif_0A_0B_0C_0D,
                                            v_AB3_AB2_AB1_A % vDif_0B_0C_0D_0E ) +
                               vB4 * vOut_08_09_0A_0B;
            
            vOut_10_11_12_13 = MakeVector4( v_AB3_AB2_AB1_A % vDif_0C_0D_0E_0F,
                                            v_AB3_AB2_AB1_A % vDif_0D_0E_0F_10,
                                            v_AB3_AB2_AB1_A % vDif_0E_0F_10_11,
                                            v_AB3_AB2_AB1_A % vDif_0F_10_11_12 ) +
                               vB4 * vOut_0C_0D_0E_0F;
            
            #else
            
            vOut_04_05_06_07 = ( ( vA       * vDif_03_04_05_06 +
                                   vA_x_vB1 * vDif_02_03_04_05 ) +
                                 ( vA_x_vB2 * vDif_01_02_03_04 +
                                   vA_x_vB3 * vDif_00_01_02_03 ) ) +
                                        vB4 * vOut_00_01_02_03;
            
            vOut_08_09_0A_0B = ( ( vA       * vDif_07_08_09_0A +
                                   vA_x_vB1 * vDif_06_07_08_09 ) +
                                 ( vA_x_vB2 * vDif_05_06_07_08 +
                                   vA_x_vB3 * vDif_04_05_06_07 ) ) +
                                        vB4 * vOut_04_05_06_07;
            
            vOut_0C_0D_0E_0F = ( ( vA       * vDif_0B_0C_0D_0E +
                                   vA_x_vB1 * vDif_0A_0B_0C_0D ) +
                                 ( vA_x_vB2 * vDif_09_0A_0B_0C +
                                   vA_x_vB3 * vDif_08_09_0A_0B ) ) +
                                        vB4 * vOut_08_09_0A_0B;
            
            vOut_10_11_12_13 = ( ( vA       * vDif_0F_10_11_12 +
                                   vA_x_vB1 * vDif_0E_0F_10_11 ) +
                                 ( vA_x_vB2 * vDif_0D_0E_0F_10 +
                                   vA_x_vB3 * vDif_0C_0D_0E_0F ) ) +
                                        vB4 * vOut_0C_0D_0E_0F;
            
            #endif
            
            pBuffer[ uQuadSampleIndex + 0 ] = vOut_04_05_06_07 * vGain; // TODO: Pre-multiply by gain for all but vOut_10_11_12_13
            pBuffer[ uQuadSampleIndex + 1 ] = vOut_08_09_0A_0B * vGain;
            pBuffer[ uQuadSampleIndex + 2 ] = vOut_0C_0D_0E_0F * vGain;
            pBuffer[ uQuadSampleIndex + 3 ] = vOut_10_11_12_13 * vGain;
            uQuadSampleIndex += 4;
        }
        
         m_vIn_00_01_02_03[uCh] =  vIn_10_11_12_13;
        m_vOut_00_01_02_03[uCh] = vOut_10_11_12_13;
    }
            
    #endif

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
