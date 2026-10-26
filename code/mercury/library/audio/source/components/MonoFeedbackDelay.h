#ifndef _MONOFEEDBACKDELAY_H_
#define _MONOFEEDBACKDELAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoFeedbackDelay
////////////////////////////////////////////////////////////////////////////////
    class MonoFeedbackDelay : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			AudioFloat fMaxLength;
            Option optLength;
            Option optFeedback;
            Option optWetGain;
            Option optWetLevel;
            Option optDryGain;
            Option optDryLevel;
        };

        struct State
        {
            F32 fLength;
            F32 fFeedback;
            F32 fWet;
            F32 fDry;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoFeedbackDelay(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        void Update(Instance* pInstance);
        void DoMessage(Instance* pInstance, U32 uMessage);

#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return StaticGetNumMessages(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticGetNumMessages() { return NumMessages; }
#ifndef PLATFORM_SPU
        const char* Type() const { return "MonoFeedbackDelay"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoFeedbackDelay); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eFeedbackDelay; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->fMaxLength = 0.5f;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 1.f;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 0.25f;
            pData->optFeedback.ctrl.sCom = -1;
            pData->optFeedback.ctrl.uOut = 0;
            pData->optFeedback.fVal = 0.5f;
            pData->optWetGain.ctrl.sCom = -1;
            pData->optWetGain.ctrl.uOut = 0;
            pData->optWetGain.fVal = 0.f;
            pData->optWetLevel.ctrl.sCom = -1;
            pData->optWetLevel.ctrl.uOut = 0;
            pData->optWetLevel.fVal = 1.f;
            pData->optDryGain.ctrl.sCom = -1;
            pData->optDryGain.ctrl.uOut = 0;
            pData->optDryGain.fVal = 1.f;
            pData->optDryLevel.ctrl.sCom = -1;
            pData->optDryLevel.ctrl.uOut = 0;
            pData->optDryLevel.fVal = 1.f;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

    private:
		ixCore m_uDSP;
    };
}

#endif // _MONOFEEDBACKDELAY_H_

