#ifndef _MONOTOAMBISONICSMIXER_H_
#define _MONOTOAMBISONICSMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoToAmbisonicsMixer
////////////////////////////////////////////////////////////////////////////////
    class MonoToAmbisonicsMixer : public MixerComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			Option optGain;
			Option optLevel;
			Option optAzimuth;
			Option optElevation;
            ArraySize uEffects;
            ArraySize uInputs;
        };

        struct State
        {
            F32 fOutputGain;
            F32 fAzimuth;
            F32 fElevation;

#if defined (AMBISONICS_1ST_ORDER)
            F32 afEncoder[ 3 ];
#endif
#if defined (AMBISONICS_2ND_ORDER)
            F32 afEncoder[ 5 ];
#endif
#if defined (AMBISONICS_3RD_ORDER)
            F32 afEncoder[ 8 ];
#endif
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
		MonoToAmbisonicsMixer(Object* pObject) : MixerComponent(pObject) {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }

        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);

#if !defined (PLATFORM_SPU)
        void ReleaseContext(Instance* pInstance);
#endif

        const char* Type() const { return "MonoToAmbisonicsMixer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagAmbisonics | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoToAmbisonicsMixer); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32) + ((Data*)pData)->uInputs * sizeof(InputGain); }
#if !defined (PLATFORM_SPU)
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
			pData->optAzimuth.ctrl.sCom = -1;
			pData->optAzimuth.ctrl.uOut = 0;
			pData->optAzimuth.fVal = 0.f;
			pData->optElevation.ctrl.sCom = -1;
			pData->optElevation.ctrl.uOut = 0;
			pData->optElevation.fVal = 0.f;
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

#endif // _MONOTOAMBISONICSMIXER_H_

