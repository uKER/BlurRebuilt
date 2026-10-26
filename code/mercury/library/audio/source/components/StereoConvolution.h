#ifndef _STEREOCONVOLUTION_H_
#define _STEREOCONVOLUTION_H_
/*
namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoConvolution
////////////////////////////////////////////////////////////////////////////////
    class StereoConvolution audio_inherits_component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  wave server
            //WaveManager* pWaveManager;
            //  waves
			U32 strBank;
			U32 strWaveL;
			U32 strWaveR;
            U32 uImpulseLength;
		    ComOutHandle outDryGain;
            F32 fDryGain;
		    ComOutHandle outWetGain;
            F32 fWetGain;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        StereoConvolution(Object* pObject) : Component(pObject)
        {
            m_pData = NULL;
            m_pDSP = NULL;
            m_pImpulseL = NULL;
            m_pImpulseR = NULL;
        }

        void* Init(void* pData);
        void Update(U8* pObjContext);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "ConvolutionEffect"; }
        const char* Name();
        const bool IsA(U32 uFlags) { return (uFlags & (flagMono | flagTimeEffect)) != 0; }

        //const Audio::Core::DSP::IEffect* DSPEffect(U8* pObjContext) const { return m_pDSP->GetEffect(); }

    private:
        const Data* m_pData;

        Core::DSP::IConvolution* m_pDSP;

        Core::WaveData* m_pImpulseL;
        Core::WaveData* m_pImpulseR;
    };
}
*/
#endif // _STEREOCONVOLUTION_H_

