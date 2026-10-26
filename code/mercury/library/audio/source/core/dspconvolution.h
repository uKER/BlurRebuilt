#ifndef _DSPCONVOLUTION_H_
#define _DSPCONVOLUTION_H_

/*
static bool g_bDoIt = true;

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Convolution
////////////////////////////////////////////////////////////////////////////////
            template <typename DataType, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uDownSample>
		    class Convolution : public IConvolution, public Effect<DataType, uSampleRate, uFrameLength, uChannels>
		    {
		    public:
			    Convolution(U32 uNumFrames);
			    ~Convolution() {}

                void Process(void* pFrameData);

                void SetImpulse(U32 uChannel, const Audio::Core::WaveData* pImpulse);

                //IEffect* GetEffect()
                //{
                //    return (IEffect*)(Effect<DataType, uSampleRate, uFrameLength, uChannels>*)this;
                //}

            private:
                U32 m_uNumFrames;

                F32* m_apfImpulse[uChannels];

                F32 m_fDryGain;
                F32 m_fWetGain;

                DataType* m_apConvolution[uChannels];

                F32* m_apfResample[uChannels];
                F32 m_afLastConvolution[uChannels];
            };
	    };
	};
};

////////////////////////////////////////////////////////////////////////////////
//	Convolution
////////////////////////////////////////////////////////////////////////////////
template <typename DataType, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uDownSample>
Audio::Core::DSP::Convolution<DataType, uSampleRate, uFrameLength, uChannels, uDownSample>::Convolution(U32 uNumFrames)
{
    m_uNumFrames = uNumFrames;

    m_fDryGain = 0.0f;//0.85f;
    m_fWetGain = 0.1f;//0.5f;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        m_apfImpulse[uCh] = NULL;
        m_apConvolution[uCh] = NULL;
        m_afLastConvolution[uCh] = 0.f;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <typename DataType, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uDownSample>
void Audio::Core::DSP::Convolution<DataType, uSampleRate, uFrameLength, uChannels, uDownSample>::Process(void* pFrameData)
{
    if (g_bDoIt == false)
        return;

    U32 uFrom;
    U32 uTo;

    //  Assert num channels == 1

    F32 fDryGain = m_fDryGain;
    F32 fWetGain = m_fWetGain;

    U32 uLength = m_uNumFrames * uFrameLength;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        F32* aAllSamples = (F32*)pFrameData;
        F32* aSample = &aAllSamples[uCh * uFrameLength];

        if (uDownSample == 1)
            Frame::Copy(m_apfResample[uCh], aSample);
        else if (uDownSample == 2)
        {
    //        for (int i=0; i<uFrameLength/2; i++)
    //            m_apfResample[uCh][i] = (aSample[i*2] + aSample[i*2+1]) / 2.f;

            __vector4 const* __restrict pSample = (__vector4*)aSample;
            __vector4* __restrict pResample = (__vector4*)m_apfResample[uCh];
            
            __declspec(align(16)) char fPermEven[16] = {0,1,2,3, 8,9,10,11,   16,17,18,19, 24,25,26,27};
            __declspec(align(16)) char fPermOdd[16] =  {4,5,6,7, 12,13,14,15, 20,21,22,23, 28,29,30,31};

            register const __vector4 vPermEven = __lvx(fPermEven, 0);
            register const __vector4 vPermOdd =  __lvx(fPermOdd, 0);

            //  half frame
            for (U32 chunk = 0; chunk < 2; chunk++)
            {
                // Load chunk of 128 samples
                register __vector4 vSamples00 = pSample[0];
                register __vector4 vSamples01 = pSample[1];
                register __vector4 vSamples02 = pSample[2];
                register __vector4 vSamples03 = pSample[3];
                register __vector4 vSamples04 = pSample[4];
                register __vector4 vSamples05 = pSample[5];
                register __vector4 vSamples06 = pSample[6];
                register __vector4 vSamples07 = pSample[7];
                register __vector4 vSamples08 = pSample[8];
                register __vector4 vSamples09 = pSample[9];
                register __vector4 vSamples10 = pSample[10];
                register __vector4 vSamples11 = pSample[11];
                register __vector4 vSamples12 = pSample[12];
                register __vector4 vSamples13 = pSample[13];
                register __vector4 vSamples14 = pSample[14];
                register __vector4 vSamples15 = pSample[15];
                register __vector4 vSamples16 = pSample[16];
                register __vector4 vSamples17 = pSample[17];
                register __vector4 vSamples18 = pSample[18];
                register __vector4 vSamples19 = pSample[19];
                register __vector4 vSamples20 = pSample[20];
                register __vector4 vSamples21 = pSample[21];
                register __vector4 vSamples22 = pSample[22];
                register __vector4 vSamples23 = pSample[23];
                register __vector4 vSamples24 = pSample[24];
                register __vector4 vSamples25 = pSample[25];
                register __vector4 vSamples26 = pSample[26];
                register __vector4 vSamples27 = pSample[27];
                register __vector4 vSamples28 = pSample[28];
                register __vector4 vSamples29 = pSample[29];
                register __vector4 vSamples30 = pSample[30];
                register __vector4 vSamples31 = pSample[31];
                
                //  Get even permutes
                register __vector4 vSamplesEven00 = __vperm(vSamples00, vSamples01, vPermEven);
                register __vector4 vSamplesEven01 = __vperm(vSamples02, vSamples03, vPermEven);
                register __vector4 vSamplesEven02 = __vperm(vSamples04, vSamples05, vPermEven);
                register __vector4 vSamplesEven03 = __vperm(vSamples06, vSamples07, vPermEven);
                register __vector4 vSamplesEven04 = __vperm(vSamples08, vSamples09, vPermEven);
                register __vector4 vSamplesEven05 = __vperm(vSamples10, vSamples11, vPermEven);
                register __vector4 vSamplesEven06 = __vperm(vSamples12, vSamples13, vPermEven);
                register __vector4 vSamplesEven07 = __vperm(vSamples14, vSamples15, vPermEven);
                register __vector4 vSamplesEven08 = __vperm(vSamples16, vSamples17, vPermEven);
                register __vector4 vSamplesEven09 = __vperm(vSamples18, vSamples19, vPermEven);
                register __vector4 vSamplesEven10 = __vperm(vSamples20, vSamples21, vPermEven);
                register __vector4 vSamplesEven11 = __vperm(vSamples22, vSamples23, vPermEven);
                register __vector4 vSamplesEven12 = __vperm(vSamples24, vSamples25, vPermEven);
                register __vector4 vSamplesEven13 = __vperm(vSamples26, vSamples27, vPermEven);
                register __vector4 vSamplesEven14 = __vperm(vSamples28, vSamples29, vPermEven);
                register __vector4 vSamplesEven15 = __vperm(vSamples30, vSamples31, vPermEven);

                //  Get odd permutes
                register __vector4 vSamplesOdd00 = __vperm(vSamples00, vSamples01, vPermOdd);
                register __vector4 vSamplesOdd01 = __vperm(vSamples02, vSamples03, vPermOdd);
                register __vector4 vSamplesOdd02 = __vperm(vSamples04, vSamples05, vPermOdd);
                register __vector4 vSamplesOdd03 = __vperm(vSamples06, vSamples07, vPermOdd);
                register __vector4 vSamplesOdd04 = __vperm(vSamples08, vSamples09, vPermOdd);
                register __vector4 vSamplesOdd05 = __vperm(vSamples10, vSamples11, vPermOdd);
                register __vector4 vSamplesOdd06 = __vperm(vSamples12, vSamples13, vPermOdd);
                register __vector4 vSamplesOdd07 = __vperm(vSamples14, vSamples15, vPermOdd);
                register __vector4 vSamplesOdd08 = __vperm(vSamples16, vSamples17, vPermOdd);
                register __vector4 vSamplesOdd09 = __vperm(vSamples18, vSamples19, vPermOdd);
                register __vector4 vSamplesOdd10 = __vperm(vSamples20, vSamples21, vPermOdd);
                register __vector4 vSamplesOdd11 = __vperm(vSamples22, vSamples23, vPermOdd);
                register __vector4 vSamplesOdd12 = __vperm(vSamples24, vSamples25, vPermOdd);
                register __vector4 vSamplesOdd13 = __vperm(vSamples26, vSamples27, vPermOdd);
                register __vector4 vSamplesOdd14 = __vperm(vSamples28, vSamples29, vPermOdd);
                register __vector4 vSamplesOdd15 = __vperm(vSamples30, vSamples31, vPermOdd);

                //  Downsample
                pResample[0]  = __vaddfp(vSamplesEven00, vSamplesOdd00);
                pResample[1]  = __vaddfp(vSamplesEven01, vSamplesOdd01);
                pResample[2]  = __vaddfp(vSamplesEven02, vSamplesOdd02);
                pResample[3]  = __vaddfp(vSamplesEven03, vSamplesOdd03);
                pResample[4]  = __vaddfp(vSamplesEven04, vSamplesOdd04);
                pResample[5]  = __vaddfp(vSamplesEven05, vSamplesOdd05);
                pResample[6]  = __vaddfp(vSamplesEven06, vSamplesOdd06);
                pResample[7]  = __vaddfp(vSamplesEven07, vSamplesOdd07);
                pResample[8]  = __vaddfp(vSamplesEven08, vSamplesOdd08);
                pResample[9]  = __vaddfp(vSamplesEven09, vSamplesOdd09);
                pResample[10] = __vaddfp(vSamplesEven10, vSamplesOdd10);
                pResample[11] = __vaddfp(vSamplesEven11, vSamplesOdd11);
                pResample[12] = __vaddfp(vSamplesEven12, vSamplesOdd12);
                pResample[13] = __vaddfp(vSamplesEven13, vSamplesOdd13);
                pResample[14] = __vaddfp(vSamplesEven14, vSamplesOdd14);
                pResample[15] = __vaddfp(vSamplesEven15, vSamplesOdd15);
                
                // Next chunk of 128 samples
                pSample += 32;
                pResample += 16;
            }
        }
        else if (uDownSample == 4)
        {
            for (int i=0; i<uFrameLength/4; i++)
                m_apfResample[uCh][i] = (aSample[i*4] + aSample[i*4+1] + aSample[i*4+2] + aSample[i*4+3]) / 4.f;
        }

        //  shuffle tail along
        for (U32 uTail = 0; uTail < m_uNumFrames * uDownSample - 1; uTail++)
        {
            uFrom = (uTail + 1) * uFrameLength / uDownSample;
            uTo = uTail * uFrameLength / uDownSample;

            //  TODO: last frame is < uFrameLength
            Frame::SubCopy(&m_apConvolution[uCh][uTo], &m_apConvolution[uCh][uFrom]);
            if (uDownSample < 4)
                Frame::SubCopy(&m_apConvolution[uCh][uTo + 0x40], &m_apConvolution[uCh][uFrom + 0x40]);
            if (uDownSample < 2)
            {
                Frame::SubCopy(&m_apConvolution[uCh][uTo + 0x80], &m_apConvolution[uCh][uFrom + 0x80]);
                Frame::SubCopy(&m_apConvolution[uCh][uTo + 0xc0], &m_apConvolution[uCh][uFrom + 0xc0]);
            }
        }

        //  clear last frame of convolution tail
        uTo = (m_uNumFrames * uDownSample - 1) * uFrameLength / uDownSample;
        Frame::SubZero(&m_apConvolution[uCh][uTo]);
        if (uDownSample < 4)
            Frame::SubZero(&m_apConvolution[uCh][uTo + 0x40]);
        if (uDownSample < 2)
        {
            Frame::SubZero(&m_apConvolution[uCh][uTo + 0x80]);
            Frame::SubZero(&m_apConvolution[uCh][uTo + 0xc0]);
        }

        __vector4 const* __restrict pImpulse = (__vector4*)m_apfImpulse[uCh];
        __dcbt( 0x000, pImpulse );
        
        __vector4* __restrict pBaseConvolution = (__vector4*)m_apConvolution[uCh];
        __dcbt(0x000, pBaseConvolution);
        __dcbt(0x080, pBaseConvolution);
        
        __vector4 const* __restrict pBaseSample = (__vector4*)m_apfResample[uCh];

        for (U32 uImpulse = 0; uImpulse < uLength; uImpulse += 4)
        {
            register const __vector4 impulse = pImpulse[uCh];
            
            // Prefetch next impulse cache line(s)
            __dcbt( 0x080, pImpulse );
            //__dcbt( 0x100, pImpulse );
            
            register const __vector4 impulse0 = __vspltw(impulse, 0);
            register const __vector4 impulse1 = __vspltw(impulse, 1);
            register const __vector4 impulse2 = __vspltw(impulse, 2);
            register const __vector4 impulse3 = __vspltw(impulse, 3);
            
            __vector4 const* __restrict pSample = pBaseSample;
            __vector4* __restrict pConvolution = pBaseConvolution;
            
            for ( U32 chunk = 0; chunk < 4 / uDownSample; chunk++ )
            {
                // Load chunk of 64 samples
                register __vector4 sampleA00 = pSample[0];
                register __vector4 sampleA01 = pSample[1];
                register __vector4 sampleA02 = pSample[2];
                register __vector4 sampleA03 = pSample[3];
                register __vector4 sampleA04 = pSample[4];
                register __vector4 sampleA05 = pSample[5];
                register __vector4 sampleA06 = pSample[6];
                register __vector4 sampleA07 = pSample[7];
                register __vector4 sampleA08 = pSample[8];
                register __vector4 sampleA09 = pSample[9];
                register __vector4 sampleA10 = pSample[10];
                register __vector4 sampleA11 = pSample[11];
                register __vector4 sampleA12 = pSample[12];
                register __vector4 sampleA13 = pSample[13];
                register __vector4 sampleA14 = pSample[14];
                register __vector4 sampleA15 = pSample[15];
                register __vector4 sampleA16 = __vzero();
                
                // Load chunk of 64 convolutions
                register __vector4 convolution00 = pConvolution[0];
                register __vector4 convolution01 = pConvolution[1];
                register __vector4 convolution02 = pConvolution[2];
                register __vector4 convolution03 = pConvolution[3];
                register __vector4 convolution04 = pConvolution[4];
                register __vector4 convolution05 = pConvolution[5];
                register __vector4 convolution06 = pConvolution[6];
                register __vector4 convolution07 = pConvolution[7];
                register __vector4 convolution08 = pConvolution[8];
                register __vector4 convolution09 = pConvolution[9];
                register __vector4 convolution10 = pConvolution[10];
                register __vector4 convolution11 = pConvolution[11];
                register __vector4 convolution12 = pConvolution[12];
                register __vector4 convolution13 = pConvolution[13];
                register __vector4 convolution14 = pConvolution[14];
                register __vector4 convolution15 = pConvolution[15];
                register __vector4 convolution16 = pConvolution[16];
                
                // Prefetch next pair(s) of convolution cache lines
                __dcbt( 0x100, pConvolution );
                __dcbt( 0x180, pConvolution );
                //__dcbt( 0x200, pConvolution );
                //__dcbt( 0x280, pConvolution );
                
                // Accumulate impulse0 * sample into convolution
                convolution00 = __vmaddfp( impulse0, sampleA00, convolution00 );
                convolution01 = __vmaddfp( impulse0, sampleA01, convolution01 );
                convolution02 = __vmaddfp( impulse0, sampleA02, convolution02 );
                convolution03 = __vmaddfp( impulse0, sampleA03, convolution03 );
                convolution04 = __vmaddfp( impulse0, sampleA04, convolution04 );
                convolution05 = __vmaddfp( impulse0, sampleA05, convolution05 );
                convolution06 = __vmaddfp( impulse0, sampleA06, convolution06 );
                convolution07 = __vmaddfp( impulse0, sampleA07, convolution07 );
                convolution08 = __vmaddfp( impulse0, sampleA08, convolution08 );
                convolution09 = __vmaddfp( impulse0, sampleA09, convolution09 );
                convolution10 = __vmaddfp( impulse0, sampleA10, convolution10 );
                convolution11 = __vmaddfp( impulse0, sampleA11, convolution11 );
                convolution12 = __vmaddfp( impulse0, sampleA12, convolution12 );
                convolution13 = __vmaddfp( impulse0, sampleA13, convolution13 );
                convolution14 = __vmaddfp( impulse0, sampleA14, convolution14 );
                convolution15 = __vmaddfp( impulse0, sampleA15, convolution15 );
                convolution16 = __vmaddfp( impulse0, sampleA16, convolution16 );
                
                // Rotate samples right
                register __vector4 sampleB00 = __vsldoi( __vzero(), sampleA00, 12 );
                register __vector4 sampleB01 = __vsldoi( sampleA00, sampleA01, 12 );
                register __vector4 sampleB02 = __vsldoi( sampleA01, sampleA02, 12 );
                register __vector4 sampleB03 = __vsldoi( sampleA02, sampleA03, 12 );
                register __vector4 sampleB04 = __vsldoi( sampleA03, sampleA04, 12 );
                register __vector4 sampleB05 = __vsldoi( sampleA04, sampleA05, 12 );
                register __vector4 sampleB06 = __vsldoi( sampleA05, sampleA06, 12 );
                register __vector4 sampleB07 = __vsldoi( sampleA06, sampleA07, 12 );
                register __vector4 sampleB08 = __vsldoi( sampleA07, sampleA08, 12 );
                register __vector4 sampleB09 = __vsldoi( sampleA08, sampleA09, 12 );
                register __vector4 sampleB10 = __vsldoi( sampleA09, sampleA10, 12 );
                register __vector4 sampleB11 = __vsldoi( sampleA10, sampleA11, 12 );
                register __vector4 sampleB12 = __vsldoi( sampleA11, sampleA12, 12 );
                register __vector4 sampleB13 = __vsldoi( sampleA12, sampleA13, 12 );
                register __vector4 sampleB14 = __vsldoi( sampleA13, sampleA14, 12 );
                register __vector4 sampleB15 = __vsldoi( sampleA14, sampleA15, 12 );
                register __vector4 sampleB16 = __vsldoi( sampleA15, sampleA16, 12 );
                
                // Accumulate impulse1 * sample into convolution
                convolution00 = __vmaddfp( impulse1, sampleB00, convolution00 );
                convolution01 = __vmaddfp( impulse1, sampleB01, convolution01 );
                convolution02 = __vmaddfp( impulse1, sampleB02, convolution02 );
                convolution03 = __vmaddfp( impulse1, sampleB03, convolution03 );
                convolution04 = __vmaddfp( impulse1, sampleB04, convolution04 );
                convolution05 = __vmaddfp( impulse1, sampleB05, convolution05 );
                convolution06 = __vmaddfp( impulse1, sampleB06, convolution06 );
                convolution07 = __vmaddfp( impulse1, sampleB07, convolution07 );
                convolution08 = __vmaddfp( impulse1, sampleB08, convolution08 );
                convolution09 = __vmaddfp( impulse1, sampleB09, convolution09 );
                convolution10 = __vmaddfp( impulse1, sampleB10, convolution10 );
                convolution11 = __vmaddfp( impulse1, sampleB11, convolution11 );
                convolution12 = __vmaddfp( impulse1, sampleB12, convolution12 );
                convolution13 = __vmaddfp( impulse1, sampleB13, convolution13 );
                convolution14 = __vmaddfp( impulse1, sampleB14, convolution14 );
                convolution15 = __vmaddfp( impulse1, sampleB15, convolution15 );
                convolution16 = __vmaddfp( impulse1, sampleB16, convolution16 );
                
                // Rotate samples right
                sampleA00 = __vsldoi( __vzero(), sampleB00, 12 );
                sampleA01 = __vsldoi( sampleB00, sampleB01, 12 );
                sampleA02 = __vsldoi( sampleB01, sampleB02, 12 );
                sampleA03 = __vsldoi( sampleB02, sampleB03, 12 );
                sampleA04 = __vsldoi( sampleB03, sampleB04, 12 );
                sampleA05 = __vsldoi( sampleB04, sampleB05, 12 );
                sampleA06 = __vsldoi( sampleB05, sampleB06, 12 );
                sampleA07 = __vsldoi( sampleB06, sampleB07, 12 );
                sampleA08 = __vsldoi( sampleB07, sampleB08, 12 );
                sampleA09 = __vsldoi( sampleB08, sampleB09, 12 );
                sampleA10 = __vsldoi( sampleB09, sampleB10, 12 );
                sampleA11 = __vsldoi( sampleB10, sampleB11, 12 );
                sampleA12 = __vsldoi( sampleB11, sampleB12, 12 );
                sampleA13 = __vsldoi( sampleB12, sampleB13, 12 );
                sampleA14 = __vsldoi( sampleB13, sampleB14, 12 );
                sampleA15 = __vsldoi( sampleB14, sampleB15, 12 );
                sampleA16 = __vsldoi( sampleB15, sampleB16, 12 );
                
                // Accumulate impulse2 * sample into convolution
                convolution00 = __vmaddfp( impulse2, sampleA00, convolution00 );
                convolution01 = __vmaddfp( impulse2, sampleA01, convolution01 );
                convolution02 = __vmaddfp( impulse2, sampleA02, convolution02 );
                convolution03 = __vmaddfp( impulse2, sampleA03, convolution03 );
                convolution04 = __vmaddfp( impulse2, sampleA04, convolution04 );
                convolution05 = __vmaddfp( impulse2, sampleA05, convolution05 );
                convolution06 = __vmaddfp( impulse2, sampleA06, convolution06 );
                convolution07 = __vmaddfp( impulse2, sampleA07, convolution07 );
                convolution08 = __vmaddfp( impulse2, sampleA08, convolution08 );
                convolution09 = __vmaddfp( impulse2, sampleA09, convolution09 );
                convolution10 = __vmaddfp( impulse2, sampleA10, convolution10 );
                convolution11 = __vmaddfp( impulse2, sampleA11, convolution11 );
                convolution12 = __vmaddfp( impulse2, sampleA12, convolution12 );
                convolution13 = __vmaddfp( impulse2, sampleA13, convolution13 );
                convolution14 = __vmaddfp( impulse2, sampleA14, convolution14 );
                convolution15 = __vmaddfp( impulse2, sampleA15, convolution15 );
                convolution16 = __vmaddfp( impulse2, sampleA16, convolution16 );
                
                // Rotate samples right
                sampleB00 = __vsldoi( __vzero(), sampleA00, 12 );
                sampleB01 = __vsldoi( sampleA00, sampleA01, 12 );
                sampleB02 = __vsldoi( sampleA01, sampleA02, 12 );
                sampleB03 = __vsldoi( sampleA02, sampleA03, 12 );
                sampleB04 = __vsldoi( sampleA03, sampleA04, 12 );
                sampleB05 = __vsldoi( sampleA04, sampleA05, 12 );
                sampleB06 = __vsldoi( sampleA05, sampleA06, 12 );
                sampleB07 = __vsldoi( sampleA06, sampleA07, 12 );
                sampleB08 = __vsldoi( sampleA07, sampleA08, 12 );
                sampleB09 = __vsldoi( sampleA08, sampleA09, 12 );
                sampleB10 = __vsldoi( sampleA09, sampleA10, 12 );
                sampleB11 = __vsldoi( sampleA10, sampleA11, 12 );
                sampleB12 = __vsldoi( sampleA11, sampleA12, 12 );
                sampleB13 = __vsldoi( sampleA12, sampleA13, 12 );
                sampleB14 = __vsldoi( sampleA13, sampleA14, 12 );
                sampleB15 = __vsldoi( sampleA14, sampleA15, 12 );
                sampleB16 = __vsldoi( sampleA15, sampleA16, 12 );
                
                // Accumulate impulse3 * sample into convolution
                convolution00 = __vmaddfp( impulse3, sampleB00, convolution00 );
                convolution01 = __vmaddfp( impulse3, sampleB01, convolution01 );
                convolution02 = __vmaddfp( impulse3, sampleB02, convolution02 );
                convolution03 = __vmaddfp( impulse3, sampleB03, convolution03 );
                convolution04 = __vmaddfp( impulse3, sampleB04, convolution04 );
                convolution05 = __vmaddfp( impulse3, sampleB05, convolution05 );
                convolution06 = __vmaddfp( impulse3, sampleB06, convolution06 );
                convolution07 = __vmaddfp( impulse3, sampleB07, convolution07 );
                convolution08 = __vmaddfp( impulse3, sampleB08, convolution08 );
                convolution09 = __vmaddfp( impulse3, sampleB09, convolution09 );
                convolution10 = __vmaddfp( impulse3, sampleB10, convolution10 );
                convolution11 = __vmaddfp( impulse3, sampleB11, convolution11 );
                convolution12 = __vmaddfp( impulse3, sampleB12, convolution12 );
                convolution13 = __vmaddfp( impulse3, sampleB13, convolution13 );
                convolution14 = __vmaddfp( impulse3, sampleB14, convolution14 );
                convolution15 = __vmaddfp( impulse3, sampleB15, convolution15 );
                convolution16 = __vmaddfp( impulse3, sampleB16, convolution16 );
                
                // Store convolution back
                pConvolution[0]  = convolution00;
                pConvolution[1]  = convolution01;
                pConvolution[2]  = convolution02;
                pConvolution[3]  = convolution03;
                pConvolution[4]  = convolution04;
                pConvolution[5]  = convolution05;
                pConvolution[6]  = convolution06;
                pConvolution[7]  = convolution07;
                pConvolution[8]  = convolution08;
                pConvolution[9]  = convolution09;
                pConvolution[10] = convolution10;
                pConvolution[11] = convolution11;
                pConvolution[12] = convolution12;
                pConvolution[13] = convolution13;
                pConvolution[14] = convolution14;
                pConvolution[15] = convolution15;
                pConvolution[16] = convolution16;
                
                // Next chunk of 64 samples and convolutions
                pSample += 16;
                pConvolution += 16;
            }
            
            pImpulse += 1;
            pBaseConvolution += 1;
        }

        if (uDownSample == 1)
        {
            //  mix dry + wet
            if (fDryGain == 1.f)
            {
                if (fWetGain == 1.f)                    //  d=1,w=1 -> add frames
                    Frame::Add(aSample, aSample, m_apConvolution[uCh]);
                else if (fWetGain != 0.f)               //  d=1, w!=1/0, frame add gain frame
                    Frame::AddGain(aSample, aSample, m_apConvolution[uCh], fWetGain);
                //  else d=1,w=0 - do nothing
            }
            else if (fDryGain == 0.f)
            {
                if (fWetGain == 1.f)                    //  d=0,w=1 -> copy frame
                    Frame::Copy(aSample, m_apConvolution[uCh]);
                else if (fWetGain == 0.f)               //  d=0,w=0 -> zero frame
                    Frame::Zero(aSample);
                else                                    //  d=0,w!=1/0 -> gain frame
                    Frame::Gain(aSample, m_apConvolution[uCh], fWetGain);
            }
            else
            {
                if (fWetGain == 1.f)                    //  d!=1/0,w=1 -> frame add gain frame
                    Frame::AddGain(aSample, m_apConvolution[uCh], aSample, fDryGain);
                else if (fWetGain == 0.f)               //  d!=1/0,w=0 -> gain frame
                    Frame::Gain(aSample, aSample, fDryGain);
                else                                    //  d!=1/0,w!=1/0 -> gain frame add gain frame
                    Frame::GainAddGain(aSample, aSample, fDryGain, m_apConvolution[uCh], fWetGain);
            }
        }
        else if (uDownSample == 2)
        {
            //  mix convolution back into dry signal
            F32 fPrevConv = m_afLastConvolution[uCh];
            for (int i = 0; i<uFrameLength / 2; i++)
            {
                aSample[i * 2] = fDryGain * aSample[i * 2] + fWetGain * (fPrevConv + (m_apConvolution[uCh][i] - fPrevConv) / 2.f);
                aSample[i * 2 + 1] = fDryGain * aSample[i * 2 + 1] + fWetGain * m_apConvolution[uCh][i];

                fPrevConv = m_apConvolution[uCh][i];
            }

            m_afLastConvolution[uCh] = fPrevConv;
        }
        else if (uDownSample == 4)
        {
            //  mix convolution back into dry signal
            F32 fPrevConv = m_afLastConvolution[uCh];
            for (int i = 0; i<uFrameLength / 4; i++)
            {
                aSample[i * 4] = fDryGain * aSample[i * 4] + fWetGain * (fPrevConv + (m_apConvolution[uCh][i] - fPrevConv) * 0.25f);
                aSample[i * 4 + 1] = fDryGain * aSample[i * 4 + 1] + fWetGain * (fPrevConv + (m_apConvolution[uCh][i] - fPrevConv) * 0.5f);
                aSample[i * 4 + 2] = fDryGain * aSample[i * 4 + 2] + fWetGain * (fPrevConv + (m_apConvolution[uCh][i] - fPrevConv) * 0.75f);
                aSample[i * 4 + 3] = fDryGain * aSample[i * 4 + 3] + fWetGain * m_apConvolution[uCh][i];

                fPrevConv = m_apConvolution[uCh][i];
            }

            m_afLastConvolution[uCh] = fPrevConv;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <typename DataType, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uDownSample>
void Audio::Core::DSP::Convolution<DataType, uSampleRate, uFrameLength, uChannels, uDownSample>::SetImpulse(U32 uChannel, const Audio::Core::WaveData* pImpulse)
{
    if (m_apfImpulse[uChannel])
    {
        if (pImpulse->uType == Audio::Core::uPCM32BitFloat)
        {
            F32* pfImpulse = (F32*)pImpulse->pData;

            for (U32 uFrames = 0; uFrames < m_uNumFrames; uFrames++)
                Frame::Copy(&m_apfImpulse[uChannel][uFrames * uFrameLength], &pfImpulse[uFrames * uFrameLength]);
        }
        else if (pImpulse->uType == Audio::Core::uPCM16BitSignedInt)
        {
            S16* psImpulse = (S16*)pImpulse->pData;

            for (U32 uFrames = 0; uFrames < m_uNumFrames; uFrames++)
            {
                for (U32 uSample = 0; uSample < uFrameLength; uSample++)
                    m_apfImpulse[uChannel][uFrames * uFrameLength + uSample] = (F32)psImpulse[uFrames * uFrameLength + uSample] / 32768.f;
            }
        }
    }
}
*/
#endif // _DSPCONVOLUTION_H_
