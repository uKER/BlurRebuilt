#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowPass1stOrder::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRLowPass1stOrder
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRLowPass1stOrder::IIRLowPass1stOrder( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	using namespace Audio::Maths;
    
	m_eType = Core::DSP::Effect::eIIRLowPass1stOrder;

	m_fTargetCutoff = fSampleRate * 0.5f;
    m_fPrevCutoff = 0.f;
    m_fCutoff = fSampleRate * 0.5f;
    m_fGain = 1.f;
    
    #if defined( AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1 ) // JASOND
    const Vector4 v4_zero = MakeVector4( g_VectorZero );
    #endif
    
    for ( U32 uCh = 0; uCh < uChannels; uCh++ )
    {
        #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1 ) // JASOND
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
bool Audio::Core::DSP::IIRLowPass1stOrder::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.IIRLowPass1stOrder.Process");

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
        m_fA = 0.5f * (1.f - m_fB);

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

    #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1 ) // Version 0 : Original // JASOND
    
    //  each channel
    for ( U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
        F32 fIn1  = m_fIn1[uCh];
        F32 fOut1 = m_fOut1[uCh];
        
        //  each sample
        for ( U32 uSample = 0; uSample < uFrameLength; uSample++ )
        {
            const S32 i = uCh * uFrameLength + uSample;
            
            const F32 fIn0  = aSample[i];
            
#ifdef PLATFORM_PC
			static const F32 fTinyVal = 1e-18;
            const F32 fOut0 = m_fA * ( fIn0 + fIn1 ) + m_fB * fOut1 + fTinyVal;
#else
            const F32 fOut0 = m_fA * ( fIn0 + fIn1 ) + m_fB * fOut1;
#endif
			aSample[i] = m_fGain * fOut0;
            
            fIn1  = fIn0;
            fOut1 = fOut0;
        }

#ifdef AUDIO_CHECK_SAMPLES

		if (fabs(m_fIn1[uCh]) > 100.f || !_finite(m_fIn1[uCh]))
			AUDIO_CORE_ERROR(DBSTR" Invalid sample in effect '%s'", m_pchInfo);

		if (fabs(m_fOut1[uCh]) > 100.f || !_finite(m_fOut1[uCh]))
			AUDIO_CORE_ERROR(DBSTR" Invalid sample in effect '%s'", m_pchInfo);
#endif // AUDIO_CHECK_SAMPLES        

        //  store previous samples
        m_fIn1[uCh] = fIn1;
        m_fOut1[uCh] = fOut1;
    }
    
    #elif 0 // Version 1 : Expanded & Unrolled 4 times
    
    const F32 fA( m_fA );
    
    const F32 fB1( m_fB );
    const F32 fB2( fB1 * fB1 );
    const F32 fB3( fB1 * fB2 );
    const F32 fB4( fB2 * fB2 );
    
    const F32 fGain( m_fGain );
    
    //  each channel
    S32 i = 0;
    for ( U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
        F32 fIn0  = m_fIn0[uCh];
        F32 fIn1  = m_fIn1[uCh];
        F32 fIn2  = m_fIn2[uCh];
        F32 fIn3  = m_fIn3[uCh];
        F32 fIn4;
        F32 fIn5;
        F32 fIn6;
        F32 fIn7;
        
        F32 fOut0 = m_fOut0[uCh];
        F32 fOut1 = m_fOut1[uCh];
        F32 fOut2 = m_fOut2[uCh];
        F32 fOut3 = m_fOut3[uCh];
        F32 fOut4;
        F32 fOut5;
        F32 fOut6;
        F32 fOut7;
        
        F32 fA_x_fIn0_plus_fIn1 = fA * ( fIn0 + fIn1 );
        F32 fA_x_fIn1_plus_fIn2 = fA * ( fIn1 + fIn2 );
        F32 fA_x_fIn2_plus_fIn3 = fA * ( fIn2 + fIn3 );
        F32 fA_x_fIn3_plus_fIn4;
        F32 fA_x_fIn4_plus_fIn5;
        F32 fA_x_fIn5_plus_fIn6;
        F32 fA_x_fIn6_plus_fIn7;
        
        //  each sample
        for ( U32 uSample = 0; uSample < uFrameLength; uSample+=4, i+=4 )
        {
            //const F32 fIn4 = aSample[i+0];
            //const F32 fIn5 = aSample[i+1];
            //const F32 fIn6 = aSample[i+2];
            //const F32 fIn7 = aSample[i+3];
            fIn4 = aSample[i+0];
            fIn5 = aSample[i+1];
            fIn6 = aSample[i+2];
            fIn7 = aSample[i+3];
            
            //fA_x_fIn0_plus_fIn1 = fA * ( fIn0 + fIn1 ); // Same as fA_x_fIn4_plus_fIn5 from previous loop!
            //fA_x_fIn1_plus_fIn2 = fA * ( fIn1 + fIn2 ); // Same as fA_x_fIn5_plus_fIn6 from previous loop!
            //fA_x_fIn2_plus_fIn3 = fA * ( fIn2 + fIn3 ); // Same as fA_x_fIn6_plus_fIn7 from previous loop!
            fA_x_fIn3_plus_fIn4 = fA * ( fIn3 + fIn4 );
            fA_x_fIn4_plus_fIn5 = fA * ( fIn4 + fIn5 );
            fA_x_fIn5_plus_fIn6 = fA * ( fIn5 + fIn6 );
            fA_x_fIn6_plus_fIn7 = fA * ( fIn6 + fIn7 );
            
            // const F32 fOut4 = fA * ( fIn3 + fIn4 ) +
            //                   fB * ( fA * ( fIn2 + fIn3 ) +
            //                          fB * ( fA * ( fIn1 + fIn2 ) +
            //                                 fB * ( fA * ( fIn0 + fIn1 ) +
            //                                        fB * fOut0 ) ) );
			
            //const F32 fOut4 = fB4 * fOut0
            //                + fB3 * fA_x_fIn0_plus_fIn1
            //                + fB2 * fA_x_fIn1_plus_fIn2
            //                + fB1 * fA_x_fIn2_plus_fIn3
            //                +       fA_x_fIn3_plus_fIn4;
			//
            //const F32 fOut5 = fB4 * fOut1
            //                + fB3 * fA_x_fIn1_plus_fIn2
            //                + fB2 * fA_x_fIn2_plus_fIn3
            //                + fB1 * fA_x_fIn3_plus_fIn4
            //                +       fA_x_fIn4_plus_fIn5;
            //
            //const F32 fOut6 = fB4 * fOut2
            //                + fB3 * fA_x_fIn2_plus_fIn3
            //                + fB2 * fA_x_fIn3_plus_fIn4
            //                + fB1 * fA_x_fIn4_plus_fIn5
            //                +       fA_x_fIn5_plus_fIn6;
            //
            //const F32 fOut7 = fB4 * fOut3
            //                + fB3 * fA_x_fIn3_plus_fIn4
            //                + fB2 * fA_x_fIn4_plus_fIn5
            //                + fB1 * fA_x_fIn5_plus_fIn6
            //                +       fA_x_fIn6_plus_fIn7;
            
            fOut4 = fA_x_fIn3_plus_fIn4;
            fOut5 = fA_x_fIn4_plus_fIn5;
            fOut6 = fA_x_fIn5_plus_fIn6;
            fOut7 = fA_x_fIn6_plus_fIn7;
            
            fOut4 += fB1 * fA_x_fIn2_plus_fIn3;
            fOut5 += fB1 * fA_x_fIn3_plus_fIn4;
            fOut6 += fB1 * fA_x_fIn4_plus_fIn5;
            fOut7 += fB1 * fA_x_fIn5_plus_fIn6;
            
            fOut4 += fB2 * fA_x_fIn1_plus_fIn2;
            fOut5 += fB2 * fA_x_fIn2_plus_fIn3;
            fOut6 += fB2 * fA_x_fIn3_plus_fIn4;
            fOut7 += fB2 * fA_x_fIn4_plus_fIn5;
            
            fOut4 += fB3 * fA_x_fIn0_plus_fIn1;
            fOut5 += fB3 * fA_x_fIn1_plus_fIn2;
            fOut6 += fB3 * fA_x_fIn2_plus_fIn3;
            fOut7 += fB3 * fA_x_fIn3_plus_fIn4;
            
            fOut4 += fB4 * fOut0;
            fOut5 += fB4 * fOut1;
            fOut6 += fB4 * fOut2;
            fOut7 += fB4 * fOut3;
			
			aSample[i+0] = fGain * fOut4;
			aSample[i+1] = fGain * fOut5;
			aSample[i+2] = fGain * fOut6;
			aSample[i+3] = fGain * fOut7;
            
            fA_x_fIn0_plus_fIn1 = fA_x_fIn4_plus_fIn5;
            fA_x_fIn1_plus_fIn2 = fA_x_fIn5_plus_fIn6;
            fA_x_fIn2_plus_fIn3 = fA_x_fIn6_plus_fIn7;
            //fIn0 = fIn4; // Not needed in loop but MUST be accounted for before storing previous samples after loop!
            //fIn1 = fIn5;
            //fIn2 = fIn6;
            fIn3 = fIn7; // This one IS needed for calculating fA_x_fIn3_plus_fIn4 in next iteration of loop
            
            fOut0 = fOut4;
            fOut1 = fOut5;
            fOut2 = fOut6;
            fOut3 = fOut7;
        }
        
        fIn0 = fIn4; // Account for optimisation AFTER loop!
        fIn1 = fIn5;
        fIn2 = fIn6;
        
        //  store previous samples
        
        m_fIn0[uCh] = fIn0;
        m_fIn1[uCh] = fIn1;
        m_fIn2[uCh] = fIn2;
        m_fIn3[uCh] = fIn3;
        
        m_fOut0[uCh] = fOut0;
        m_fOut1[uCh] = fOut1;
        m_fOut2[uCh] = fOut2;
        m_fOut3[uCh] = fOut3;
    }
    
    #elif 0 // Version 2 : Expanded, Unrolled 8 times & Vectorised
    
    using namespace Audio::Maths;
    
    const Vector4 fA( MakeVector4( m_fA ) );
    
    const Vector4 fB1( MakeVector4( m_fB ) );
    const Vector4 fB2( fB1 * fB1 );
    const Vector4 fB3( fB1 * fB2 );
    const Vector4 fB4( fB2 * fB2 );
    const Vector4 fB5( fB2 * fB3 );
    const Vector4 fB6( fB3 * fB3 );
    const Vector4 fB7( fB3 * fB4 );
    const Vector4 fB8( fB4 * fB4 );
    
    const Vector4 fGain( MakeVector4( m_fGain ) );
    
    //  each channel
    S32 i = 0;
    for ( U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
        Vector4 fIn0123( m_fIn0123[uCh] );
        Vector4 fIn4567( m_fIn4567[uCh] );
        
        Vector4 fOut0123( m_fOut0123[uCh] );
        Vector4 fOut4567( m_fOut4567[uCh] );
        
        //  each sample
        for ( U32 uSample = 0; uSample < uFrameLength; uSample+=8, i+=8 )
        {
            const Vector4 fIn89AB( LoadAligned( &aSample[i+0] ) );
            const Vector4 fInCDEF( LoadAligned( &aSample[i+4] ) );
            
            
            const Vector4 fA_x_fIn0123_plus_fIn1234( fA * ( fIn0123 + RotateLeft<1>( fIn0123, fIn4567 ) ) );
            const Vector4 fA_x_fIn4567_plus_fIn5678( fA * ( fIn4567 + RotateLeft<1>( fIn4567, fIn89AB ) ) );
            const Vector4 fA_x_fIn89AB_plus_fIn9ABC( fA * ( fIn89AB + RotateLeft<1>( fIn89AB, fInCDEF ) ) );
            const Vector4 fA_x_fInCDEF_plus_fInDEF0( fA * ( fInCDEF + RotateLeft<1>( fInCDEF, fIn0123 ) ) );
            
            const Vector4 fA_x_fInBCDE_plus_fInCDEF( RotateLeft<3>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
            const Vector4 fA_x_fInABCD_plus_fInBCDE( RotateLeft<2>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
            const Vector4 fA_x_fIn9ABC_plus_fInABCD( RotateLeft<1>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
            
            const Vector4 fA_x_fIn789A_plus_fIn89AB( RotateLeft<3>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
            const Vector4 fA_x_fIn6789_plus_fIn789A( RotateLeft<2>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
            const Vector4 fA_x_fIn5678_plus_fIn6789( RotateLeft<1>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
            
            const Vector4 fA_x_fIn3456_plus_fIn4567( RotateLeft<3>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
            const Vector4 fA_x_fIn2345_plus_fIn3456( RotateLeft<2>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
            const Vector4 fA_x_fIn1234_plus_fIn2345( RotateLeft<1>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
            
            Vector4 fOut89AB = fA_x_fIn789A_plus_fIn89AB +
                   ( ( ( fB1 * fA_x_fIn6789_plus_fIn789A +
                         fB2 * fA_x_fIn5678_plus_fIn6789 ) +
                       ( fB3 * fA_x_fIn4567_plus_fIn5678 +
                         fB4 * fA_x_fIn3456_plus_fIn4567 ) ) +
                     ( ( fB5 * fA_x_fIn2345_plus_fIn3456 +
                         fB6 * fA_x_fIn1234_plus_fIn2345 ) +
                       ( fB7 * fA_x_fIn0123_plus_fIn1234 +
                         fB8 * fOut0123 ) ) );
            
            Vector4 fOutCDEF = fA_x_fInBCDE_plus_fInCDEF +
                   ( ( ( fB1 * fA_x_fInABCD_plus_fInBCDE +
                         fB2 * fA_x_fIn9ABC_plus_fInABCD ) +
                       ( fB3 * fA_x_fIn89AB_plus_fIn9ABC +
                         fB4 * fA_x_fIn789A_plus_fIn89AB ) ) +
                     ( ( fB5 * fA_x_fIn6789_plus_fIn789A +
                         fB6 * fA_x_fIn5678_plus_fIn6789 ) +
                       ( fB7 * fA_x_fIn4567_plus_fIn5678 +
                         fB8 * fOut4567 ) ) );
            
            StoreAligned( fGain * fOut89AB, &aSample[i+0] );
            StoreAligned( fGain * fOutCDEF, &aSample[i+4] );
            
            fIn0123 = fIn89AB;
            fIn4567 = fInCDEF;
            
            fOut0123 = fOut89AB;
            fOut4567 = fOutCDEF;
        }
        
        //  store previous samples
        
        m_fIn0123[uCh] = fIn0123;
        m_fIn4567[uCh] = fIn4567;
        
        m_fOut0123[uCh] = fOut0123;
        m_fOut4567[uCh] = fOut4567;
    }
    
    #elif 0 // Version 3 : Expanded, Unrolled 16 times & Vectorised
    
    using namespace Audio::Maths;
    
    const Vector4 fGain( MakeVector4( m_fGain ) );
    
    const Vector4 fA( MakeVector4( m_fA ) );
    
    const Vector4 fB01( MakeVector4( m_fB ) );
    const Vector4 fB02( fB01 * fB01 );
    const Vector4 fB03( fB01 * fB02 );
    const Vector4 fB04( fB02 * fB02 );
    const Vector4 fB05( fB02 * fB03 );
    const Vector4 fB06( fB03 * fB03 );
    const Vector4 fB07( fB03 * fB04 );
    const Vector4 fB08( fB04 * fB04 );
    const Vector4 fB09( fB04 * fB05 );
    const Vector4 fB10( fB05 * fB05 );
    const Vector4 fB11( fB05 * fB06 );
    const Vector4 fB12( fB06 * fB06 );
    const Vector4 fB13( fB06 * fB07 );
    const Vector4 fB14( fB07 * fB07 );
    const Vector4 fB15( fB07 * fB08 );
    const Vector4 fB16( fB08 * fB08 );
    
    Vector4 fIn_00_01_02_03;
    Vector4 fIn_04_05_06_07;
    Vector4 fIn_08_09_0A_0B;
    Vector4 fIn_0C_0D_0E_0F;
    Vector4 fIn_10_11_12_13;
    Vector4 fIn_14_15_16_17;
    Vector4 fIn_18_19_1A_1B;
    Vector4 fIn_1C_1D_1E_1F;
    
    Vector4 fB16_x_fOut_00_01_02_03;
    Vector4 fB16_x_fOut_04_05_06_07;
    Vector4 fB16_x_fOut_08_09_0A_0B;
    Vector4 fB16_x_fOut_0C_0D_0E_0F;
    Vector4 fOut_10_11_12_13;
    Vector4 fOut_14_15_16_17;
    Vector4 fOut_18_19_1A_1B;
    Vector4 fOut_1C_1D_1E_1F;
    
    Vector4 fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04;
    Vector4 fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05;
    Vector4 fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06;
    Vector4 fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07;
    Vector4 fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08;
    Vector4 fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09;
    Vector4 fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A;
    Vector4 fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B;
    Vector4 fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C;
    Vector4 fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D;
    Vector4 fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E;
    Vector4 fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F;
    Vector4 fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10;
    Vector4 fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11;
    Vector4 fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12;
    Vector4 fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13;
    Vector4 fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14;
    Vector4 fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15;
    Vector4 fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16;
    Vector4 fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17;
    Vector4 fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18;
    Vector4 fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19;
    Vector4 fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A;
    Vector4 fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B;
    Vector4 fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C;
    Vector4 fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D;
    Vector4 fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E;
    Vector4 fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F;
    Vector4 fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX;
    
    //  each channel
    S32 i = 0;
    for ( U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
        fIn_00_01_02_03 = m_fIn_00_01_02_03[uCh];
        fIn_04_05_06_07 = m_fIn_04_05_06_07[uCh];
        fIn_08_09_0A_0B = m_fIn_08_09_0A_0B[uCh];
        fIn_0C_0D_0E_0F = m_fIn_0C_0D_0E_0F[uCh];
        
        fB16_x_fOut_00_01_02_03 = m_fB16_x_fOut_00_01_02_03[uCh];
        fB16_x_fOut_04_05_06_07 = m_fB16_x_fOut_04_05_06_07[uCh];
        fB16_x_fOut_08_09_0A_0B = m_fB16_x_fOut_08_09_0A_0B[uCh];
        fB16_x_fOut_0C_0D_0E_0F = m_fB16_x_fOut_0C_0D_0E_0F[uCh];
        
        // -----------------------------------------------------------------------------------------------------------------------------------------------
        // Loop optimisation: pre-calculate from initial 16 input values, can then be updated from next values after each iteration
        fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA * ( fIn_00_01_02_03 + RotateLeft<1>( fIn_00_01_02_03, fIn_04_05_06_07 ) );
        fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA * ( fIn_04_05_06_07 + RotateLeft<1>( fIn_04_05_06_07, fIn_08_09_0A_0B ) );
        fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA * ( fIn_08_09_0A_0B + RotateLeft<1>( fIn_08_09_0A_0B, fIn_0C_0D_0E_0F ) );
        fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 = fA * ( fIn_0C_0D_0E_0F + RotateLeft<1>( fIn_0C_0D_0E_0F, fIn_0C_0D_0E_0F ) ); // *** MUST BE RECOMPUTED IN LOOP BECAUSE RIGHTMOST ELEMENT IS INVALID! ***
        
        fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = RotateLeft<1>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
        fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = RotateLeft<2>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
        fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = RotateLeft<3>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
        
        fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = RotateLeft<1>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
        fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = RotateLeft<2>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
        fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = RotateLeft<3>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
        
        fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = RotateLeft<1>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ); // These three don't actually use the invalid rightmost element!
        fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = RotateLeft<2>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
        fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = RotateLeft<3>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
        // -----------------------------------------------------------------------------------------------------------------------------------------------
        
        //  each sample
        for ( U32 uSample = 0; uSample < uFrameLength; uSample+=16, i+=16 )
        {
            fIn_10_11_12_13 = LoadAligned( &aSample[i+0x0] );
            fIn_14_15_16_17 = LoadAligned( &aSample[i+0x4] );
            fIn_18_19_1A_1B = LoadAligned( &aSample[i+0x8] );
            fIn_1C_1D_1E_1F = LoadAligned( &aSample[i+0xC] );
            
            
            // ----------------------------------------------------------------------------------------------------------------------
            // Loop optimisation: Don't need to calc here, pre-compute before loop & update after each iteration
            //fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA * ( fIn_00_01_02_03 + RotateLeft<1>( fIn_00_01_02_03, fIn_04_05_06_07 ) );
            //fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA * ( fIn_04_05_06_07 + RotateLeft<1>( fIn_04_05_06_07, fIn_08_09_0A_0B ) );
            //fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA * ( fIn_08_09_0A_0B + RotateLeft<1>( fIn_08_09_0A_0B, fIn_0C_0D_0E_0F ) );
            // ----------------------------------------------------------------------------------------------------------------------
            fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 = fA * ( fIn_0C_0D_0E_0F + RotateLeft<1>( fIn_0C_0D_0E_0F, fIn_10_11_12_13 ) ); // *** MUST BE RECOMPUTED IN LOOP BECAUSE RIGHTMOST ELEMENT IS INVALID! ***
            fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 = fA * ( fIn_10_11_12_13 + RotateLeft<1>( fIn_10_11_12_13, fIn_14_15_16_17 ) );
            fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 = fA * ( fIn_14_15_16_17 + RotateLeft<1>( fIn_14_15_16_17, fIn_18_19_1A_1B ) );
            fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C = fA * ( fIn_18_19_1A_1B + RotateLeft<1>( fIn_18_19_1A_1B, fIn_1C_1D_1E_1F ) );
            fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX = fA * ( fIn_1C_1D_1E_1F + RotateLeft<1>( fIn_1C_1D_1E_1F, fIn_1C_1D_1E_1F ) ); // NOTE: XX is an unused element so can safely be set to any garbage value!
            
            // ---------------------------------
            // Moved up for better scheduling
            //fIn_00_01_02_03 = fIn_10_11_12_13; // Not needed anymore inside loop but MUST be accounted for after the loop!
            //fIn_04_05_06_07 = fIn_14_15_16_17;
            //fIn_08_09_0A_0B = fIn_18_19_1A_1B;
            fIn_0C_0D_0E_0F = fIn_1C_1D_1E_1F;
            // ---------------------------------
            
            // ----------------------------------------------------------------------------------------------------------------------
            // Loop optimisation: Don't need to calc here, pre-compute before loop & update after each iteration
            //fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = RotateLeft<1>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
            //fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = RotateLeft<2>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
            //fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = RotateLeft<3>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
            //
            //fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = RotateLeft<1>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
            //fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = RotateLeft<2>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
            //fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = RotateLeft<3>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
            //
            //fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = RotateLeft<1>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
            //fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = RotateLeft<2>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
            //fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = RotateLeft<3>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
            // ----------------------------------------------------------------------------------------------------------------------
            
            fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11 = RotateLeft<1>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
            fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 = RotateLeft<2>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
            fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13 = RotateLeft<3>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
            
            fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15 = RotateLeft<1>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
            fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 = RotateLeft<2>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
            fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17 = RotateLeft<3>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
            
            fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19 = RotateLeft<1>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
            fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A = RotateLeft<2>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
            fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B = RotateLeft<3>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
            
            fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D = RotateLeft<1>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
            fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E = RotateLeft<2>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
            fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F = RotateLeft<3>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
            
            // const F32 fOut10 =        fA * ( fIn0F + fIn10 ) +
            //                    fB1  * fA * ( fIn0E + fIn0F ) +
            //                    fB2  * fA * ( fIn0D + fIn0E ) +
            //                    fB3  * fA * ( fIn0C + fIn0D ) +
            //                    fB4  * fA * ( fIn0B + fIn0C ) +
            //                    fB5  * fA * ( fIn0A + fIn0B ) +
            //                    fB6  * fA * ( fIn09 + fIn0A ) +
            //                    fB7  * fA * ( fIn08 + fIn09 ) +
            //                    fB8  * fA * ( fIn07 + fIn08 ) +
            //                    fB9  * fA * ( fIn06 + fIn07 ) +
            //                    fB10 * fA * ( fIn05 + fIn06 ) +
            //                    fB11 * fA * ( fIn04 + fIn05 ) +
            //                    fB12 * fA * ( fIn03 + fIn04 ) +
            //                    fB13 * fA * ( fIn02 + fIn03 ) +
            //                    fB14 * fA * ( fIn01 + fIn02 ) +
            //                    fB15 * fA * ( fIn00 + fIn01 ) +
            //                    fB16 *       fOut00;
            
            #if 0
            
            fOut_10_11_12_13 = ( ( ( (        fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
                                       fB01 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
                                     ( fB02 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
                                       fB03 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) )     +
                                   ( ( fB04 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
                                       fB05 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
                                     ( fB06 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
                                       fB07 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) ) )   +
                                 ( ( ( fB08 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B         +
                                       fB09 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A )       +
                                     ( fB10 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09         +
                                       fB11 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) )     +
                                   ( ( fB12 * fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07         +
                                       fB13 * fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 )       +
                                     ( fB14 * fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05         +
                                       fB15 * fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 ) ) ) ) +
                                       fB16_x_fOut_00_01_02_03;
            
            fOut_14_15_16_17 = ( ( ( (        fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
                                       fB01 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
                                     ( fB02 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
                                       fB03 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) )     +
                                   ( ( fB04 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
                                       fB05 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
                                     ( fB06 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
                                       fB07 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) ) )   +
                                 ( ( ( fB08 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
                                       fB09 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
                                     ( fB10 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
                                       fB11 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) )     +
                                   ( ( fB12 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B         +
                                       fB13 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A )       +
                                     ( fB14 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09         +
                                       fB15 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) ) ) ) +
                                       fB16_x_fOut_04_05_06_07;
            
            fOut_18_19_1A_1B = ( ( ( (        fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B         +
                                       fB01 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A )       +
                                     ( fB02 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19         +
                                       fB03 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) )     +
                                   ( ( fB04 * fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
                                       fB05 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
                                     ( fB06 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
                                       fB07 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) ) )   +
                                 ( ( ( fB08 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
                                       fB09 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
                                     ( fB10 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
                                       fB11 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) )     +
                                   ( ( fB12 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
                                       fB13 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
                                     ( fB14 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
                                       fB15 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) ) ) ) +
                                       fB16_x_fOut_08_09_0A_0B;
            
            fOut_1C_1D_1E_1F = ( ( ( (        fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F         +
                                       fB01 * fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E )       +
                                     ( fB02 * fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D         +
                                       fB03 * fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C ) )     +
                                   ( ( fB04 * fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B         +
                                       fB05 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A )       +
                                     ( fB06 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19         +
                                       fB07 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) ) )   +
                                 ( ( ( fB08 * fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
                                       fB09 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
                                     ( fB10 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
                                       fB11 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) )     +
                                   ( ( fB12 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
                                       fB13 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
                                     ( fB14 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
                                       fB15 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) ) ) ) +
                                       fB16_x_fOut_0C_0D_0E_0F;
            
            #else
            
            // Eliminate common sub-expressions that are different only in powers of fB
            // Now this is starting to get really messy...
            
            const Vector4 tempA1 = ( (        fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13   +
                                       fB01 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 ) +
                                     ( fB02 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11   +
                                       fB03 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) );
            
            const Vector4 tempA2 = ( ( fB04 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F   +
                                       fB05 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E ) +
                                     ( fB06 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D   +
                                       fB07 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) );
            
            const Vector4 tempA3 = ( ( fB08 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B   +
                                       fB09 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A ) +
                                     ( fB10 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09   +
                                       fB11 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) );
            
            const Vector4 tempA4 = ( ( fB12 * fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07   +
                                       fB13 * fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 ) +
                                     ( fB14 * fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05   +
                                       fB15 * fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 ) );
            
            const Vector4 tempB1 = ( (        fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17   +
                                       fB01 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 ) +
                                     ( fB02 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15   +
                                       fB03 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) );
            
            const Vector4 tempC1 = ( (        fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B   +
                                       fB01 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A ) +
                                     ( fB02 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19   +
                                       fB03 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) );
            
            const Vector4 tempD1 = ( (        fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F   +
                                       fB01 * fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E ) +
                                     ( fB02 * fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D   +
                                       fB03 * fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C ) );
            
            fOut_10_11_12_13 = ( ( tempA1 + tempA2 ) + ( tempA3 + tempA4 ) ) + fB16_x_fOut_00_01_02_03;
            
            fOut_14_15_16_17 = ( ( tempB1 + ( fB04 * tempA1 ) ) + ( ( fB04 * tempA2 ) + ( fB04 * tempA3 ) ) ) + fB16_x_fOut_04_05_06_07;
            
            fOut_18_19_1A_1B = ( ( tempC1 + ( fB04 * tempB1 ) ) + ( ( fB08 * tempA1 ) + ( fB08 * tempA2 ) ) ) + fB16_x_fOut_08_09_0A_0B;
            
            fOut_1C_1D_1E_1F = ( ( tempD1 + ( fB04 * tempC1 ) ) + ( ( fB08 * tempB1 ) + ( fB12 * tempA1 ) ) ) + fB16_x_fOut_0C_0D_0E_0F;
            
            #endif
            
            // -----------------------------------------------------------------------------------
            // Loop optimisation:
            fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14;
            fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15;
            fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16;
            fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17;
            fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18;
            fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19;
            fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A;
            fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B;
            fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C;
            fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D;
            fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E;
            fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F;
            // -----------------------------------------------------------------------------------
            
            StoreAligned( fGain * fOut_10_11_12_13, &aSample[i+0x0] );
            StoreAligned( fGain * fOut_14_15_16_17, &aSample[i+0x4] );
            StoreAligned( fGain * fOut_18_19_1A_1B, &aSample[i+0x8] );
            StoreAligned( fGain * fOut_1C_1D_1E_1F, &aSample[i+0xC] );
            
            // ---------------------------------
            // Moved up for better scheduling
            ////fIn_00_01_02_03 = fIn_10_11_12_13; // Not needed anymore inside loop but MUST be accounted for after the loop!
            ////fIn_04_05_06_07 = fIn_14_15_16_17;
            ////fIn_08_09_0A_0B = fIn_18_19_1A_1B;
            //fIn_0C_0D_0E_0F = fIn_1C_1D_1E_1F;
            // ---------------------------------
            
            fB16_x_fOut_00_01_02_03 = fB16 * fOut_10_11_12_13;
            fB16_x_fOut_04_05_06_07 = fB16 * fOut_14_15_16_17;
            fB16_x_fOut_08_09_0A_0B = fB16 * fOut_18_19_1A_1B;
            fB16_x_fOut_0C_0D_0E_0F = fB16 * fOut_1C_1D_1E_1F;
        }
        
        // ------------------------------------------------------------------------
        // Not needed anymore inside loop but MUST be accounted for after the loop!
        fIn_00_01_02_03 = fIn_10_11_12_13;
        fIn_04_05_06_07 = fIn_14_15_16_17;
        fIn_08_09_0A_0B = fIn_18_19_1A_1B;
        // ------------------------------------------------------------------------
        
        //  store previous samples
        
        m_fIn_00_01_02_03[uCh] = fIn_00_01_02_03;
        m_fIn_04_05_06_07[uCh] = fIn_04_05_06_07;
        m_fIn_08_09_0A_0B[uCh] = fIn_08_09_0A_0B;
        m_fIn_0C_0D_0E_0F[uCh] = fIn_0C_0D_0E_0F;
        
        m_fB16_x_fOut_00_01_02_03[uCh] = fB16_x_fOut_00_01_02_03;
        m_fB16_x_fOut_04_05_06_07[uCh] = fB16_x_fOut_04_05_06_07;
        m_fB16_x_fOut_08_09_0A_0B[uCh] = fB16_x_fOut_08_09_0A_0B;
        m_fB16_x_fOut_0C_0D_0E_0F[uCh] = fB16_x_fOut_0C_0D_0E_0F;
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
    
    Vector4 vIn_00_01_02_03 = v4_zero;
    Vector4 vIn_01_02_03_04 = v4_zero;
    Vector4 vIn_02_03_04_05 = v4_zero;
    Vector4 vIn_03_04_05_06 = v4_zero;
    Vector4 vIn_04_05_06_07 = v4_zero;
    Vector4 vIn_05_06_07_08 = v4_zero;
    Vector4 vIn_06_07_08_09 = v4_zero;
    Vector4 vIn_07_08_09_0A = v4_zero;
    Vector4 vIn_08_09_0A_0B = v4_zero;
    Vector4 vIn_09_0A_0B_0C = v4_zero;
    Vector4 vIn_0A_0B_0C_0D = v4_zero;
    Vector4 vIn_0B_0C_0D_0E = v4_zero;
    Vector4 vIn_0C_0D_0E_0F = v4_zero;
    Vector4 vIn_0D_0E_0F_10 = v4_zero;
    Vector4 vIn_0E_0F_10_11 = v4_zero;
    Vector4 vIn_0F_10_11_12 = v4_zero;
    Vector4 vIn_10_11_12_13 = v4_zero;
    
    Vector4 vOut_00_01_02_03 = v4_zero;
    Vector4 vOut_04_05_06_07 = v4_zero;
    Vector4 vOut_08_09_0A_0B = v4_zero;
    Vector4 vOut_0C_0D_0E_0F = v4_zero;
    Vector4 vOut_10_11_12_13 = v4_zero;
    
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
            
            const Vector4 vSum_00_01_02_03 = vIn_00_01_02_03 + vIn_01_02_03_04;
            const Vector4 vSum_01_02_03_04 = vIn_01_02_03_04 + vIn_02_03_04_05;
            const Vector4 vSum_02_03_04_05 = vIn_02_03_04_05 + vIn_03_04_05_06;
            const Vector4 vSum_03_04_05_06 = vIn_03_04_05_06 + vIn_04_05_06_07;
            const Vector4 vSum_04_05_06_07 = vIn_04_05_06_07 + vIn_05_06_07_08;
            const Vector4 vSum_05_06_07_08 = vIn_05_06_07_08 + vIn_06_07_08_09;
            const Vector4 vSum_06_07_08_09 = vIn_06_07_08_09 + vIn_07_08_09_0A;
            const Vector4 vSum_07_08_09_0A = vIn_07_08_09_0A + vIn_08_09_0A_0B;
            const Vector4 vSum_08_09_0A_0B = vIn_08_09_0A_0B + vIn_09_0A_0B_0C;
            const Vector4 vSum_09_0A_0B_0C = vIn_09_0A_0B_0C + vIn_0A_0B_0C_0D;
            const Vector4 vSum_0A_0B_0C_0D = vIn_0A_0B_0C_0D + vIn_0B_0C_0D_0E;
            const Vector4 vSum_0B_0C_0D_0E = vIn_0B_0C_0D_0E + vIn_0C_0D_0E_0F;
            const Vector4 vSum_0C_0D_0E_0F = vIn_0C_0D_0E_0F + vIn_0D_0E_0F_10;
            const Vector4 vSum_0D_0E_0F_10 = vIn_0D_0E_0F_10 + vIn_0E_0F_10_11;
            const Vector4 vSum_0E_0F_10_11 = vIn_0E_0F_10_11 + vIn_0F_10_11_12;
            const Vector4 vSum_0F_10_11_12 = vIn_0F_10_11_12 + vIn_10_11_12_13;
            
            #if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
            
            vOut_04_05_06_07 = MakeVector4( v_AB3_AB2_AB1_A % vSum_00_01_02_03,
                                            v_AB3_AB2_AB1_A % vSum_01_02_03_04,
                                            v_AB3_AB2_AB1_A % vSum_02_03_04_05, 
                                            v_AB3_AB2_AB1_A % vSum_03_04_05_06 ) +
                               vB4 * vOut_00_01_02_03;
            
            vOut_08_09_0A_0B = MakeVector4( v_AB3_AB2_AB1_A % vSum_04_05_06_07,
                                            v_AB3_AB2_AB1_A % vSum_05_06_07_08,
                                            v_AB3_AB2_AB1_A % vSum_06_07_08_09,
                                            v_AB3_AB2_AB1_A % vSum_07_08_09_0A ) +
                               vB4 * vOut_04_05_06_07;
            
            vOut_0C_0D_0E_0F = MakeVector4( v_AB3_AB2_AB1_A % vSum_08_09_0A_0B,
                                            v_AB3_AB2_AB1_A % vSum_09_0A_0B_0C,
                                            v_AB3_AB2_AB1_A % vSum_0A_0B_0C_0D,
                                            v_AB3_AB2_AB1_A % vSum_0B_0C_0D_0E ) +
                               vB4 * vOut_08_09_0A_0B;
            
            vOut_10_11_12_13 = MakeVector4( v_AB3_AB2_AB1_A % vSum_0C_0D_0E_0F,
                                            v_AB3_AB2_AB1_A % vSum_0D_0E_0F_10,
                                            v_AB3_AB2_AB1_A % vSum_0E_0F_10_11,
                                            v_AB3_AB2_AB1_A % vSum_0F_10_11_12 ) +
                               vB4 * vOut_0C_0D_0E_0F;
            
            #else
            
            vOut_04_05_06_07 = ( ( vA       * vSum_03_04_05_06 +
                                   vA_x_vB1 * vSum_02_03_04_05 ) +
                                 ( vA_x_vB2 * vSum_01_02_03_04 +
                                   vA_x_vB3 * vSum_00_01_02_03 ) ) +
                                        vB4 * vOut_00_01_02_03;
            
            vOut_08_09_0A_0B = ( ( vA       * vSum_07_08_09_0A +
                                   vA_x_vB1 * vSum_06_07_08_09 ) +
                                 ( vA_x_vB2 * vSum_05_06_07_08 +
                                   vA_x_vB3 * vSum_04_05_06_07 ) ) +
                                        vB4 * vOut_04_05_06_07;
            
            vOut_0C_0D_0E_0F = ( ( vA       * vSum_0B_0C_0D_0E +
                                   vA_x_vB1 * vSum_0A_0B_0C_0D ) +
                                 ( vA_x_vB2 * vSum_09_0A_0B_0C +
                                   vA_x_vB3 * vSum_08_09_0A_0B ) ) +
                                        vB4 * vOut_08_09_0A_0B;
            
            vOut_10_11_12_13 = ( ( vA       * vSum_0F_10_11_12 +
                                   vA_x_vB1 * vSum_0E_0F_10_11 ) +
                                 ( vA_x_vB2 * vSum_0D_0E_0F_10 +
                                   vA_x_vB3 * vSum_0C_0D_0E_0F ) ) +
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
            
    #endif // Versions
    
    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}

