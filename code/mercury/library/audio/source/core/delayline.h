#ifndef _DELAYLINE_H_
#define _DELAYLINE_H_
/*
namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	DelayLine
////////////////////////////////////////////////////////////////////////////////
		template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uOverlaps, U32 uEffects>
		class DelayLine : public IDelayLine, public Processor<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength * (uOverlaps + 1), uChannels, uEffects>
		{
		public:
            DelayLine()
            {
                m_uCurrentBuffer = 0;

                for (U32 uOverlap = 0; uOverlap < uOverlaps; uOverlap++)
                    m_apBufferedData[uOverlaps] = NULL;
            }
			audio_virtual ~DelayLine() {}

            void Process(U32 uFrameCount);

            bool SetEffect(U32 uEffect, Audio::Core::DSP::Effect* pEffect)
            {
                return Generator<DataTypeOut, uSampleRate, uFrameLength * (uOverlaps + 1), uChannels, uEffects>::SetEffect(uEffect, pEffect);
            }

            bool SetInput(Audio::Core::IGenerator* pInput)
            {
                return Processor<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength * (uOverlaps + 1), uChannels, uEffects>::SetInput(pInput);
            }

        private:
            U32 m_uCurrentBuffer;
            void* m_apBufferedData[uOverlaps];

		};

////////////////////////////////////////////////////////////////////////////////
//	DelayBlend
////////////////////////////////////////////////////////////////////////////////
		template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uOverlaps, U32 uEffects>
		class DelayBlend : public IDelayBlend, public Processor<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>
		{
		public:
            DelayBlend()
            {
                m_pFrameBuffer = NULL;
            }
			audio_virtual ~DelayBlend() {}

            void Process(U32 uFrameCount);

            bool SetEffect(U32 uEffect, Audio::Core::DSP::Effect* pEffect)
            {
                return Generator<DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::SetEffect(uEffect, pEffect);
            }

            bool SetInput(Audio::Core::IGenerator* pInput)
            {
                return Processor<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::SetInput(pInput);
            }

        private:
            void* m_pFrameBuffer;

		};
    };
};

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uOverlaps, U32 uEffects>
void Audio::Core::DelayLine<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength, uChannels, uOverlaps, uEffects>::Process(U32 uFrameCount)
{
	if (m_uFrameCount != uFrameCount)
	{
        //  if a valid input
        if (m_apInputs[0])
        {
            m_apInputs[0]->Process(uFrameCount);

		    DataTypeIn* pInputFrame = (DataTypeIn*)m_apInputs[0]->FrameData();
		    DataTypeOut* pOutputFrame = (DataTypeOut*)m_pFrameData;

			for (U32 uOverlap = 0; uOverlap < uOverlaps; uOverlap++)
			{
                DataTypeIn* pBufferedFrame = (DataTypeIn*)m_apBufferedData[m_uCurrentBuffer];

				//	do buffered frames
				for (U32 uCh = 0; uCh < uChannels; uCh++)
				{
                    U32 uChOffset = uCh * uFrameLength;

					//	for each sample
                    for (U32 uSample = 0; uSample < uFrameLength; uSample++)
    					pOutputFrame[uChOffset + uSample] = pBufferedFrame[uChOffset + uSample];
				}

                pOutputFrame += uChannels * uFrameLength;

                m_uCurrentBuffer++;
                if (m_uCurrentBuffer == uOverlaps)
                    m_uCurrentBuffer = 0;
            }

			//	do current frame
			for (U32 uCh = 0; uCh < uChannels; uCh++)
			{
                U32 uChOffset = uCh * uFrameLength;

				//	for each sample
                for (U32 uSample = 0; uSample < uFrameLength; uSample++)
    				pOutputFrame[uChOffset + uSample] = pInputFrame[uChOffset + uSample];
			}

            //  copy current frame to prev
            memcpy(m_apBufferedData[m_uCurrentBuffer], pInputFrame, uChannels * uFrameLength * sizeof(DataTypeIn));

            m_uCurrentBuffer++;
            if (m_uCurrentBuffer == uOverlaps)
                m_uCurrentBuffer = 0;
        }
 
        //  process effects
        Generator<DataTypeOut, uSampleRate, uFrameLength * (uOverlaps + 1), uChannels, uEffects>::Process(uFrameCount);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
template <typename DataTypeIn, typename DataTypeOut, U32 uSampleRate, U32 uFrameLength, U16 uChannels, U32 uOverlaps, U32 uEffects>
void Audio::Core::DelayBlend<DataTypeIn, DataTypeOut, uSampleRate, uFrameLength, uChannels, uOverlaps, uEffects>::Process(U32 uFrameCount)
{
	if (m_uFrameCount != uFrameCount)
	{
        //  if a valid input
        if (m_apInputs[0])
        {
            m_apInputs[0]->Process(uFrameCount);

		    DataTypeIn* pInputFrame = (DataTypeIn*)m_apInputs[0]->FrameData();
		    DataTypeOut* pOutputFrame = (DataTypeOut*)m_pFrameData;

			for (U32 uOverlap = 0; uOverlap < uOverlaps; uOverlap++)
			{
                DataTypeIn* pBufferedFrame = (DataTypeIn*)m_pFrameBuffer;

                F32 fSlice = (F32)uFrameLength / (F32)uOverlaps;

                F32 fGainStep = 1.f / fSlice;

				//	do buffered frames
				for (U32 uCh = 0; uCh < uChannels; uCh++)
				{
                    U32 uChOffset = uCh * uFrameLength;

                    if (uOverlap == 0)
                    {
                        F32 fGain = 1.f;

					    //	for each sample
                        for (U32 uSample = 0; uSample < (U32)fSlice; uSample++)
                        {
    					    pOutputFrame[uChOffset + uSample] = fGain * pBufferedFrame[uFrameLength + uChOffset + uSample];
                            fGain -= fGainStep;
                        }
                    }
                    else
                    {
                        F32 fGain = 0.f;

					    //	for each sample
                        for (U32 uSample = (U32)(fSlice * uOverlap); uSample < (U32)(fSlice * (1 + uOverlap)); uSample++)
                        {
    					    pOutputFrame[uChOffset + uSample] += fGain * pBufferedFrame[uChOffset + uSample];
                            fGain += fGainStep;
                        }
                        for (U32 uSample = (U32)(fSlice * (1 + uOverlap)); uSample < (U32)(fSlice * (2 + uOverlap)); uSample++)
                        {
    					    pOutputFrame[uChOffset + uSample] += fGain * pBufferedFrame[uChOffset + uSample];
                            fGain -= fGainStep;
                        }
                    }
                    if (uOverlap == uOverlaps - 1)
                    {
                        F32 fGain = 0.f;

					    //	for each sample
                        for (U32 uSample = uFrameLength - (U32)fSlice; uSample < uFrameLength; uSample++)
                        {
    					    pOutputFrame[uChOffset + uSample] += fGain * pInputFrame[uChOffset + uSample];
                            fGain += fGainStep;
                        }
                    }
				}

                pOutputFrame += uChannels * uFrameLength;
            }

            //  copy current frame to buffer
            memcpy(m_pFrameBuffer, pInputFrame, (uOverlaps + 1) * uChannels * uFrameLength * sizeof(DataTypeIn));
        }

        //  process effects
        Generator<DataTypeOut, uSampleRate, uFrameLength, uChannels, uEffects>::Process(uFrameCount);
	}
}
*/
#endif // _DELAYLINE_H_

