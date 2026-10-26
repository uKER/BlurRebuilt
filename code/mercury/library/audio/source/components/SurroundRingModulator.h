#ifndef _SURROUNDRINGMODULATOR_H_
#define _SURROUNDRINGMODULATOR_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundRingModulator
////////////////////////////////////////////////////////////////////////////////
    class SurroundRingModulator : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optRate;
            Option optWet;
            Option optDry;
        };

        struct State
        {
            F32 fRate;
            F32 fWet;
            F32 fDry;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,
            ReSyncInPhaseMsg,
            ReSyncOutPhaseMsg,

            NumMessages
        };

    public:
        SurroundRingModulator(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return NumMessages; }
        const char* Type() const { return "SurroundRingModulator"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundRingModulator); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eRingModulator; }

        static void DefaultData(Data* pData)
        {
            pData->optRate.ctrl.sCom = -1;
            pData->optRate.ctrl.uOut = 0;
            pData->optRate.fVal = 1000.f;
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

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Enable") == 0)
				return 0;
			if (strcmp(pchMessage, "Bypass") == 0)
				return 1;
			else
				return -1;
		}

    private:
		ixCore m_uDSP;
    };
}

#endif // _SURROUNDRINGMODULATOR_H_

