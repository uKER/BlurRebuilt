#ifndef _DSPCLIPPER_H_
#define _DSPCLIPPER_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Clipper
////////////////////////////////////////////////////////////////////////////////
            template <U16 uChannels>
		    class Clipper : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLevel,
				};

		    public:
				static bool bDisableAll;

		    public:
			    Clipper();
			    ~Clipper() {}

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eLevel:
						m_fLevel = Maths::Max(fVal, 0.f);
						break;
					}
				}

		    private:
                F32 m_fLevel;

                F32 m_fPrevLevel;
		    };
	    }
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Clipper
////////////////////////////////////////////////////////////////////////////////
template <U16 uChannels>
Audio::Core::DSP::Clipper<uChannels>::Clipper()
{
	m_eType = Core::DSP::Effect::eClipper;

	m_fLevel = 1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <U16 uChannels>
bool Audio::Core::DSP::Clipper<uChannels>::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	if (bProcess)
	{
		U32 uIndex = 0;

		F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

		//	constant clip level
		if (m_fLevel == m_fPrevLevel)
		{

			//  each channel
			for (U32 uChannel = 0; uChannel < uChannels; uChannel++)
			{
				//	each sample
				for (U32 uSample = 0; uSample < uFrameLength; uSample++)
				{
					F32 fIn = Maths::Abs(aSample[uIndex]);
					F32 fSgn = Maths::Select(aSample[uIndex], 1.f, -1.f);
					F32 fOut = Maths::Select(m_fLevel - fIn, fIn, fIn * Maths::RcpEst(1.f + fIn - m_fLevel));

					aSample[uIndex] = fSgn * fOut;

					uIndex++;
				}
			}
		}
		//	ramp clip level across frame
		else
		{
			F32 fDelta = (m_fLevel - m_fPrevLevel) * Core::fRcpFrameLength;

			//  each channel
			for (U32 uChannel = 0; uChannel < uChannels; uChannel++)
			{
				F32 fLevel = m_fPrevLevel;

				for (U32 uSample = 0; uSample < uFrameLength; uSample++)
				{
					F32 fIn = Maths::Abs(aSample[uIndex]);
					F32 fSgn = Maths::Select(aSample[uIndex], 1.f, -1.f);
					F32 fOut = Maths::Select(fLevel - fIn, fIn, fIn * Maths::RcpEst(1.f + fIn - fLevel));

					aSample[uIndex] = fSgn * fOut;

					fLevel += fDelta;

					uIndex++;
				}
			}
		}
	}

    return true;
}

#endif // _DSPCLIPPER_H_

