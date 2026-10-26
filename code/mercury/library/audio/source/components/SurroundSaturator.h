#ifndef _SURROUNDSATURATOR_H_
#define _SURROUNDSATURATOR_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundSaturator
////////////////////////////////////////////////////////////////////////////////
    class SurroundSaturator : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optDrive;
            Option optClip;
            Option optGain;
            Option optLevel;
        };

        struct State
        {
            F32 fDrive;
            F32 fClip;
            F32 fLevel;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        SurroundSaturator(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "SurroundSaturator"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundSaturator); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eSaturator; }

        static void DefaultData(Data* pData)
        {
            pData->optDrive.ctrl.sCom = -1;
            pData->optDrive.ctrl.uOut = 0;
            pData->optDrive.fVal = 0.f;
            pData->optClip.ctrl.sCom = -1;
            pData->optClip.ctrl.uOut = 0;
            pData->optClip.fVal = 1.f;
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

#endif // _SURROUNDSATURATOR_H_

