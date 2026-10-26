#ifndef _MONODELAY_H_
#define _MONODELAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoDelay
////////////////////////////////////////////////////////////////////////////////
    class MonoDelay : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			AudioFloat fMaxLength;
			AudioFloat fDownSample;
            Option optLength;
            Option optWet;
            Option optDry;
        };

        struct State
        {
            F32 fLength;
			F32 fWet;
			F32 fDry;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,
            ResetMsg,

            NumMessages
        };

    public:
        MonoDelay(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        void Update(Instance* pInstance);
        void DoMessage( Instance* pInstance, U32 uMessage );

#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return StaticGetNumMessages(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticGetNumMessages() { return NumMessages; }
#ifndef PLATFORM_SPU
        const char* Type() const { return "MonoDelay"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoDelay); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eDelay; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->fMaxLength = 1.f;
			pData->fDownSample = 0;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 0.5f;
            pData->optWet.ctrl.sCom = -1;
            pData->optWet.ctrl.uOut = 0;
            pData->optWet.fVal = 1.f;
            pData->optDry.ctrl.sCom = -1;
            pData->optDry.ctrl.uOut = 0;
            pData->optDry.fVal = 0.f;
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
			if (strcmp(pchMessage, "Reset") == 0)
				return 2;
			else
				return -1;
		}

	private:
		ixCore m_uDSP;
    };
}

#endif // _MONODELAY_H_

