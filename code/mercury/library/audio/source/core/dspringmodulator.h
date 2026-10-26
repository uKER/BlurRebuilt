#ifndef _DSPRingModulator_H_
#define _DSPRingModulator_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	RingModulator
////////////////////////////////////////////////////////////////////////////////
		    class RingModulator : public Effect
		    {
		    public:
				enum ParamIn
				{
					eRate,
					eWet,
					eDry,
				};

		    public:
				static bool bDisableAll;
				static const U32 sMaxChannels = 8;

		    public:
			    RingModulator( const char* pchInfo, U16 uChannels );

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				void Sync( bool bPhaseLocked )
				{
					m_bSync = true;
					m_bPhaseLocked = bPhaseLocked;
				}

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eRate:
						m_fDelta = 2.f * Maths::Sin( Maths::fPi * Maths::Min( fVal, fSampleRate / 6.f ) / fSampleRate );
						break;
					case eWet:
						m_fWet = fVal;
						break;
					case eDry:
						m_fDry = fVal;
						break;
					}
				}

		    private:
				F32 m_fAngle;

				F32 m_afSin[ sMaxChannels ];
				F32 m_afCos[ sMaxChannels ];

                F32 m_fDelta;
                F32 m_fWet;
                F32 m_fDry;

				bool m_bSync;
				bool m_bPhaseLocked;
			};
	    }
	}
}

#endif // _DSPRingModulator_H_

