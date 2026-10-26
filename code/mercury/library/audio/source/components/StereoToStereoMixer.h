#ifndef _STEREOTOSTEREOMIXER_H_
#define _STEREOTOSTEREOMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StereoToStereoMixer
////////////////////////////////////////////////////////////////////////////////
    class StereoToStereoMixer : public MixerComponent
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
            F32 afGain[ 2 ][ 2 ];

			bool bActive;
			bool bMuted;
        };

        struct InputGain
        {
			InputGain()
			{
				optLeftToLeftGain.ctrl.sCom = -1;
				optLeftToLeftGain.ctrl.uOut = 0;
				optLeftToLeftGain.fVal = 0.f;
				optLeftToLeftLevel.ctrl.sCom = -1;
				optLeftToLeftLevel.ctrl.uOut = 0;
				optLeftToLeftLevel.fVal = 1.f;
				optLeftToRightGain.ctrl.sCom = -1;
				optLeftToRightGain.ctrl.uOut = 0;
				optLeftToRightGain.fVal = Maths::fMinusInfinity;
				optLeftToRightLevel.ctrl.sCom = -1;
				optLeftToRightLevel.ctrl.uOut = 0;
				optLeftToRightLevel.fVal = 0.f;
				optRightToLeftGain.ctrl.sCom = -1;
				optRightToLeftGain.ctrl.uOut = 0;
				optRightToLeftGain.fVal = Maths::fMinusInfinity;
				optRightToLeftLevel.ctrl.sCom = -1;
				optRightToLeftLevel.ctrl.uOut = 0;
				optRightToLeftLevel.fVal = 0.f;
				optRightToRightGain.ctrl.sCom = -1;
				optRightToRightGain.ctrl.uOut = 0;
				optRightToRightGain.fVal = 0.f;
				optRightToRightLevel.ctrl.sCom = -1;
				optRightToRightLevel.ctrl.uOut = 0;
				optRightToRightLevel.fVal = 1.f;
				sInputCom = -1;
			}

			Option optLeftToLeftGain;
			Option optLeftToLeftLevel;
			Option optLeftToRightGain;
			Option optLeftToRightLevel;
			Option optRightToLeftGain;
			Option optRightToLeftLevel;
			Option optRightToRightGain;
			Option optRightToRightLevel;
            Com sInputCom;
        };

    public:
		StereoToStereoMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "StereoToStereoMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagAudioOutput); }

		static U32 Size() { return sizeof(StereoToStereoMixer); }
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

#endif // _STEREOTOSTEREOMIXER_H_

