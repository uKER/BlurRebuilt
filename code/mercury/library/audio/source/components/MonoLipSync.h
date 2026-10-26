#ifndef _MONOLIPSYNC_H_
#define _MONOLIPSYNC_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoLipSync
////////////////////////////////////////////////////////////////////////////////
    class MonoLipSync : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optBlendRMS;
            Option optBlendPitch;
            Option optSilenceThreshold;
            Option optClosedMouthWidth;
			Option optSibPostScale;
        };

        struct State
        {
            F32 fBlendRMS;
            F32 fBlendPitch;
            F32 fSilenceThreshold;
            F32 fClosedMouthWidth;
			F32 fSibPostScale;
			F32 fRMSOutput;
			F32 fPitchOutput;
			F32 fSibOutput;
			bool bActive;
        };

		enum Controls
        {
            RMSCtrl,
            PitchCtrl,
            SibDifferenceCtrl
        
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoLipSync(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoLipSync"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoLipSync); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eLipSync; }

		F32 Value(Instance* pInstance, U32 uOutput = 0);

        static void DefaultData(Data* pData)
        {
            pData->optBlendRMS.ctrl.sCom = -1;
            pData->optBlendRMS.ctrl.uOut = 0;
            pData->optBlendRMS.fVal = 0.75f;
            pData->optBlendPitch.ctrl.sCom = -1;
            pData->optBlendPitch.ctrl.uOut = 0;
            pData->optBlendPitch.fVal = 0.90f;
            pData->optSilenceThreshold.ctrl.sCom = -1;
            pData->optSilenceThreshold.ctrl.uOut = 0;
            pData->optSilenceThreshold.fVal = 0.023f;
            pData->optClosedMouthWidth.ctrl.sCom = -1;
            pData->optClosedMouthWidth.ctrl.uOut = 0;
            pData->optClosedMouthWidth.fVal = 0.355f;
			pData->optSibPostScale.ctrl.sCom = -1;
            pData->optSibPostScale.ctrl.uOut = 0;
            pData->optSibPostScale.fVal = 40.f;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "RMS") == 0)
                return 0;
            else if (strcmp(pchControl, "Pitch") == 0)
                return 1;
            else if (strcmp(pchControl, "SibDifference") == 0)
                return 2;
            else
                return -1;
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
        inline void Enable(U8* pStateData);
        inline void Bypass(U8* pStateData);

    private:
		ixCore m_uDSP;
    };
}

#endif // _MONOLIPSYNC_H_

