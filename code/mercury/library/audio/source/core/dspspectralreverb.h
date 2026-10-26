#ifndef _dspspectralreverb_H_
#define _dspspectralreverb_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)	// AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#define USING_STATIC_BUFFERS_FOR_DYNAMIC

#define USING_MIDFI_REVERB

#ifdef USING_HIFI_REVERB
#define FFTSIZE 4096
#endif

#ifdef USING_MIDFI_REVERB
#define FFTSIZE 1024
#endif

#ifdef USING_LOFI_REVERB
#define FFTSIZE 256
#endif



enum WindowType
{
	E_WINDOWTYPE_RECTANGULAR,
	E_WINDOWTYPE_HAMMING,
	E_WINDOWTYPE_HANN,

	E_WINDOWTYPE_TOTAL
};

struct ReverbParameters
{
	ReverbParameters()
	{

		sampleRate = 48000;
		fftSize = FFTSIZE;
		hopSize = 4;
		frameSize = 256;
		roomLevel = .6;
		dryGain = 1;
		decayValue = 4;
		lowShelfCutoffFreq = 200;
		lowShelfGain = 1;
		highShelfCutoffFreq = 10000;
		highShelfGain = 1;
		channels = 1;
	
	}

	F32 sampleRate;
	int fftSize;
	int hopSize;
	int frameSize;
	F32 roomLevel;
	F32 dryGain;
	F32 decayValue;
	F32 lowShelfCutoffFreq;
	F32 lowShelfGain;
	F32 highShelfCutoffFreq;
	F32 highShelfGain;
	F32 channels;

};


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	SpectralReverb
////////////////////////////////////////////////////////////////////////////////
		    class SpectralReverb : public Effect
		    {
		    public:
				enum ParamIn
				{
					eBand1DecayTime,
					eBand2DecayTime,
					eBand3DecayTime,
					eBand4DecayTime,
					eBand5DecayTime,
					eBand6DecayTime,

					eMasterDecayTime,

					eReverbGain,

					eSmoothFilter,

					eSmoothingRatio,
					ePreset,
					eHighPassCutoffFreq,
					eHighPassAttenuation,
					eLowPassCutoffFreq,
					eLowPassAttenuation,
				};

				enum ParamOut
				{
					/*eRMSOutput,
					ePitchOutput,
					eSibDifference*/
				};

				enum Buffers
				{
					Buffers_Queue,
					Buffers_Window,
					Buffers_Sum,
					Buffers_Reverb,
					Buffers_Hann,
					

#ifdef USING_STATIC_BUFFERS_FOR_DYNAMIC
					Buffers_Phase,
					Buffers_Mag,
					
					Buffers_Real,
					Buffers_Imag,
					Buffers_Temp,
#endif

					Buffers_Filter,


				//	Buffers_Preset,
					//Buffers_Preset_Tunnel_1,
					//Buffers_Preset_Yard_1,

					//Buffers_Preset_Flat,
					//Buffers_Preset_Hall_2,
					//Buffers_Preset_Bathroom_1,



					Buffers_Max
				};

				enum DynamicBuffers
				{
					DynamicBuffers_Phase,
					DynamicBuffers_Mag,
					
					DynamicBuffers_Real,
					DynamicBuffers_Imag,

					DynamicBuffers_Temp,
					DynamicBuffers_Window,

					DynamicBuffers_Max
				};

				enum E_ReverbPresets
				{
					E_ReverbPreset_Hangar_1,
					E_ReverbPreset_Tunnel_1,
					E_ReverbPreset_Yard_1,


					//E_ReverbPreset_Flat,

					//E_ReverbPreset_Hall_2,

					//E_ReverbPreset_Bathroom_1,


					E_ReverbPreset_Max
				};

		    public:
				static bool bDisableAll;

		    public:
//#ifndef PLATFORM_SPU
			    SpectralReverb( const char* pchInfo, U16 uChannels );
			    ~SpectralReverb();
//#endif

				U32 DynamicBuffersRequired( const ProcessInfo& )
				{
					#ifdef USING_STATIC_BUFFERS_FOR_DYNAMIC
						return 0;
					#else
						return m_uChannels * DynamicBuffers_Max * 4;
					#endif
				}
				void SetDynamicBuffer( const ProcessInfo& info, U32 uOffset )
				{
					m_uDynamicBuffer = uOffset;
				}
				U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
                {
					m_uBuffer = uSize;

					U32 fftSize = FFTSIZE;
					//U32 staticBuffers = (U32)Buffers_Max;

                    //  delay buffer, length is 1 + num frames to delay
                    uSize += m_uChannels * fftSize * Buffers_Max;

					return uSize;
                }
				U32 TrailFrames( const ProcessInfo& info )
				{
					return (U32)(m_MasterDecayTime * fSampleRate * fRcpFrameLength);
				}

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
//#ifndef PLATFORM_SPU
					switch (eParam)
					{

					//case eBand1DecayTime:
					//	m_Band1DecayTime = fVal;
					//	m_Band1RecursiveDecay = CalculateRecursiveDecay(m_Band1DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					//case eBand2DecayTime:
					//	m_Band2DecayTime = fVal;
					//	m_Band2RecursiveDecay = CalculateRecursiveDecay(m_Band2DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					//case eBand3DecayTime:
					//	m_Band3DecayTime = fVal;
					//	m_Band3RecursiveDecay = CalculateRecursiveDecay(m_Band3DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					//case eBand4DecayTime:
					//	m_Band4DecayTime = fVal;
					//	m_Band4RecursiveDecay = CalculateRecursiveDecay(m_Band4DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					//case eBand5DecayTime:
					//	m_Band5DecayTime = fVal;
					//	m_Band5RecursiveDecay = CalculateRecursiveDecay(m_Band5DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					//case eBand6DecayTime:
					//	m_Band6DecayTime = fVal;
					//	m_Band6RecursiveDecay = CalculateRecursiveDecay(m_Band6DecayTime*m_MasterDecayTime,m_reverbParams);
					//	break;

					case eMasterDecayTime:
						m_MasterDecayTime = fVal;
						//m_Band1RecursiveDecay = CalculateRecursiveDecay(m_Band1DecayTime*m_MasterDecayTime,m_reverbParams);
						//m_Band2RecursiveDecay = CalculateRecursiveDecay(m_Band2DecayTime*m_MasterDecayTime,m_reverbParams);
						//m_Band3RecursiveDecay = CalculateRecursiveDecay(m_Band3DecayTime*m_MasterDecayTime,m_reverbParams);
						//m_Band4RecursiveDecay = CalculateRecursiveDecay(m_Band4DecayTime*m_MasterDecayTime,m_reverbParams);
						//m_Band5RecursiveDecay = CalculateRecursiveDecay(m_Band5DecayTime*m_MasterDecayTime,m_reverbParams);
						//m_Band6RecursiveDecay = CalculateRecursiveDecay(m_Band6DecayTime*m_MasterDecayTime,m_reverbParams);

						break;

					case eReverbGain:
						m_ReverbGain = fVal;
						break;

					//case eSmoothFilter:
					//	m_SmoothFilter = (F32)fVal;
					//	break;

					case eSmoothingRatio:
						m_presetSmoothingRatio = (F32)fVal;
						break;

					case ePreset:
						m_preset = (S32)fVal;
						break;

					case eHighPassCutoffFreq:
						m_presetHighPassCutoffFreq = static_cast<S32>(fVal);
						
						break;

					case eHighPassAttenuation:
						m_presetHighPassAttenuation = fVal;
						break;

					case eLowPassCutoffFreq:
						m_presetLowPassCutoffFreq = static_cast<S32>(fVal);
						break;

					case eLowPassAttenuation:
						m_presetLowPassAttenuation = fVal;
						break;

					default:
						{
						break;
						}
					}
//#endif	//	#ifndef PLATFORM_SPU
				}

				inline F32 GetParam(ParamOut eParam)
				{
					switch (eParam)
					{
						case 1:
							break;
					default:
						return 0.f;
					}
				}

				void RectangularToPolarConversion(F32 * signal, int frameSize);
				void PolarToRectangularConversion(F32 * signal, int frameSize);
				void RectangularToPolarConversionNewFormat(F32 * signal, int frameSize);
				void PolarToRectangularConversionNewFormat(F32 * signal, int frameSize);
				void Window(F32 * signal,int frameSize,WindowType eWindow,F32 fAmpGain = 1);
				inline F32 CalculateRecursiveDecay(F32 reverbTime,ReverbParameters reverbParameters)
				{
					F32 attenuationValue = 1;
					//formula for attenuation value is 10 pow (-3*framesize/hopsize)/decayTime*sr)
					F32 aboveLine = -3.f*(reverbParameters.fftSize/reverbParameters.hopSize);
					F32 belowLine = reverbTime * reverbParameters.sampleRate;
					F32 result = aboveLine/belowLine;
					attenuationValue = pow(10,result);

					return attenuationValue;
				}

				F32 randDouble(F32 low, F32 high);
				void MagnitudeDecay(F32 * signal,int frameSize,ReverbParameters reverbParameters);
				void MagnitudeDecayNewFormat(F32 * signal,int frameSize,ReverbParameters reverbParameters);
				void * aligned_memory_alloc(void * pa, size_t size, size_t alignment );  
				inline int fastrand();
				inline void fast_srand( int seed );

				void PreparePresetBuffers(E_ReverbPresets currentPreset);
				void DoPresetSmoothing();
				void DoPresetHighPassFiltering();
				void DoPresetLowPassFiltering();

		    public:
				F32 m_Band1DecayTime;
				F32 m_Band2DecayTime;
				F32 m_Band3DecayTime;
				F32 m_Band4DecayTime;
				F32 m_Band5DecayTime;
				F32 m_Band6DecayTime;

				F32 m_MasterDecayTime;
				F32 m_prevMasterDecayTime;

				F32 m_ReverbGain;

				F32 m_SmoothFilter;

				F32 m_prevGain;

				F32  *m_queueBuffer;
				F32  *m_windowBuffer;
				F32  *m_sumBuffer;
				F32  *m_pReverbBuffer;
				F32  *m_pTempBuffer;
				F32  *m_pHannBuffer;

				F32  *m_phaseBuffer;
				F32  *m_magBuffer;

				F32  *m_realBuffer;
				F32  *m_imagBuffer;

			//	F32 *m_pPresetBuffer;

				F32  *m_pFilterBuffer;

				int m_PVSize;

				S32 m_hopSize;
				S32 m_hopSamples;

				
				ReverbParameters m_reverbParams;
				S32 m_fftSize;

				F32 m_Band1RecursiveDecay;
				F32 m_Band2RecursiveDecay;
				F32 m_Band3RecursiveDecay;
				F32 m_Band4RecursiveDecay;
				F32 m_Band5RecursiveDecay;
				F32 m_Band6RecursiveDecay; 

				F32 roomDecayMax;
				F32 roomDecayTimeOverride;
				F32 roomDecayTime;

				U32 m_uBuffer;
				U32 m_uDynamicBuffer;
				bool m_firstTimeProcessing;

				S32 m_presetHighPassCutoffFreq ;
				S32 m_presetLowPassCutoffFreq ;
				F32 m_presetHighPassAttenuation ;
				F32 m_presetLowPassAttenuation  ;
				F32 m_presetSmoothingRatio;
				S32 m_preset;

				S32 m_prevPresetHighPassCutoffFreq;
				S32 m_prevPresetLowPassCutoffFreq;
				F32 m_prevPresetHighPassAttenuation;
				F32 m_prevPresetLowPassAttenuation;
				F32 m_prevPresetSmoothingRatio;
				S32 m_prevPreset;
				S32 m_queueBufferReadIndex;
				void *m_PresetPointer[E_ReverbPreset_Max];


		    };
	    }
	}
}
#endif // _dspspectralreverb_H_

