#ifndef _DSPIIRPEAKEQ_H_
#define _DSPIIRPEAKEQ_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IIRPeakEQ
////////////////////////////////////////////////////////////////////////////////
		    class IIRPeakEQ : public Effect
		    {
		    public:
				enum ParamIn
				{
					eCentre,
					eQ,
					eGain,
					eClamp
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    IIRPeakEQ( const char* pchInfo, U16 uChannels );
			    ~IIRPeakEQ() {}
#endif

                bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch (eParam)
					{
					case eCentre:
						m_fCentre = Maths::Clamp( fVal, 20.f, fNyquist );
						break;

					case eQ:
						m_fQ = Maths::Clamp( fVal, 0.f, 1.f );
						break;

					case eGain:
						m_fGain = Maths::Max( fVal, Maths::fEpsilon );
						break;

					case eClamp:
						m_fClamp = Maths::Max( fVal, 1.f );
						break;
					}
				}

		    private:
                F32 m_fCentre;
				F32 m_fQ;
                F32 m_fGain;
                F32 m_fClamp;

                F32 m_fPrevCentre;
				F32 m_fPrevQ;
                F32 m_fPrevGain;

                F32 m_fA1;
                F32 m_fA2;
                F32 m_fB0;
                F32 m_fB1;

                F32 m_fIn1[6];
                F32 m_fOut1[6];
                F32 m_fIn2[6];
                F32 m_fOut2[6];
            };
	    };
	};
};

#endif // _DSPIIRPEAKEQ_H_

