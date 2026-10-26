#ifndef _MONOONESHOTSOUND_H_
#define _MONOONESHOTSOUND_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoOneShotSound
////////////////////////////////////////////////////////////////////////////////
    class MonoOneShotSound : public SoundComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			StrOffset strBank;
			StrOffset strWave;
			Option optPitch;
		    Option optFrequency;
		    Option optGain;
		    Option optLevel;
			Event evFinished;
            ArraySize uEffects;
            ArraySize uInstances;
        };

        struct State
        {
            S32 sFormat;
            U32 uInstance;
            F32 fFrequency;
            F32 fLevel;
            F32 fPosition;
            U32 u32ComponentOffset;
			U32 uNumActive;

			bool bActive;
			bool bMuted;
		};

        enum Controls
        {
            PositionCtrl,
            ActiveCtrl
        };

        enum Messages
        {
            PlayMsg,
            StopMsg,
			PrimeMsg,
			FlushMsg,

            NumMessages
        };

    public:
        MonoOneShotSound(Object* pObject) : SoundComponent(pObject)
        {
			m_uDecoder = ixDecoderNone;
			m_uDecoderSize = 0;
			m_uWave = ixWaveNone;
			m_uWaveSize = 0;
			m_uMixer = ixMixerNone;
			m_uEffectOffset = 0;
			m_hWave = INVALID_HANDLE;
        }

#if !defined (PLATFORM_SPU)
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif

        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);
#ifndef PLATFORM_SPU
		void Release();
#endif

#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return StaticGetNumMessages(); }
#endif
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MonoOneShotSound"; }

#ifndef PLATFORM_SPU
		const U32 Flags() { return StaticFlags(); }
#endif
		static U32 StaticFlags() { return (flagUpdater | flagControl | flagMono | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoOneShotSound); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32); }
#if !defined (PLATFORM_SPU)
		U32 CoreMemory(U32 uOffset);

		void ConfigureAudio(Instance* pInstance);
#endif

		const U32 AudioOutType(Instance* pInstance) const;
		const U32 AudioOutOffset(Instance* pInstance) const;

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->strBank = -1;
            pData->strWave = -1;
            pData->uInstances = 1;
            pData->optPitch.ctrl.sCom = -1;
            pData->optPitch.ctrl.uOut = 0;
            pData->optPitch.fVal = 0.f;
            pData->optFrequency.ctrl.sCom = -1;
            pData->optFrequency.ctrl.uOut = 0;
            pData->optFrequency.fVal = 1.f;
            pData->optGain.ctrl.sCom = -1;
            pData->optGain.ctrl.uOut = 0;
            pData->optGain.fVal = 0.f;
            pData->optLevel.ctrl.sCom = -1;
            pData->optLevel.ctrl.uOut = 0;
            pData->optLevel.fVal = 1.f;
			pData->evFinished.sCom = -1;
			pData->evFinished.uMsg = 0;
            pData->uEffects = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->strBank == -1)
                return false;
            else if (pData->strWave == -1)
                return false;
            else
                return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Position") == 0)
                return 0;
            if (strcmp(pchControl, "Active") == 0)
                return 1;
            else
                return -1;
        }

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Play") == 0)
				return 0;
			if (strcmp(pchMessage, "Stop") == 0)
				return 1;
			if (strcmp(pchMessage, "Prime") == 0)
				return 2;
			if (strcmp(pchMessage, "Flush") == 0)
				return 3;
			else
				return -1;
		}

//        static void FinishedCallback(U32 u32StateOffset);
#ifndef AUDIO_TOOLS
		Core::Mixer* Mixer(Instance* pInstance) { return (m_uMixer == ixMixerNone) ? NULL : (Core::Mixer*)pInstance->GetCoreData(m_uMixer); } /*const*/

		Core::Wave* WaveBase(Instance* pInstance)  { return (m_uWave == ixWaveNone) ? NULL : (Core::Wave*)pInstance->GetCoreData(m_uWave); } 
		Core::Wave* WaveInstance(Core::Wave* pBaseWave, U32 uIndex)  { return (uIndex == (U32)-1 || !pBaseWave) ? NULL : (Core::Wave*)((U8*)pBaseWave + uIndex * m_uWaveSize); } 

		Core::Decoder* DecoderBase(Instance* pInstance) { return (m_uDecoder == ixDecoderNone) ? NULL : (Core::Decoder*)pInstance->GetCoreData(m_uDecoder); } /*const*/
		Core::Decoder* DecoderInstance(Core::Decoder* pBaseDecoder, U32 uIndex) { return (uIndex == (U32)-1 || !pBaseDecoder) ? NULL : (Core::Decoder*)((U8*)pBaseDecoder + uIndex * m_uDecoderSize); }
#endif

    private:
        void Play(Instance* pInstance);
        void Stop(Instance* pInstance);
        void Prime(Instance* pInstance);
        void Flush(Instance* pInstance);

        void Finished(Instance* pInstance);

#if defined (AUDIO_ENABLE_PROFILING)
		bool IsActive(Instance* pInstance);
		void SetMute(Instance* pInstance, bool bMuted);
#endif

    private:
		WM_WAVE_HANDLE m_hWave;

		ixCore m_uDecoder;
		ixCore m_uDecoderSize;
		ixCore m_uMixer;
		ixCore m_uWave;
		ixCore m_uWaveSize;
	};
}

#endif // _MONOONESHOTSOUND_H_

