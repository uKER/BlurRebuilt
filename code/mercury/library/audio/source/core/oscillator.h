#ifndef _OSCILLATOR_H_
#define _OSCILLATOR_H_
/*
namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Oscillator
////////////////////////////////////////////////////////////////////////////////
		class Oscillator : public IOscillator, public Generator
		{
		public:
			Oscillator();
			Oscillator(const char* pchInfo);
			audio_virtual ~Oscillator() {}

			bool Process(const ProcessInfo& info, bool bProcess);

            void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL) {}

		public:
            //  oscillator operations
			inline const F32 Frequency() const { return m_fFreq; }
			inline const F32 Phase() const { return m_fPhase; }

            inline bool SetFrequency(F32 fFreq)
            {
                m_fFreq = fFreq;
                return true;
            }

            inline bool SetPhase(F32 fPhase)
            {
                m_fPhase = fPhase;
                return true;
            }

        private:
			F32 m_fFreq;
			F32 m_fPhase;
		};
	};
};

////////////////////////////////////////////////////////////////////////////////
//	Oscillator
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Oscillator::Oscillator() : Generator(NULL)
{
	Generator<uEffects, uOutputs>::m_eType = eOscillator;

    m_fPhase = 0.f;
    m_fFreq = 440.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Oscillator
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Oscillator::Oscillator(const char* pchInfo) : Generator(pchInfo)
{
    m_fPhase = 0.f;
    m_fFreq = 440.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Oscillator::Process(const ProcessInfo& info, bool bProcess)
{
	if (Generator<uEffects, uOutputs>::m_uFrameCount != info.uFrameNum)
	{
        F32 fVolume = (Generator<uEffects, uOutputs>::m_uFrameCount & 32) ? 1.f : 1.f;

        F32 fPhase;

		F32 fTimeStep = Maths::f2Pi * m_fFreq * fRcpSampleRate;

		F32* AUDIO_RESTRICT pfData = Generator<uEffects, uOutputs>::m_pfFrameData;

		U32 uOffset = 0;

		for (U32 uCh = 0; uCh < Generator<uEffects, uOutputs>::m_u32NumChannels; uCh++)
		{
            fPhase = m_fPhase;

		    for (U32 uCount = 0; uCount < uFrameLength; uCount++)
		    {
				F32 fValue = fVolume * Maths::Sin(fPhase);

				pfData[uOffset] = fValue;

                fPhase += fTimeStep;

                if (fPhase > Maths::fPi)
                    fPhase -= Maths::f2Pi;

				uOffset++;
			}
		}

        m_fPhase = fPhase;

        //  process effects
        //Generator<uChannelsOut, uEffects, uOutputs>::Process(info, bProcess);
		Generator<uEffects, uOutputs>::ProcessEffects(pfData, info, bProcess);

		m_fFreq = 440.f + 110.f * Maths::Sin(0.25f * Maths::f2Pi * (F32)Generator<uEffects, uOutputs>::m_uFrameCount / 60.f);
	}

    return true;
}
*/
#endif // _OSCILLATOR_H_

