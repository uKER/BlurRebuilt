#ifndef _MONOONESHOTSOUNDTABLE_H_
#define _MONOONESHOTSOUNDTABLE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoOneShotSoundTable
////////////////////////////////////////////////////////////////////////////////
    class MonoOneShotSoundTable : public SoundComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
			StrOffset strBank;
		    Option optIndex;
		    Option optPitch;
		    Option optFrequency;
		    Option optGain;
		    Option optLevel;
			Event evFinished;
            ArraySize uEffects;
            ArraySize uInstances;
            ArraySize uWaves;
        };

        struct InputWave
        {
            //  the wave
			StrOffset strWave;
        };

        struct State
        {
            U32 uLastFrame;
//			bool bStop;
            U32 uInstance;
            F32 fIndex;
            F32 fPitch;
            F32 fFrequency;
            F32 fGain;
            F32 fLevel;
            F32 fPosition;
            U32 uIndex;
            U32 u32ComponentOffset;
			U32 uNumActive;
			bool bPlay;
			bool bActive;
			bool bMuted;
			bool bWasMuted;
        };

        enum Controls
        {
            EntriesCtrl,
            ActiveCtrl,
            PositionCtrl
        };

        enum Messages
        {
            PlayMsg,
            StopMsg,
            PrimeMsg,
            FlushMsg,

            NumMessages
        };

	private:
        struct DecoderTableEntry
        {
			ixCore uDecoder;
			ixCore uDecoderSize;
        };

        struct WaveTableEntry
        {
			ixCore uWave;
			ixCore uWaveSize;
        };

    public:
        MonoOneShotSoundTable(Object* pObject) : SoundComponent(pObject)
        {
			m_uWaveOffset = 0;
			m_uEffectOffset = 0;
			m_uDecoderTable = ixDecoderNone;
			m_uWaveTable = ixWaveNone;
			m_uMixer = ixMixerNone;
		}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
		static U32 WaveHandleArraySize(void* pData) 
		{ 
			return ((Data*)pData)->uWaves * sizeof(WM_WAVE_HANDLE) + 4; // extra 4 bytes is for alignment
		}
		// the array of wave handles occupy the memory directly after the object.
		// we make sure the address is 4 byte aligned
		WM_WAVE_HANDLE* WaveHandleArrayPtr() { return (WM_WAVE_HANDLE*)(((INTPTR)(this+1)+3) & ~(INTPTR)3); } // [Blur2PC][x64] full address

        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);
		void Release();

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MonoOneShotSoundTable"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl | flagMono | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoOneShotSoundTable); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uWaves * sizeof(InputWave) + ((Data*)pData)->uEffects * sizeof(U32); }
#if !defined (PLATFORM_SPU)
		U32 CoreMemory(U32 uOffset);

        void ConfigureAudio(Instance* pInstance);
#endif

		const U32 AudioOutType(Instance* pInstance) const;
		const U32 AudioOutOffset(Instance* pInstance) const;

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->uInstances = 1;
            pData->uWaves = 0;
            pData->optIndex.ctrl.sCom = -1;
            pData->optIndex.ctrl.uOut = 0;
            pData->optIndex.fVal = 0.f;
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
            return false;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Entries") == 0)
                return 0;
            else if (strcmp(pchControl, "Active") == 0)
                return 1;
            else if (strcmp(pchControl, "Position") == 0)
                return 2;

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

			return -1;
		}

//        static void FinishedCallback(U32 u32StateOffset);
#ifndef AUDIO_TOOLS
		Core::Mixer* Mixer(Instance* pInstance) { return (m_uMixer == ixMixerNone) ? NULL : (Core::Mixer*)pInstance->GetCoreData(m_uMixer); }

		DecoderTableEntry* DecoderTable(Instance* pInstance) { return (m_uDecoderTable == ixDecoderNone) ? NULL : (DecoderTableEntry*)pInstance->GetCoreData(m_uDecoderTable); }
		WaveTableEntry* WaveTable(Instance* pInstance) { Assert(m_uWaveTable != ixWaveNone); return (m_uWaveTable == ixWaveNone) ? NULL : (WaveTableEntry*)pInstance->GetCoreData(m_uWaveTable); }
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
		ixInit m_uWaveOffset;

		ixCore m_uDecoderTable;
		ixCore m_uMixer;
		ixCore m_uWaveTable;
    };
}

#endif // _MONOONESHOTSOUNDTABLE_H_

