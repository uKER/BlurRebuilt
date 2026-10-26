#ifndef _MONORETURN_H_
#define _MONORETURN_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoReturn
////////////////////////////////////////////////////////////////////////////////
    class MonoReturn : public SoundComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Option optGain;
		    Option optLevel;
            ArraySize uEffects;
        };

        struct State
        {
            F32 fLevel;

			bool bActive;
			bool bMuted;
		};

        enum Controls
        {
            ActiveCtrl
        };

    public:
        MonoReturn(Object* pObject) : SoundComponent(pObject)
        {
			m_uEffectOffset = 0;
        }

#if !defined (PLATFORM_SPU)
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif

        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
#ifndef PLATFORM_SPU
		void Release();
#endif

        const char* Type() const { return "MonoReturn"; }

#ifndef PLATFORM_SPU
		const U32 Flags() { return StaticFlags(); }
#endif
		static U32 StaticFlags() { return (flagUpdater | flagControl | flagMono | flagAudioOutput); }

		static U32 Size() { return sizeof(MonoReturn); }
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
            pData->optGain.ctrl.sCom = -1;
            pData->optGain.ctrl.uOut = 0;
            pData->optGain.fVal = 0.f;
            pData->optLevel.ctrl.sCom = -1;
            pData->optLevel.ctrl.uOut = 0;
            pData->optLevel.fVal = 1.f;
            pData->uEffects = 0;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Active") == 0)
                return 0;
            else
                return -1;
        }

		ixCore Return( ) { return m_uReturn; }

    private:
#if defined (AUDIO_ENABLE_PROFILING)
		bool IsActive(Instance* pInstance);
		void SetMute(Instance* pInstance, bool bMuted);
#endif

    private:
		ixCore m_uReturn;
	};
}

#endif // _MONORETURN_H_

