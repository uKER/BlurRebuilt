#ifndef _MONOTOSTEREOMIXER_H_
#define _MONOTOSTEREOMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoToStereoMixer
////////////////////////////////////////////////////////////////////////////////
    class MonoToStereoMixer : public MixerComponent
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
            F32 fGain;
            F32 fPan;
            F32 fLeft;
            F32 fRight;

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
		MonoToStereoMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }

        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value( Instance* pInstance, U32 uOutput = 0 );

        const char* Type() const { return "MonoToStereoMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagStereo | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoToStereoMixer); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32) + ((Data*)pData)->uInputs * sizeof(InputGain); }
		U32 CoreMemory(U32 uOffset);

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

#endif // _MONOTOSTEREOMIXER_H_

