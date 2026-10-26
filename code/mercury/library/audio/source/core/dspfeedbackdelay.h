#ifndef _DSPFEEDBACKDELAY_H_
#define _DSPFEEDBACKDELAY_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	FeedbackDelay
////////////////////////////////////////////////////////////////////////////////
		    class FeedbackDelay : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLength,
					eFeedback,
					eWet,
					eDry,
				};

		    public:
				static bool bDisableAll;

		    public:
			    FeedbackDelay( const char* pchInfo, U16 uChannels, U32 u32NumFrames );
				~FeedbackDelay() {}

			    U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
                {
					m_uBuffer = uSize;

                    //  delay buffer, length is 1 + num frames to delay
                    uSize += m_uChannels * (m_u32NumFrames + 1) * uFrameLength;

					return uSize;
                }
				U32 TrailFrames( const ProcessInfo& info )
				{
					//	TODO: generate suitable value
					return 0;
				}

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eLength:
						{
							S32 sLength = (S32)( fVal * fSampleRate * fRcpFrameLength );
							m_uLength = (U32)Maths::Clamp( sLength, 1, (S32)m_u32NumFrames - 1 );
						}
						break;

					case eFeedback:
						m_fFeedback = Maths::Clamp( fVal, -1.f, 1.f );
						break;

					case eWet:
						m_fWet = Maths::Max( fVal, 0.f );
						break;

					case eDry:
						m_fDry = Maths::Max( fVal, 0.f );
						break;
					}
				}

		    private:
                U32 m_uBuffer;

                F32 m_fFeedback;
                F32 m_fWet;
                F32 m_fDry;

                U32 m_uLength;

                U32 m_uPrevLength;

                U32 m_uCurr;

				U32 m_u32NumFrames;

				U32 m_uPrevFrameNum;
		    };
	    }
	}
}

#endif // _DSPFEEDBACKDELAY_H_

