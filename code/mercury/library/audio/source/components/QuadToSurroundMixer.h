#ifndef _QUADTOSURROUNDMIXER_H_
#define _QUADTOSURROUNDMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	QuadToSurroundMixer
////////////////////////////////////////////////////////////////////////////////
    class QuadToSurroundMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Flags bfFlags;
		    Option optGain;
			Option optLevel;
			Option optFrontLeftAzimuth;
			Option optFrontLeftElevation;
		    Option optFrontLeftWidth;
		    Option optFrontLeftDistance;
		    Option optFrontRightAzimuth;
		    Option optFrontRightElevation;
		    Option optFrontRightWidth;
		    Option optFrontRightDistance;
			Option optRearLeftAzimuth;
			Option optRearLeftElevation;
			Option optRearLeftWidth;
			Option optRearLeftDistance;
			Option optRearRightAzimuth;
			Option optRearRightElevation;
			Option optRearRightWidth;
			Option optRearRightDistance;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 fOutputGain;
            F32 afAzimuth[ 4 ];
            F32 afElevation[ 4 ];
            F32 afDistance[ 4 ];
            F32 afWidth[ 4 ];

            Speakers asVolumes[ 4 ];

			bool bActive;
			bool bMuted;
		};

        struct InputGain
        {
			InputGain()
			{
				optFrontLeftGain.ctrl.sCom = -1;
				optFrontLeftGain.ctrl.uOut = 0;
				optFrontLeftGain.fVal = 0.f;
				optFrontLeftLevel.ctrl.sCom = -1;
				optFrontLeftLevel.ctrl.uOut = 0;
				optFrontLeftLevel.fVal = 1.f;
				optFrontRightGain.ctrl.sCom = -1;
				optFrontRightGain.ctrl.uOut = 0;
				optFrontRightGain.fVal = 0.f;
				optFrontRightLevel.ctrl.sCom = -1;
				optFrontRightLevel.ctrl.uOut = 0;
				optFrontRightLevel.fVal = 1.f;
				optRearLeftGain.ctrl.sCom = -1;
				optRearLeftGain.ctrl.uOut = 0;
				optRearLeftGain.fVal = 0.f;
				optRearLeftLevel.ctrl.sCom = -1;
				optRearLeftLevel.ctrl.uOut = 0;
				optRearLeftLevel.fVal = 1.f;
				optRearRightGain.ctrl.sCom = -1;
				optRearRightGain.ctrl.uOut = 0;
				optRearRightGain.fVal = 0.f;
				optRearRightLevel.ctrl.sCom = -1;
				optRearRightLevel.ctrl.uOut = 0;
				optRearRightLevel.fVal = 1.f;
				sInputCom = -1;
			}

			Option optFrontLeftGain;
			Option optFrontLeftLevel;
			Option optFrontRightGain;
			Option optFrontRightLevel;
			Option optRearLeftGain;
			Option optRearLeftLevel;
			Option optRearRightGain;
			Option optRearRightLevel;
            Com sInputCom;
        };

    public:
		QuadToSurroundMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "QuadToSurroundMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagAudioOutput); }

		static U32 Size() { return sizeof(QuadToSurroundMixer); }
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
		    pData->optFrontLeftAzimuth.ctrl.sCom = -1;
		    pData->optFrontLeftAzimuth.ctrl.uOut = 0;
            pData->optFrontLeftAzimuth.fVal = -0.25f;
		    pData->optFrontLeftElevation.ctrl.sCom = -1;
		    pData->optFrontLeftElevation.ctrl.uOut = 0;
            pData->optFrontLeftElevation.fVal = 0.f;
		    pData->optFrontLeftWidth.ctrl.sCom = -1;
		    pData->optFrontLeftWidth.ctrl.uOut = 0;
            pData->optFrontLeftWidth.fVal = 0.f;
		    pData->optFrontLeftDistance.ctrl.sCom = -1;
		    pData->optFrontLeftDistance.ctrl.uOut = 0;
            pData->optFrontLeftDistance.fVal = Maths::fMax;
		    pData->optFrontRightAzimuth.ctrl.sCom = -1;
		    pData->optFrontRightAzimuth.ctrl.uOut = 0;
            pData->optFrontRightAzimuth.fVal = 0.25f;
		    pData->optFrontRightElevation.ctrl.sCom = -1;
		    pData->optFrontRightElevation.ctrl.uOut = 0;
            pData->optFrontRightElevation.fVal = 0.f;
		    pData->optFrontRightWidth.ctrl.sCom = -1;
		    pData->optFrontRightWidth.ctrl.uOut = 0;
            pData->optFrontRightWidth.fVal = 0.f;
		    pData->optFrontRightDistance.ctrl.sCom = -1;
		    pData->optFrontRightDistance.ctrl.uOut = 0;
            pData->optFrontRightDistance.fVal = Maths::fMax;
		    pData->optRearLeftAzimuth.ctrl.sCom = -1;
		    pData->optRearLeftAzimuth.ctrl.uOut = 0;
            pData->optRearLeftAzimuth.fVal = -0.75f;
		    pData->optRearLeftElevation.ctrl.sCom = -1;
		    pData->optRearLeftElevation.ctrl.uOut = 0;
            pData->optRearLeftElevation.fVal = 0.f;
		    pData->optRearLeftWidth.ctrl.sCom = -1;
		    pData->optRearLeftWidth.ctrl.uOut = 0;
            pData->optRearLeftWidth.fVal = 0.f;
		    pData->optRearLeftDistance.ctrl.sCom = -1;
		    pData->optRearLeftDistance.ctrl.uOut = 0;
            pData->optRearLeftDistance.fVal = Maths::fMax;
		    pData->optRearRightAzimuth.ctrl.sCom = -1;
		    pData->optRearRightAzimuth.ctrl.uOut = 0;
            pData->optRearRightAzimuth.fVal = 0.75f;
		    pData->optRearRightElevation.ctrl.sCom = -1;
		    pData->optRearRightElevation.ctrl.uOut = 0;
            pData->optRearRightElevation.fVal = 0.f;
		    pData->optRearRightWidth.ctrl.sCom = -1;
		    pData->optRearRightWidth.ctrl.uOut = 0;
            pData->optRearRightWidth.fVal = 0.f;
		    pData->optRearRightDistance.ctrl.sCom = -1;
		    pData->optRearRightDistance.ctrl.uOut = 0;
            pData->optRearRightDistance.fVal = Maths::fMax;
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

#endif // _QUADTOSURROUNDMIXER_H_

