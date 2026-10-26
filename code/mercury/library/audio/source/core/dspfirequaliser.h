#ifndef _DSPFIREQUALISER_H_
#define _DSPFIREQUALISER_H_
/*
namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	FIREqualiser
////////////////////////////////////////////////////////////////////////////////
            template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
		    class FIREqualiser : public IFIREqualiser, public Effect<FloatEulor, uSampleRate, uFrameLength, uChannels>
		    {
		    public:
			    FIREqualiser();
			    ~FIREqualiser() {}

			    void Process(void* pFrameData);

                void SetGain(U32 uBand, F32 fGain)
                {
                    //  Assert (uBand < uFrameLength / 2)

                    m_afGain[uBand] = fGain;
                }

                void SetGains(F32* afGains)
                {
                    memcpy(m_afGain, afGains, uFrameLength / 2 * sizeof(F32));
                }

                //IEffect* GetEffect() { return (IEffect*)this; }

		    private:
			    F32 m_afGain[uFrameLength / 2];
		    };
	    };
	};
};

////////////////////////////////////////////////////////////////////////////////
//	FIREqualiser
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
Audio::Core::DSP::FIREqualiser<uSampleRate, uFrameLength, uChannels>::FIREqualiser()
{
	m_eType = Core::DSP::Effect::eFIREqualiser;

    F32 fLo = 0.f;//(F32)uFrameLength / 340.f;
    F32 fHi = 20.f;//(F32)uFrameLength / 4.1f;

    U32 uBand;
    for (uBand = 0; uBand < uFrameLength / 2; uBand++)
    {
        F32 fGain;
        fGain = 1.f;//(F32)uBand / (F32)(uFrameLength / 2);

        //if (uBand < fLo)
        //    fGain = 0.f;
        if (uBand > fHi)
            fGain = 0.f;

      	m_afGain[uBand] = fGain;
      	//m_afGain[uFrameLength - 1 - uBand] = fGain;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U32 uSampleRate, U32 uFrameLength, U16 uChannels>
void Audio::Core::DSP::FIREqualiser<uSampleRate, uFrameLength, uChannels>::Process(void* pFrameData)
{
    F32 afGain[uFrameLength / 2];

    memcpy(afGain, m_afGain, uFrameLength / 2 * sizeof(F32));

    FloatEulor* aSample = (FloatEulor*)pFrameData;

    U32 uEnd = uFrameLength - 1;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        for (U32 uBand = 0; uBand < uFrameLength / 2; uBand++)
        {
            U32 uOffsetStart = uCh * uFrameLength + uBand;
            U32 uOffsetEnd = uEnd - uOffsetStart;
      	    F32 fGain = afGain[uBand];

            aSample[uOffsetStart].fX *= fGain;
            aSample[uOffsetStart].fY *= fGain;
            aSample[uOffsetEnd].fX *= fGain;
            aSample[uOffsetEnd].fY *= fGain;
        }
    }
}
*/
#endif // _DSPFIREQUALISER_H_

