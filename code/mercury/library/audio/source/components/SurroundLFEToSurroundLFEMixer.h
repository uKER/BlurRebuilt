#ifndef _SURROUNDLFETOSURROUNDLFEMIXER_H_
#define _SURROUNDLFETOSURROUNDLFEMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SurroundLFEToSurroundLFEMixer
////////////////////////////////////////////////////////////////////////////////
    class SurroundLFEToSurroundLFEMixer : public MixerComponent
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
            F32 fWidth;
            F32 fGain;

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
        SurroundLFEToSurroundLFEMixer(Object* pObject) : MixerComponent(pObject)
        {
			m_uMixer = 0;
			m_uEffectOffset = 0;
			m_uInputOffset = 0;
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);

        const char* Type() const { return "SurroundLFEToSurroundLFEMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagSurroundLFE | flagAudioOutput); }

		static U32 Size() { return sizeof(SurroundLFEToSurroundLFEMixer); }
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

#endif // _SURROUNDLFETOSURROUNDLFEMIXER_H_

