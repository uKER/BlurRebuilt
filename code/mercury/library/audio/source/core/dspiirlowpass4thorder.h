#ifndef _DSPIIRLOWPASS4THORDER_H_
#define _DSPIIRLOWPASS2NDORDER_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IIRLowPass4thOrder
////////////////////////////////////////////////////////////////////////////////
		    class IIRLowPass4thOrder : public Effect
		    {
		    public:
				enum ParamIn
				{
					eCutoff,
					eQ,
					eGain,
					eClamp
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    IIRLowPass4thOrder( const char* pchInfo, U16 uChannels );
			    ~IIRLowPass4thOrder() { }
#endif

                bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eCutoff:
						m_fTargetCutoff = Maths::Clamp( fVal, 20.f, (F32)uSampleRate );
						break;

					case eQ:
						m_fQ = Maths::Clamp( fVal, 0.f, 1.f );
						break;

					case eGain:
						m_fGain = Maths::Max( fVal, 0.f );
						break;

					case eClamp:
						m_fClamp = Maths::Min( fVal, 1.f );
						break;
					}
				}

		    private:
				F32 m_fTargetCutoff;

                F32 m_fCutoff;
                F32 m_fQ;
                F32 m_fGain;
				F32 m_fClamp;

                F32 m_fPrevCutoff;
                F32 m_fPrevQ;

				F32 m_fP;
				F32 m_fR;
				F32 m_fK;

				F32 m_fY1[8];
				F32 m_fY2[8];
				F32 m_fY3[8];
				F32 m_fY4[8];
				F32 m_fPrevX[8];
				F32 m_fPrevY1[8];
				F32 m_fPrevY2[8];
				F32 m_fPrevY3[8];
            };
	    };
	};
};

#endif // _DSPIIRLOWPASS2NDORDER_H_

