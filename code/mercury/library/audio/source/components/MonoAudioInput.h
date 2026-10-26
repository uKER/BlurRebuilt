#ifndef _MONOAUDIOINPUT_H_
#define _MONOAUDIOINPUT_H_

namespace Audio
{
	// forward decls for parser
	namespace Core { class Output; }
	namespace Core { class Input; }
////////////////////////////////////////////////////////////////////////////////
//	MonoAudioInput
////////////////////////////////////////////////////////////////////////////////
    class MonoAudioInput : public AudioComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
        };

        struct State
        {
			F32* pFrameInput;
			bool bActive;
			bool bMuted;
        };

    public:
        MonoAudioInput(Object* pObject) : AudioComponent(pObject)
        {
			m_uInput = 0;
        }
        ~MonoAudioInput();

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);

        const char* Type() const { return "MonoAudioInput"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
        void Update(Instance* pInstance);
		static U32 StaticFlags() { return (flagUpdater | flagReciever | flagMono | flagAudioOutput | flagInputConnector | flagAudioConnector); }

        static U32 Size() { return sizeof(MonoAudioInput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
#ifndef PLATFORM_SPU
		U32 CoreMemory(U32 uOffset);
#endif	//	#ifndef PLATFORM_SPU

		const U32 AudioOutType(Instance* pInstance) const
		{
			return Core::eInput;
		}
		const U32 AudioOutOffset(Instance* pInstance) const
		{
			return m_uInput;
		}

		Audio::Core::Input* AudioInput(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance( Instance* pThisInstance );
		Com GetOtherComponent( Instance* pThisInstance );
		bool IsActive( Instance* pInstance );
		void SetMute( Instance* pInstance, bool bMuted );
#endif

    private:
		ixCore m_uInput;
    };
}

#endif // _MONOAUDIOINPUT_H_
