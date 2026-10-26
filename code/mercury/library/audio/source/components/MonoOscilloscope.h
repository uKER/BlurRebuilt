#ifndef _MONOOSCILLOSCOPE_H_
#define _MONOOSCILLOSCOPE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoOscilloscope
////////////////////////////////////////////////////////////////////////////////
    class MonoOscilloscope : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;

            Option optLength;
            Option optLevel;
        };			  

        struct State
        {
			F32 fLength;
			F32 fLevel;
			F32 afHighs[ 24 ];
			F32 afLows[ 24 ];
			bool bActive;
		};

        enum Controls
        {
            High00Ctrl,
            High01Ctrl,
            High02Ctrl,
            High03Ctrl,
            High04Ctrl,
            High05Ctrl,
            High06Ctrl,
            High07Ctrl,
            High08Ctrl,
            High09Ctrl,
            High10Ctrl,
            High11Ctrl,
            High12Ctrl,
            High13Ctrl,
            High14Ctrl,
            High15Ctrl,
            High16Ctrl,
            High17Ctrl,
            High18Ctrl,
            High19Ctrl,
            High20Ctrl,
            High21Ctrl,
            High22Ctrl,
            High23Ctrl,
            Low00Ctrl,
            Low01Ctrl,
            Low02Ctrl,
            Low03Ctrl,
            Low04Ctrl,
            Low05Ctrl,
            Low06Ctrl,
            Low07Ctrl,
            Low08Ctrl,
            Low09Ctrl,
            Low10Ctrl,
            Low11Ctrl,
            Low12Ctrl,
            Low13Ctrl,
            Low14Ctrl,
            Low15Ctrl,
            Low16Ctrl,
            Low17Ctrl,
            Low18Ctrl,
            Low19Ctrl,
            Low20Ctrl,
            Low21Ctrl,
            Low22Ctrl,
            Low23Ctrl
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,
            ResetMsg,

            NumMessages
        };

    public:
        MonoOscilloscope(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoOscilloscope"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoOscilloscope); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eOscilloscope; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 0.001f;
            pData->optLevel.ctrl.sCom = -1;
            pData->optLevel.ctrl.uOut = 0;
            pData->optLevel.fVal = 1.f;
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
			if (strcmp(pchControl, "High00") == 0)
                return 0;
            else if (strcmp(pchControl, "High01") == 0)
                return 1;
            else if (strcmp(pchControl, "High02") == 0)
                return 2;
            else if (strcmp(pchControl, "High03") == 0)
                return 3;
            else if (strcmp(pchControl, "High04") == 0)
                return 4;
            else if (strcmp(pchControl, "High05") == 0)
                return 5;
            else if (strcmp(pchControl, "High06") == 0)
                return 6;
            else if (strcmp(pchControl, "High07") == 0)
                return 7;
            else if (strcmp(pchControl, "High08") == 0)
                return 8;
            else if (strcmp(pchControl, "High09") == 0)
                return 9;
            else if (strcmp(pchControl, "High10") == 0)
                return 10;
            else if (strcmp(pchControl, "High11") == 0)
                return 11;
            else if (strcmp(pchControl, "High12") == 0)
                return 12;
            else if (strcmp(pchControl, "High13") == 0)
                return 13;
            else if (strcmp(pchControl, "High14") == 0)
                return 14;
            else if (strcmp(pchControl, "High15") == 0)
                return 15;
            else if (strcmp(pchControl, "High16") == 0)
                return 16;
            else if (strcmp(pchControl, "High17") == 0)
                return 17;
            else if (strcmp(pchControl, "High18") == 0)
                return 18;
            else if (strcmp(pchControl, "High19") == 0)
                return 19;
            else if (strcmp(pchControl, "High20") == 0)
                return 20;
            else if (strcmp(pchControl, "High21") == 0)
                return 21;
            else if (strcmp(pchControl, "High22") == 0)
                return 22;
            else if (strcmp(pchControl, "High23") == 0)
                return 23;
			else if (strcmp(pchControl, "Low00") == 0)
                return 24;
            else if (strcmp(pchControl, "Low01") == 0)
                return 25;
            else if (strcmp(pchControl, "Low02") == 0)
                return 26;
            else if (strcmp(pchControl, "Low03") == 0)
                return 27;
            else if (strcmp(pchControl, "Low04") == 0)
                return 28;
            else if (strcmp(pchControl, "Low05") == 0)
                return 29;
            else if (strcmp(pchControl, "Low06") == 0)
                return 30;
            else if (strcmp(pchControl, "Low07") == 0)
                return 31;
            else if (strcmp(pchControl, "Low08") == 0)
                return 32;
            else if (strcmp(pchControl, "Low09") == 0)
                return 33;
            else if (strcmp(pchControl, "Low10") == 0)
                return 34;
            else if (strcmp(pchControl, "Low11") == 0)
                return 35;
            else if (strcmp(pchControl, "Low12") == 0)
                return 36;
            else if (strcmp(pchControl, "Low13") == 0)
                return 37;
            else if (strcmp(pchControl, "Low14") == 0)
                return 38;
            else if (strcmp(pchControl, "Low15") == 0)
                return 39;
            else if (strcmp(pchControl, "Low16") == 0)
                return 40;
            else if (strcmp(pchControl, "Low17") == 0)
                return 41;
            else if (strcmp(pchControl, "Low18") == 0)
                return 42;
            else if (strcmp(pchControl, "Low19") == 0)
                return 43;
            else if (strcmp(pchControl, "Low20") == 0)
                return 44;
            else if (strcmp(pchControl, "Low21") == 0)
                return 45;
            else if (strcmp(pchControl, "Low22") == 0)
                return 46;
            else if (strcmp(pchControl, "Low23") == 0)
                return 47;
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

#endif // _MONOOSCILLOSCOPE_H_


