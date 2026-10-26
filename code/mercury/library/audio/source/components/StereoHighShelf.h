#ifndef _StereoHIGHSHELF_H_
#define _StereoHIGHSHELF_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoHighShelf
////////////////////////////////////////////////////////////////////////////////
    class StereoHighShelf : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optCutoff;
            Option optPitchShift;
            Option optFreqShift;
            Option optSlope;
            Option optGain;
            Option optLevel;
			Option optClamp;
        };

        struct State
        {
            F32 fCutoff;
            F32 fSlope;
            F32 fPitchShift;
            F32 fFreqShift;
            F32 fGain;
            F32 fLevel;
            F32 fClamp;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        StereoHighShelf(Object* pObject) : Component(pObject)
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
        U32 GetNumMessages() { return NumMessages; }
        const char* Type() const { return "StereoHighShelf"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagTimeEffect); }

        static U32 Size() { return sizeof(StereoHighShelf); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::eIIRHighShelf; }

        static void DefaultData(Data* pData)
        {
            pData->optCutoff.ctrl.sCom = -1;
            pData->optCutoff.ctrl.uOut = 0;
            pData->optCutoff.fVal = 440.f;
            pData->optSlope.ctrl.sCom = -1;
            pData->optSlope.ctrl.uOut = 0;
            pData->optSlope.fVal = 0.f;
            pData->optPitchShift.ctrl.sCom = -1;
            pData->optPitchShift.ctrl.uOut = 0;
            pData->optPitchShift.fVal = 0.f;
            pData->optFreqShift.ctrl.sCom = -1;
            pData->optFreqShift.ctrl.uOut = 0;
            pData->optFreqShift.fVal = 1.f;
            pData->optGain.ctrl.sCom = -1;
            pData->optGain.ctrl.uOut = 0;
            pData->optGain.fVal = 0.f;
            pData->optLevel.ctrl.sCom = -1;
            pData->optLevel.ctrl.uOut = 0;
            pData->optLevel.fVal = 1.f;
            pData->optClamp.ctrl.sCom = -1;
            pData->optClamp.ctrl.uOut = 0;
            pData->optClamp.fVal = 1.f;
		}

        static bool ValidateData(Data* pData)
        {
            return true;
        }

    private:
		ixCore m_uDSP;
    };
}

#endif // _StereoHIGHSHELF_H_

