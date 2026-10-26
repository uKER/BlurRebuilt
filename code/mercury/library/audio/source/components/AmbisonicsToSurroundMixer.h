#ifndef _AMBISONICSTOSURROUNDMIXER_H_
#define _AMBISONICSTOSURROUNDMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	AmbisonicsToSurroundMixer
////////////////////////////////////////////////////////////////////////////////
    class AmbisonicsToSurroundMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Option optGain;
			Option optLevel;
			Option optDistance;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 fOutputGain;
            F32 fDistance;

            F32 afSpeakerVolumes[ MAX_NUM_SURROUND_SPEAKERS ];

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
		AmbisonicsToSurroundMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);

        const char* Type() const { return "AmbisonicsToSurroundMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurround | flagAudioOutput); }

		static U32 Size() { return sizeof(AmbisonicsToSurroundMixer); }
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
			pData->optDistance.ctrl.sCom = -1;
			pData->optDistance.ctrl.uOut = 0;
            pData->optDistance.fVal = Maths::fMax;
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

#endif // _AMBISONICSTOSURROUNDMIXER_H_

