#ifndef _DSPLIPSYNC_H_
#define _DSPLIPSYNC_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)	// AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	LipSync
////////////////////////////////////////////////////////////////////////////////
		    class LipSync : public Effect
		    {
		    public:
				enum ParamIn
				{
					eBlendRMS,
					eBlendPitch,
					eSilenceThreshold,
					eClosedMouthWidth,
					eSibPostScale
				};

				enum ParamOut
				{
					eRMSOutput,
					ePitchOutput,
					eSibDifference
				};

				enum Buffers
				{
					Buffers_ham,
					Buffers_deri,
					Buffers_deri_x,
					Buffers_deri_f,
					Buffers_deri_Imag,
					Buffers_deri_Real,
					Buffers_Imag,
					Buffers_Real,
					Buffers_fft,
					Buffers_amp,
					Buffers_testBufferSorensen,
					Buffers_signal,
					
					Buffers_Max
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    LipSync( const char* pchInfo, U16 uChannels );
			    ~LipSync() {}
#endif

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
                {
					m_uBuffer = uSize;

					//U32 fftSize = 1024;
					//U32 staticBuffers = (U32)Buffers_Max;

                    //  delay buffer, length is 1 + num frames to delay
                    uSize += m_uChannels * uFrameLength * Buffers_Max;

					return uSize;
                }

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eBlendRMS:
						m_fblendRMS = fVal;
						break;

					case eBlendPitch:
						m_fblendPitch = fVal;
						break;

					case eSilenceThreshold:
						m_fsilenceThreshold = fVal;
						break;

					case eClosedMouthWidth:
						m_fclosedMouthWidth = fVal;
						break;

					case eSibPostScale:
						m_fSibPostScale = fVal;
						break;
					}
				}

				inline F32 GetParam(ParamOut eParam)
				{
					switch (eParam)
					{
					case eRMSOutput:
						return m_fRMSOutput;

					case ePitchOutput:
						return m_fPitchOutput;

					case eSibDifference:
						return m_fSibDifference;

					default:
						return 0.f;
					}
				}

				void GetPitchRMS(F32 *signal, F32 sampleRate,F32 *PitchOutput, F32 *RMSOutput);
				//void realfft_split(F32 *data,long n);

		    public:
          		F32 m_fprevRMS;
				F32 m_fprevOrigRMS;
				F32 m_fprevPitch;
				F32 m_fblendRMS;
				F32 m_fblendPitch;
				F32 m_fsilenceThreshold;
				F32 m_fclosedMouthWidth;
				F32 m_fprevSib;
				F32 m_fprevSibOutput;
				F32 m_fblendSib;
				F32 m_fsibThreshold;
				F32 m_fsibScale;
				F32 m_fpitchThreshold;
				F32 m_fSibPostScale;

				F32 m_ftotalSilence;

				F32 m_ffreqdiv;//

				F32 m_fcurve;
				F32 m_fscale;

				F32 m_fsibSmoothScale;
				F32 m_fsilSmoothScale;
				F32 m_fsilRMSSmoothScale;

				F32 m_fSibDiff;

				F32 m_fRMSOutput;
				F32 m_fPitchOutput;
				F32 m_fSibDifference;

				F32 m_fSubFrameRunningTotalRMS;		
				F32 m_fSubFrameRunningTotalPitch;	
				F32 m_fSubFrameRunningTotalSibDiff;

				bool  m_bhasSib;

				F32* hambuffer; 
				F32* deribuffer;
				F32* deri_x;
				F32* deri_f;
				F32* deri_Imag_buffer;
				F32* deri_Real_buffer;
				F32* Imag_buffer;
				F32* Real_buffer;

				F32* fftBuffer;
				F32* ampbuffer;
				F32* testBufferSorensen;

				F32* signalBuffer;
				U32 m_uBuffer;

				bool m_firstTimeProcessing;
		    };
	    }
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#endif // _DSPLIPSYNC_H_

