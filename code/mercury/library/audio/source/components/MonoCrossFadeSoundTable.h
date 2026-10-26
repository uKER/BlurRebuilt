#ifndef _MONOONESHOTSOUNDTABLE_H_
#define _MONOONESHOTSOUNDTABLE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoOneShotSoundTable
////////////////////////////////////////////////////////////////////////////////
    class MonoOneShotSoundTable audio_inherits_component
    {
    public:
        struct Data
        {
			StrOffset strName;
            U32 uInstances;
			S32 strBank;
            U32 uWaves;
		    Option optIndex;
		    Option optPitch;
		    Option optFrequency;
		    Option optGain;
		    Option optLevel;
            S32 sFinishedScr;
            ArraySize uEffects;
        };

        struct InputWave
        {
            //  the wave
			S32 strWave;
        };

        struct Context
        {
            U32 uLastFrame;
            Core::IWave** apWaveA;
            Core::IWave** apWaveB;
            Core::IMixer* pMixer;
            Core::Decoder** apDecoder;
			bool bPlay;
//			bool bStop;
            U32 uInstance;
            F32 fIndex;
            F32 fPitch;
            F32 fFrequency;
            F32 fGain;
            F32 fLevel;
            U32 uIndexA;
            U32 uIndexB;
            F32 fLevelA;
            F32 fLevelB;
            MonoOneShotSoundTable* pThis;
            U8* pObjContext;
			U32 uNumActive;
        };

        enum Controls
        {
            EntriesCtrl,
            ActiveCtrl
        };

        enum Messages
        {
            PlayMsg,
            StopMsg,

            NumMessages
        };

    public:
        MonoOneShotSoundTable(Object* pObject) : Component(pObject)
        {
            m_pData = NULL;
            m_aEffects = NULL;
            m_aWaves = NULL;
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(Context); }
        void InitContext(U8* pObjContext);
        void ReleaseContext(U8* pObjContext);
        void Update(U8* pObjContext);
        F32 Value(U8* pObjContext, U32 uOutput = 0);
        void DoMessage(U8* pObjContext, U32 uMessage);
		void Release();

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MonoOneShotSoundTable"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl | flagMono | flagAudioOutput); }

        static U32 DataSize(U32 uEffects) { return sizeof(Data) + uEffects * sizeof(U32); }

        void ConfigureAudio(U8* pObjContext);

        const Audio::Core::IGenerator* AudioOut(U8* pObjContext) const;

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
            pData->sFinishedScr = -1;
            pData->uEffects = 0;
        }

        static bool ValidateData(Data* pData)
        {
            return false;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Active") == 0)
                return 0;
            else
                return -1;
        }

        static void FinishedCallback(void* pContext);

    private:
        void Play(U8* pObjContext);
        void Stop(U8* pObjContext);
        void Finished(U8* pObjContext);

    private:
        const Data* m_pData;
        const U32* m_aEffects;
        const InputWave* m_aWaves;
    };
}

#endif // _MONOONESHOTSOUNDTABLE_H_

