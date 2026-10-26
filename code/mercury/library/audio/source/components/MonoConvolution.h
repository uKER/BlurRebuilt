#ifndef _MONOCONVOLUTION_H_
#define _MONOCONVOLUTION_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoConvolution
////////////////////////////////////////////////////////////////////////////////
    class MonoConvolution : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  the impulse
            U32 uImpulseCom;
            //  length
            U32 uImpulseLength;
            //  dry gain
		    S32 sDryGainCom;
		    U32 uDryGainOut;
            F32 fDryGainVal;
            //  wet gain
		    S32 sWetGainCom;
		    U32 uWetGainOut;
            F32 fWetGainVal;
        };

        struct State
        {
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoConvolution(Object* pObject) : Component(pObject)
        {
//            m_pDSP = NULL;
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MonoConvolution"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoConvolution); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Enable") == 0)
				return 0;
			if (strcmp(pchMessage, "Bypass") == 0)
				return 1;
			else
				return -1;
		}
        //const Audio::Core::DSP::IEffect* DSPEffect(U8* pStateData) const { return m_pDSP->GetEffect(); }

    private:
        //Core::DSP::Convolution<F32, 48000, 256, 1, 1>* m_pDSP;
    };
}

#endif // _MONOCONVOLUTION_H_

