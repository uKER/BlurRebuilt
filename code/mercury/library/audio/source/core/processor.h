#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_
/*
namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Processor
////////////////////////////////////////////////////////////////////////////////
		template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U32 uChannels, U32 uEffects>
		class Processor : public IProcessor, public Transformer<DataTypeOut, uSampleRate, uFrameLength, 1, uChannels, uEffects>
		{
		public:
			Processor()
			{
				m_apInputs[0] = NULL;
			}
			audio_virtual ~Processor() {}

			void Process(U32 uFrameCount);

            U32 MemoryUse(U32 uSize)
            {
                return Transformer<DataTypeOut, uSampleRate, uFrameLength, 1, uChannels, uEffects>::MemoryUse(uSize);
            }

            void* Allocate(void* pBuffer)
            {
                return Transformer<DataTypeOut, uSampleRate, uFrameLength, 1, uChannels, uEffects>::Allocate(pBuffer);
            }

            bool SetEffect(U32 uEffect, Audio::Core::DSP::Effect* pEffect)
            {
                return Generator<DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::SetEffect(uEffect, pEffect);
            }

            bool SetInput(Audio::Core::IGenerator* pInput)
            {
                return Transformer<DataTypeOut, uSampleRate, uFrameLength, 1, uChannels, uEffects>::SetInput(0, pInput);
            }

            //  allows access to base class from interface
            IGenerator* Output() { return (IGenerator*)this; }
		};
	};
};

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U32 uChannels, U32 uEffects>
void Audio::Core::Processor<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::Process(U32 uFrameCount)
{
	if (m_uFrameCount != uFrameCount)
	{
        m_apInputs[0]->Process(uFrameCount);

		DataTypeOut* pOutputFrame = (DataTypeOut*)m_pFrameData;

        //  if a valid input
        if (m_apInputs[0])
        {
			DataTypeIn* pInputFrame = (DataTypeIn*)m_apInputs[0]->FrameData();

			//	do frame
			for (U32 uCh = 0; uCh < uChannels; uCh++)
			{
                U32 uChOffset = uCh * uFrameLength;

				//	for each sample
                for (U32 uSample = 0; uSample < uFrameLength; uSample++)
    				pOutputFrame[uChOffset + uSample] = pInputFrame[uChOffset + uSample];
			}
        }

        //  process effects
        Generator<DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::Process(uFrameCount);
	}
}
*/
#endif // _PROCESSOR_H_