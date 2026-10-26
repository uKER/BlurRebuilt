#ifndef _STEREOPITCHSHIFT_H_
#define _STEREOPITCHSHIFT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoPitchShift
////////////////////////////////////////////////////////////////////////////////
    class StereoPitchShift : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optPitchShift;
            Option optSequenceLength;
            Option optSeekWindowLength;
			Option optOverlapLength;
        };

        struct State
        {
            F32 fPitchShift;			
			F32 fSequenceLength;
			F32 fSeekWindowLength;
			F32 fOverlapLength;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        StereoPitchShift(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "StereoPitchShift"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagTimeEffect); }

        static U32 Size() { return sizeof(StereoPitchShift); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::ePitchShift; }

        static void DefaultData(Data* pData)
        {
			pData->optPitchShift.ctrl.sCom = -1; 
            pData->optPitchShift.ctrl.uOut = 0; 
            pData->optPitchShift.fVal = 0.0f; 
			pData->optSequenceLength.ctrl.sCom = -1; 
            pData->optSequenceLength.ctrl.uOut = 0; 
            pData->optSequenceLength.fVal = 82; 
			pData->optSeekWindowLength.ctrl.sCom = -1; 
            pData->optSeekWindowLength.ctrl.uOut = 0; 
            pData->optSeekWindowLength.fVal = 14; 
			pData->optOverlapLength.ctrl.sCom = -1; 
            pData->optOverlapLength.ctrl.uOut = 0; 
            pData->optOverlapLength.fVal = 12; 
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

#endif // _STEREOPITCHSHIFT_H_
