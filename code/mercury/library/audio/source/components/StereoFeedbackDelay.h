#ifndef _STEREOFEEDBACKDELAY_H_
#define _STEREOFEEDBACKDELAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoFeedbackDelay
////////////////////////////////////////////////////////////////////////////////
    class StereoFeedbackDelay : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			AudioFloat fMaxLength;
            Option optLength;
            Option optFeedback;
            Option optWet;
            Option optDry;
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
        StereoFeedbackDelay(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "StereoFeedbackDelay"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagTimeEffect | flagTransmitter/*for DSP mem*/); }

        static U32 Size() { return sizeof(StereoFeedbackDelay); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eFeedbackDelay; }

		//	stops base class asserts
		audio_virtual void Generate(Instance* pInstance) {}

		static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->fMaxLength = 0.5f;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 0.25f;
            pData->optFeedback.ctrl.sCom = -1;
            pData->optFeedback.ctrl.uOut = 0;
            pData->optFeedback.fVal = 0.5f;
            pData->optWet.ctrl.sCom = -1;
            pData->optWet.ctrl.uOut = 0;
            pData->optWet.fVal = 1.f;
            pData->optDry.ctrl.sCom = -1;
            pData->optDry.ctrl.uOut = 0;
            pData->optDry.fVal = 1.f;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

    private:
		ixCore m_uDSP;
    };
}

#endif // _STEREOFEEDBACKDELAY_H_

