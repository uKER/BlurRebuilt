#ifndef _MONOSpectralReverb_H_
#define _MONOSpectralReverb_H_

#define NEW_REVERB_PARAMS

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoSpectralReverb
////////////////////////////////////////////////////////////////////////////////
    class MonoSpectralReverb : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optBand1_DecayTime;
			Option optBand2_DecayTime;
			Option optBand3_DecayTime;
			Option optBand4_DecayTime;
			Option optBand5_DecayTime;
			Option optBand6_DecayTime;

			Option optMaster_DecayTime;

			Option optGain;
			Option optLevel;

			Option optSmoothFilter;

#ifdef NEW_REVERB_PARAMS
			Option optSmoothingRatio;
			Option optPreset;
			Option optHighPassCutoffFreq;
			Option optHighPassAttenuation;
			Option optLowPassCutoffFreq;
			Option optLowPassAttenuation;
#endif

	
        };

        struct State
        {
            F32 fBand1_DecayTime;
			F32 fBand2_DecayTime;
			F32 fBand3_DecayTime;
			F32 fBand4_DecayTime;
			F32 fBand5_DecayTime;
			F32 fBand6_DecayTime;

			F32 fMaster_DecayTime;

			F32	fGain;

			F32 fSmoothFilter;

#ifdef NEW_REVERB_PARAMS
			F32 fSmoothingRatio;	
			S32 sPreset;
			S32 sHighPassCutoffFreq;
			F32 fHighPassAttenuation;		
			S32 sLowPassCutoffFreq;
			F32 fLowPassAttenuation;
#endif

			bool bActive;
        };

		enum Controls
        {
      /*      RMSCtrl,
            PitchCtrl,
            SibDifferenceCtrl*/
        
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoSpectralReverb(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "MonoSpectralReverb"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoSpectralReverb); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eSpectralReverb; }

		F32 Value(Instance* pInstance, U32 uOutput = 0);

        static void DefaultData(Data* pData)
        {

			pData->optBand1_DecayTime.ctrl.sCom = -1;
            pData->optBand1_DecayTime.ctrl.uOut = 0;
            pData->optBand1_DecayTime.fVal = 0.54f;

			pData->optBand2_DecayTime.ctrl.sCom = -1;
            pData->optBand2_DecayTime.ctrl.uOut = 0;
            pData->optBand2_DecayTime.fVal = 1.53f;

			pData->optBand3_DecayTime.ctrl.sCom = -1;
            pData->optBand3_DecayTime.ctrl.uOut = 0;
            pData->optBand3_DecayTime.fVal = 2.87;

			pData->optBand4_DecayTime.ctrl.sCom = -1;
            pData->optBand4_DecayTime.ctrl.uOut = 0;
            pData->optBand4_DecayTime.fVal = 3.39;

			pData->optBand5_DecayTime.ctrl.sCom = -1;
            pData->optBand5_DecayTime.ctrl.uOut = 0;
            pData->optBand5_DecayTime.fVal = 2.06;

			pData->optBand6_DecayTime.ctrl.sCom = -1;
            pData->optBand6_DecayTime.ctrl.uOut = 0;
            pData->optBand6_DecayTime.fVal = 1.63;

			pData->optMaster_DecayTime.ctrl.sCom = -1;
            pData->optMaster_DecayTime.ctrl.uOut = 0;
            pData->optMaster_DecayTime.fVal = 1.0f;

			pData->optGain.ctrl.sCom = -1;
            pData->optGain.ctrl.uOut = 0;
            pData->optGain.fVal = 0.0;

			pData->optLevel.ctrl.sCom = -1;
            pData->optLevel.ctrl.uOut = 0;
            pData->optLevel.fVal = 1;

			pData->optSmoothFilter.ctrl.sCom = -1;
            pData->optSmoothFilter.ctrl.uOut = 0;
            pData->optSmoothFilter.fVal = 1;

#ifdef NEW_REVERB_PARAMS
			pData->optSmoothingRatio.ctrl.sCom = -1;
            pData->optSmoothingRatio.ctrl.uOut = 0;
            pData->optSmoothingRatio.fVal = 0.6;

			pData->optPreset.ctrl.sCom = -1;
            pData->optPreset.ctrl.uOut = 0;
            pData->optPreset.fVal = 2;

			pData->optHighPassCutoffFreq.ctrl.sCom = -1;
            pData->optHighPassCutoffFreq.ctrl.uOut = 0;
            pData->optHighPassCutoffFreq.fVal = 565;

			pData->optHighPassAttenuation.ctrl.sCom = -1;
            pData->optHighPassAttenuation.ctrl.uOut = 0;
            pData->optHighPassAttenuation.fVal = -18;

			pData->optLowPassCutoffFreq.ctrl.sCom = -1;
            pData->optLowPassCutoffFreq.ctrl.uOut = 0;
            pData->optLowPassCutoffFreq.fVal = 7500;

			pData->optLowPassAttenuation.ctrl.sCom = -1;
            pData->optLowPassAttenuation.ctrl.uOut = 0;
            pData->optLowPassAttenuation.fVal = -3;
#endif

  }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
 /*           if (strcmp(pchControl, "RMS") == 0)
                return 0;
            else if (strcmp(pchControl, "Pitch") == 0)
                return 1;
            else if (strcmp(pchControl, "SibDifference") == 0)
                return 2;
            else*/
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

#endif // _MONOSpectralReverb_H_

