#ifndef _SurroundPEAKEQ_H_
#define _SurroundPEAKEQ_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundPeakEQ
////////////////////////////////////////////////////////////////////////////////
    class SurroundPeakEQ : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optCentre;
            Option optPitchShift;
            Option optFreqShift;
            Option optQ;
            Option optGain;
            Option optLevel;
			Option optClamp;
        };

        struct State
        {
            F32 fCentre;
            F32 fQ;
            F32 fPitchShift;
            F32 fFreqShift;
            F32 fGain;
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
        SurroundPeakEQ(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "SurroundPeakEQ"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundPeakEQ); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eIIRPeakEQ; }

        static void DefaultData(Data* pData)
        {
            pData->optCentre.ctrl.sCom = -1;
            pData->optCentre.ctrl.uOut = 0;
            pData->optCentre.fVal = 440.f;
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

#endif // _SurroundPEAKEQ_H_

