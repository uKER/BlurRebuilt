#ifndef _STEREOTOSURROUNDMIXER_H_
#define _STEREOTOSURROUNDMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoToSurroundMixer
////////////////////////////////////////////////////////////////////////////////
    class StereoToSurroundMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Flags bfFlags;
			Option optGain;
			Option optLevel;
			Option optLeftAzimuth;
			Option optLeftElevation;
			Option optLeftWidth;
			Option optLeftDistance;
			Option optRightAzimuth;
			Option optRightElevation;
			Option optRightWidth;
			Option optRightDistance;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 fOutputGain;
            F32 afAzimuth[ 2 ];
            F32 afElevation[ 2 ];
            F32 afWidth[ 2 ];
            F32 afDistance[ 2 ];

            Speakers asVolumes[ 2 ];

			bool bActive;
			bool bMuted;
        };

        struct InputGain
        {
			InputGain()
			{
				optLeftGain.ctrl.sCom = -1;
				optLeftGain.ctrl.uOut = 0;
				optLeftGain.fVal = 0.f;
				optLeftLevel.ctrl.sCom = -1;
				optLeftLevel.ctrl.uOut = 0;
				optLeftLevel.fVal = 1.f;
				optRightGain.ctrl.sCom = -1;
				optRightGain.ctrl.uOut = 0;
				optRightGain.fVal = 0.f;
				optRightLevel.ctrl.sCom = -1;
				optRightLevel.ctrl.uOut = 0;
				optRightLevel.fVal = 1.f;
				sInputCom = -1;
			}

			Option optLeftGain;
			Option optLeftLevel;
			Option optRightGain;
			Option optRightLevel;
            Com sInputCom;
        };

    public:
		StereoToSurroundMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "StereoToSurroundMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagAudioOutput); }

		static U32 Size() { return sizeof(StereoToSurroundMixer); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32) + ((Data*)pData)->uInputs * sizeof(InputGain); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif

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
			pData->optLeftAzimuth.ctrl.sCom = -1;
			pData->optLeftAzimuth.ctrl.uOut = 0;
            pData->optLeftAzimuth.fVal = -0.5f;
			pData->optLeftElevation.ctrl.sCom = -1;
			pData->optLeftElevation.ctrl.uOut = 0;
            pData->optLeftElevation.fVal = 0.f;
			pData->optLeftWidth.ctrl.sCom = -1;
			pData->optLeftWidth.ctrl.uOut = 0;
            pData->optLeftWidth.fVal = 0.f;
			pData->optLeftDistance.ctrl.sCom = -1;
			pData->optLeftDistance.ctrl.uOut = 0;
            pData->optLeftDistance.fVal = Maths::fMax;
			pData->optRightAzimuth.ctrl.sCom = -1;
			pData->optRightAzimuth.ctrl.uOut = 0;
            pData->optRightAzimuth.fVal = 0.5f;
			pData->optRightElevation.ctrl.sCom = -1;
			pData->optRightElevation.ctrl.uOut = 0;
            pData->optRightElevation.fVal = 0.f;
			pData->optRightWidth.ctrl.sCom = -1;
			pData->optRightWidth.ctrl.uOut = 0;
            pData->optRightWidth.fVal = 0.f;
			pData->optRightDistance.ctrl.sCom = -1;
			pData->optRightDistance.ctrl.uOut = 0;
            pData->optRightDistance.fVal = Maths::fMax;
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

#endif // _STEREOTOSURROUNDMIXER_H_

