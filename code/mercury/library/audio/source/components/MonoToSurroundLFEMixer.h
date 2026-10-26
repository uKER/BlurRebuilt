#ifndef _MONOTOSURROUNDLFEMIXER_H_
#define _MONOTOSURROUNDLFEMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoToSurroundLFEMixer
////////////////////////////////////////////////////////////////////////////////
    class MonoToSurroundLFEMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Flags bfFlags;
			Option optGain;
			Option optLevel;
			Option optAzimuth;
			Option optElevation;
			Option optWidth;
			Option optDistance;
			Option optLFELevel;
			Option optLFEGain;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 fOutputGain;
            F32 fAzimuth;
            F32 fElevation;
            F32 fWidth;
            F32 fDistance;

			Speakers sVolumes;

			bool bActive;
			bool bMuted;
        };

        struct InputGain
        {
			InputGain()
			{
				optGain.ctrl.sCom = -1;
				optGain.ctrl.uOut = 0;
				optGain.fVal = 0.f;
				optLevel.ctrl.sCom = -1;
				optLevel.ctrl.uOut = 0;
				optLevel.fVal = 1.f;
				sInputCom = -1;
			}

			Option optGain;
			Option optLevel;
            Com sInputCom;
        };

    public:
		MonoToSurroundLFEMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#ifndef PLATFORM_SPU
        void ReleaseContext(Instance* pInstance);
#endif
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "MonoToSurroundLFEMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurroundLFE | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoToSurroundLFEMixer); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32) + ((Data*)pData)->uInputs * sizeof(InputGain); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

		const U32 AudioOutType(Instance* pInstance) const;
		const U32 AudioOutOffset(Instance* pInstance) const
		{
			return m_uMixer;
		}

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
		    pData->optGain.ctrl.sCom = -1;
		    pData->optGain.ctrl.uOut = 0;
			pData->optGain.fVal = 0.f;
			pData->optLevel.ctrl.sCom = -1;
			pData->optLevel.ctrl.uOut = 0;
			pData->optLevel.fVal = 1.f;
		    pData->optAzimuth.ctrl.sCom = -1;
		    pData->optAzimuth.ctrl.uOut = 0;
            pData->optAzimuth.fVal = 0.f;
		    pData->optElevation.ctrl.sCom = -1;
		    pData->optElevation.ctrl.uOut = 0;
            pData->optElevation.fVal = 0.f;
		    pData->optWidth.ctrl.sCom = -1;
		    pData->optWidth.ctrl.uOut = 0;
            pData->optWidth.fVal = 0.f;
		    pData->optDistance.ctrl.sCom = -1;
		    pData->optDistance.ctrl.uOut = 0;
            pData->optDistance.fVal = Maths::fMax;
            pData->optLFEGain.ctrl.sCom = -1;
            pData->optLFEGain.ctrl.uOut = 0;
            pData->optLFEGain.fVal = 0.f;
            pData->optLFELevel.ctrl.sCom = -1;
            pData->optLFELevel.ctrl.uOut = 0;
            pData->optLFELevel.fVal = 1.f;
            pData->uEffects = 0;
            pData->uInputs = 0;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		U16 GetNumInputs();
		Com GetInputCom(U32 uIndex);
		bool IsActive(Instance* pInstance);
		void SetMute(Instance* pInstance, bool bMuted);
#endif

		Com GetInput( U8* pInitData, U32 uInput );
    };
}

#endif // _MONOTOSURROUNDLFEMIXER_H_

