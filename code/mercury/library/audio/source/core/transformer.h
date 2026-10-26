#ifndef _TRANSFORMER_H_
#define _TRANSFORMER_H_
/*
namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Transformer
////////////////////////////////////////////////////////////////////////////////
		template <U32 uInputs, U32 uChannelsOut, U32 uEffects>
		class Transformer : public Generator<uChannelsOut, uEffects>
		{
		public:
			Transformer();
			audio_virtual ~Transformer() {}

			U32 MemoryUse(U32 uSize);
			void* Allocate(void* pBuffer);

			audio_virtual bool Process(U32 uFrameCount) = 0;

			bool SetInput(U32 uInput, Audio::Core::IGenerator* pInput);

		protected:
			//	array of inputs
            IGenerator* m_apInputs[uInputs];
		};
	};
};

////////////////////////////////////////////////////////////////////////////////
//	Transformer
////////////////////////////////////////////////////////////////////////////////
template <U32 uInputs, U32 uChannelsOut, U32 uEffects>
Audio::Core::Transformer<uInputs, uChannelsOut, uEffects>::Transformer()
{
	for (U32 uCount = 0; uCount < uInputs; uCount++)
		m_apInputs[uCount] = NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	MemoryUse
////////////////////////////////////////////////////////////////////////////////
template <U32 uInputs, U32 uChannelsOut, U32 uEffects>
U32 Audio::Core::Transformer<uInputs, uChannelsOut, uEffects>::MemoryUse(U32 uSize)
{
    if (!m_bDynamicBuffMem)
    {
	    for (U32 uInput = 0; uInput < uInputs; uInput++)
	    {
            if (m_apInputs[uInput])
                uSize = m_apInputs[uInput]->MemoryUse(uSize);
	    }
    }

    return Generator<uChannelsOut, uEffects>::MemoryUse(uSize);
}

////////////////////////////////////////////////////////////////////////////////
//	Allocate
////////////////////////////////////////////////////////////////////////////////
template <U32 uInputs, U32 uChannelsOut, U32 uEffects>
void* Audio::Core::Transformer<uInputs, uChannelsOut, uEffects>::Allocate(void* pBuffer)
{
    Assert(pBuffer != NULL);

    //  TODO: verify all effect's datatype/srate/length/ch match

    if (m_pFrameData == NULL)
    {
	    for (U32 uInput = 0; uInput < uInputs; uInput++)
	    {
            if (m_apInputs[uInput])
			    pBuffer = m_apInputs[uInput]->Allocate(pBuffer);
	    }
    }

    return Generator<uChannelsOut, uEffects>::Allocate(pBuffer);
}

////////////////////////////////////////////////////////////////////////////////
//	SetInput
////////////////////////////////////////////////////////////////////////////////
template <U32 uInputs, U32 uChannelsOut, U32 uEffects>
bool Audio::Core::Transformer<uInputs, uChannelsOut, uEffects>::SetInput(U32 uInput, Audio::Core::IGenerator* pInput)
{
	if(uInput < uInputs)
	{
        m_apInputs[uInput] = pInput;
		return true;
	}
	else
		return false;
}
*/
#endif // _TRANSFORMER_H_

