#ifndef _DSPIIRHIGHSHELF_H_
#define _DSPIIRHIGHSHELF_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IIRHighShelf
//		Cutoff = 20Hz to 1/2 nyquist
//		Slope = (flat) 0 to 1 (steep)
//		Gain (shelf) = -48dB to +12dB
////////////////////////////////////////////////////////////////////////////////
		    class IIRHighShelf : public Effect
		    {
		    public:
				enum ParamIn
				{
					eCutoff,
					eSlope,
					eGain,
					eClamp
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    IIRHighShelf( const char* pchInfo, U16 uChannels );
			    ~IIRHighShelf() { }
#endif

                bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eCutoff:
						m_fTargetCutoff = Maths::Clamp( fVal, 20.f, (F32)( uSampleRate / 2 ) );
						break;

					case eSlope:
						m_fSlope = Maths::Clamp( fVal, Maths::fEpsilon, 1.f );
						break;

					case eGain:
						m_fGain = Maths::Clamp( fVal, Maths::fMinus48dB, 4.f );
						break;

					case eClamp:
						m_fClamp = Maths::Min( fVal, 1.f );
						break;
					}
				}

			public://private:
				F32 m_fTargetCutoff;

                F32 m_fCutoff;
				F32 m_fSlope;
                F32 m_fGain;
                F32 m_fClamp;

                F32 m_fPrevCutoff;
				F32 m_fPrevSlope;
                F32 m_fPrevGain;

                F32 m_fA1;
                F32 m_fA2;
                F32 m_fB0;
                F32 m_fB1;
                F32 m_fB2;

                F32 m_fIn1[6];
                F32 m_fOut1[6];
                F32 m_fIn2[6];
                F32 m_fOut2[6];
            };
	    };
	};
};

#endif // _DSPIIRHIGHSHELF_H_

