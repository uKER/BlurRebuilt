#ifndef _SURROUND_FLANGER_H_
#define _SURROUND_FLANGER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundFlanger
////////////////////////////////////////////////////////////////////////////////
    class SurroundFlanger : public FlangerComponent
    {
    public:
        SurroundFlanger(Object* pObject) : FlangerComponent(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU

#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return StaticGetNumMessages(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticGetNumMessages() { return NumMessages; }
#ifndef PLATFORM_SPU
        const char* Type() const { return "SurroundFlanger"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundFlanger); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eFlanger; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->fMaxLength = 10.f;
            pData->optDelay.ctrl.sCom = -1;
            pData->optDelay.ctrl.uOut = 0;
            pData->optDelay.fVal = 0.5f;
            pData->optDepth.ctrl.sCom = -1;
            pData->optDepth.ctrl.uOut = 0;
            pData->optDepth.fVal = 5.f;
            pData->optRate.ctrl.sCom = -1;
            pData->optRate.ctrl.uOut = 0;
            pData->optRate.fVal = 0.25f;
            pData->optFeedback.ctrl.sCom = -1;
            pData->optFeedback.ctrl.uOut = 0;
            pData->optFeedback.fVal = 0.25f;
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
    };
}

#endif // _SURROUND_FLANGER_H_


