#ifndef _SURROUNDLOWPASS_H_
#define _SURROUNDLOWPASS_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundLowPass
////////////////////////////////////////////////////////////////////////////////
    class SurroundLowPass : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optCutoff; 
            Option optPitchShift; 
            Option optFreqShift; 
            Option optGain; 
            Option optLevel; 
        };

        struct State
        {
            F32 fCutoff;
            F32 fLevel;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        SurroundLowPass(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "SurroundLowPass"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundLowPass); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eIIRLowPass1stOrder; }

        static void DefaultData(Data* pData)
        {
            pData->optCutoff.ctrl.sCom = -1; 
            pData->optCutoff.ctrl.uOut = 0; 
            pData->optCutoff.fVal = 440.f; 
            pData->optPitchShift.ctrl.sCom = -1;
            pData->optPitchShift.ctrl.uOut = 0; 
            pData->optPitchShift.fVal = 0.f; 
            pData->optFreqShift.ctrl.sCom = -1; 
            pData->optFreqShift.ctrl.uOut = 0; 
            pData->optFreqShift.fVal = 1.f; 
            pData->optGain.ctrl.sCom = -1; 
            pData->optGain.ctrl.uOut = 0; 
            pData->optGain.fVal = 0.f; 
            pData->optLevel.ctrl.sCom = -1; 
            pData->optLevel.ctrl.uOut = 0; 
            pData->optLevel.fVal = 1.f; 
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

    private:
		ixCore m_uDSP;
    };
}

#endif // _SURROUNDLOWPASS_H_

