#ifndef _OVERLAPMIXER_H_
#define _OVERLAPMIXER_H_
/*
namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	DelayLine
////////////////////////////////////////////////////////////////////////////////
        template <U32 uInputs, U32 uChannelsIn, U32 uChannelsOut, U32 uEffects>
		class OverlapMixer : public Mixer<uInputs, uChannelsIn, uChannelsOut, uEffects>
		{
		public:
            OverlapMixer() {}
			audio_virtual ~OverlapMixer() {}

            void Process(U32 uFrameCount);

            bool SetEffect(U32 uEffect, Audio::Core::DSP::Effect* pEffect)
            {
                return Generator<uChannelsOut, uEffects>::SetEffect(uEffect, pEffect);
            }

            bool SetInput(U32 uInput, Audio::Core::IGenerator* pInput)
            {
                return Mixer<uInputs, uChannelsIn, uChannelsOut, uEffects>::SetInput(uInput, pInput);
            }

        private:
			F32* m_pPrevFrame;
			//F32* m_pPrevFrame;
		};
    };
};

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_PS3
template <U32 uInputs, U32 uChannelsIn, U32 uChannelsOut, U32 uEffects>
void Audio::Core::OverlapMixer<uInputs, uChannelsIn, uChannelsOut, uEffects>::Process(U32 uFrameCount)
{
	if (Generator<uChannelsOut, uEffects>::m_uFrameCount != uFrameCount)
	{
        F32* pFrameData = (F32*)Generator<uChannelsOut, uEffects>::m_pFrameData;

        //  for each overlap buffers
        for (U32 uShift = 1; uShift > 0; uShift--)
        {
	        //	for each output channel
	        for (U32 uChOut = 0; uChOut < uChannelsOut; uChOut++)
	        {
                F32* pfFrom = &pFrameData[uFrameLength * uChannelsOut * (uDelay - 1) + uFrameLength * uChOut];
                F32* pfTo = &pFrameData[uFrameLength * uChannelsOut * uDelay + uFrameLength * uChOut];

                Frame::Copy(pfTo, pfFrom);
            }
        }

        Mixer<uInputs, uChannelsIn, uChannelsOut, uEffects>::Process(uFrameCount);
    }
}
#endif
*/
#endif // _OVERLAPMIXER_H_

