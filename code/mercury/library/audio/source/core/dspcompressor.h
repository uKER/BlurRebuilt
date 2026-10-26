#ifndef _DSPCOMPRESSOR_H_
#define _DSPCOMPRESSOR_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Compressor
////////////////////////////////////////////////////////////////////////////////
		    class Compressor : public Effect
		    {
		    public:
				enum ParamIn
				{
					eAttack,
					eRelease,
					eThreshold,
					eRatio,
					eGain,
					eSideChainRMS,
					eSideChainEnabled
				};

				enum ParamOut
				{
					eAttenuation
				};

		    public:
				static bool bDisableAll;

		    public:
			    Compressor( const char* pchInfo, U16 uChannels );
			    ~Compressor() { }

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( const ParamIn eParam, F32 fVal )
				{
					switch (eParam)
					{
					case eAttack:
						m_fRcpAttack = fFrameLength / ( Maths::Max( fVal, 0.001f ) * Core::fSampleRate );	//	needs hi-q
						break;

					case eRelease:
						m_fRcpRelease = fFrameLength / ( Maths::Max( fVal, 0.01f ) * Core::fSampleRate );	//	needs hi-q
						break;

					case eThreshold:
						m_fThreshold = Maths::Clamp( fVal, 0.f, 1.f );
						break;

					case eRatio:
						m_fRcpRatio = Maths::Clamp( Maths::RcpEst( fVal ), 0.f, 1.f );
						break;

					case eGain:
						m_fGain = Maths::Max( fVal, 0.f );
						break;

					case eSideChainRMS:
						m_fRMS = Maths::Clamp( fVal, 0.f, 1.f );
						break;

					case eSideChainEnabled:
						m_bSideChainEnabled = fVal != 0.f ? true : false;
						break;
					}
				}

				inline F32 GetParam( ParamOut eParam )
				{
					switch ( eParam )
					{
					case eAttenuation:
						return m_fComp;

					default:
						return 0.f;
					}
				}

				//inline void SetAttackParam( F32 fVal ) { m_fAttack = Maths::Max( fVal, 0.001f ); }
				//inline void SetReleaseParam( F32 fVal ) { m_fRelease = Maths::Max( fVal, 0.1f ); }
				//inline void SetThresholdParam( F32 fVal ) { m_fThreshold = Maths::Clamp( fVal, 0.f, 1.f ); }
				//inline void SetRatioParam( F32 fVal ) { m_fRatio = Maths::Min( fVal, 0.f ); }
				//inline void SetGainParam( F32 fVal ) { m_fGain = Maths::Max( fVal, 0.f ); }
				//inline void SetSideChainRMSParam( F32 fVal ) { m_fRMS = Maths::Clamp( fVal, 0.f, 1.f ); }
				//inline void SetSideChainEnabledParam( bool bVal ) { m_bSideChainEnabled = bVal; }

				//inline const F32 GetAttenuationParam( bool bVal ) const { return m_fComp; }

		    public: //private:
                F32 m_fA;
                F32 m_fB;
                F32 m_fPrev;
				F32 m_fMean[ 6 ];
				F32 m_fRMS;

				F32 m_fSideChainRMS;
				F32 m_fRcpAttack;
			    F32 m_fRcpRelease;
                F32 m_fThreshold;
			    F32 m_fRcpRatio;
                F32 m_fGain;

                F32 m_fComp;

                F32 m_fPrevComp;

				bool m_bSideChainEnabled;
			};
	    }
	}
}

#endif // _DSPCOMPRESSOR_H_

