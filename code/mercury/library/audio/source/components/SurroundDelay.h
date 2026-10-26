#ifndef _SurroundDELAY_H_
#define _SurroundDELAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundDelay
////////////////////////////////////////////////////////////////////////////////
    class SurroundDelay : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optLength;
        };

        struct State
        {
            F32 fLength;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        SurroundDelay(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "SurroundDelay"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundDelay); }
		static U32 DataSize() { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eDelay; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 1.f;
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

#endif // _SurroundDELAY_H_

