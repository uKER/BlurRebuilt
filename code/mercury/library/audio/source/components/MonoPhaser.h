#ifndef _PHASER_H_
#define _PHASER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoPhaser
////////////////////////////////////////////////////////////////////////////////
	class MonoPhaser : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optDry;
            Option optWet; 
            Option optAngle;
            Option optSpeed;
            Option optWidth;
			Option optFeedback;
			//StrOffset strName;
			//Option optDryLevel;
			//Option optDryGain;
			//Option optWetGain; 
			//Option optWetLevel; 
			//Option optSpeed;
			//Option optWidth;
			//Option optFeedback;
        };			  

        struct State
        {
            F32 fDry;
            F32 fWet; 
            F32 fAngle;
            F32 fSpeed;
        	F32 fWidth;
			F32 fFeedback;
		};

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoPhaser(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoPhaser"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoPhaser); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::ePhaser; }

        static void DefaultData(Data* pData)
        {
            pData->optDry.ctrl.sCom = -1; 
            pData->optDry.ctrl.uOut = 0; 
            pData->optDry.fVal = 1.f; 
            pData->optWet.ctrl.sCom = -1;
            pData->optWet.ctrl.uOut = 0; 
            pData->optWet.fVal = -1.f; 
            pData->optAngle.ctrl.sCom = -1; 
            pData->optAngle.ctrl.uOut = 0; 
            pData->optAngle.fVal = 0.f; 
            pData->optSpeed.ctrl.sCom = -1; 
            pData->optSpeed.ctrl.uOut = 0; 
            pData->optSpeed.fVal = 100.f; 
            pData->optWidth.ctrl.sCom = -1; 
            pData->optWidth.ctrl.uOut = 0; 
            pData->optWidth.fVal = 0.5f; 
			pData->optFeedback.ctrl.sCom = -1; 
            pData->optFeedback.ctrl.uOut = 0; 
            pData->optFeedback.fVal = 0.f; 
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

#endif // _PHASER_H_


