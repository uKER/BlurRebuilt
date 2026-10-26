#ifndef _SurroundRESONANTLOWPASS_H_
#define _SurroundRESONANTLOWPASS_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundResonantLowPass
////////////////////////////////////////////////////////////////////////////////
    class SurroundResonantLowPass : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optCutoff;
            Option optPitchShift;
            Option optFreqShift;
            Option optQ;
            Option optGain;
            Option optLevel;
			Option optClamp;
        };

        struct State
        {
            F32 fCutoff;
            F32 fQ;
            F32 fLevel;
            F32 fClamp;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        SurroundResonantLowPass(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "SurroundResonantLowPass"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundResonantLowPass); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eIIRLowPass2ndOrder; }

        static void DefaultData(Data* pData)
        {
            pData->optCutoff.ctrl.sCom = -1;
            pData->optCutoff.ctrl.uOut = 0;
            pData->optCutoff.fVal = 440.f;
            pData->optQ.ctrl.sCom = -1;
            pData->optQ.ctrl.uOut = 0;
            pData->optQ.fVal = 0.f;
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
            pData->optClamp.ctrl.sCom = -1;
            pData->optClamp.ctrl.uOut = 0;
            pData->optClamp.fVal = 1.f;
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

#endif // _SurroundRESONANTLOWPASS_H_

