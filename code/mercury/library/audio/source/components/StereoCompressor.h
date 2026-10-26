#ifndef _STEREOCOMPRESSOR_H_
#define _STEREOCOMPRESSOR_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoCompressor
////////////////////////////////////////////////////////////////////////////////
    class StereoCompressor : public CompressorComponent
    {
    public:
        StereoCompressor(Object* pObject) : CompressorComponent(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "StereoCompressor"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagTimeEffect); }

        static U32 Size() { return sizeof(StereoCompressor); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex)
		{
			return Core::DSP::Effect::eCompressor;
		}

        static void DefaultData(Data* pData)
        {
            pData->ctrlSideChainRMS.sCom = -1;
            pData->ctrlSideChainRMS.uOut = 0;
            pData->optAttack.ctrl.sCom = -1;
            pData->optAttack.ctrl.uOut = 0;
            pData->optAttack.fVal = 0.001f;
            pData->optRelease.ctrl.sCom = -1;
            pData->optRelease.ctrl.uOut = 0;
            pData->optRelease.fVal = 0.5f;
            pData->optThresholdGain.ctrl.sCom = -1;
            pData->optThresholdGain.ctrl.uOut = 0;
            pData->optThresholdGain.fVal = 0.f;
            pData->optThresholdLevel.ctrl.sCom = -1;
            pData->optThresholdLevel.ctrl.uOut = 0;
            pData->optThresholdLevel.fVal = 1.f;
            pData->optRatio.ctrl.sCom = -1;
            pData->optRatio.ctrl.uOut = 0;
            pData->optRatio.fVal = 10.f;
            pData->optMakeUpGain.ctrl.sCom = -1;
            pData->optMakeUpGain.ctrl.uOut = 0;
            pData->optMakeUpGain.fVal = 0.f;
            pData->optMakeUpLevel.ctrl.sCom = -1;
            pData->optMakeUpLevel.ctrl.uOut = 0;
            pData->optMakeUpLevel.fVal = 1.f;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

		        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Value") == 0)
                return 0;
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
    };
}

#endif // _STEREOCOMPRESSOR_H_

