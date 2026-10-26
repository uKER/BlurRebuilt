#ifndef _CONVOLUTION_H_
#define _CONVOLUTION_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	IConvolution
////////////////////////////////////////////////////////////////////////////////
            class IConvolution
		    {
		    public:
                audio_virtual bool Process(void* pFrameData) = 0;

                audio_virtual Effect* GetEffect() = 0;
		    };

////////////////////////////////////////////////////////////////////////////////
//	Convolution
////////////////////////////////////////////////////////////////////////////////
            template <typename DataType, U32 uSampleRate, U32 uFrameLength, U32 uNumFrames, U16 uChannels>
		    class Convolution : public IGain, public Effect<DataType, uSampleRate, uFrameLength, uChannels>
		    {
		    public:
			    Convolution();
			    ~Convolution() {}

			    U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
                {
                    return uSize;
                }

                bool Process(void* pFrameData);

                Effect* GetEffect() { return (IEffect*)this; }

		    private:
                DataType* m_pConvolution;
		    };
	    };
	};
};

#include "convolution.cpp"

#endif // _CONVOLUTION_H_