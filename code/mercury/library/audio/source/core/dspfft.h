#ifndef _DSPFFT_H_
#define _DSPFFT_H_
/*
namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	FFT
////////////////////////////////////////////////////////////////////////////////
            template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
		    class FFT : public Effect<FloatEulor, uSampleRate, uFrameLength, uChannels>
		    {
		    public:
			    FFT();
			    ~FFT() {}

			    void Process(void* pFrameData);

                //IEffect* GetEffect()
                //{
                //    return (IEffect*)(Effect<FloatEulor, uSampleRate, uFrameLength, uChannels>*)this;
                //}

		    private:
                void Convert(F32* aSamples, U32 uSize);
		    };

////////////////////////////////////////////////////////////////////////////////
//	InverseFFT
////////////////////////////////////////////////////////////////////////////////
            template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
		    class InverseFFT : public Effect<FloatEulor, uSampleRate, uFrameLength, uChannels>
		    {
		    public:
			    InverseFFT();
			    ~InverseFFT() {}

			    void Process(void* pFrameData);

                //IEffect* GetEffect()
                //{
                //    return (IEffect*)(Effect<FloatEulor, uSampleRate, uFrameLength, uChannels>*)this;
                //}

		    private:
                void Convert(F32* aSamples, U32 uSize);
		    };
        };
	};
};

////////////////////////////////////////////////////////////////////////////////
//	FFT
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
Audio::Core::DSP::FFT<uSampleRate, uFrameLength, uChannels>::FFT()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
void Audio::Core::DSP::FFT<uSampleRate, uFrameLength, uChannels>::Process(void* pFrameData)
{
    //  Assert framelength 2^n
    //FloatEulor* aSample = (FloatEulor*)pFrameData;
    F32* aSample = (F32*)pFrameData;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        for (S32 sCount = uFrameLength - 1; sCount > -1; sCount--)
        {
            //  hanning window TODO: precompute
            F32 fHanning = 0.5f * (1.f - cosf(Audio::f2Pi * (F32)sCount / (F32)uFrameLength));
            aSample[2 * sCount] = fHanning * aSample[sCount];
            aSample[2 * sCount + 1] = 0.f;
        }

        Convert((F32*)&aSample[uCh * uFrameLength], uFrameLength * 2);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
void Audio::Core::DSP::FFT<uSampleRate, uFrameLength, uChannels>::Convert(F32* aSamples, U32 uSize)
{
    S32 i, j, k, i5, i6, i7, i8, i0, id, i1, i2, i3, i4, n2, n4, n8;
    F32 t1, t2, t3, t4, t5, t6, a3, ss1, ss3, cc1, cc3, a, e, sqrt2;

    sqrt2 = 1.4142135623730950488016887242097f;
    n4 = uSize - 1;

    //  shuffle
    for (i = 0, j = 0, n2 = uSize / 2; i < n4; i++)
    {
        if (i<j)
        {
            t1 = aSamples[j];
            aSamples[j] = aSamples[i];
            aSamples[i] = t1;
        }
        k = n2;
        while (k <= j)
        {
            j -= k;
            k >>= 1;	
        }
        j += k;
    }

    //  length two butterflies	
    i0 = 0;
    id = 4;
    do
    {
        for (; i0 < n4; i0 += id)
        { 
            i1 = i0 + 1;
            t1 = aSamples[i0];
            aSamples[i0] = t1 + aSamples[i1];
            aSamples[i1] = t1 - aSamples[i1];
        }
        id <<= 1;
        i0 = id - 2;
        id <<= 1;
    } while (i0 < n4);

    //  L shaped butterflies
    n2 = 2;
    for (k = uSize; k > 2; k >>= 1)
    {  
        n2 <<= 1;
        n4 = n2 >> 2;
        n8 = n2 >> 3;
        e = 2.f * Audio::fPi / n2;
        i1 = 0;
        id = n2 << 1;
        do
        { 
            for (; i1 < (S32)uSize; i1 += id)
            {
                i2 = i1 + n4;
                i3 = i2 + n4;
                i4 = i3 + n4;
                t1 = aSamples[i4] + aSamples[i3];
                aSamples[i4] -= aSamples[i3];
                aSamples[i3] = aSamples[i1] - t1;
                aSamples[i1] += t1;
                if (n4 != 1)
                {
                    i0 = i1 + n8;
                    i2 += n8;
                    i3 += n8;
                    i4 += n8;
                    t1 = (aSamples[i3] + aSamples[i4]) / sqrt2;
                    t2 = (aSamples[i3] - aSamples[i4]) / sqrt2;
                    aSamples[i4] = aSamples[i2] - t1;
                    aSamples[i3] = -aSamples[i2] - t1;
                    aSamples[i2] = aSamples[i0] - t2;
                    aSamples[i0] += t2;
                }
            }
            id <<= 1;
            i1 = id - n2;
            id <<= 1;
        } while (i1 < (S32)uSize);
        a = e;
        for (j = 2; j <= n8; j++)
        {  
            a3 = 3 * a;
            cc1 = cos(a);
            ss1 = sin(a);
            cc3 = cos(a3);
            ss3 = sin(a3);
            a = j * e;
            i = 0;
            id = n2 << 1;
            do
            {
                for (; i < (S32)uSize; i += id)
                {  
                    i1 = i + j - 1;
                    i2 = i1 + n4;
                    i3 = i2 + n4;
                    i4 = i3 + n4;
                    i5 = i + n4 - j + 1;
                    i6 = i5 + n4;
                    i7 = i6 + n4;
                    i8 = i7 + n4;
                    t1 = aSamples[i3] * cc1 + aSamples[i7] * ss1;
                    t2 = aSamples[i7] * cc1 - aSamples[i3] * ss1;
                    t3 = aSamples[i4] * cc3 + aSamples[i8] * ss3;
                    t4 = aSamples[i8] * cc3 - aSamples[i4] * ss3;
                    t5 = t1 + t3;
                    t6 = t2 + t4;
                    t3 = t1 - t3;
                    t4 = t2 - t4;
                    t2 = aSamples[i6] + t6;
                    aSamples[i3] = t6 - aSamples[i6];
                    aSamples[i8] = t2;
                    t2 = aSamples[i2] - t3;
                    aSamples[i7] = -aSamples[i2] - t3;
                    aSamples[i4] = t2;
                    t1 = aSamples[i1] + t5;
                    aSamples[i6] = aSamples[i1] - t5;
                    aSamples[i1] = t1;
                    t1 = aSamples[i5] + t4;
                    aSamples[i5] -= t4;
                    aSamples[i2] = t1;
                }
                id <<= 1;
                i = id - n2;
                id <<= 1;
            } while (i < (S32)uSize);
        }
    }

    //  division by array length
    for (i = 0; i < (S32)uSize; i++)
        aSamples[i] /= uSize;
}

////////////////////////////////////////////////////////////////////////////////
//	InverseFFT
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
Audio::Core::DSP::InverseFFT<uSampleRate, uFrameLength, uChannels>::InverseFFT()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
void Audio::Core::DSP::InverseFFT<uSampleRate, uFrameLength, uChannels>::Process(void* pFrameData)
{
    //  Assert framelength 2^n
    //FloatEulor* aSample = (FloatEulor*)pFrameData;
    F32* aSample = (F32*)pFrameData;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        Convert((F32*)&aSample[uCh * uFrameLength], uFrameLength * 2);

        for (U32 uCount = 0; uCount < uFrameLength; uCount++)
            aSample[uCount] = aSample[2 * uCount];
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
void Audio::Core::DSP::InverseFFT<uSampleRate, uFrameLength, uChannels>::Convert(F32* aSamples, U32 uSize)
{
    S32 i5, i6, i7, i8, id, i2, i3, i4, n2, n4;
    U32 i, i0, i1, n1, j, n8, k;
    F32 t1, t2, t3, t4, t5, a3, ss1, ss3, cc1, cc3, a, e, sqrt2;

    sqrt2 = 1.4142135623730950488016887242097f;

    n1 = uSize - 1;
    n2 = uSize << 1;
    for (k = uSize; k > 2; k >>= 1)
    {  
        id = n2;
        n2 >>= 1;
        n4 = n2 >> 2;
        n8 = n2 >> 3;
        e = 2.f * Audio::fPi / n2;
        i1 = 0;
        do
        { 
            for (; i1 < uSize; i1 += id)
            {
                i2 = i1 + n4;
                i3 = i2 + n4;
                i4 = i3 + n4;
                t1 = aSamples[i1] - aSamples[i3];
                aSamples[i1] += aSamples[i3];
                aSamples[i2] *= 2;
                aSamples[i3] = t1 - 2 * aSamples[i4];
                aSamples[i4] = t1 + 2 * aSamples[i4];
                if (n4 != 1)
                {
                    i0 = i1 + n8;
                    i2 += n8;
                    i3 += n8;
                    i4 += n8;
                    t1 =(aSamples[i2] - aSamples[i0]) / sqrt2;
                    t2 =(aSamples[i4] + aSamples[i3]) / sqrt2;
                    aSamples[i0] += aSamples[i2];
                    aSamples[i2] = aSamples[i4] - aSamples[i3];
                    aSamples[i3] = 2 * (-t2 - t1);
                    aSamples[i4] = 2 * (-t2 + t1);
                }
            }
            id <<= 1;
            i1 = id - n2;
            id <<= 1;
        } while (i1 < n1);
        a = e;
        for (j = 2; j <= n8; j++)
        {  
            a3 = 3 * a;
            cc1 = cos(a);
            ss1 = sin(a);
            cc3 = cos(a3);
            ss3 = sin(a3);
            a = j * e;
            i = 0;
            id = n2 << 1;
            do
            {
                for (; i < uSize; i += id)
                {  
                    i1 = i + j - 1;
                    i2 = i1 + n4;
                    i3 = i2 + n4;
                    i4 = i3 + n4;
                    i5 = i + n4 - j + 1;
                    i6 = i5 + n4;
                    i7 = i6 + n4;
                    i8 = i7 + n4;
                    t1 = aSamples[i1] - aSamples[i6];
                    aSamples[i1] += aSamples[i6];
                    t2 = aSamples[i5] - aSamples[i2];
                    aSamples[i5] += aSamples[i2];
                    t3 = aSamples[i8] + aSamples[i3];
                    aSamples[i6] = aSamples[i8] - aSamples[i3];
                    t4 = aSamples[i4] + aSamples[i7];
                    aSamples[i2] = aSamples[i4] - aSamples[i7];
                    t5 = t1 - t4;
                    t1 += t4;
                    t4 = t2 - t3;
                    t2 += t3;
                    aSamples[i3] = t5 * cc1 + t4 * ss1;
                    aSamples[i7] = -t4 * cc1 + t5 * ss1;
                    aSamples[i4] = t1 * cc3 - t2 * ss3;
                    aSamples[i8] = t2 * cc3 + t1 * ss3;
                }
                id <<= 1;
                i = id - n2;
                id <<= 1;
            } while (i < n1);
        }
    }	

    i0 = 0;
    id = 4;
    do
    {
        for (; i0 < n1; i0 += id)
        { 
            i1 = i0 + 1;
            t1 = aSamples[i0];
            aSamples[i0] = t1 + aSamples[i1];
            aSamples[i1] = t1 - aSamples[i1];
        }
        id <<= 1;
        i0 = id - 2;
        id <<= 1;
    } while (i0 < n1);

    //  aSamples shuffling
    for (i = 0, j = 0, n2 = uSize / 2; i < n1; i++)
    {
        if (i < j)
        {
            t1 = aSamples[j];
            aSamples[j] = aSamples[i];
            aSamples[i] = t1;
        }
        k = n2;
        while (k <= j)
        {
            j -= k;
            k >>= 1;	
        }
        j += k;
    }	
}
*/
#endif // _DSPFFT_H_

