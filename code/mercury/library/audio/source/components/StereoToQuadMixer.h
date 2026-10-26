#ifndef _STEREOTOQUADMIXER_H_
#define _STEREOTOQUADMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoToQuadMixer
////////////////////////////////////////////////////////////////////////////////
    class StereoToQuadMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Option optGain;
			Option optLevel;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 afGain[ 2 ][ 4 ];

			bool bActive;
			bool bMuted;
        };

        struct InputGain
        {
			InputGain()
			{
				optLeftToFrontLeftGain.ctrl.sCom = -1;
				optLeftToFrontLeftGain.ctrl.uOut = 0;
				optLeftToFrontLeftGain.fVal = 0.f;
				optLeftToFrontLeftLevel.ctrl.sCom = -1;
				optLeftToFrontLeftLevel.ctrl.uOut = 0;
				optLeftToFrontLeftLevel.fVal = 1.f;
				optLeftToFrontRightGain.ctrl.sCom = -1;
				optLeftToFrontRightGain.ctrl.uOut = 0;
				optLeftToFrontRightGain.fVal = Maths::fMinusInfinity;
				optLeftToFrontRightLevel.ctrl.sCom = -1;
				optLeftToFrontRightLevel.ctrl.uOut = 0;
				optLeftToFrontRightLevel.fVal = 0.f;
				optLeftToRearLeftGain.ctrl.sCom = -1;
				optLeftToRearLeftGain.ctrl.uOut = 0;
				optLeftToRearLeftGain.fVal = 0.f;
				optLeftToRearLeftLevel.ctrl.sCom = -1;
				optLeftToRearLeftLevel.ctrl.uOut = 0;
				optLeftToRearLeftLevel.fVal = 1.f;
				optLeftToRearRightGain.ctrl.sCom = -1;
				optLeftToRearRightGain.ctrl.uOut = 0;
				optLeftToRearRightGain.fVal = 0.f;
				optLeftToRearRightLevel.ctrl.sCom = -1;
				optLeftToRearRightLevel.ctrl.uOut = 0;
				optLeftToRearRightLevel.fVal = -1.f;
				optRightToFrontLeftGain.ctrl.sCom = -1;
				optRightToFrontLeftGain.ctrl.uOut = 0;
				optRightToFrontLeftGain.fVal = Maths::fMinusInfinity;
				optRightToFrontLeftLevel.ctrl.sCom = -1;
				optRightToFrontLeftLevel.ctrl.uOut = 0;
				optRightToFrontLeftLevel.fVal = 0.f;
				optRightToFrontRightGain.ctrl.sCom = -1;
				optRightToFrontRightGain.ctrl.uOut = 0;
				optRightToFrontRightGain.fVal = 0.f;
				optRightToFrontRightLevel.ctrl.sCom = -1;
				optRightToFrontRightLevel.ctrl.uOut = 0;
				optRightToFrontRightLevel.fVal = 1.f;
				optRightToRearLeftGain.ctrl.sCom = -1;
				optRightToRearLeftGain.ctrl.uOut = 0;
				optRightToRearLeftGain.fVal = 0.f;
				optRightToRearLeftLevel.ctrl.sCom = -1;
				optRightToRearLeftLevel.ctrl.uOut = 0;
				optRightToRearLeftLevel.fVal = -1.f;
				optRightToRearRightGain.ctrl.sCom = -1;
				optRightToRearRightGain.ctrl.uOut = 0;
				optRightToRearRightGain.fVal = 0.f;
				optRightToRearRightLevel.ctrl.sCom = -1;
				optRightToRearRightLevel.ctrl.uOut = 0;
				optRightToRearRightLevel.fVal = 1.f;
				sInputCom = -1;
			}

			Option optLeftToFrontLeftGain;
			Option optLeftToFrontLeftLevel;
			Option optLeftToFrontRightGain;
			Option optLeftToFrontRightLevel;
			Option optLeftToRearLeftGain;
			Option optLeftToRearLeftLevel;
			Option optLeftToRearRightGain;
			Option optLeftToRearRightLevel;
			Option optRightToFrontLeftGain;
			Option optRightToFrontLeftLevel;
			Option optRightToFrontRightGain;
			Option optRightToFrontRightLevel;
			Option optRightToRearLeftGain;
			Option optRightToRearLeftLevel;
			Option optRightToRearRightGain;
			Option optRightToRearRightLevel;
            Com sInputCom;
        };

    public:
		StereoToQuadMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "StereoToQuadMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagQuad | flagAudioOutput); }

		static U32 Size() { return sizeof(StereoToQuadMixer); }
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

#endif // _STEREOTOQUADMIXER_H_

