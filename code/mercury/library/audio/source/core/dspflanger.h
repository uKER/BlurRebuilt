#ifndef _DSPFLANGER_H_
#define _DSPFLANGER_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Flanger
////////////////////////////////////////////////////////////////////////////////
		    class Flanger : public Effect
		    {
		    public:
				enum ParamIn
				{
					eDelay,
					eDepth,
					eRate,
					eFeedback,
					eWet,
					eDry,
				};

		    public:
				static bool bDisableAll;
				static const U32 sMaxChannels = 8;

				class LinearSRC
				{
				public:
					LinearSRC( ) : m_fHistory( 0.f ) { }

					void Resample( F32* afDst, U32 uOutSize, F32 fDry, const F32* afSrc, U32 uInSize, F32 fWet );
					void Resample( F32* afDst, U32 uOutSize, const F32* afSrc, U32 uInSize );

				private:
					inline F32 Interpolate( F32 fSampleA, F32 fSampleB, F32 fFraction )
					{
						return fSampleA + fFraction * ( fSampleB - fSampleA );
					}

				private:
					F32 m_fHistory;
				};

		    public:
			    Flanger( const char* pchInfo, U16 uChannels, U32 uNumFrames );

				U32 DynamicBuffersRequired( const ProcessInfo& )
				{
					//	the effect reuses this buffer for each channel
					return 1;
				}

				void SetDynamicBuffer( const ProcessInfo& info, U32 uOffset )
				{
					m_uScratchBuffer = uOffset;
				}

				U32 StaticBufferOffsets( const ProcessInfo& info, U32 uSize )
                {
					m_uDelayBuffer = uSize;

					//  delay buffer, length is num frames to delay
                    uSize += m_uChannels * m_uBufferLength;

                    return uSize;
                }

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				void Reset() { m_bReset = true; }

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eDelay:
						m_fDelay = 0.001f * Maths::Max( fVal, 0.f ); // delay is in ms
						break;
					case eDepth:
						m_fDepth = 0.001f * Maths::Max( fVal, 0.f ); // depth is in ms
						break;
					case eRate:
						m_fDelta = Maths::Max( fVal, 0.f ) * Maths::f2Pi * Core::fFrameLength * Core::fRcpSampleRate; // delta is angle step per frame
						break;
					case eFeedback:
						m_fFeedback = Maths::Clamp( fVal, -1.f, 1.f );
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
				void Read( const ProcessInfo& info, F32* pFrameBuffer, F32* pfDelayBuffer, F32 fGain );
				void Write( const ProcessInfo& info, LinearSRC& src, const F32* pFrameBuffer, F32* pfDelayBuffer, U32 uWriteIndex, U32 uTargetLength );

		    private:
				// delay static buffer index
                U32 m_uDelayBuffer;

				// scratch dynamic buffer index
                U32 m_uScratchBuffer;

				// length of delay buffer in samples
				U32 m_uBufferLength;

				// current angle
			    F32 m_fAngle;
			    F32 m_fPhaseShift;

                F32 m_fDry;
                F32 m_fWet;
                F32 m_fDelay;
                F32 m_fDepth;
                F32 m_fDelta;
				F32 m_fFeedback;

				// read/write sample indices into delay buffer
                U32 m_uRead;
			    U32 m_uWrite[ sMaxChannels ];
				LinearSRC m_src[ sMaxChannels ];

				bool m_bReset;
		    };
	    };
	};
};

#endif // _DSPFLANGER_H_
