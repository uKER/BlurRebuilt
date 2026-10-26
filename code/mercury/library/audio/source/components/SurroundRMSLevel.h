#ifndef _SurroundRMSLEVEL_H_
#define _SurroundRMSLEVEL_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundRMSLevel
////////////////////////////////////////////////////////////////////////////////
    class SurroundRMSLevel : public Component
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
			bool bActive;
		};

        enum Controls
        {
            ValueCtrl
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        SurroundRMSLevel(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
		void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "SurroundRMSLevel"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagTimeEffect); }

        static U32 Size() { return sizeof(SurroundRMSLevel); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eRMSLevel; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 0.01f;

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

        static S32 GetControl(const char* pchControl)
        {
			if (strcmp(pchControl, "Value") == 0)
                return 0;
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

#endif // _SurroundRMSLEVEL_H_


