#ifndef _DSPEFFECT_H_
#define _DSPEFFECT_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Effect
////////////////////////////////////////////////////////////////////////////////
            class Effect
		    {
			public:
				static void* operator new(size_t sz, void* pHere) { return pHere; };
				static void operator delete(void* ptr) { };

			public:
				enum Type
				{
					eUndefined = -1,
					eGain,
					ePeakLevel,
					eRMSLevel,
					eIIRLowPass1stOrder,
					eIIRLowPass2ndOrder,
					eIIRLowPass4thOrder,
					eIIRHighPass1stOrder,
					eIIRHighPass2ndOrder,
					eIIRPeakEQ,
					eIIRBandPass,
					eIIRLowShelf,
					eIIRHighShelf,
					eDelay,
					eSaturator,
					eConvolution,
					eLipSync,
					ePhaser,
					eClipper,
					eCompressor,
					eFeedbackDelay,

					ePitchShift,
					eSpectralReverb,
					eSpectralEnvelope,
					eFlanger,
					eBitCrusher,
					eRingModulator,
					eOscilloscope,
					eSend,

					eNumEffectTypes
				};

		    public:
			    Effect( const char* pchInfo, U16 uChannels )
				{
				    m_pchInfo = pchInfo;
					m_uChannels = uChannels;
					m_bActive = true;
				}
			    audio_virtual ~Effect() {}

				inline const Type GetType() const { return m_eType; }

				void SetActive( bool bActive ) { m_bActive = bActive; }

		   protected:
#ifdef PLATFORM_SPU
				U32 m_uPad;
#endif
				Type m_eType;
				U16 m_uChannels;
				bool m_bActive;

			protected:
				const char* m_pchInfo;
            };

			struct Element
			{
				Effect::Type eType;
				U32 uOffset;
			};
	    };
	};
};

#endif // _DSPEFFECT_H_

