#ifndef _MONODISTORTION_H_
#define _MONODISTORTION_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoDistortion
////////////////////////////////////////////////////////////////////////////////
    class MonoDistortion : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optDrive;
            Option optCutoff;
            Option optSlope;
            Option optCutGain;
            Option optCutLevel;
            Option optBoostGain;
            Option optBoostLevel;
            Option optGain;
            Option optLevel;
        };

        struct State
        {
            F32 fDrive;
            F32 fCutLevel;
            F32 fCutoff;
            F32 fSlope;
            F32 fBoostLevel;
            F32 fLevel;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoDistortion(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoDistortion"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoDistortion); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP[uIndex]; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex)
		{
			switch (uIndex)
			{
			case 0:
			case 2:
				return Core::DSP::Effect::eIIRLowShelf;
			case 1:
				return Core::DSP::Effect::eSaturator;
			default:
				return (U32)Core::DSP::Effect::eUndefined;
			}
		}
		U32 DSPEffectCount( ) { return 3; }

        static void DefaultData(Data* pData)
        {
            pData->optDrive.ctrl.sCom = -1;
            pData->optDrive.ctrl.uOut = 0;
            pData->optDrive.fVal = 0.f;
            pData->optCutoff.ctrl.sCom = -1;
            pData->optCutoff.ctrl.uOut = 0;
            pData->optCutoff.fVal = 500.f;
            pData->optSlope.ctrl.sCom = -1;
            pData->optSlope.ctrl.uOut = 0;
            pData->optSlope.fVal = 1.f;
            pData->optCutGain.ctrl.sCom = -1;
            pData->optCutGain.ctrl.uOut = 0;
            pData->optCutGain.fVal = 0.f;
            pData->optCutLevel.ctrl.sCom = -1;
            pData->optCutLevel.ctrl.uOut = 0;
            pData->optCutLevel.fVal = 1.f;
            pData->optBoostGain.ctrl.sCom = -1;
            pData->optBoostGain.ctrl.uOut = 0;
            pData->optBoostGain.fVal = 0.f;
            pData->optBoostLevel.ctrl.sCom = -1;
            pData->optBoostLevel.ctrl.uOut = 0;
            pData->optBoostLevel.fVal = 1.f;
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
		ixCore m_uDSP[ 3 ];
    };
}

#endif // _MONODISTORTION_H_

