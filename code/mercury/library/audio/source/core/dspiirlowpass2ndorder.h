#ifndef _DSPIIRLOWPASS2NDORDER_H_
#define _DSPIIRLOWPASS2NDORDER_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IIRLowPass2ndOrder
////////////////////////////////////////////////////////////////////////////////
		    class IIRLowPass2ndOrder : public Effect
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
			    IIRLowPass2ndOrder( const char* pchInfo, U16 uChannels );
			    ~IIRLowPass2ndOrder() { }
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
						m_fClamp = Maths::Max( fVal, 1.f );
						break;
					}
				}

		    public://private:
				F32 m_fTargetCutoff;

                F32 m_fCutoff;
                F32 m_fQ;
                F32 m_fGain;
				F32 m_fClamp;

                F32 m_fPrevCutoff;
                F32 m_fPrevQ;
                F32 m_fPrevGain;

                F32 m_fA1;
                F32 m_fA2;
                F32 m_fB0;
                F32 m_fB1;

                F32 m_fPrevIn0[6];
                F32 m_fPrevOut0[6];
                F32 m_fPrevIn1[6];
                F32 m_fPrevOut1[6];
            };
	    };
	};
};

#endif // _DSPIIRLOWPASS2NDORDER_H_

