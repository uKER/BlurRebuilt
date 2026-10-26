#ifndef _DSPSpectralEnvelope_H_
#define _DSPSpectralEnvelope_H_

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
//	SpectralEnvelope
////////////////////////////////////////////////////////////////////////////////
		    class SpectralEnvelope : public Effect
		    {
		    public:
				//enum ParamIn
				//{
				//	eBlendRMS,
				//	eBlendPitch,
				//	eSilenceThreshold,
				//	eClosedMouthWidth,
				//	eSibPostScale
				//};

				enum ParamOut
				{
					eBin1,
					eBin2,
					eBin3,
					eBin4,
					eBin5,
					eBin6,
					eBin7,
					eBin8,
					eBin9,
					eBin10,
					eBin11,
					eBin12,
					eBin13,
					eBin14,
					eBin15,
					eBin16,
				};

				enum StaticBuffers
				{
					Buffers_Window,
					Buffers_Spectral,

					Buffers_Max
				};

			/*	enum DynamicBuffers
				{
					DynamicBuffers_Phase,
					DynamicBuffers_Mag,

					DynamicBuffers_Max
				};*/

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    SpectralEnvelope( const char* pchInfo, U16 uChannels );
			    ~SpectralEnvelope();
#endif

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				//void DynamicBufferDepths( const ProcessInfo& info, BufferStack& buffers )
				//{
				//	m_uDynamicBuffer = buffers.GetDepth();	//	at this stage just an index

				//	AUDIO_CORE_DEBUG( "SpectralReverb::DynamicBufferDepths() framebuffers index: %d [%s]", m_uDynamicBuffer, m_pchInfo );

				//	//	TODO: replace 0 with number of frames required
				//	U32 fftSize = uFrameLength;
				//	
				//	U32 uSize = 1;
				//	buffers.SetSize( buffers.GetSize() > uSize ? buffers.GetSize() : uSize );
				//}
				//void DynamicBufferOffsets( const ProcessInfo& info, BufferStack& buffers )
				//{
				//	m_uDynamicBuffer = uFrameLength * buffers.GetOffset( m_uDynamicBuffer );
				//}

				//U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
    //            {
				//	m_uBuffer = uSize;

				//	U32 fftSize = uFrameLength;
				//	//U32 staticBuffers = (U32)Buffers_Max;

    //                //  delay buffer, length is 1 + num frames to delay
    //                uSize += m_uChannels * fftSize * sizeof(F32);

				//	return uSize;
    //            }

				U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
                {
					m_uBuffer = uSize;

					U32 fftSize = uFrameLength;
					//U32 staticBuffers = (U32)Buffers_Max;

                    //  delay buffer, length is 1 + num frames to delay
                    uSize += m_uChannels * fftSize * Buffers_Max;

					return uSize;
                }

				//inline void SetParam(ParamIn eParam, F32 fVal)
				//{
				//	//switch (eParam)
				//	//{

				//	//}
				//}

				inline F32 GetParam(ParamOut eParam)
				{
					switch (eParam)
					{
					case eBin1:
						return m_fBin1_Magnitude;

					case eBin2:
						return m_fBin2_Magnitude;

					case eBin3:
						return m_fBin3_Magnitude;

					case eBin4:
						return m_fBin4_Magnitude;

					case eBin5:
						return m_fBin5_Magnitude;

					case eBin6:
						return m_fBin6_Magnitude;

					case eBin7:
						return m_fBin7_Magnitude;

					case eBin8:
						return m_fBin8_Magnitude;

					case eBin9:
						return m_fBin9_Magnitude;

					case eBin10:
						return m_fBin10_Magnitude;

					case eBin11:
						return m_fBin11_Magnitude;

					case eBin12:
						return m_fBin12_Magnitude;

					case eBin13:
						return m_fBin13_Magnitude;

					case eBin14:
						return m_fBin14_Magnitude;

					case eBin15:
						return m_fBin15_Magnitude;

					case eBin16:
						return m_fBin16_Magnitude;


					default:
						return 0.f;
					}
				}

				void * aligned_memory_alloc(void * pa, size_t size, size_t alignment );  
				void RectangularToPolarConversionNewFormat(F32 * signal, int fftSize);
				//void realfft_split(F32 *data,long n);

		    public:
				F32 m_fBin1_Magnitude;
				F32 m_fBin2_Magnitude;
				F32 m_fBin3_Magnitude;
				F32 m_fBin4_Magnitude;
				F32 m_fBin5_Magnitude;
				F32 m_fBin6_Magnitude;
				F32 m_fBin7_Magnitude;
				F32 m_fBin8_Magnitude;
				F32 m_fBin9_Magnitude;
				F32 m_fBin10_Magnitude;
				F32 m_fBin11_Magnitude;
				F32 m_fBin12_Magnitude;
				F32 m_fBin13_Magnitude;
				F32 m_fBin14_Magnitude;
				F32 m_fBin15_Magnitude;
				F32 m_fBin16_Magnitude;

			//	F32  *m_phaseBuffer;
			//	F32  *m_magBuffer;
				int m_PVSize;
				U16 m_uChannels;

				F32  *m_windowBuffer;
				F32  *m_spectralBuffer;
			//	void* m_windowBuffer_Unaligned;
				S32 m_fftSize;
				S32 frameLength;
				bool m_firstTimeProcessing;
				U32 m_uBuffer;
				U32 m_uDynamicBuffer;

		    };
	    }
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#endif // _DSPSpectralEnvelope_H_

