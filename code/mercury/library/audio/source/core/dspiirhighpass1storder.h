#ifndef _DSPIIRHIGHPASS1STORDER_H_
#define _DSPIIRHIGHPASS1STORDER_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IIRHighPass1stOrder
////////////////////////////////////////////////////////////////////////////////
		    class IIRHighPass1stOrder : public Effect
		    {
		    public:
				enum ParamIn
				{
					eCutoff,
					eGain
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    IIRHighPass1stOrder( const char* pchInfo, U16 uChannels );
			    ~IIRHighPass1stOrder() { }
#endif

                bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eCutoff:
						m_fTargetCutoff = Maths::Clamp( fVal, 20.f, (F32)uSampleRate );
						break;

					case eGain:
						m_fGain = Maths::Max( fVal, 0.f );
						break;
					}
				}

		    public://private:
				F32 m_fTargetCutoff;

                F32 m_fCutoff;
                F32 m_fGain;

                F32 m_fPrevCutoff;

                F32 m_fA;
                F32 m_fB;

//                #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1 ) // JASOND
                F32 m_fIn1[6];
                F32 m_fOut1[6];
 //               #else
                Maths::Vector4 m_vIn_00_01_02_03[ 6 ];
                Maths::Vector4 m_vOut_00_01_02_03[ 6 ];
 //               #endif
            };
	    };
	};
};

#endif // _DSPIIRHIGHPASS1STORDER_H_

