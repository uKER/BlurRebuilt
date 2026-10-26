#ifndef _DSPDELAY_H_
#define _DSPDELAY_H_

//#define AUDIO_DELAY_DOWNSAMPLE_2

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Delay
////////////////////////////////////////////////////////////////////////////////
		    class Delay : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLength,
					eWet,
					eDry,
				};

		    public:
				static bool bDisableAll;

		    public:
			    Delay( const char* pchInfo, U16 uChannels, U32 uNumFrames, U32 uDownSample );

			    U32 StaticBufferOffsets( const ProcessInfo& info, U32 uSize )
                {
					m_uBuffer = uSize;

					//  delay buffer, length is num frames to delay
                    uSize += m_uChannels * m_uBufferLength;

                    return uSize;
                }
				U32 TrailFrames( const ProcessInfo& info )
				{
					return (U32)((F32)m_sCurrentLength * fRcpFrameLength);
				}

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				void Reset() { m_bReset = true; }

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eLength:
						m_fTargetLength = Maths::Max( fVal, 0.f );
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
				void Resample( F32* afDst, U32 uOutSize, F32 fDry, F32* afSrc, U32 uInSize, F32 fWet, F32 fPrev );

		    private:
				// downsample buffer num octaves
				U32 m_uDownSample;

				// index for delay buffer base
                U32 m_uBuffer;

				// length of delay buffer in samples
				U32 m_uBufferLength;

				// delay length in samples
                F32 m_fTargetLength;

				// delay length in samples
			    S32 m_sCurrentLength;

                F32 m_fPrevSample;

                F32 m_fWet;
                F32 m_fDry;

				// read/write sample indices into delay buffer
                U32 m_uRead;
			    U32 m_uWrite;

				F32 m_fPrevIn;
				F32 m_fPrevOut;

				bool m_bReset;
		    };
		}
	}
}

#endif // _DSPDELAY_H_

