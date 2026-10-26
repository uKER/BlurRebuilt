#ifndef _DSPOSCILLOSCOPE_H_
#define _DSPOSCILLOSCOPE_H_


#define AUDIO_OSC_STEP_SIZE 25
#define AUDIO_OSC_NUM_OUTPUTS 24

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
			//template< class T >
			//class TripleBuffer
			//{
			//public:
			//	TripleBuffer()
			//	{
			//		m_uRead = 0;
			//		m_uWrite = 1;
			//	}

			//	T& PeekRead() { return m_data[ m_uRead ]; }
			//	T& PeekWrite() { return m_data[ m_uWrite ]; }

			//	bool IncRead()
			//	{
			//		U32 uNext = m_uRead + 1;

			//		if( uNext == 3 )
			//			uNext = 0;

			//		if( uNext != m_uWrite )
			//			m_uRead = uNext;
			//	}
			//	bool IncWrite()
			//	{
			//		U32 uNext = m_uWrite + 1;

			//		if( uNext == 3 )
			//			uNext = 0;

			//		if( uNext != m_uRead )
			//			m_uWrite = uNext;
			//	}

			//private:
			//	U32 m_uRead;
			//	U32 m_uWrite;
			//	T m_data[ 3 ];
			//};

		    class Oscilloscope : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLength,
					eLevel
				};

				enum ParamOut
				{
					eHigh,
					eLow,
				};

			public:
				static bool bDisableAll;
				static F32 fMinHigh;
				static F32 fMaxLow;

		    public:
			    Oscilloscope( const char* pchInfo, U16 uChannels );
			    ~Oscilloscope() {}

                bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eLength:	//	in ms
						//m_uLength = (U32)( Maths::Max( fVal * 0.001f * fSampleRate, 1.f ) );
						break;

					case eLevel:
						m_fLevel = fVal;
						break;
					}
				}

				inline void Reset() { m_bReset = true; }

				inline F32 GetParam( ParamOut eParam, S32 sIndex = 0 )
				{
					switch( eParam )
					{
					case eHigh:
						{
							Assert( sIndex >= 0 && sIndex < AUDIO_OSC_NUM_OUTPUTS );

							S32 sBin = m_sCurr + sIndex + 1;	//	+1 as curr bin is reset to 0

							if( sBin >= AUDIO_OSC_STEP_SIZE )
								sBin -= AUDIO_OSC_STEP_SIZE;

							return m_afHigh[ sBin ];
						}
						break;

					case eLow:
						{
							Assert( sIndex >= 0 && sIndex < AUDIO_OSC_NUM_OUTPUTS );

							S32 sBin = m_sCurr + sIndex + 1;	//	+1 as curr bin is reset to 0

							if( sBin >= AUDIO_OSC_STEP_SIZE )
								sBin -= AUDIO_OSC_STEP_SIZE;

							return m_afLow[ sBin ];
						}
						break;

					default:
						return 0.f;
					}
				}

				inline void GetHighs( F32* afData, U32 uLength )
				{
#ifndef AUDIO_DISABLE_DSP
					Assert( uLength <= AUDIO_OSC_STEP_SIZE );

					S32 sStart = m_sCurr + 1;
					if( sStart == AUDIO_OSC_STEP_SIZE )
						sStart = 0;
					
					if( sStart + uLength > AUDIO_OSC_STEP_SIZE )
					{
						U32 uHalf = AUDIO_OSC_STEP_SIZE - sStart;
						memcpy( afData, &m_afHigh[ sStart ], uHalf * sizeof(F32) );
						memcpy( &afData[ uHalf ], m_afHigh, ( uLength - uHalf ) * sizeof(F32) );
					}
					else
					{
						memcpy( afData, &m_afHigh[ sStart ], uLength * sizeof(F32) );
					}
#endif
				}

				inline void GetLows( F32* afData, U32 uLength )
				{
#ifndef AUDIO_DISABLE_DSP
					Assert( uLength <= AUDIO_OSC_STEP_SIZE );

					S32 sStart = m_sCurr + 1;
					if( sStart == AUDIO_OSC_STEP_SIZE )
						sStart = 0;

					if( sStart + uLength > AUDIO_OSC_STEP_SIZE )
					{
						U32 uHalf = AUDIO_OSC_STEP_SIZE - sStart;
						memcpy( afData, &m_afLow[ sStart ], uHalf * sizeof(F32) );
						memcpy( &afData[ uHalf ], m_afLow, ( uLength - uHalf ) * sizeof(F32) );
					}
					else
					{
						memcpy( afData, &m_afLow[ sStart ], uLength * sizeof(F32) );
					}
#endif
				}

			private:
				bool m_bReset;
				U32 m_uLength;
				F32 m_fLevel;
				U32 m_uCount;
				S32 m_sCurr;
                F32 m_afHigh[ AUDIO_OSC_STEP_SIZE ];
                F32 m_afLow[ AUDIO_OSC_STEP_SIZE ];
			};
	    };
	};
};

#endif // _DSPOSCILLOSCOPE_H_

