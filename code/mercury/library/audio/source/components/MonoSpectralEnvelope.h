#ifndef _MONOSpectralEnvelope_H_
#define _MONOSpectralEnvelope_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoSpectralEnvelope
////////////////////////////////////////////////////////////////////////////////
    class MonoSpectralEnvelope : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;

        };

        struct State
        {
			bool bActive;
        };

		enum Controls
        {
			Bin1_MagnitudeCtrl,
			Bin2_MagnitudeCtrl,
			Bin3_MagnitudeCtrl,
			Bin4_MagnitudeCtrl,
			Bin5_MagnitudeCtrl,
			Bin6_MagnitudeCtrl,
			Bin7_MagnitudeCtrl,
			Bin8_MagnitudeCtrl,
			Bin9_MagnitudeCtrl,
			Bin10_MagnitudeCtrl,
			Bin11_MagnitudeCtrl,
			Bin12_MagnitudeCtrl,
			Bin13_MagnitudeCtrl,
			Bin14_MagnitudeCtrl,
			Bin15_MagnitudeCtrl,
			Bin16_MagnitudeCtrl
          
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoSpectralEnvelope(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoSpectralEnvelope"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoSpectralEnvelope); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eSpectralEnvelope; }

		F32 Value(Instance* pInstance, U32 uOutput = 0);

        static void DefaultData(Data* pData)
        {
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
			if (strcmp(pchControl, "Bin1_Magnitude") == 0)
                return 0;
            else if (strcmp(pchControl, "Bin2_Magnitude") == 0)
                return 1;
            else if (strcmp(pchControl, "Bin3_Magnitude") == 0)
                return 2;
            else if (strcmp(pchControl, "Bin4_Magnitude") == 0)
                return 3;
            else if (strcmp(pchControl, "Bin5_Magnitude") == 0)
                return 4;
            else if (strcmp(pchControl, "Bin6_Magnitude") == 0)
                return 5;
            else if (strcmp(pchControl, "Bin7_Magnitude") == 0)
                return 6;
            else if (strcmp(pchControl, "Bin8_Magnitude") == 0)
                return 7;
            else if (strcmp(pchControl, "Bin9_Magnitude") == 0)
                return 8;
            else if (strcmp(pchControl, "Bin10_Magnitude") == 0)
                return 9;
            else if (strcmp(pchControl, "Bin11_Magnitude") == 0)
                return 10;
            else if (strcmp(pchControl, "Bin12_Magnitude") == 0)
                return 11;
            else if (strcmp(pchControl, "Bin13_Magnitude") == 0)
                return 12;
            else if (strcmp(pchControl, "Bin14_Magnitude") == 0)
                return 13;
            else if (strcmp(pchControl, "Bin15_Magnitude") == 0)
                return 14;
            else if (strcmp(pchControl, "Bin16_Magnitude") == 0)
                return 15;
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

#endif // _MONOSpectralEnvelope_H_

